#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "types.h"

IMAGE_T *image;
KERNEL_T **L1_kernel, **L2_kernel;
MAP_T  **L1_maps, **L2_maps, **L1_maps_parr, **L2_maps_parr;
SCALE_T *L1_kernel_scale, *L2_kernel_scale, *L3_kernel_scale, **L1_kernel_scale_shard, **L2_kernel_scale_shard;

MAP_T *** maps;
KERNEL_T ***kernels;
SCALE_T **kernel_scales;

#include "papi.h"
#include "params.h"
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
	unsigned num_maps[NUM_LAYERS] = NUM_MAPS;
	unsigned kernel_widths[NUM_LAYERS] = KERNEL_WIDTHS;
	unsigned map_widths[NUM_LAYERS] = {60,30,15};
	unsigned map_heights[NUM_LAYERS] = {60,30,15};
	image = (IMAGE_T *)malloc(IMAGE_WIDTH*IMAGE_HEIGHT*sizeof(IMAGE_T));
	maps = (MAP_T ***)malloc(NUM_LAYERS*sizeof(MAP_T **));
	kernels = (KERNEL_T ***)malloc(NUM_LAYERS*sizeof(KERNEL_T **));
	kernel_scales = (SCALE_T **)malloc(NUM_LAYERS*sizeof(SCALE_T *));

	for (i=0;i<NUM_LAYERS;i++){
		maps[i] = (MAP_T **)malloc(num_maps[i]*sizeof(MAP_T *));
		kernels[i] = (KERNEL_T **)malloc(num_maps[i]*sizeof(KERNEL_T *));
		kernel_scales[i] = (SCALE_T *)malloc(num_maps[i]*sizeof(SCALE_T));
		for (j=0;j<num_maps[i];j++){
			maps[i][j] = (MAP_T *)malloc(map_widths[i]*map_heights[i]*sizeof(MAP_T));
			kernels[i][j] = (MAP_T *)malloc(kernel_widths[i]*kernel_widths[i]*sizeof(KERNEL_T));
		}
	}
	
	/********************************** INITIALIZATION OF ARRAYS **************************************/
	init_test_image(image);
	init_weights(kernels,kernel_scales,num_maps,kernel_widths);

	return 0;
	/****************************************** CPU-ONLY solver ******************************************/
/*
	int t;
	//start taking note of time, and start event counters
	t0=gettime();

	for (t=0;t<timesteps;t++){
		//accum vector for L1 maps, since it is fully-connected + it is add-convolve-once		
		MAP_T *accum_L1 = (MAP_T *)malloc(L1_MAP_SIZE*sizeof(MAP_T));//for L2, since it is fully-connected
		for (i=0;i<L1_MAP_SIZE;i++)
			accum_L1[i] = 0;

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

		conv_width = L1_MAP_WIDTH - L2_KERNEL_WIDTH + 1; //118
		conv_height = L1_MAP_HEIGHT - L2_KERNEL_HEIGHT + 1; //80
		INTERMEDIATE_T *filter2D_out_L2 = (INTERMEDIATE_T *)malloc(conv_width*conv_height*sizeof(INTERMEDIATE_T));
		for (i=0;i<L2_MAPS;i++){
			//assuming L2_KERNEL_WIDTH === L2_KERNEL_HEIGHT always...
			filter2D(L2_kernel[i],accum_L1,filter2D_out_L2,L2_KERNEL_WIDTH,L1_MAP_HEIGHT,L1_MAP_WIDTH,L2_kernel_scale[i]);
			subsample(filter2D_out_L2,L2_maps[i],DOWN_FAC2,conv_height,conv_width);
		}
	}
*/	
	//stop taking note of time, and stop event counters
	t1=gettime();

	printf("[SEQUENTIAL]Runtime=%lld\n",t1-t0);

	return 0;
}
