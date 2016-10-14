#ifndef _INNER_PROD_LAYER_H_
#define _INNER_PROD_LAYER_H_
#include "struct_defs.h"

typedef struct {
	// No of input neurons
	int nInput;
	// No of output neurons
	int nOutput;
	// No of fraction bits in weight and bias representation
	int nKerFractionBits;
	// No of fraction bits in activation representation
	int nMapFractionBits;
} IP_INFO_T;

// Inner product layer context
typedef struct {
	// Floating point output map buffer for this layer.
	FL_MAP_PIXEL *pFloatOutput;
	// Fixed point output map buffer for this layer.
	FP_MAP_PIXEL *pFixOutput;
	// Fixed point weight
	FP_KERNEL *pFixWeight;
	// Floating point weight matrix
	FL_KERNEL *pFloatWeight;
	// Fixed point bias
	FP_KERNEL *pFixBias;
	// Floating point bias
	FL_KERNEL *pFloatBias;
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyrArithMode;
	// Layer optimization mode
	OPT_TYPE_E optType;
	// Parameters
	IP_INFO_T ipInfo;

} IP_LYR_CTX_T;

APP_STATUS_E inner_prod_layer(IP_LYR_CTX_T *pIpCtx,
	FL_MAP_PIXEL *pFloatInput,
	FP_MAP_PIXEL *pFixInput);
#endif // _INNER_PROD_LAYER_H_
