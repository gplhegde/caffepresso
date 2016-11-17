#ifndef _SMAX_LAYER_H
#define _SMAX_LAYER_H
#include "struct_defs.h"

// Softmax layer context
typedef struct {
	// Floating point output map buffer for this layer. This contains the final probability of the classes
	FLT_MAP *p_float_output;
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyr_arith_mode;
	// No of input neurons
	int no_inputs;
} SMAX_LYR_CTX_T;

STATUS_E dsp_smax_layer(FIX_MAP *p_input,	// pointer to input features
	int N,				// total number of features(pixels) present in the input
	int no_frac_bits,	// number of fraction bits used to represent the inputs to this layer
	FIX_MAP *p_output	// pointer to output probabilities
	);
#endif // _SMAX_LAYER_H
