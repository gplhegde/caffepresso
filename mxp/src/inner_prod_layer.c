#include "inner_prod_layer.h"
#include "debug_control.h"

static inline float float_dot_prod(float *pInput , float *pWeight, int len) {
    int e;
    float sop;

    sop = 0;
    for (e = 0; e < len; e++) {
        sop += pInput[e] * pWeight[e];
    }

    return sop;
}

static inline void float_vect_add(float *pVect1, float *pVect2, int vLen, float *pSum) {
    int e;
    for ( e = 0; e < vLen; e++) {
        pSum[e] = pVect1[e] + pVect2[e];
    }
}

APP_STATUS_E scalar_float_ip_layer(IP_LYR_CTX_T *pIpCtx, FL_MAP_PIXEL *pFloatInput) {

	int n;

	for ( n = 0; n < pIpCtx->ipInfo.nOutput; n++) {
		pIpCtx->pFloatOutput[n] = float_dot_prod(pFloatInput, pIpCtx->pFloatWeight + n * pIpCtx->ipInfo.nInput, pIpCtx->ipInfo.nInput);
	}
	float_vect_add(pIpCtx->pFloatOutput, pIpCtx->pFloatBias, pIpCtx->ipInfo.nOutput, pIpCtx->pFloatOutput);
	return SUCCESS;

}

APP_STATUS_E scalar_fix_ip_layer(IP_LYR_CTX_T *pIpCtx, FP_MAP_PIXEL *pFixInput) {

	// TODO
	return FAILED;
}

APP_STATUS_E inner_prod_layer(IP_LYR_CTX_T *pIpCtx, FL_MAP_PIXEL *pFloatInput, FP_MAP_PIXEL *pFixInput) {

	APP_STATUS_E retStatus = FAILED;
    switch(pIpCtx->lyrArithMode) {
        case FLOAT_POINT:
            switch(pIpCtx->optType) {
                case SCALAR:
					DBG_INFO("Scalar floating point inner product layer\n");
                    retStatus = scalar_float_ip_layer(pIpCtx, pFloatInput);
                    break;
                case VECTOR_NEON:
                    REL_INFO("No supported as of now\n");
					retStatus = UNSUPPORTED_FEATURE;
                    break;
                case VECTOR_MXP:
                    REL_INFO("MXP does not support floating point\n");
					retStatus = UNSUPPORTED_FEATURE;
                    break;
                default:
                    REL_INFO("Invalid optimization type\n");
                    break;
            }
            break;
        case FIXED_POINT:
			REL_INFO("FIXED_POINT mode for INNER_PROD layer is not supported as of now\n");
            switch(pIpCtx->optType) {
                case SCALAR:
					DBG_INFO("Scalar fixed point inner product layer\n");
                    REL_INFO("Not supported as of now\n");
					retStatus = UNSUPPORTED_FEATURE;
                    break;
                case VECTOR_NEON:
                    REL_INFO("Not supported as of now\n");
					retStatus = UNSUPPORTED_FEATURE;
                    break;
                case VECTOR_MXP:
					DBG_INFO("Vector MXP fixed point inner product layer\n");
                    REL_INFO("Not supported as of now\n");
					retStatus = UNSUPPORTED_FEATURE;
                    break;
                default:
                    REL_INFO("Invalid optimization type\n");
            }
            break;
        default:
            break;
    }
    return retStatus;
}
