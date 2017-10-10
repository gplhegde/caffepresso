#include "inner_prod_layer.h"
#include "debug_control.h"
#include "misc_utils.h"

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

APP_STATUS_E inner_prod_layer(IP_LYR_CTX_T *pIpCtx, FL_MAP_PIXEL *pFloatInput) {

	APP_STATUS_E retStatus = FAILED;
    switch(pIpCtx->lyrArithMode) {
        case FLOAT_POINT:
            switch(pIpCtx->optType) {
                case SCALAR:
					// DBG_INFO("Scalar floating point inner product layer\n");
                    retStatus = scalar_float_ip_layer(pIpCtx, pFloatInput);
                    break;
                default:
                    REL_INFO("Invalid optimization type\n");
                    break;
            }
            break;
		default:
			REL_INFO("Only FP arithmetic supported.\n");
            break;
    }
    return retStatus;
}
