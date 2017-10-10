/******************************************************************************************
*
*	Author: Siddhartha (sidmontu at gmail dot com)
*
*	About: The main program that evaluates the deep learning layers on the ARM and
*	ARM+Epiphany implementations. Requires PAPI for timing peformance analysis.
*
*	---------------------------------------- STATUS ---------------------------------------
*	EPIPHANY SOLVER UNDER DEVELOPMENT (NOT COMPLETE)
*	Note: The Epiphany solver is currently being re-optimized for solving deep CNN layers
*	for faster performance, and tighter control with the Caffepresso backend.
*
******************************************************************************************/

#include "common.h"
#include "debug_control.h"
#include "cnn_app.h"
#include "caffe_proto_params.h"

#ifndef _MAC_OSX_
#include "papi.h"
#endif // _MAC_OSX_

#include "caffe_frontend.h"
#include "cnn_layers.h"
#include "app_init.h"
#include "misc_utils.h"
#include "epiphany_support.h"

#ifndef _MAC_OSX_
long_long get_papi_time(){
    return PAPI_get_virt_usec();
}
#endif // _MAC_OSX_

APP_STATUS_E solve_with_cpu(IMAGE_T *pImage, int img_width, int img_height, int num_images, int *label);

int main(int argc, char **argv) {
	int i, j, k;
	APP_STATUS_E ret;
	IMAGE_T *pImage;
	int label = -1, imgWidth, imgHeight;

#ifndef _MAC_OSX_
	long_long t0, t1; // PAPI timings
#endif // _MAC_OSX_

	// main network parameter init and memory allocation
	main_cnn_app_init();

	// NOTE: Right now, only NO_INPUT_MAPS = 1 is supported
	// i.e. only one image can be run through the model for inference.
	// TODO: Support batched inference
	pImage = malloc(INPUT_IMG_HEIGHT * INPUT_IMG_WIDTH * NO_INPUT_MAPS * sizeof(IMAGE_T));
	if (argc > 1) {
		read_in_image(argv[1],INPUT_IMG_WIDTH,INPUT_IMG_HEIGHT,pImage);
	} else {
		init_random_image(INPUT_IMG_WIDTH,INPUT_IMG_HEIGHT,pImage);
	}

#ifndef _MAC_OSX_
	t0 = get_papi_time();
#endif // _MAC_OSX_

	ret = cpu_cnn_app(pImage,INPUT_IMG_WIDTH,INPUT_IMG_HEIGHT,1,&label); // note: num_images = 1

	#ifndef _MAC_OSX_
	t1 = get_papi_time();
	printf("[CPU] Runtime = %lld usec\n",t1-t0);
#endif // _MAC_OSX_

	if (ret != SUCCESS) {
		printf("[FAILURE] CPU Solver returned with FAIL status\n");
	} else {
		printf("[SUCCESS] CPU Solver completed successfully Label = %d.\n",label);
	}

#ifndef CPU_ONLY
	////////// One-time steps to setup the network configurations on Epiphany
	// merge CONV + POOL layers into CONV_POOL
	CNN_LYR_NODE_T mergedLayers[NO_DEEP_LAYERS];
	int numMergedLayers = merge_layers(cnnLayerNodes,NO_DEEP_LAYERS,mergedLayers);
	printf("Number of layers reduced from %d to %d\n",NO_DEEP_LAYERS,numMergedLayers);

#ifndef _MAC_OSX_
	t0 = get_papi_time();
#endif // _MAC_OSX_

	// run the epiphany CNN app
	ret = epiphany_cnn_app(pImage,INPUT_IMG_WIDTH,INPUT_IMG_HEIGHT,1,numMergedLayers,mergedLayers,&label);

#ifndef _MAC_OSX_
	t1 = get_papi_time();
	printf("[EPIPHANY] Runtime = %lld usec\n",t1-t0);
#endif // _MAC_OSX_

	if (ret != SUCCESS) {
		printf("[FAILURE] EPIPHANY Solver returned with FAIL status\n");
	} else {
		printf("[SUCCESS] EPIPHANY Solver completed successfully Label = %d.\n",label);
	}

#endif // CPU_ONLY
	
	REL_INFO("Releasing buffers\n");
	cnn_app_memfree(cnnLayerNodes, NO_DEEP_LAYERS);

	printf("Application complete\n");
	free(pImage);
	return 0;
}
