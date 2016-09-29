#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <e-hal.h>
#include "types.h"

IMAGE_T *image;
KERNEL_T **L1_kernel, **L2_kernel;
MAP_T  **L1_maps, **L2_maps, **L1_maps_parr, **L2_maps_parr;
SCALE_T *L1_kernel_scale, *L2_kernel_scale, *L3_kernel_scale, **L1_kernel_scale_shard, **L2_kernel_scale_shard;

#include "papi.h"
#include "parameters.h"
#include "address.h"
#include "filter2D.h"
#include "pool_subsample.h"
#include "support.h"
#include "patch.h"

#define ERR_TOL 0.1
#define E_NUM_PARAMS 10

//function to get wall clock time
long_long gettime(){
	return PAPI_get_virt_usec();
}

int main(int argc, char **argv){

	unsigned timesteps = atoi(argv[1]);

	printf("Total Deep Learning timesteps = %d\n",timesteps);	

	//declare variables and events to monitor
	unsigned i,j,k;
	long_long t0, t1;

	//malloc vectors
	image = (IMAGE_T *)malloc(IMAGE_SIZE*sizeof(IMAGE_T));
	L1_maps = (MAP_T **)malloc(L1_MAPS*sizeof(MAP_T *));
	L2_maps = (MAP_T **)malloc(L2_MAPS*sizeof(MAP_T *));
	L1_kernel = (KERNEL_T **)malloc(L1_MAPS*sizeof(KERNEL_T *));
	L1_kernel_scale = (SCALE_T *)malloc(L1_MAPS*sizeof(SCALE_T));
	L2_kernel = (KERNEL_T **)malloc(L2_MAPS*sizeof(KERNEL_T *));
	L2_kernel_scale = (SCALE_T *)malloc(L2_MAPS*sizeof(SCALE_T));

	for (i=0;i<L1_MAPS;i++){
		L1_maps[i] = (MAP_T *)malloc(L1_MAP_SIZE*sizeof(MAP_T));
		L1_kernel[i] = (KERNEL_T *)malloc(L1_KERNEL_SIZE*sizeof(KERNEL_T));
	}
	for (i=0;i<L2_MAPS;i++){	
		L2_maps[i] = (MAP_T *)malloc(L2_MAP_SIZE*sizeof(MAP_T));
		L2_kernel[i] = (KERNEL_T *)malloc(L2_KERNEL_SIZE*sizeof(KERNEL_T));
	}

	/********************************** INITIALIZATION OF ARRAYS **************************************/
	init_test_image(image);
	init_weights(L1_kernel,L1_kernel_scale,L2_kernel,L2_kernel_scale);

	/****************************************** CPU-ONLY solver ******************************************/
	int t;
	//start taking note of time, and start event counters
	t0=gettime();

	for (t=0;t<timesteps;t++){
		//accum vector for L1 maps, since it is fully-connected + it is add-convolve-once		
		MAP_T *accum_L1 = (MAP_T *)malloc(L1_MAP_SIZE*sizeof(MAP_T));//for L2, since it is fully-connected
		for (i=0;i<L1_MAP_SIZE;i++)
			accum_L1[i] = 0;

		/****************** LAYER 1 *********************/
		//to store intermediate results
		unsigned conv_width = IMAGE_WIDTH - L1_KERNEL_WIDTH + 1; //496
		unsigned conv_height = IMAGE_HEIGHT - L1_KERNEL_HEIGHT + 1; //346
		INTERMEDIATE_T *filter2D_out = (INTERMEDIATE_T *)malloc(conv_width*conv_height*sizeof(INTERMEDIATE_T));
		for (i=0;i<L1_MAPS;i++){
			//assuming L1_KERNEL_WIDTH === L1_KERNEL_HEIGHT always...
			filter2D(L1_kernel[i],image,filter2D_out,L1_KERNEL_WIDTH,IMAGE_HEIGHT,IMAGE_WIDTH,L1_kernel_scale[i]);
			subsample(filter2D_out,L1_maps[i],DOWN_FAC1,conv_height,conv_width);
			for (j=0;j<L1_MAP_SIZE;j++)
				accum_L1[j] += L1_maps[i][j];
		}

		/****************** LAYER 2 *********************/	
		conv_width = L1_MAP_WIDTH - L2_KERNEL_WIDTH + 1; //118
		conv_height = L1_MAP_HEIGHT - L2_KERNEL_HEIGHT + 1; //80
		INTERMEDIATE_T *filter2D_out_L2 = (INTERMEDIATE_T *)malloc(conv_width*conv_height*sizeof(INTERMEDIATE_T));
		for (i=0;i<L2_MAPS;i++){
			//assuming L2_KERNEL_WIDTH === L2_KERNEL_HEIGHT always...
			filter2D(L2_kernel[i],accum_L1,filter2D_out_L2,L2_KERNEL_WIDTH,L1_MAP_HEIGHT,L1_MAP_WIDTH,L2_kernel_scale[i]);
			subsample(filter2D_out_L2,L2_maps[i],DOWN_FAC2,conv_height,conv_width);
		}
	}
	
	//stop taking note of time, and stop event counters
	t1=gettime();

	printf("[SEQUENTIAL]Runtime=%lld\n",t1-t0);


	/****************************************** CPU-EPIPHANY solver ******************************************/

	//device handlers
	e_platform_t platform;
	e_epiphany_t dev;

	//initialize device
	e_init(NULL);
	e_reset_system();
	e_get_platform_info(&platform);

	// Open a workgroup
	e_open(&dev, 0, 0, platform.rows, platform.cols);

	unsigned *out_patch_height = (unsigned *)malloc(sizeof(unsigned));
	unsigned *out_patch_width = (unsigned *)malloc(sizeof(unsigned));
	GLOBAL_CONSTANTS_T *params = (GLOBAL_CONSTANTS_T *)malloc(E_NUM_PARAMS*sizeof(GLOBAL_CONSTANTS_T));
	
	GLOBAL_CONSTANTS_T patch_widths[NUM_LAYERS] = PATCH_WIDTH;
	GLOBAL_CONSTANTS_T patch_heights[NUM_LAYERS] = PATCH_HEIGHT;
	GLOBAL_CONSTANTS_T map_heights[NUM_LAYERS] = MAP_HEIGHT;
	GLOBAL_CONSTANTS_T map_widths[NUM_LAYERS] = MAP_WIDTH;
	GLOBAL_CONSTANTS_T subsampling_factors[NUM_LAYERS] = SUBSAMPLING_FACTORS;
	GLOBAL_CONSTANTS_T kernel_widths[NUM_LAYERS] = KERNEL_WIDTH;
	GLOBAL_CONSTANTS_T num_maps[NUM_LAYERS] = NUM_MAPS;
	GLOBAL_CONSTANTS_T kernel_offsets[NUM_LAYERS] = DRAM_KERNEL_OFFSETS;
	GLOBAL_CONSTANTS_T kernel_scale_offsets[NUM_LAYERS] = DRAM_KERNEL_SCALE_OFFSETS;
	ADDR_T dram_kernel_ptrs[NUM_LAYERS] = E_DRAM_KERNEL_PTRS;
	ADDR_T patch_ptrs[NUM_LAYERS] = E_DRAM_PATCH_PTRS;
	ADDR_T map_ptrs[NUM_LAYERS] = E_DRAM_MAP_PTRS;
	
	t0=gettime();

	//load kernels + load scales
	//construct kernels
	KERNEL_T **kernels = (KERNEL_T **)malloc(NUM_LAYERS*sizeof(KERNEL_T *));
	SCALE_T **kernel_scales = (SCALE_T **)malloc(NUM_LAYERS*sizeof(SCALE_T *));

	e_mem_t emem;
	e_alloc(&emem,0x01000000,DRAM_TOTAL_SIZE);

	for (i=0;i<NUM_LAYERS;i++){
		unsigned kernel_size = kernel_widths[i]*kernel_widths[i];
		kernels[i] = (KERNEL_T *)malloc(num_maps[i]*kernel_size*sizeof(KERNEL_T));
		kernel_scales[i] = (SCALE_T *)malloc(num_maps[i]*sizeof(SCALE_T));
		for (j=0;j<num_maps[i];j++){
			for (k=0;k<kernel_size;k++){
				kernels[i][j*kernel_size+k] = 2.0f;//L1_kernel[j][k];
			}
			kernel_scales[i][j] = 2.0f;//L1_kernel_scale[j];
		}
		e_write(&emem,0,0,kernel_offsets[i],kernels[i],num_maps[i]*kernel_size*sizeof(KERNEL_T));
		e_write(&emem,0,0,kernel_scale_offsets[i],kernel_scales[i],num_maps[i]*sizeof(SCALE_T));
	}
	
	t1=gettime();

	printf("[SETUP_TIME]Runtime=%lld\n",t1-t0);

	//start taking note of time
	t0=gettime();

	//intialization -- every deep learning network starts with image as container
	IMAGE_T *container = image;
	unsigned container_width = IMAGE_WIDTH;

	for (i=0;i<NUM_LAYERS;i++){
		printf("Evaluating layer %d\n",i);
		patch(patch_heights[i],patch_widths[i],out_patch_height,out_patch_width,1,map_heights[i],map_widths[i],subsampling_factors[i],kernel_widths[i]);
		int num_patches = (map_widths[i]/patch_widths[i])*(map_heights[i]/patch_heights[i]);
		unsigned container_overlap_size = num_patches*(*out_patch_height)*(*out_patch_width);
		IMAGE_T *container_overlap = (IMAGE_T *)malloc(container_overlap_size*sizeof(IMAGE_T));
		//flattens and transfers container
		int patches_per_row = map_widths[i]/patch_widths[i];
		int patches_per_col = map_heights[i]/patch_heights[i];
		int patch_row,patch_col;
		unsigned overlap_cntr = 0;
		for (k=0;k<patches_per_col;k++){
			for (j=0;j<patches_per_row;j++){
				for (patch_row=0;patch_row<*out_patch_height;patch_row++){
					for (patch_col=0;patch_col<*out_patch_width;patch_col++){
						int index = k*container_width*patch_heights[i] + j*patch_widths[i];
						container_overlap[overlap_cntr] = container[index+patch_row*container_width+patch_col];
						overlap_cntr++;
					}
				}
			}
		}
		
		if (i==0){
			e_write(&emem,0,0,0,container,container_overlap_size*sizeof(IMAGE_T));
		} else {
			e_write(&emem,0,0,DRAM_INTERMEDIATE_MAP_OFFSET,container,container_overlap_size*sizeof(MAP_T));
		}
		
		//printf("Out patch height = %d, out patch width = %d\n",*out_patch_height,*out_patch_width);

		params[0] = num_maps[i]/NUM_PES;
		params[1] = num_patches;
		params[2] = kernel_widths[i];
		params[3] = *out_patch_height;
		params[4] = *out_patch_width;
		params[5] = subsampling_factors[i];
		params[6] = dram_kernel_ptrs[i];
		params[7] = map_ptrs[i];
		params[8] = patch_ptrs[i];
		params[9] = map_widths[i];

		// CORE initializations
		//printf("Initializing cores...\n");
		FLAG_T done = 0xbeefdead;
		//transfer params
		for (j=0;j<platform.rows;j++){
			for (k=0;k<platform.cols;k++){
				e_write(&dev,j,k,PARAMETERS_ADDR,params,E_NUM_PARAMS*sizeof(GLOBAL_CONSTANTS_T));
				if (j == 0 && k == 0)
					e_write(&dev,j,k,DONE_ADDR,&done,sizeof(FLAG_T));
			}
		}
	
		//call the epiphany to run.
		//e_start_group(&dev);
		//load srec
		e_reset_group(&dev);
		e_load_group("pe.srec", &dev, 0, 0, platform.rows, platform.cols, E_TRUE);
		//printf("Program loaded and running...\n");

		done = 0;
		//unsigned patch_number = 0;
		//unsigned new_patch_number = 0;
		while (done != 0xdeadbeef){
			//e_read(&dev,0,0,DONE_ADDR+4,&new_patch_number,sizeof(unsigned));
		//	if (new_patch_number != patch_number){
		//		printf("eCore 0 has evaluated %d patches!\n",new_patch_number);
		//		patch_number = new_patch_number;
		//	}
			e_read(&dev,0,0,DONE_ADDR,&done,sizeof(unsigned));
		}

		//fetch the constructed map
		MAP_T *fetched = (MAP_T *)malloc(map_heights[i]*map_widths[i]*sizeof(MAP_T));
		e_read(&emem,0,0,DRAM_INTERMEDIATE_MAP_OFFSET,fetched,map_heights[i]*map_widths[i]*sizeof(MAP_T));
		container = fetched;
		container_width = map_widths[i];
	}
	
	t1=gettime();

	printf("[PARALLEL]Runtime=%lld\n",t1-t0);

	//free DRAM memory
	e_free(&emem);

	// Close the workgroup
	e_close(&dev);

	// Finalize e-platform connection
	e_finalize();

	return 0;
}
