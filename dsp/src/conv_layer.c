#include "conv_layer.h"
#include "struct_defs.h"

STATUS_E dsp_fix_conv_layer(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int ker_size,		// kernel size. We support only square sized kernels
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {

	STATUS_E status = FAILED;
	//TODO
	// Refer to https://github.com/gplhegde/caffepresso/blob/master/mxp/src/conv_layer.c:scalar_fix_conv_layer function
	// for implementation details.
	return status;
}

STATUS_E dsp_flt_conv_layer(FLT_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FLT_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FLT_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int ker_size,		// kernel size. We support only square sized kernels
	int stride,			// convolution window stride in both horizontal and vertical direction.
	FLT_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {

	STATUS_E status = FAILED;
	//TODO
	// Refer to https://github.com/gplhegde/caffepresso/blob/master/mxp/src/conv_layer.c:scalar_fix_conv_layer function
	// for implementation details.
	return status;
}

STATUS_E dsp_conv_layer(CONV_LYR_CTX_T *p_conv_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_map) {
	STATUS_E status = FAILED;

	if(p_conv_ctx->lyr_arith_mode == FIXED_POINT) {
		status = dsp_fix_conv_layer(p_fix_in_map,
			p_conv_ctx->p_fix_ker,
			p_conv_ctx->p_fix_bias,
			p_conv_ctx->conv_info.map_h,
			p_conv_ctx->conv_info.map_w,
			p_conv_ctx->conv_info.no_inputs,
			p_conv_ctx->conv_info.no_outputs,
			p_conv_ctx->conv_info.ker_size,
			p_conv_ctx->conv_info.stride,
			p_conv_ctx->conv_info.no_ker_frac_bits,
			p_conv_ctx->p_fix_output
			);
	} else {
		status = dsp_flt_conv_layer(p_flt_in_maps,
			p_conv_ctx->p_flt_ker,
			p_conv_ctx->p_flt_bias,
			p_conv_ctx->conv_info.map_h,
			p_conv_ctx->conv_info.map_w,
			p_conv_ctx->conv_info.no_inputs,
			p_conv_ctx->conv_info.no_outputs,
			p_conv_ctx->conv_info.ker_size,
			p_conv_ctx->conv_info.stride,
			p_conv_ctx->p_flt_output
			);
	}

	return status;
}
