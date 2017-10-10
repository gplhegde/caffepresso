#ifndef _SMAX_LAYER_H_
#define _SMAX_LAYER_H_
#include "struct_defs.h"


// Softmax layer context
typedef struct {
	// Floating point output map buffer for this layer. This contains the final probability of the classes
	FL_MAP_PIXEL *pFloatOutput;
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyrArithMode;
	// Layer optimization mode
	OPT_TYPE_E optType;
	// No of input neurons
	int nInputs;

} SMAX_LYR_CTX_T;

APP_STATUS_E softmax_layer(
	SMAX_LYR_CTX_T *pSmaxCtx,
	FL_MAP_PIXEL *pFloatInput
);
#endif // _SMAX_LAYER_H_
