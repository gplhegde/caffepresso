#include "smax_layer.h"
#include "struct_defs.h"

STATUS_E dsp_smax_layer(FIX_MAP *p_input,	// pointer to input features
	int N,				// total number of features(pixels) present in the input
	int no_frac_bits,	// number of fraction bits used to represent the inputs to this layer
	FIX_MAP *p_output	// pointer to output probabilities
	) {

	STATUS_E status = FAILED;
	//TODO
	// Please refer to https://github.com/gplhegde/caffepresso/blob/master/mxp/src/smax_layer.c
	// for implementation details.

	// Hints: see if exp() is supported in DSP libs
	// 
	return status;
}
