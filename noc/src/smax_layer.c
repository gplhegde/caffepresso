#include "debug_control.h"
#include "smax_layer.h"

APP_STATUS_E scalar_float_smax_layer(SMAX_LYR_CTX_T *pSmaxCtx, FL_MAP_PIXEL *pInput) {

	// TODO
	return SUCCESS;
}

APP_STATUS_E softmax_layer(SMAX_LYR_CTX_T *pSmaxCtx, FL_MAP_PIXEL *pFloatInput) {

	APP_STATUS_E retStatus = FAILED;
    switch(pSmaxCtx->lyrArithMode) {
        case FLOAT_POINT:
            switch(pSmaxCtx->optType) {
                case SCALAR:
                    retStatus = scalar_float_smax_layer(pSmaxCtx, pFloatInput);
                    break;
                default:
                    REL_INFO("Invalid optimization type\n");
                    break;
            }
            break;
        default:
            REL_INFO("Only floating-point arithmetic is supported\n");
            break;
    }
    return retStatus;
}
