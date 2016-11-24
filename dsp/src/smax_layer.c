#include "smax_layer.h"
#include "struct_defs.h"
#include <math.h>

STATUS_E dsp_flt_smax_layer(FLT_MAP *p_input,	// pointer to input features
	int N,				// total number of features(pixels) present in the input
	FLT_MAP *p_output	// pointer to output probabilities
	) {
	int i;
	float sum = 0.0;
	STATUS_E status = SUCCESS;

	for(i = 0; i < N; i++) {
		p_output[i] = exp(p_input[i]);
		sum += p_output[i];
	}
	for(i = 0; i < N; i++) {
		p_output[i] = p_output[i] / sum;
	}
	return status;
}

STATUS_E dsp_smax_layer(SMAX_LYR_CTX_T *p_smax_ctx, FLT_MAP *p_flt_input) {
	STATUS_E status = FAILED;

	status = dsp_flt_smax_layer(p_flt_input, p_smax_ctx->no_inputs, p_smax_ctx->p_float_output);
	return status;
}
