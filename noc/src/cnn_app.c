#include "app_init.h"
#include "debug_control.h"
#include "misc_utils.h"

APP_STATUS_E main_cnn_app_init() {
	APP_STATUS_E status = SUCCESS;
	
	caffe_layer_ctx_init();
	REL_INFO("Initialized context from the LUT\n");

	cnn_layer_internal_param_init();
	REL_INFO("Initialized app configurations and internal context params\n");

	cnn_app_malloc(cnnLayerNodes, NO_DEEP_LAYERS);
	REL_INFO("Allocated buffers for all layers\n");

	cnn_app_model_init(cnnLayerNodes, NO_DEEP_LAYERS);
	REL_INFO("Initialized model weights and biases of all layers\n");
}

APP_STATUS_E epiphany_cnn_app(
	IMAGE_T *pImage,
	int image_width,
	int image_height,
	int num_images,
	int numMergedLayers,
	CNN_LYR_NODE_T *mergedLayers,
	int *pLabel
) {
	int lyr, prevMapH, prevMapW, prevNmaps;
	CONV_LYR_CTX_T *pConvCtx;
	POOL_LYR_CTX_T *pPoolCtx;
	ACT_LYR_CTX_T *pActCtx;
	IP_LYR_CTX_T * pIpCtx;
	SMAX_LYR_CTX_T *pSmaxCtx;
	FL_MAP_PIXEL *pInFloatMap, *pFloatInput;
	float var;
	APP_STATUS_E status = SUCCESS;

	// allocate memory for input maps.
	if((NULL == (pInFloatMap = (FL_MAP_PIXEL *)malloc(image_width * image_height * num_images * sizeof(FL_MAP_PIXEL))))) {
		REL_INFO("Malloc failed\n");
		return MALLOC_FAIL;
	}

	prevMapH = image_height;
	prevMapW = image_width;
	prevNmaps = num_images;

	// mean and contrast normalization
	mean_normalize(pImage, prevMapH * prevNmaps, prevMapW, &var, pInFloatMap);

	pConvCtx = (CONV_LYR_CTX_T *)mergedLayers[0].pLyrCtx;

	pFloatInput = pInFloatMap;
	
	// main processing loop
	for (lyr = 0; lyr < numMergedLayers; lyr++) {
		switch(mergedLayers[lyr].lyrType) {
			case CONV:
				// TODO: run only convolution on Epiphany
				break;
			case POOL:
				// TODO: run only pooling on Epiphany
				break;
			case CONV_POOL :
				// TODO: run conv_pool layer on Epiphany
				break;
			case ACT:
				// TODO: Run activation layer on CPU
				break;
			case INNER_PROD:
				// TODO: Run inner-product layer on CPU
				break;
			case SOFTMAX:
				// TODO: Run softmax layer on CPU
				break;
			default:
				REL_INFO("Unsupported layer\n");
				return UNSUPPORTED_FEATURE;
		}
	}
	// TODO: take last layer output and find the max probability/label
	*pLabel = -2; // dummy label for now.

	return status;
}

APP_STATUS_E cpu_cnn_app(
	IMAGE_T *pImage,
	int img_width,
	int img_height,
	int num_images,
	int *label
) {
	int lyr, prevMapH, prevMapW, prevNmaps;
	CONV_LYR_CTX_T *pConvCtx;
	POOL_LYR_CTX_T *pPoolCtx;
	ACT_LYR_CTX_T *pActCtx;
	IP_LYR_CTX_T * pIpCtx;
	SMAX_LYR_CTX_T *pSmaxCtx;
	FL_MAP_PIXEL *pInFloatMap, *pFloatInput;
	float var;
	APP_STATUS_E status = SUCCESS;

	// allocate memory for input maps.
	if((NULL == (pInFloatMap = (FL_MAP_PIXEL *)malloc(img_width * img_height * num_images * sizeof(FL_MAP_PIXEL))))) {
		REL_INFO("Malloc failed\n");
		return MALLOC_FAIL;
	}

	prevMapH = img_height;
	prevMapW = img_width;
	prevNmaps = num_images;

	// mean and contrast normalization
	mean_normalize(pImage, prevMapH * prevNmaps, prevMapW, &var, pInFloatMap);

	pConvCtx = (CONV_LYR_CTX_T *)cnnLayerNodes[0].pLyrCtx;
	pFloatInput = pInFloatMap;
	
	// main processing loop
	for (lyr = 0; lyr < NO_DEEP_LAYERS; lyr++) {
		switch(cnnLayerNodes[lyr].lyrType) {
			case CONV:
				pConvCtx = (CONV_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				// compute this layer's output
				cnn_conv_layer(pConvCtx, pFloatInput, MAP_ISOLATED);
				// Assign this output to next layer's input
				pFloatInput = pConvCtx->pFloatOutput;
				prevMapH = (pConvCtx->convInfo.mapH + 2*pConvCtx->convInfo.pad - pConvCtx->convInfo.K + 1 + pConvCtx->convInfo.stride - 1)/pConvCtx->convInfo.stride;
				prevMapW = (pConvCtx->convInfo.mapW + 2*pConvCtx->convInfo.pad - pConvCtx->convInfo.K + 1 + pConvCtx->convInfo.stride - 1)/pConvCtx->convInfo.stride;
				prevNmaps = pConvCtx->convInfo.nOutMaps;
				break;
			case POOL:
				pPoolCtx = (POOL_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;				
				cnn_pool_layer(pPoolCtx, pFloatInput, MAP_ISOLATED);
				pFloatInput = pPoolCtx->pFloatOutput;
				prevMapH =( pPoolCtx->poolInfo.mapH + 2*pPoolCtx->poolInfo.pad - pPoolCtx->poolInfo.winSize + 1 + pPoolCtx->poolInfo.stride - 1)/ pPoolCtx->poolInfo.stride;
				prevMapW =( pPoolCtx->poolInfo.mapW + 2*pPoolCtx->poolInfo.pad - pPoolCtx->poolInfo.winSize + 1 + pPoolCtx->poolInfo.stride - 1)/ pPoolCtx->poolInfo.stride;
				prevNmaps = pPoolCtx->poolInfo.nMaps;
				break;
			case ACT:
				pActCtx = (ACT_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				cnn_activation_layer(pActCtx, pFloatInput);
				pFloatInput = pActCtx->pFloatOutput;
				prevMapH = pActCtx->actInfo.mapH;
				prevMapW = pActCtx->actInfo.mapW;
				prevNmaps = pActCtx->actInfo.nMaps;
				break;
			case INNER_PROD:
				pIpCtx = (IP_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;				
				if (pIpCtx->lyrArithMode != FLOAT_POINT) {
					DBG_INFO("Only floating-point arithmetic is supported.\n");
				}
				inner_prod_layer(pIpCtx, pFloatInput);
				pFloatInput = pIpCtx->pFloatOutput;
				prevMapH = 1;
				prevMapW = pIpCtx->ipInfo.nOutput;
				prevNmaps = 1;
				break;
			case SOFTMAX:
				pSmaxCtx = (SMAX_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;				
				softmax_layer(pSmaxCtx, pFloatInput);
				pFloatInput = pSmaxCtx->pFloatOutput;
				break;
			default:
				REL_INFO("Unsupported layer\n");
				return UNSUPPORTED_FEATURE;
		}
	}

	return status;
}
