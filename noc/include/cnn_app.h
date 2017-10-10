#ifndef _CNN_APP_H_
#define _CNN_APP_H_
#include "struct_defs.h"
#include "caffe_frontend.h"

APP_STATUS_E main_cnn_app_init();

// Run deep CNN layers only on CPU
APP_STATUS_E cpu_cnn_app(
	IMAGE_T *pImage,
	int img_width,
	int img_height,
	int num_images,
	int *label
);

// Run deep CNN layers on CPU+Epiphany (Accelerated)
APP_STATUS_E epiphany_cnn_app(
	IMAGE_T *pImage,
	int image_width,
	int image_height,
	int num_images,
	int numMergedLayers,
	CNN_LYR_NODE_T *mergedLayers,
	int *pLabel
);

#endif // _CNN_APP_H_
