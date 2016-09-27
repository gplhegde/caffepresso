#include "debug_control.h"
#include "smax_layer.h"

APP_STATUS_E scalar_float_smax_layer(SMAX_LYR_CTX_T *pSmaxCtx, FL_MAP_PIXEL *pInput) {

	// TODO
	return SUCCESS;
}

APP_STATUS_E softmax_layer(SMAX_LYR_CTX_T *pSmaxCtx, FL_MAP_PIXEL *pFloatInput, FP_MAP_PIXEL *pFixInput) {

	APP_STATUS_E retStatus = FAILED;
    switch(pSmaxCtx->lyrArithMode) {
        case FLOAT_POINT:
            switch(pSmaxCtx->optType) {
                case SCALAR:
                    retStatus = scalar_float_smax_layer(pSmaxCtx, pFloatInput);
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
            switch(pSmaxCtx->optType) {
                case SCALAR:
                    REL_INFO("Not supported as of now\n");
					retStatus = UNSUPPORTED_FEATURE;
                    break;
                case VECTOR_NEON:
                    REL_INFO("Not supported as of now\n");
					retStatus = UNSUPPORTED_FEATURE;
                    break;
                case VECTOR_MXP:
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
