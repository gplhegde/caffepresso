#include "inner_prod_layer.h"
#include "debug_control.h"
#include "misc_utils.h"
#include "vbx.h"
#include "vbx_port.h"

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

static inline float fix16_dot_prod(FP_MAP_PIXEL *pInput , FP_KERNEL *pWeight, int len, int shift) {
    int e;
    int32_t sop;

    sop = 0;
    for (e = 0; e < len; e++) {
        sop += pInput[e] * pWeight[e];
    }
	// TODO:hanfle to overflow
    return (FP_MAP_PIXEL)(sop >> shift);
}

static inline void fix16_vect_add(FP_MAP_PIXEL *pVect1, FP_MAP_PIXEL *pVect2, int vLen, FP_MAP_PIXEL *pSum) {
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

	int n;
	REL_ASSERT(pIpCtx->ipInfo.nKerFractionBits == pIpCtx->ipInfo.nMapFractionBits);
	for (n = 0; n < pIpCtx->ipInfo.nOutput; n++) {
		pIpCtx->pFixOutput[n] = fix16_dot_prod(pFixInput, pIpCtx->pFixWeight + n * pIpCtx->ipInfo.nInput,
			pIpCtx->ipInfo.nInput, pIpCtx->ipInfo.nKerFractionBits);
	}
	// FIXME: make sure both vectors are in same Q format
	fix16_vect_add(pIpCtx->pFixOutput, pIpCtx->pFixBias, pIpCtx->ipInfo.nOutput, pIpCtx->pFixOutput);
	return SUCCESS;
}

APP_STATUS_E mxp_mtx_vec_mult(FP_KERNEL *pWeight, FP_MAP_PIXEL *pAct, FP_KERNEL *pBias, int nRows, int nCols, int shift, FP_MAP_PIXEL *pOut) {
	// TODO: Use matrix mult API from MXP software development kit for more efficiency
	FP_KERNEL *spBias, *spWeightPing, *spWeightPong, *spTemp;
	FP_MAP_PIXEL *spAct, *spOut;
	int32_t *spAcc, *spExt1, *spExt2;
	int out;
	if((NULL == (spBias = vbx_sp_malloc(nRows * sizeof(FP_KERNEL)))) ||
		(NULL == (spAct = vbx_sp_malloc(nCols * sizeof(FP_MAP_PIXEL)))) ||
		(NULL == (spExt1 = vbx_sp_malloc(nCols * sizeof(int32_t)))) ||
		(NULL == (spExt2 = vbx_sp_malloc(nCols * sizeof(int32_t)))) ||
		(NULL == (spAcc = vbx_sp_malloc(nRows * sizeof(int32_t)))) ||
		(NULL == (spOut = vbx_sp_malloc(nRows * sizeof(FP_MAP_PIXEL)))) ||
		(NULL == (spWeightPing = vbx_sp_malloc(nRows * sizeof(FP_KERNEL)))) ||
		(NULL == (spWeightPong = vbx_sp_malloc(nRows * sizeof(FP_KERNEL))))) {
		REL_INFO("SP buffer allocation failed\n");
		return SP_MALLOC_FAIL;
	}
	vbx_dcache_flush_all();
	// reset the accumulator
	vbx_set_vl(nRows);
	vbx(SVW, VMUL, spAcc, 0, spAcc);

	vbx_set_vl(nCols);

	vbx_dma_to_vector(spWeightPing, pWeight, nCols * sizeof(FP_KERNEL));
	vbx_dma_to_vector(spBias, pBias, nRows * sizeof(FP_KERNEL));
	vbx_dma_to_vector(spAct, pAct, nCols * sizeof(FP_MAP_PIXEL));

	vbx(VVHW, VMOV, spExt1, spAct, 0);
	for(out = 0; out < nRows; out++) {
		// double buffering the weight matrix, one row at a time
		if(out < nRows - 1) {
			vbx_dma_to_vector(spWeightPong, pWeight + (out+1)*nCols, nCols * sizeof(FP_KERNEL));
		}
		
		// dot product
		// FIXME: vbx_acc is not working as expected. Even if the output type is W, the sum of prod is restricted to 16b
		// Contact VectorBlox to get clarification on this behavior

		// TEMPFIX: Use extra buffer to sign extend both input and weights to 32b and then do 32b x 32b + 32b ==> 32b dot prod
		vbx(VVHW, VMOV, spExt2, spWeightPing, 0);
		//vbx_acc(VVHW, VMUL, spAcc + out, spWeightPing, spAct);
		vbx_acc(VVWW, VMUL, spAcc + out, spExt2, spExt1);
		spTemp = spWeightPing;
		spWeightPing = spWeightPong;
		spWeightPong = spTemp;
	}
	// convert to 16bit and add bias
	vbx_set_vl(nRows);
	// TODO:hanfle to overflow
	vbx(SVWH, VSHR, spOut, shift, spAcc);
	vbx(VVHH, VADD, spOut, spBias, spOut);

	// send to host memory
	vbx_dma_to_host(pOut, spOut, nRows * sizeof(FP_MAP_PIXEL));
	vbx_sync();
	vbx_sp_free();
	return SUCCESS;
}

APP_STATUS_E vector_fix_ip_layer(IP_LYR_CTX_T *pIpCtx, FP_MAP_PIXEL *pFixInput) {
	APP_STATUS_E status;

	status = mxp_mtx_vec_mult(pIpCtx->pFixWeight, pFixInput, pIpCtx->pFixBias,
		pIpCtx->ipInfo.nOutput, pIpCtx->ipInfo.nInput,
		pIpCtx->ipInfo.nKerFractionBits, pIpCtx->pFixOutput);

	return status;
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
            switch(pIpCtx->optType) {
                case SCALAR:
					DBG_INFO("Scalar fixed point inner product layer\n");
					retStatus = scalar_fix_ip_layer(pIpCtx, pFixInput);
                    break;
                case VECTOR_NEON:
                    REL_INFO("Not supported as of now\n");
					retStatus = UNSUPPORTED_FEATURE;
                    break;
                case VECTOR_MXP:
					DBG_INFO("Vector MXP fixed point inner product layer\n");
					retStatus = vector_fix_ip_layer(pIpCtx, pFixInput);
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
