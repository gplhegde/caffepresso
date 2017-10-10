#ifndef _ACTIVATION_LAYER_H_
#define _ACTIVATION_LAYER_H_
#include "struct_defs.h"
#include "caffe_frontend.h"

//========================================
//========================================
// Activation layer parameters.
typedef struct {
	int nMaps;
	int mapH;
	int mapW;
	ACT_TYPE_E actType;
} ACT_INFO_T;

//========================================
//========================================
// Activation layer context
typedef struct {
	// Floating point output map buffer for this layer.
	FL_MAP_PIXEL *pFloatOutput;
	// Fixed point output map buffer for this layer.
	FP_MAP_PIXEL *pFixOutput;
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyrArithMode;
	// Layer optimization mode
	OPT_TYPE_E optType;
	// Parameters
	ACT_INFO_T actInfo;

} ACT_LYR_CTX_T;
//========================================
//========================================

/* \brief Abstract activation layer supporting float/fixed point and scalar/vector operations
*
* @param pActCtx        : Activation layer context. Refer to activation_layer.h for structure info.
* @param pFloatInMaps   : Floating point input feature maps.
* @return               : Activation layer status.
*/
APP_STATUS_E cnn_activation_layer(
	ACT_LYR_CTX_T *pActCtx,
	FL_MAP_PIXEL *pFloatInMaps
);

#endif // _ACTIVATION_LAYER_H
