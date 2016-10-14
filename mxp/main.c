#include <stdio.h>
#include <string.h>
#include "debug_control.h"
#include "cnn_app.h"
#include "caffe_proto_params.h"
#include "vbx.h"
#ifndef VBX_SIMULATOR
#include "vbx_test.h"
#include "sim_image.h"
#endif

void mxp_init();

#ifdef VBX_SIMULATOR
void print_help(char **argv) {
	printf("Usage : %s\n"
		"-m sample -i <image path>\n"
		"\tOR\t\n"
		"-m test -f <image list file> -d <image dir> [-n <no images to test>]\n",argv[0]);
}
#endif

int main(
#ifdef VBX_SIMULATOR
	int argc, char **argv
#endif
) {
	uint8_t *pImage;
	int label, imgWidth, imgHeight;

#ifdef VBX_SIMULATOR

	char * mode = NULL;
	char * imgName = NULL;
	char * imgListFile = NULL;
	char * imgDir = NULL;
	int noTestImgs = -1;
	if(argc == 1) {
		print_help(argv);
		return -1;
	}

	// parse arguments and decide the application mode.
	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-m")) {
			mode = argv[++i];
		} else if (!strcmp(argv[i], "-i")){
			imgName = argv[++i];
		} else if(!strcmp(argv[i], "-f")) {
			imgListFile = argv[++i];
		} else if(!strcmp(argv[i], "-d")) {
			imgDir = argv[++i];
		} else if(!strcmp(argv[i], "-n")) {
			noTestImgs = atoi(argv[++i]);
		}
	}
#endif

	// MXP environment init
	mxp_init();
	// main network parameter init and memory allocation
	main_cnn_app_init();

#ifdef VBX_SIMULATOR
	if(!strcmp(mode, "sample")) {
		// TODO handle color image as well
    	pImage = read_gray_image(imgName, &imgHeight, &imgWidth);
    	if ((imgHeight != INPUT_IMG_HEIGHT) || (imgWidth != INPUT_IMG_WIDTH)) {
    	    REL_INFO("The image size is more than the allocated buffer size.\n");
    	    REL_INFO("Image size = %dx%d\tAllocate buffer size = %dx%d\n", imgWidth, imgHeight, INPUT_IMG_WIDTH, INPUT_IMG_HEIGHT);
    	    REL_INFO("Change the image size definitions in caffe_proto_params.h\n");
    	    return FAILED;
    	}
	} else {
		printf("Full test mode not supported yet\n");
		return FAILED;
	}
#else
	// NOTE: pass pointer to frame buffer instead of this malloc in real world application.
	pImage = malloc(INPUT_IMG_HEIGHT * INPUT_IMG_WIDTH * NO_INPUT_MAPS * sizeof(uint8_t));
#endif

	//TODO: enable this once block optimizer is complete
	//main_cnn_app(pImage, &label);
	printf("Application complete\n");
	free(pImage);
	return 0;
}



void mxp_init() {
#ifdef VBX_SIMULATOR
	// Simulator init
	REL_INFO("Initializing MXP Simulator\n");
	vbxsim_init(16,     //vector_lanes
		64,     //KB scratchpad_size
		256,    //max masked waves
		16,     //fractional_bits (word)
		15,     //fractional_bits (half)
		4);     //fractional_bits (byte)
#else
	REL_INFO("MXP Initialization....\n");
	vbx_test_init();
	vbx_timestamp_start();
#endif
}

