#include "app_init.h"
#include "debug_control.h"
#include "misc_utils.h"
#include <string.h>

APP_STATUS_E init_conv_kernels(CONV_LYR_CTX_T *pConvCtx) {

	int map, coeff, k, i;

	// generate random kernel and convert them to fix point
	for ( i = 0; i < pConvCtx->convInfo.nInMaps; i++) {
		for (k = 0; k < pConvCtx->convInfo.nOutMaps; k++) {
			generate_random_data(pConvCtx->pFloatKer + (i * pConvCtx->convInfo.nOutMaps + k) * pConvCtx->convInfo.K * pConvCtx->convInfo.K,
				pConvCtx->convInfo.K * pConvCtx->convInfo.K, i*k+k);
			float_to_fix_data(pConvCtx->pFloatKer + (i * pConvCtx->convInfo.nOutMaps + k) * pConvCtx->convInfo.K * pConvCtx->convInfo.K,
				pConvCtx->convInfo.K * pConvCtx->convInfo.K,
				pConvCtx->convInfo.nKerFractionBits,
				pConvCtx->pFixKer + (i * pConvCtx->convInfo.nOutMaps + k) * pConvCtx->convInfo.K * pConvCtx->convInfo.K);
		}
	}
	// Kernel extension
	if (pConvCtx->optType == VECTOR_MXP) {
		for ( i = 0; i < pConvCtx->convInfo.nInMaps; i++) {
			for ( k = 0; k < pConvCtx->convInfo.K * pConvCtx->convInfo.K; k++) {
				for (map = 0; map < pConvCtx->convInfo.nOutMaps; map++) {
					// Repeate the coefficient image width no of times
					for (coeff = 0; coeff < pConvCtx->blkInfo.blkW; coeff++) {
						*(pConvCtx->ppExtKer[i * pConvCtx->convInfo.K * pConvCtx->convInfo.K + k] + pConvCtx->blkInfo.blkW * map + coeff) =
							pConvCtx->pFixKer[(i * pConvCtx->convInfo.nOutMaps + map) * pConvCtx->convInfo.K * pConvCtx->convInfo.K + k];
					}
				}
			}
		}
	}

	// Init bias of conv layer.
	generate_random_data(pConvCtx->pFloatBias, pConvCtx->convInfo.nOutMaps, 123);
	float_to_fix_data(pConvCtx->pFloatBias,
		pConvCtx->convInfo.nOutMaps,
		pConvCtx->convInfo.nKerFractionBits,
		pConvCtx->pFixBias);
	return SUCCESS;
}

APP_STATUS_E init_ip_layer_params(IP_LYR_CTX_T *pIpCtx) {
	generate_random_data(pIpCtx->pFloatWeight, pIpCtx->ipInfo.nOutput * pIpCtx->ipInfo.nInput, 1234);
	generate_random_data(pIpCtx->pFloatBias, pIpCtx->ipInfo.nOutput, 4321);
	
	return SUCCESS;
}

