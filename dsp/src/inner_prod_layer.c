#include "inner_prod_layer.h"
#include "struct_defs.h"

static inline float float_dot_prod(float *p_input , float *p_weight, int len) {
    int e;
    float sop;

    sop = 0;
    for (e = 0; e < len; e++) {
        sop += p_input[e] * p_weight[e];
    }

    return sop;
}

static inline void float_vect_add(float *p_vect1, float *p_vect2, int v_len, float *p_sum) {
    int e;
    for ( e = 0; e < v_len; e++) {
        p_sum[e] = p_vect1[e] + p_vect2[e];
    }
}

STATUS_E dsp_fix_ip_layer(FIX_MAP *p_input,	// pointer to input features
	FIX_KER *p_weight,	// pointer to weight matrix stored in [no_outputs][no_inputs] manner
	FIX_KER *p_bias,	// pointer to bias units
	int no_inputs,		// number of input units to this layer
	int no_outputs,		// number of output units
	int shift,			// shift used to convert the dot product to 16b. Perform conversion before adding bias
	FIX_MAP *p_output	// pointer to output features.
	) {

	STATUS_E status = FAILED;
	// TODO 
	// Refer to https://github.com/gplhegde/caffepresso/blob/master/mxp/src/inner_prod_layer.c:scalar_fix_ip_layer function for
	// implementation details.
	return status;
}

STATUS_E dsp_flt_ip_layer(FLT_MAP *p_input,	// pointer to input features
	FLT_KER *p_weight,	// pointer to weight matrix stored in [no_outputs][no_inputs] manner
	FLT_KER *p_bias,	// pointer to bias units
	int no_inputs,		// number of input units to this layer
	int no_outputs,		// number of output units
	FLT_MAP *p_output	// pointer to output features.
	) {
	int n;
	STATUS_E status = FAILED;
	

	for ( n = 0; n < no_outputs; n++) {
		p_output[n] = float_dot_prod(p_input, p_weight + n * no_inputs, no_inputs);
	}
	float_vect_add(p_output, p_bias, no_outputs, p_output);

	return status;
}

STATUS_E dsp_ip_layer(IP_LYR_CTX_T *p_ip_ctx, FLT_MAP *p_flt_input, FIX_MAP *p_fix_input) {
	STATUS_E status = FAILED;

	if(p_ip_ctx->lyr_arith_mode == FIXED_POINT) {
		status = dsp_fix_ip_layer(p_fix_input,
			p_ip_ctx->p_fix_weight,
			p_ip_ctx->p_fix_bias,
			p_ip_ctx->ip_info.no_inputs,
			p_ip_ctx->ip_info.no_outputs,
			p_ip_ctx->ip_info.no_ker_frac_bits,
			p_ip_ctx->p_fix_output
			);
	} else {
		status = dsp_flt_ip_layer(p_flt_input,
			p_ip_ctx->p_flt_weight,
			p_ip_ctx->p_flt_bias,
			p_ip_ctx->ip_info.no_inputs,
			p_ip_ctx->ip_info.no_outputs,
			p_ip_ctx->p_flt_output
			);
	}
	return status;
}
