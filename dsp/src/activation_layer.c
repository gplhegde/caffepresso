#include "activation_layer.h"
#include <math.h>
#include "debug_control.h"

extern unsigned int core_id;

STATUS_E dsp_fix_activation_layer(FIX_MAP *p_input,	// pointer to input features
	int N,				// total number of features(pixels) present in the input
	ACT_TYPE_E act_type,// type of activation. ReLU, sigmoid, tanh
	FIX_MAP *p_output	// pointer to output features. input size = output size
	) {

	int pixel;
	STATUS_E ret_status = FAILED;
    switch(act_type) {
        // Sigmoidal activation
        case SIGMOID:
			REL_INFO("This activation function is not supported in Fixed point\n");
			ret_status = UNSUPPORTED_FEATURE;
            break;
        // Hyperbolic tan activation
        case TANH:
			REL_INFO("This activation function is not supported in Fixed point\n");
			ret_status = UNSUPPORTED_FEATURE;
            break;
        // Rectified linear activation
        case RELU:
        	// TODO: see if DSPLIB provides any intrinsic based implementation.
            for (pixel = 0; pixel < N; pixel++) {
            	// TODO: see if there are optimized way for this in DSP lib
                if(p_output[pixel] < 0) {
                	p_output[pixel] = 0;
                }
            }
            ret_status = SUCCESS;
            break;
        default:
            REL_INFO("Invalid OR Unsupported activation type\n");
            break;
    }
	return ret_status;
}


STATUS_E dsp_flt_activation_layer(FLT_MAP *p_input,	// pointer to input features
	int N,				// total number of features(pixels) present in the input
	ACT_TYPE_E act_type,// type of activation. ReLU, sigmoid, tanh
	FLT_MAP *p_output	// pointer to output features. input size = output size
	) {

	int pixel;
	STATUS_E ret_status = FAILED;

	switch(act_type) {
		// Sigmoidal activation
		case SIGMOID:
			for (pixel = 0; pixel < N; pixel++) {
				p_output[pixel] = 1 / (1 + exp(-p_input[pixel]));
			}
			ret_status = SUCCESS;
			break;
		// Hyperbolic tan activation
		case TANH:
			for (pixel = 0; pixel < N; pixel++) {
				p_output[pixel] = tanh(p_input[pixel]);
			}
			ret_status = SUCCESS;
			break;
		// Rectified linear activation
		case RELU:
			for (pixel = 0; pixel < N; pixel++) {
            	// TODO: see if there are optimized way for this in DSP lib
                if(p_output[pixel] < 0) {
                	p_output[pixel] = 0;
                }
			}
			ret_status = SUCCESS;
			break;
		default:
			REL_INFO("Invalid OR Unsupported activation type\n");
			break;
	}
	return ret_status;
}


STATUS_E dsp_activation_layer(ACT_LYR_CTX_T *p_act_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_maps) {
	int N;
	unsigned int buff_offset;
	STATUS_E ret_status = FAILED;

	N = p_act_ctx->act_info.map_h * p_act_ctx->act_info.map_w * p_act_ctx->no_maps[core_id];
	buff_offset = core_id * p_act_ctx->act_info.map_h * p_act_ctx->act_info.map_w * p_act_ctx->start_map[core_id];
    switch(p_act_ctx->lyr_arith_mode) {
        case FLOAT_POINT:
        	ret_status = dsp_flt_activation_layer(p_flt_in_maps + buff_offset,
        		N,
				p_act_ctx->act_info.act_type,
				p_act_ctx->p_flt_output + buff_offset);
        	break;
        case FIXED_POINT:
        	ret_status = dsp_fix_activation_layer(p_fix_in_maps + buff_offset,
        		N,
				p_act_ctx->act_info.act_type,
				p_act_ctx->p_fix_output + buff_offset);
            break;
        default:
            break;
    }
    return ret_status;
}