APP_STATUS_E caffe_cnn_layer_malloc(void *pLyrCtx, CNN_LAYER_TYPE_E lyrType) {
	int k, iW, iH, oW, oH;
	
	// FIXME: Buffers for floating and pixed point mode are allocated. Need to allocate only 1 based on configuration
	// Keeping both allocations for cross verification btw float and fixed point computations.
	switch(lyrType) {
		case CONV:
		{
			CONV_LYR_CTX_T *pConvCtx = (CONV_LYR_CTX_T *)pLyrCtx;
			iW = pConvCtx->convInfo.mapW + 2*pConvCtx->convInfo.pad;
			iH = pConvCtx->convInfo.mapH + 2*pConvCtx->convInfo.pad;
			oW = (iW - pConvCtx->convInfo.K + 1 + pConvCtx->convInfo.stride - 1) / pConvCtx->convInfo.stride;
			oH = (iH - pConvCtx->convInfo.K + 1 + pConvCtx->convInfo.stride - 1) / pConvCtx->convInfo.stride;
			if ((NULL == (pConvCtx->pFloatOutput = (FL_MAP_PIXEL *)malloc(oH * oW * pConvCtx->convInfo.nOutMaps * sizeof(FL_MAP_PIXEL)))) ||
				(NULL == (pConvCtx->pFixOutput = (FP_MAP_PIXEL *)malloc(oH * oW * pConvCtx->convInfo.nOutMaps * sizeof(FP_MAP_PIXEL)))) ||
				(NULL == (pConvCtx->pFloatBias = (FL_KERNEL *)malloc(pConvCtx->convInfo.nOutMaps * sizeof (FL_KERNEL)))) ||
				(NULL == (pConvCtx->pFixBias = (FP_KERNEL *)malloc(pConvCtx->convInfo.nOutMaps * sizeof (FP_KERNEL)))) ||
				(NULL == (pConvCtx->pFloatKer = (FL_KERNEL *)malloc(pConvCtx->convInfo.nInMaps *
				pConvCtx->convInfo.K * pConvCtx->convInfo.K * pConvCtx->convInfo.nOutMaps * sizeof (FL_KERNEL)))) ||
				(NULL == (pConvCtx->pFixKer = (FP_KERNEL *)malloc(pConvCtx->convInfo.nInMaps *
				pConvCtx->convInfo.K * pConvCtx->convInfo.K * pConvCtx->convInfo.nOutMaps * sizeof (FP_KERNEL))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			if (pConvCtx->optType == VECTOR_MXP) {
				if(NULL == (pConvCtx->ppExtKer = malloc(pConvCtx->convInfo.nInMaps * pConvCtx->convInfo.K *
					pConvCtx->convInfo.K * sizeof(FP_KERNEL *)))) {
					REL_INFO("Malloc failed\n");
					return MALLOC_FAIL;
				}
				for ( k = 0; k < pConvCtx->convInfo.K * pConvCtx->convInfo.K * pConvCtx->convInfo.nInMaps; k++) {
					if (NULL == (pConvCtx->ppExtKer[k] = (FP_KERNEL *)malloc(pConvCtx->convInfo.nOutMaps *
						pConvCtx->blkInfo.blkW * sizeof(FP_KERNEL)))) {
						REL_INFO("Malloc failed\n");
						return MALLOC_FAIL;
					}
				}
			}
			break;
		}

		case POOL:
		{
			POOL_LYR_CTX_T *pPoolCtx = (POOL_LYR_CTX_T *)pLyrCtx;
			iW = pPoolCtx->poolInfo.mapW + 2*pPoolCtx->poolInfo.pad;
			iH = pPoolCtx->poolInfo.mapH + 2*pPoolCtx->poolInfo.pad;
			oW = (iW - pPoolCtx->poolInfo.winSize + 1 + pPoolCtx->poolInfo.stride - 1) / pPoolCtx->poolInfo.stride;
			oH = (iH - pPoolCtx->poolInfo.winSize + 1 + pPoolCtx->poolInfo.stride - 1) / pPoolCtx->poolInfo.stride;

			if ((NULL == (pPoolCtx->pFloatOutput = (FL_MAP_PIXEL *)malloc(oH * oW * pPoolCtx->poolInfo.nMaps * sizeof(FL_MAP_PIXEL)))) ||
				(NULL == (pPoolCtx->pFixOutput = (FP_MAP_PIXEL *)malloc(oH * oW * pPoolCtx->poolInfo.nMaps * sizeof(FP_MAP_PIXEL))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			break;
		}

		case ACT:	
		{
			ACT_LYR_CTX_T *pActCtx = (ACT_LYR_CTX_T *)pLyrCtx;
			if ((NULL == (pActCtx->pFloatOutput = (FL_MAP_PIXEL *)malloc(pActCtx->actInfo.nMaps *
				pActCtx->actInfo.mapW * pActCtx->actInfo.mapH * sizeof(FL_MAP_PIXEL)))) ||
				(NULL == (pActCtx->pFixOutput = (FP_MAP_PIXEL *)malloc(pActCtx->actInfo.nMaps *
				pActCtx->actInfo.mapW * pActCtx->actInfo.mapH * sizeof(FP_MAP_PIXEL))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			break;
		}

		case INNER_PROD:
		{
			IP_LYR_CTX_T *pIpCtx = (IP_LYR_CTX_T *)pLyrCtx;
			if ((NULL == (pIpCtx->pFloatOutput = (FL_MAP_PIXEL *)malloc(pIpCtx->ipInfo.nOutput * sizeof(FL_MAP_PIXEL)))) ||
				(NULL == (pIpCtx->pFixOutput = (FP_MAP_PIXEL *)malloc(pIpCtx->ipInfo.nOutput * sizeof(FP_MAP_PIXEL)))) ||
				(NULL == (pIpCtx->pFloatWeight = (FL_MAP_PIXEL *)malloc(pIpCtx->ipInfo.nOutput *
				pIpCtx->ipInfo.nInput * sizeof(FL_MAP_PIXEL)))) ||
				(NULL == (pIpCtx->pFixWeight = (FP_MAP_PIXEL *)malloc(pIpCtx->ipInfo.nOutput *
				pIpCtx->ipInfo.nInput * sizeof(FP_MAP_PIXEL)))) ||
				(NULL == (pIpCtx->pFloatBias = (FL_MAP_PIXEL *)malloc(pIpCtx->ipInfo.nOutput * sizeof(FL_MAP_PIXEL)))) ||
				(NULL == (pIpCtx->pFixBias = (FP_MAP_PIXEL *)malloc(pIpCtx->ipInfo.nOutput * sizeof(FP_MAP_PIXEL))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			break;
		}
		case SOFTMAX:
		{
			SMAX_LYR_CTX_T *pSmaxCtx = (SMAX_LYR_CTX_T *)pLyrCtx;
			if(NULL == (pSmaxCtx->pFloatOutput = (FL_MAP_PIXEL *)malloc(pSmaxCtx->nInputs * sizeof(FL_MAP_PIXEL)))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			break;
		}
		default:
			REL_INFO("Unsupported layer\n");
			break;
	}
	return SUCCESS;
}

APP_STATUS_E caffe_cnn_layer_mem_free(void *pLyrCtx, CNN_LAYER_TYPE_E lyrType) {

	int k;

	switch(lyrType) {
		case CONV:
		{
			CONV_LYR_CTX_T *pConvCtx = (CONV_LYR_CTX_T *)pLyrCtx;
			free(pConvCtx->pFloatOutput);
			free(pConvCtx->pFixOutput);
			free(pConvCtx->pFloatBias);
			free(pConvCtx->pFixBias);
			free(pConvCtx->pFloatKer);
			free(pConvCtx->pFixKer);
			if (pConvCtx->optType == VECTOR_MXP) {
				for (k = 0; k < pConvCtx->convInfo.K * pConvCtx->convInfo.K * pConvCtx->convInfo.nInMaps; k++) {
					free(pConvCtx->ppExtKer[k]);
				}
				free(pConvCtx->ppExtKer);	
			}
			break;
		}

		case POOL:
		{
			POOL_LYR_CTX_T *pPoolCtx = (POOL_LYR_CTX_T *)pLyrCtx;
			free(pPoolCtx->pFloatOutput);
			free(pPoolCtx->pFixOutput);
			break;
		}

		case INNER_PROD:
		{
			IP_LYR_CTX_T *pIpCtx = (IP_LYR_CTX_T *)pLyrCtx;
			free(pIpCtx->pFloatOutput);
			free(pIpCtx->pFixOutput);
			free(pIpCtx->pFloatWeight);
			free(pIpCtx->pFixWeight);
			free(pIpCtx->pFloatBias);
			free(pIpCtx->pFixBias);
			break;
		}

		case ACT:
		{
			ACT_LYR_CTX_T *pActCtx = (ACT_LYR_CTX_T *)pLyrCtx;
			free(pActCtx->pFloatOutput);
			free(pActCtx->pFixOutput);
			break;
		}
		case SOFTMAX:
		{
			SMAX_LYR_CTX_T *pSmaxCtx = (SMAX_LYR_CTX_T *)pLyrCtx;
			free(pSmaxCtx->pFloatOutput);
			break;	
		}

		default:
			REL_INFO("Unsupported layer\n");
			break;
	}
	return SUCCESS;
}

APP_STATUS_E cnn_app_malloc(CNN_LYR_NODE_T *pLyrNodes, int nLayers) {

	int lyr;
	CNN_LYR_NODE_T *pNode = pLyrNodes;

	for (lyr = 0; lyr < nLayers; lyr++) {
		printf("Layer = %d\n", lyr);
		caffe_cnn_layer_malloc(pNode->pLyrCtx, pNode->lyrType);
		pNode++;
	}
	return SUCCESS;
}

APP_STATUS_E cnn_app_memfree(CNN_LYR_NODE_T *pLyrNodes, int nLayers) {
	
	int lyr;
	CNN_LYR_NODE_T *pNode = pLyrNodes;

	for (lyr = 0; lyr < nLayers; lyr++) {
		caffe_cnn_layer_mem_free(pNode->pLyrCtx, pNode->lyrType);
		pNode++;
	}
	return SUCCESS;
}

APP_STATUS_E cnn_app_model_init(CNN_LYR_NODE_T *pLyrNodes, int nLayers) {

	int lyr;
	IP_LYR_CTX_T *pIpCtx;
	CONV_LYR_CTX_T *pConvCtx;
	CNN_LYR_NODE_T *pNode = pLyrNodes;

	REL_INFO("Initializing CNN model weights and biases\n");
	for (lyr = 0; lyr < nLayers; lyr++) {
		REL_INFO("Layer %d parameters are being initialized\n", lyr);
		if(pNode->lyrType == CONV) {
			pConvCtx = (CONV_LYR_CTX_T *)pNode->pLyrCtx;
			init_conv_kernels(pConvCtx);
		} else if(pNode->lyrType == INNER_PROD) {
			pIpCtx = (IP_LYR_CTX_T *)pNode->pLyrCtx;
			init_ip_layer_params(pIpCtx);
		}
		pNode++;
	}
	return SUCCESS;
}
