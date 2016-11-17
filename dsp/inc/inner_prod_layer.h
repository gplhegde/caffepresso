#ifndef _INNER_PROD_LAYER_H
#define _INNER_PROD_LAYER_H
#include "struct_defs.h"

typedef struct {
	// = 1. This is just to keep uniformity
	int map_h;
	// = 1. This is just to keep uniformity
	int map_w;
	// No of input neurons
	int no_inputs;
	// No of output neurons
	int no_outputs;
	// No of fraction bits in weight and bias representation
	int no_ker_frac_bits;
	// No of fraction bits in activation representation
	int no_map_frac_bits;
} IP_INFO_T;

// Inner product layer context
typedef struct {
	// Floating point output map buffer for this layer.
	FLT_MAP *p_flt_output;
	// Fixed point output map buffer for this layer.
	FIX_MAP *p_fix_output;
	// Fixed point weight
	FIX_KER *p_fix_weight;
	// Floating point weight matrix
	FLT_KER *p_flt_weight;
	// Fixed point bias
	FIX_KER *p_fix_bias;
	// Floating point bias
	FLT_KER *p_flt_bias;
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyr_arith_mode;
	// Parameters
	IP_INFO_T ip_info;
} IP_LYR_CTX_T;

STATUS_E dsp_ip_layer(FIX_MAP *p_input,	// pointer to input features
	FIX_KER *p_weight,	// pointer to weight matrix stored in [no_outputs][no_inputs] manner
	FIX_KER *p_bias,	// pointer to bias units
	int no_inputs,		// number of input units to this layer
	int no_outputs,		// number of output units
	int shift,			// shift used to convert the dot product to 16b. Perform conversion before adding bias
	FIX_MAP *p_output	// pointer to output features.
	);
#endif // _INNER_PROD_LAYER_H
