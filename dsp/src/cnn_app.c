#include "app_init.h"
#include "caffe_frontend.h"
#include "debug_control.h"
#include "app_profile.h"
#include "misc_utils.h"
#include "data_sync.h"

#include <ti/csl/csl_semAux.h>
#ifdef CNN_SIMULATOR
#include "sim_image.h"
#endif // CNN_SIMULATOR

extern uint32_t *p_shared_dbuff1;
extern unsigned int core_id;


STATUS_E main_cnn_app(uint8_t *p_image, int *p_label) {
	uint32_t nn_lyr;

	CNN_LAYER_TYPE_E lyr_type;

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
	static uint32_t image_cnt = 0;

	// Use first shared buffer for the input layer.
	p_fix_input = (FIX_MAP *)p_shared_dbuff1;
	p_float_input = (FLT_MAP *)p_shared_dbuff1;

	prev_map_h = INPUT_IMG_HEIGHT;
	prev_map_w = INPUT_IMG_WIDTH;
	prev_nmaps = NO_INPUT_MAPS;
	p_conv_ctx = (CONV_LYR_CTX_T *)g_cnn_layer_nodes[0].p_lyr_ctx;

	if(core_id != MASTER_CORE_ID) {
		// other cores wait for the image buffer to be initialized by the master
		wait_for_image_init(image_cnt);
	} else {
		// mean and contrast normalization
		//mean_normalize(p_image, prev_map_h * prev_nmaps, prev_map_w, &var, p_float_input);
		//float_to_fix_data(p_float_input, prev_map_h * prev_nmaps * prev_map_w, p_conv_ctx->conv_info.no_map_frac_bits, p_fix_input);
		toggle_image_init_flag(image_cnt);
	}

	prev_arith_mode = FLOAT_POINT;
	
	nn_lyr = 0;
	lyr_type = g_cnn_layer_nodes[nn_lyr].lyr_type;

	// main processing loop
	while(nn_lyr < NO_DEEP_LAYERS) {

		switch(lyr_type) {
			case CONV:
				p_conv_ctx = (CONV_LYR_CTX_T *)g_cnn_layer_nodes[nn_lyr].p_lyr_ctx;

				// compute this layer's output
				dsp_conv_layer(p_conv_ctx, p_float_input, p_fix_input);

				// Assign this output to next layer's input
				p_fix_input = p_conv_ctx->p_fix_output;
				p_float_input = p_conv_ctx->p_flt_output;
				prev_arith_mode = p_conv_ctx->lyr_arith_mode;

				break;
			case POOL:
				DBG_INFO("pool layer start\n");
				p_pool_ctx = (POOL_LYR_CTX_T *)g_cnn_layer_nodes[nn_lyr].p_lyr_ctx;

				dsp_pool_layer(p_pool_ctx, p_float_input, p_fix_input);

				p_fix_input = p_pool_ctx->p_fix_output;
				p_float_input = p_pool_ctx->p_flt_output;
				prev_arith_mode = p_pool_ctx->lyr_arith_mode;

				break;
			case ACT:
				p_act_ctx = (ACT_LYR_CTX_T *)g_cnn_layer_nodes[nn_lyr].p_lyr_ctx;

				dsp_activation_layer(p_act_ctx, p_float_input, p_fix_input);

				p_fix_input = p_act_ctx->p_fix_output;
				p_float_input = p_act_ctx->p_flt_output;
				prev_arith_mode = p_act_ctx->lyr_arith_mode;
				break;
			case INNER_PROD:
				p_ip_ctx = (IP_LYR_CTX_T *)g_cnn_layer_nodes[nn_lyr].p_lyr_ctx;

				dsp_ip_layer(p_ip_ctx, p_float_input, p_fix_input);
				p_fix_input = p_ip_ctx->p_fix_output;
				p_float_input = p_ip_ctx->p_flt_output;
				prev_arith_mode = p_ip_ctx->lyr_arith_mode;
				break;
			case SOFTMAX:
				p_smax_ctx = (SMAX_LYR_CTX_T *)g_cnn_layer_nodes[nn_lyr].p_lyr_ctx;

				// only master core performs final softmax layer.
				if(core_id == MASTER_CORE_ID) {
					dsp_smax_layer(p_smax_ctx, p_float_input);
				}
				p_float_input = p_smax_ctx->p_float_output;
				break;
			case DATA_CONVERSION:

				break;
			default:
				REL_INFO("Unsupported layer\n");
				return UNSUPPORTED_FEATURE;
		}

		signal_lyr_completion(nn_lyr);

		// data synchronization btw layers
		wait_for_maps(nn_lyr);

		// reset the other counter
		if(core_id == MASTER_CORE_ID) {
			// NOTE: It is safe to assume that the next layer will take significant time compared to the counter reset time
			reset_layer_sync_cntr(nn_lyr + 1);
		}

		// data conversion is necessary
		if(prev_arith_mode != *((LYR_ARITH_MODE_E *)g_cnn_layer_nodes[nn_lyr + 1].p_lyr_ctx)) {
			REL_INFO("Data conversion in between layers is not supported as of now\n");
			return UNSUPPORTED_FEATURE;
		} else {
			nn_lyr++;
			lyr_type = g_cnn_layer_nodes[nn_lyr].lyr_type;
		}
	}

	// reset the image init flag for this image
	if(core_id == MASTER_CORE_ID) {
		// TODO: take last layer output and find the max probability/label

		toggle_image_init_flag(image_cnt);
	}
	image_cnt++;
	return status;
}
