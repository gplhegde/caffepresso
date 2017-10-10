#include "activation_layer.h"
#include <math.h>
#include "debug_control.h"

/* \brief Floating point scalar activation layer. The activation is a element-wise operation
*
* @param pInMaps	: Input feature maps.
* @param pActInfo	: Parameters for activation layer.
* @param pOutMaps	: Buffer to store activation layer output.
* @return			: activation layer status.
*/
APP_STATUS_E scalar_float_activation_layer(FL_MAP_PIXEL *pInMaps, ACT_INFO_T *pActInfo, FL_MAP_PIXEL *pOutMaps) {

	int pixel;
	APP_STATUS_E retStatus = FAILED;

	switch(pActInfo->actType) {
		// Sigmoidal activation
		case SIGMOID:
			for (pixel = 0; pixel < pActInfo->nMaps * pActInfo->mapH * pActInfo->mapW; pixel++) {
				pOutMaps[pixel] = 1 / (1 + exp(-pInMaps[pixel]));
			}
			retStatus = SUCCESS;
			break;
		// Hyperbolic tan activation
		case TANH:
			for (pixel = 0; pixel < pActInfo->nMaps * pActInfo->mapH * pActInfo->mapW; pixel++) {
				pOutMaps[pixel] = tanh(pInMaps[pixel]);			
			}
			retStatus = SUCCESS;
			break;
		// Rectified linear activation
		case RELU:
			for (pixel = 0; pixel < pActInfo->nMaps * pActInfo->mapH * pActInfo->mapW; pixel++) {
				pOutMaps[pixel] = MAX(pInMaps[pixel], 0);			
			}				
			retStatus = SUCCESS;
			break;
		default:
			REL_INFO("Invalid OR Unsupported activation type\n");
			break;
	}
	return retStatus;
}

/* \brief Abstract activation layer supporting float/fixed point and scalar/vector operations
*
* @param pActCtx		: Activation layer context. Refer to activation_layer.h for structure info.
* @param pFloatInMaps	: Floating point input feature maps.
* @return				: Activation layer status.
*/
APP_STATUS_E cnn_activation_layer(ACT_LYR_CTX_T *pActCtx, FL_MAP_PIXEL *pFloatInMaps) {

	APP_STATUS_E retStatus = FAILED;
    switch(pActCtx->lyrArithMode) {
        case FLOAT_POINT:
            switch(pActCtx->optType) {
                case SCALAR:
					// DBG_INFO("Scalar floating point activation layer\n");
                    retStatus = scalar_float_activation_layer(pFloatInMaps,
                        &pActCtx->actInfo,
                        pActCtx->pFloatOutput);
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
