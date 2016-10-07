#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <e-hal.h>
#include "types.h"
#include "parameters.h"
#include "address.h"
#include "support.h"
#include "patch.h"

#define E_NUM_PARAMS 17

IMAGE_T *image;

int main(int argc, char **argv){

	// local variables
	unsigned i,j,k, m;

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

	unsigned *out_patch_width = (unsigned *)malloc(sizeof(unsigned));
	unsigned *out_patch_height = (unsigned *)malloc(sizeof(unsigned));
	GLOBAL_CONSTANTS_T *params = (GLOBAL_CONSTANTS_T *)malloc(E_NUM_PARAMS*sizeof(GLOBAL_CONSTANTS_T));
	
	GLOBAL_CONSTANTS_T patch_widths[NUM_LAYERS] = PATCH_WIDTH;
	GLOBAL_CONSTANTS_T patch_heights[NUM_LAYERS] = PATCH_HEIGHT;
	GLOBAL_CONSTANTS_T map_widths[NUM_LAYERS] = MAP_WIDTH;
	GLOBAL_CONSTANTS_T map_heights[NUM_LAYERS] = MAP_HEIGHT;
	GLOBAL_CONSTANTS_T subsampling_factors[NUM_LAYERS] = SUBSAMPLING_FACTORS;
	GLOBAL_CONSTANTS_T kernel_widths[NUM_LAYERS] = KERNEL_WIDTH;
	GLOBAL_CONSTANTS_T num_maps[NUM_LAYERS] = NUM_MAPS;
	GLOBAL_CONSTANTS_T window_sizes[NUM_LAYERS] = WINDOW_SIZES;
	GLOBAL_CONSTANTS_T kernel_offsets[NUM_LAYERS] = DRAM_KERNEL_OFFSETS;
	GLOBAL_CONSTANTS_T kernel_scale_offsets[NUM_LAYERS] = DRAM_KERNEL_SCALE_OFFSETS;
	GLOBAL_CONSTANTS_T dram_intermediate_map_offsets[NUM_LAYERS] = DRAM_INTERMEDIATE_MAP_OFFSETS;
	GLOBAL_CONSTANTS_T conv_strides[NUM_LAYERS] = CONV_STRIDES;
	GLOBAL_CONSTANTS_T conv_pads[NUM_LAYERS] = CONV_PAD;
	GLOBAL_CONSTANTS_T pooling_strides[NUM_LAYERS] = POOLING_STRIDES;
	ADDR_T dram_kernel_ptrs[NUM_LAYERS] = E_DRAM_KERNEL_PTRS;
	ADDR_T patch_ptrs[NUM_LAYERS] = E_DRAM_PATCH_PTRS;
	ADDR_T map_ptrs[NUM_LAYERS] = E_DRAM_MAP_PTRS;
	
	//load kernels + load scales
	//construct kernels
	image = (IMAGE_T *)malloc(IMAGE_SIZE*sizeof(IMAGE_T));
	KERNEL_T **kernels = (KERNEL_T **)malloc(NUM_LAYERS*sizeof(KERNEL_T *));
	SCALE_T **kernel_scales = (SCALE_T **)malloc(NUM_LAYERS*sizeof(SCALE_T *));

	e_mem_t emem;
	e_alloc(&emem,0x01000000,DRAM_TOTAL_SIZE);

	// Here we are just initializing image and kernel weights with random values
	// TODO: read in images and kernel weights for trained models from input files/images
	init_test_image(image);
	for (i=0;i<NUM_LAYERS;i++){
		unsigned kernel_size = kernel_widths[i]*kernel_widths[i];
		kernels[i] = (KERNEL_T *)malloc(num_maps[i]*kernel_size*sizeof(KERNEL_T));
		kernel_scales[i] = (SCALE_T *)malloc(num_maps[i]*sizeof(SCALE_T));
		for (j=0;j<num_maps[i];j++){
			for (k=0;k<kernel_size;k++){
				kernels[i][j*kernel_size+k] = (KERNEL_T)(rand()%5); // randomly initialize for now..
			}
			kernel_scales[i][j] = (SCALE_T)(rand()%10); // randomly initialize for now.. 
		}
		e_write(&emem,0,0,kernel_offsets[i],kernels[i],num_maps[i]*kernel_size*sizeof(KERNEL_T));
		e_write(&emem,0,0,kernel_scale_offsets[i],kernel_scales[i],num_maps[i]*sizeof(SCALE_T));
	}
	
	//intialization -- every deep learning network starts with image input as container
	IMAGE_T *container = image;
	unsigned container_width = IMAGE_WIDTH;
	unsigned container_height = IMAGE_HEIGHT;

	for (i=0;i<NUM_LAYERS;i++){
		printf("Evaluating layer %d\n",i);
		patch(patch_heights[i],patch_widths[i],out_patch_height,out_patch_width,1,map_heights[i],map_widths[i],subsampling_factors[i],kernel_widths[i],pooling_strides[i],conv_strides[i],conv_pads[i]);
		int num_patches = (map_widths[i]/patch_widths[i])*(map_heights[i]/patch_heights[i]);
		printf("Num patches in layer %d = %d\n",i+1,num_patches);
		printf("Layer %d, out_patch_width = %d, out_patch_height = %d\n",i+1,*out_patch_width,*out_patch_height);
		unsigned container_overlap_size = num_patches*(*out_patch_width)*(*out_patch_height);
		if (i==0){//only need to flatten image
			IMAGE_T *container_overlap = (IMAGE_T *)malloc(container_overlap_size*sizeof(IMAGE_T));
			//flattens and transfers container
			int patches_per_dim_width = map_widths[i]/patch_widths[i];
			int patches_per_dim_height = map_heights[i]/patch_heights[i];
			int patch_row,patch_col;
			unsigned overlap_cntr = 0;
			for (k=0;k<patches_per_dim_height;k++){
				for (j=0;j<patches_per_dim_width;j++){
					for (patch_row=0;patch_row<*out_patch_height;patch_row++){
						for (patch_col=0;patch_col<*out_patch_width;patch_col++){
							int index = k*container_width*patch_heights[i] + j*patch_widths[i];
							container_overlap[overlap_cntr] = container[index+patch_row*container_width+patch_col];
							overlap_cntr++;
						}
					}
				}
			}
			e_write(&emem,0,0,0,container_overlap,container_overlap_size*sizeof(IMAGE_T));
		} else {//need to flatten ALL maps
			for (m=0;m<num_maps[i-1];m++){
				MAP_T *container_overlap = (MAP_T *)malloc(container_overlap_size*sizeof(IMAGE_T));
				//flattens and transfers container
				int patches_per_dim_width = map_widths[i]/patch_widths[i];
				int patches_per_dim_height = map_heights[i]/patch_heights[i];
				int patch_row,patch_col;
				unsigned overlap_cntr = 0;
				for (k=0;k<patches_per_dim_height;k++){
					for (j=0;j<patches_per_dim_width;j++){
						for (patch_row=0;patch_row<*out_patch_height;patch_row++){
							for (patch_col=0;patch_col<*out_patch_width;patch_col++){
								int index = m*container_overlap_size + k*container_width*patch_heights[i] + j*patch_widths[i];
								container_overlap[overlap_cntr] = container[index+patch_row*container_width+patch_col];
								overlap_cntr++;
							}
						}
					}
				}
				e_write(&emem,0,0,dram_intermediate_map_offsets[i-1]+m*container_overlap_size*sizeof(MAP_T),container_overlap,container_overlap_size*sizeof(MAP_T));
			}
		}
		
		// set the parameters for next layer to be evaluated on the Epiphany
		params[0] = num_maps[i]/NUM_PES;
		params[1] = num_patches;
		params[2] = kernel_widths[i];
		params[3] = *out_patch_width;
		params[4] = *out_patch_height;
		params[5] = window_sizes[i];
		params[6] = subsampling_factors[i];
		params[7] = dram_kernel_ptrs[i];
		params[8] = map_ptrs[i];
		params[9] = patch_ptrs[i];
		params[10] = map_widths[i];
		params[11] = map_heights[i];
		params[12] = i == 0 ? 1 : num_maps[i-1];
		params[13] = i == 0 ? 1 : container_overlap_size;
		params[14] = pooling_strides[i];
		params[15] = conv_strides[i];
		params[16] = conv_pads[i];

		SCALE_T window_scale = 0;
		if (window_sizes[i] > 0)
			window_scale = 1/(window_sizes[i]*window_sizes[i]);

		// CORE initializations
		printf("Initializing cores...\n");
		FLAG_T done = 0xbeefdead;
		//transfer params
		for (j=0;j<platform.rows;j++){
			for (k=0;k<platform.cols;k++){
				e_write(&dev,j,k,PARAMETERS_ADDR,params,E_NUM_PARAMS*sizeof(GLOBAL_CONSTANTS_T));
				e_write(&dev,j,k,WINDOW_SCALE_ADDR,&window_scale,sizeof(SCALE_T));
				if (j == 0 && k == 0)
					e_write(&dev,j,k,DONE_ADDR,&done,sizeof(FLAG_T));
			}
		}
	
		//call the epiphany to run.
		//e_start_group(&dev);
		//load srec
		e_reset_group(&dev);
		e_load_group("pe.srec", &dev, 0, 0, platform.rows, platform.cols, E_TRUE);

		// poll "done" mailbox until desired value is read
		// to indicate epiphany has finished execution
		done = 0;
		while (done != 0xdeadbeef){
			e_read(&dev,0,0,DONE_ADDR,&done,sizeof(unsigned));
		}

		//fetch the constructed map
		MAP_T *fetched = (MAP_T *)malloc(num_maps[i]*map_widths[i]*map_heights[i]*sizeof(MAP_T));
		e_read(&emem,0,0,dram_intermediate_map_offsets[i],fetched,num_maps[i]*map_heights[i]*map_widths[i]*sizeof(MAP_T));
		container = fetched;
		container_width = map_widths[i];
		container_height = map_heights[i];
	}
	
	//free DRAM memory
	e_free(&emem);

	// Close the workgroup
	e_close(&dev);

	// Finalize e-platform connection
	e_finalize();

	return 0;
}
