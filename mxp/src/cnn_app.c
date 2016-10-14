#include "app_init.h"
#include "caffe_frontend.h"
#include "debug_control.h"
#include "vbx.h"
#include "vbx_port.h"
#include "app_profile.h"
#include "misc_utils.h"
#ifdef CNN_SIMULATOR
#include "sim_image.h"
#endif // CNN_SIMULATOR

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

APP_STATUS_E main_cnn_app(uint8_t *pImage, int *pLabel) {
	int lyr;
	CONV_LYR_CTX_T *pConvCtx;
	POOL_LYR_CTX_T *pPoolCtx;
	ACT_LYR_CTX_T *pActCtx;
	IP_LYR_CTX_T * pIpCtx;
	SMAX_LYR_CTX_T *pSmaxCtx;
	FP_MAP_PIXEL *pInFixMap, *pFixInput;
	FL_MAP_PIXEL *pInFloatMap, *pFloatInput;
	int prevMapH, prevMapW, prevNmaps;
	float var;
	int prevArithMode, prevFracBits;
	vbx_timestamp_t startTime;
	APP_STATUS_E status = SUCCESS;


	// allocate memory for input maps.
	if((NULL == (pInFloatMap = (FL_MAP_PIXEL *)malloc(INPUT_IMG_WIDTH * INPUT_IMG_HEIGHT * NO_INPUT_MAPS * sizeof(FL_MAP_PIXEL)))) ||
		(NULL == (pInFixMap = (FP_MAP_PIXEL *)malloc(INPUT_IMG_WIDTH * INPUT_IMG_HEIGHT * NO_INPUT_MAPS * sizeof(FP_MAP_PIXEL))))
	){
		REL_INFO("Malloc failed\n");
		return MALLOC_FAIL;
	}

	prevMapH = INPUT_IMG_HEIGHT;
	prevMapW = INPUT_IMG_WIDTH;
	prevNmaps = NO_INPUT_MAPS;

	// mean and contrast normalization
	mean_normalize((uint8_t *)pImage, prevMapH * prevNmaps, prevMapW, &var, pInFloatMap);
	//DBG_MAPS(show_float_img("norm image", pInFloatMap, prevMapH, prevMapW));

	pConvCtx = (CONV_LYR_CTX_T *)cnnLayerNodes[0].pLyrCtx;
	float_to_fix_data(pInFloatMap, prevMapH * prevNmaps * prevMapW, pConvCtx->convInfo.nMapFractionBits, pInFixMap);
	//DBG_MAPS(show_fix_img("input fix map", pInFixMap, prevMapH, prevMapW, pConvCtx->convInfo.nMapFractionBits));

	pFixInput = pInFixMap;
	pFloatInput = pInFloatMap;
	prevArithMode = FIXED_POINT;
	//prevArithMode = FLOAT_POINT;
	prevFracBits = pConvCtx->convInfo.nMapFractionBits;
	
	// main processing loop
	GET_TIME(&startTime);
	for (lyr = 0; lyr < NO_DEEP_LAYERS; lyr++) {
		DBG_INFO("Computing layer %d outputs\n", lyr);
		switch(cnnLayerNodes[lyr].lyrType) {
			case CONV:
				DBG_INFO("conv layer start\n");
				pConvCtx = (CONV_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				if (pConvCtx->lyrArithMode != prevArithMode) {
					if (pConvCtx->lyrArithMode == FLOAT_POINT) {
						DBG_INFO("Converting fix to float\n");
						fix16_to_float_data(pFixInput, prevMapH * prevNmaps * prevMapW, prevFracBits, pFloatInput);
					} else {
						DBG_INFO("Converting float to fix\n");
						float_to_fix_data(pFloatInput,  prevMapH * prevNmaps * prevMapW, prevFracBits, pFixInput);
					}
				}
				// compute this layer's output
				cnn_conv_layer(pConvCtx, pFloatInput, pFixInput, MAP_ISOLATED);
				// Assign this output to next layer's input
				pFixInput = pConvCtx->pFixOutput;
				pFloatInput = pConvCtx->pFloatOutput;
				prevArithMode = pConvCtx->lyrArithMode;
				prevFracBits = pConvCtx->convInfo.nMapFractionBits;
				prevMapH = (pConvCtx->convInfo.mapH + 2*pConvCtx->convInfo.pad - pConvCtx->convInfo.K + 1 + pConvCtx->convInfo.stride - 1)/pConvCtx->convInfo.stride;
				prevMapW = (pConvCtx->convInfo.mapW + 2*pConvCtx->convInfo.pad - pConvCtx->convInfo.K + 1 + pConvCtx->convInfo.stride - 1)/pConvCtx->convInfo.stride;
				prevNmaps = pConvCtx->convInfo.nOutMaps;
				DBG_INFO("conv layer END\n");
				break;
			case POOL:
				DBG_INFO("pool layer start\n");
				pPoolCtx = (POOL_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;				
				if (pPoolCtx->lyrArithMode != prevArithMode) {
					if (pPoolCtx->lyrArithMode == FLOAT_POINT) {
						DBG_INFO("Converting fix to float\n");
						fix16_to_float_data(pFixInput, prevMapH * prevNmaps * prevMapW, prevFracBits, pFloatInput);
					} else {
						DBG_INFO("Converting float to fix\n");
						float_to_fix_data(pFloatInput,  prevMapH * prevNmaps * prevMapW, prevFracBits, pFixInput);
					}
				}
				cnn_pool_layer(pPoolCtx, pFloatInput, pFixInput, MAP_ISOLATED);
				pFixInput = pPoolCtx->pFixOutput;
				pFloatInput = pPoolCtx->pFloatOutput;
				prevMapH =( pPoolCtx->poolInfo.mapH + 2*pPoolCtx->poolInfo.pad - pPoolCtx->poolInfo.winSize + 1 + pPoolCtx->poolInfo.stride - 1)/ pPoolCtx->poolInfo.stride;
				prevMapW =( pPoolCtx->poolInfo.mapW + 2*pPoolCtx->poolInfo.pad - pPoolCtx->poolInfo.winSize + 1 + pPoolCtx->poolInfo.stride - 1)/ pPoolCtx->poolInfo.stride;
				prevNmaps = pPoolCtx->poolInfo.nMaps;
				prevArithMode = pPoolCtx->lyrArithMode;
				DBG_INFO("pool layer END\n");
				break;
			case ACT:
				pActCtx = (ACT_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;				
				if (pActCtx->lyrArithMode != prevArithMode) {
					if (pActCtx->lyrArithMode == FLOAT_POINT) {
						DBG_INFO("Converting fix to float\n");
						fix16_to_float_data(pFixInput, prevMapH * prevNmaps * prevMapW, prevFracBits, pFloatInput);
					} else {
						DBG_INFO("Converting float to fix\n");
						float_to_fix_data(pFloatInput,  prevMapH * prevNmaps * prevMapW, prevFracBits, pFixInput);
					}
				}
				cnn_activation_layer(pActCtx, pFloatInput, pFixInput);
				pFixInput = pActCtx->pFixOutput;
				pFloatInput = pActCtx->pFloatOutput;
				prevMapH = pActCtx->actInfo.mapH;
				prevMapW = pActCtx->actInfo.mapW;
				prevNmaps = pActCtx->actInfo.nMaps;
				prevArithMode = pActCtx->lyrArithMode;
				break;
			case INNER_PROD:
				pIpCtx = (IP_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;				
				if (pIpCtx->lyrArithMode != prevArithMode) {
					if (pIpCtx->lyrArithMode == FLOAT_POINT) {
						DBG_INFO("Converting fix to float\n");
						fix16_to_float_data(pFixInput, prevMapH * prevNmaps * prevMapW, prevFracBits, pFloatInput);
					} else {
						DBG_INFO("Converting float to fix\n");
						float_to_fix_data(pFloatInput,  prevMapH * prevNmaps * prevMapW, prevFracBits, pFixInput);
					}
				}
				inner_prod_layer(pIpCtx, pFloatInput, pFixInput);
				pFixInput = pIpCtx->pFixOutput;
				pFloatInput = pIpCtx->pFloatOutput;
				prevMapH = 1;
				prevMapW = pIpCtx->ipInfo.nOutput;
				prevNmaps = 1;
				prevArithMode = pIpCtx->lyrArithMode;
				//DBG_MAPS(print_float_img(pIpCtx->pFloatOutput, 1, prevMapW));
				break;
			case SOFTMAX:
				pSmaxCtx = (SMAX_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;				
				softmax_layer(pSmaxCtx, pFloatInput, pFixInput);
				pFloatInput = pSmaxCtx->pFloatOutput;
				break;
			default:
				REL_INFO("Unsupported layer\n");
				return UNSUPPORTED_FEATURE;
		}
	}
	PRINT_RUNTIME("App main loop runtime", startTime);
	//DBG_MAPS(cvWaitKey(100000));
	// TODO: take last layer output and find the max probability/label
	REL_INFO("Relesaing buffers\n");
	cnn_app_memfree(cnnLayerNodes, NO_DEEP_LAYERS);

	return status;
}
