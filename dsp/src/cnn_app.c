#include "app_init.h"
#include "caffe_frontend.h"
#include "debug_control.h"
#include "app_profile.h"
#include "misc_utils.h"
#ifdef CNN_SIMULATOR
#include "sim_image.h"
#endif // CNN_SIMULATOR

STATUS_E main_cnn_app_init() {
	STATUS_E status = SUCCESS;
	
	caffe_layer_ctx_init();
	REL_INFO("Initialized context from the LUT\n");

	cnn_layer_internal_param_init();
	REL_INFO("Initialized app configurations and internal context params\n");

	cnn_app_malloc(g_cnn_layer_nodes, NO_DEEP_LAYERS);
	REL_INFO("Allocated buffers for all layers\n");

	cnn_app_model_init(g_cnn_layer_nodes, NO_DEEP_LAYERS);
	REL_INFO("Initialized model weights and biases of all layers\n");

	// split the computation of maps across all cores.
	workload_sharing_config(g_cnn_layer_nodes, NO_DEEP_LAYERS);
	return status;
}

STATUS_E main_cnn_app(uint8_t *p_image, int *p_label) {
	int lyr;
	CONV_LYR_CTX_T *p_conv_ctx;
	POOL_LYR_CTX_T *p_pool_ctx;
	ACT_LYR_CTX_T *p_act_ctx;
	IP_LYR_CTX_T * p_ip_ctx;
	SMAX_LYR_CTX_T *p_smax_ctx;
	FIX_MAP *p_fix_input;
	FLT_MAP *p_float_input;
	int prev_map_h, prev_map_w, prev_nmaps;
	float var;
	int prev_arith_mode, prev_frac_bits;
	uint64_t start_time;
	STATUS_E status = SUCCESS;


	// assign to ping pong buffer
	p_fix_input = NULL;
	p_float_input = NULL;

	prev_map_h = INPUT_IMG_HEIGHT;
	prev_map_w = INPUT_IMG_WIDTH;
	prev_nmaps = NO_INPUT_MAPS;

	// mean and contrast normalization
	mean_normalize(p_image, prev_map_h * prev_nmaps, prev_map_w, &var, p_float_input);

	p_conv_ctx = (CONV_LYR_CTX_T *)g_cnn_layer_nodes[0].p_lyr_ctx;
	float_to_fix_data(p_float_input, prev_map_h * prev_nmaps * prev_map_w, p_conv_ctx->conv_info.no_map_frac_bits, p_fix_input);

	prev_arith_mode = FLOAT_POINT;
	prev_frac_bits = p_conv_ctx->conv_info.no_map_frac_bits;
	
	// main processing loop
	GET_TIME(&start_time);
	for (lyr = 0; lyr < NO_DEEP_LAYERS; lyr++) {
		DBG_INFO("Computing layer %d outputs\n", lyr);
		switch(g_cnn_layer_nodes[lyr].lyr_type) {
			case CONV:
				DBG_INFO("conv layer start\n");
				p_conv_ctx = (CONV_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				if (p_conv_ctx->lyr_arith_mode != prev_arith_mode) {
					if (p_conv_ctx->lyr_arith_mode == FLOAT_POINT) {
						DBG_INFO("Converting fix to float\n");
						fix16_to_float_data(p_fix_input, prev_map_h * prev_nmaps * prev_map_w, prev_frac_bits, p_float_input);
					} else {
						DBG_INFO("Converting float to fix\n");
						float_to_fix_data(p_float_input,  prev_map_h * prev_nmaps * prev_map_w, prev_frac_bits, p_fix_input);
					}
				}
				// compute this layer's output
				//dsp_conv_layer(p_conv_ctx, p_float_input, p_fix_input);
				// Assign this output to next layer's input
				p_fix_input = p_conv_ctx->p_fix_output;
				p_float_input = p_conv_ctx->p_flt_output;
				prev_arith_mode = p_conv_ctx->lyr_arith_mode;
				prev_frac_bits = p_conv_ctx->conv_info.no_map_frac_bits;
				prev_map_h = (p_conv_ctx->conv_info.map_h + 2*p_conv_ctx->conv_info.pad - p_conv_ctx->conv_info.ker_size + 1 + p_conv_ctx->conv_info.stride - 1)/p_conv_ctx->conv_info.stride;
				prev_map_w = (p_conv_ctx->conv_info.map_w + 2*p_conv_ctx->conv_info.pad - p_conv_ctx->conv_info.ker_size + 1 + p_conv_ctx->conv_info.stride - 1)/p_conv_ctx->conv_info.stride;
				prev_nmaps = p_conv_ctx->conv_info.no_outputs;
				DBG_INFO("conv layer END\n");
				break;
			case POOL:
				DBG_INFO("pool layer start\n");
				p_pool_ctx = (POOL_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				if (p_pool_ctx->lyr_arith_mode != prev_arith_mode) {
					if (p_pool_ctx->lyr_arith_mode == FLOAT_POINT) {
						DBG_INFO("Converting fix to float\n");
						fix16_to_float_data(p_fix_input, prev_map_h * prev_nmaps * prev_map_w, prev_frac_bits, p_float_input);
					} else {
						DBG_INFO("Converting float to fix\n");
						float_to_fix_data(p_float_input,  prev_map_h * prev_nmaps * prev_map_w, prev_frac_bits, p_fix_input);
					}
				}
				//dsp_pool_layer(p_pool_ctx, p_float_input, p_fix_input);
				p_fix_input = p_pool_ctx->p_fix_output;
				p_float_input = p_pool_ctx->p_flt_output;
				prev_map_h = (p_pool_ctx->pool_info.map_h + 2*p_pool_ctx->pool_info.pad - p_pool_ctx->pool_info.win_size + 1 + p_pool_ctx->pool_info.stride - 1) / p_pool_ctx->pool_info.stride;
				prev_map_w = (p_pool_ctx->pool_info.map_w + 2*p_pool_ctx->pool_info.pad - p_pool_ctx->pool_info.win_size + 1 + p_pool_ctx->pool_info.stride - 1) / p_pool_ctx->pool_info.stride;
				prev_nmaps = p_pool_ctx->pool_info.no_outputs;
				prev_arith_mode = p_pool_ctx->lyr_arith_mode;
				DBG_INFO("pool layer END\n");
				break;
			case ACT:
				p_act_ctx = (ACT_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				if (p_act_ctx->lyr_arith_mode != prev_arith_mode) {
					if (p_act_ctx->lyr_arith_mode == FLOAT_POINT) {
						DBG_INFO("Converting fix to float\n");
						fix16_to_float_data(p_fix_input, prev_map_h * prev_nmaps * prev_map_w, prev_frac_bits, p_float_input);
					} else {
						DBG_INFO("Converting float to fix\n");
						float_to_fix_data(p_float_input,  prev_map_h * prev_nmaps * prev_map_w, prev_frac_bits, p_fix_input);
					}
				}
				dsp_activation_layer(p_act_ctx, p_float_input, p_fix_input);
				p_fix_input = p_act_ctx->p_fix_output;
				p_float_input = p_act_ctx->p_flt_output;
				prev_map_h = p_act_ctx->act_info.map_h;
				prev_map_w = p_act_ctx->act_info.map_w;
				prev_nmaps = p_act_ctx->act_info.no_outputs;
				prev_arith_mode = p_act_ctx->lyr_arith_mode;
				break;
			case INNER_PROD:
				p_ip_ctx = (IP_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				if (p_ip_ctx->lyr_arith_mode != prev_arith_mode) {
					if (p_ip_ctx->lyr_arith_mode == FLOAT_POINT) {
						DBG_INFO("Converting fix to float\n");
						fix16_to_float_data(p_fix_input, prev_map_h * prev_nmaps * prev_map_w, prev_frac_bits, p_float_input);
					} else {
						DBG_INFO("Converting float to fix\n");
						float_to_fix_data(p_float_input,  prev_map_h * prev_nmaps * prev_map_w, prev_frac_bits, p_fix_input);
					}
				}
				//dsp_inner_prod_layer(p_ip_ctx, p_float_input, p_fix_input);
				p_fix_input = p_ip_ctx->p_fix_output;
				p_float_input = p_ip_ctx->p_flt_output;
				prev_map_h = p_ip_ctx->ip_info.map_h;
				prev_map_w = p_ip_ctx->ip_info.map_w;
				prev_nmaps = p_ip_ctx->ip_info.no_outputs;
				prev_arith_mode = p_ip_ctx->lyr_arith_mode;
				break;
			case SOFTMAX:
				p_smax_ctx = (SMAX_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				//softmax_layer(p_smax_ctx, p_float_input, p_fix_input);
				p_float_input = p_smax_ctx->p_float_output;
				break;
			default:
				REL_INFO("Unsupported layer\n");
				return UNSUPPORTED_FEATURE;
		}
	}
	PRINT_RUNTIME("App main loop runtime", start_time);
	//DBG_MAPS(cvWaitKey(100000));
	// TODO: take last layer output and find the max probability/label
	REL_INFO("Relesaing buffers\n");
	cnn_app_memfree(g_cnn_layer_nodes, NO_DEEP_LAYERS);

	return status;
}
