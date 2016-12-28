#include "caffe_frontend.h"
#include "cnn_layers.h"
#include "debug_control.h"
#include "network_model.h"
#include "caffe_proto_params.h"
#include "mem_manager.h"

extern uint32_t *p_shared_dbuff1;
extern uint32_t *p_shared_dbuff2;
// Array of CNN nodes. Each node will contain layer type and a pointer to context of the corresponding layer.
// Put this list of pointers on local_memory. The actual layer specific context is allocated on shared memory which
// is initialized by Master core.
#pragma DATA_ALIGN   (g_cnn_layer_nodes, L1_CACHE_LINE_SIZE)
#pragma DATA_SECTION(g_cnn_layer_nodes, ".shared_ocm")
CNN_LYR_NODE_T g_cnn_layer_nodes[NO_DEEP_LAYERS];


void caffe_layer_ctx_init() {
	int lyr, m_h, m_w, n_maps;
	CONV_LYR_CTX_T *p_conv_ctx;
	POOL_LYR_CTX_T *p_pool_ctx;
	ACT_LYR_CTX_T *p_act_ctx;
	IP_LYR_CTX_T * p_ip_ctx;
	SMAX_LYR_CTX_T *p_smax_ctx;
	BNORM_LYR_CTX_T *p_bnorm_ctx;

	for (lyr = 0; lyr < NO_DEEP_LAYERS; lyr++) {
		// First layer to be provided with input dimensions. Subsequent layer input dimensions are
		// derived from this
		if(lyr == 0) {
			n_maps = NO_INPUT_MAPS;
			m_w = INPUT_IMG_WIDTH;
			m_h = INPUT_IMG_HEIGHT;
		} else {
			switch(cnn_param_table[lyr-1].lyrType) {
				case CONV:
					p_conv_ctx = (CONV_LYR_CTX_T *)g_cnn_layer_nodes[lyr-1].p_lyr_ctx;
					m_h = (p_conv_ctx->conv_info.map_h + 2*p_conv_ctx->conv_info.pad -
						p_conv_ctx->conv_info.ker_size + 1 + p_conv_ctx->conv_info.stride - 1) / p_conv_ctx->conv_info.stride;
					m_w = (p_conv_ctx->conv_info.map_w + 2*p_conv_ctx->conv_info.pad -
						p_conv_ctx->conv_info.ker_size + 1 + p_conv_ctx->conv_info.stride - 1) / p_conv_ctx->conv_info.stride;
					n_maps = p_conv_ctx->conv_info.no_outputs;
					break;
				case POOL:
					p_pool_ctx = (POOL_LYR_CTX_T *) g_cnn_layer_nodes[lyr-1].p_lyr_ctx;
					m_h = (p_pool_ctx->pool_info.map_h + 2*p_pool_ctx->pool_info.pad -
						p_pool_ctx->pool_info.win_size + 1 + p_pool_ctx->pool_info.stride - 1) / p_pool_ctx->pool_info.stride;
					m_w = (p_pool_ctx->pool_info.map_w + 2*p_pool_ctx->pool_info.pad -
						p_pool_ctx->pool_info.win_size + 1 + p_pool_ctx->pool_info.stride - 1) / p_pool_ctx->pool_info.stride;
					n_maps = p_pool_ctx->pool_info.no_outputs;
					break;	
				case ACT:
					p_act_ctx = (ACT_LYR_CTX_T *)g_cnn_layer_nodes[lyr-1].p_lyr_ctx;
					m_h = p_act_ctx->act_info.map_h;
					m_w = p_act_ctx->act_info.map_w;
					n_maps = p_act_ctx->act_info.no_outputs;
					break;
				case BATCH_NORM:
					p_bnorm_ctx = (BNORM_LYR_CTX_T *)g_cnn_layer_nodes[lyr-1].p_lyr_ctx;
					m_h = p_bnorm_ctx->bnorm_info.map_h;
					m_w = p_bnorm_ctx->bnorm_info.map_w;
					n_maps = p_bnorm_ctx->bnorm_info.no_inputs;
					break;
				case INNER_PROD:
					p_ip_ctx = (IP_LYR_CTX_T *)g_cnn_layer_nodes[lyr-1].p_lyr_ctx;
					m_h = p_ip_ctx->ip_info.map_h;
					m_w = p_ip_ctx->ip_info.map_w;
					n_maps = p_ip_ctx->ip_info.no_outputs;
					break;
				case SOFTMAX:
					// Softmax will be the last layer. Hence no need of this
					p_smax_ctx = (SMAX_LYR_CTX_T *)g_cnn_layer_nodes[lyr-1].p_lyr_ctx;
					m_h = 1;
					m_w = p_smax_ctx->no_inputs;
					n_maps = 1;
					break;
				default:
					REL_INFO("Unsupported CNN layer\n");
			}
		}
		g_cnn_layer_nodes[lyr].lyr_type = cnn_param_table[lyr].lyrType;
		switch(cnn_param_table[lyr].lyrType) {

			case CONV:
				g_cnn_layer_nodes[lyr].p_lyr_ctx = (CONV_LYR_CTX_T *)shared_malloc(sizeof(CONV_LYR_CTX_T));
				p_conv_ctx = (CONV_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				
				p_conv_ctx->conv_info = (CONV_INFO_T){.map_h = m_h,
					.map_w = m_w,
					.ker_size = cnn_param_table[lyr].K,
					.stride = cnn_param_table[lyr].stride,
					.pad = cnn_param_table[lyr].pad,
					.no_inputs = n_maps,
					.no_outputs = cnn_param_table[lyr].nOutMaps,
					.stride = cnn_param_table[lyr].stride,
					.pad = cnn_param_table[lyr].pad};
				break;
			case POOL:
				g_cnn_layer_nodes[lyr].p_lyr_ctx = (POOL_LYR_CTX_T *)shared_malloc(sizeof(POOL_LYR_CTX_T));
				p_pool_ctx = (POOL_LYR_CTX_T *) g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_pool_ctx->pool_info = (POOL_INFO_T) {.map_h = m_h,
					.map_w = m_w,
					.no_inputs = n_maps,
					.no_outputs = n_maps,
					.win_size = cnn_param_table[lyr].winSize,
					.stride = cnn_param_table[lyr].stride,
					.pad = cnn_param_table[lyr].pad,
					.pool_type = cnn_param_table[lyr].poolType};
				break;
			case ACT:
				g_cnn_layer_nodes[lyr].p_lyr_ctx = (ACT_LYR_CTX_T *)shared_malloc(sizeof(ACT_LYR_CTX_T));
				p_act_ctx = (ACT_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_act_ctx->act_info = (ACT_INFO_T) {.map_h = m_h,
					.map_w = m_w,
					.no_inputs = n_maps,
					.no_outputs = n_maps,
					.act_type = cnn_param_table[lyr].actType};
				break;
			case BATCH_NORM:
				g_cnn_layer_nodes[lyr].p_lyr_ctx = (BNORM_LYR_CTX_T *)shared_malloc(sizeof(BNORM_LYR_CTX_T));
				p_bnorm_ctx = (BNORM_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_bnorm_ctx->bnorm_info = (BNORM_INFO_T) {.map_h = m_h,
					.map_w = m_w,
					.no_inputs = n_maps,
					.no_outputs = n_maps
				};
				break;
			case INNER_PROD:
				g_cnn_layer_nodes[lyr].p_lyr_ctx = (IP_LYR_CTX_T *)shared_malloc(sizeof(IP_LYR_CTX_T));
				p_ip_ctx = (IP_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_ip_ctx->ip_info = (IP_INFO_T) {.map_h = 1,
					.map_w = 1,
					.no_inputs = m_h * m_w * n_maps,
					.no_outputs = cnn_param_table[lyr].nOutputs};
				break;
			case SOFTMAX:
				g_cnn_layer_nodes[lyr].p_lyr_ctx = (SMAX_LYR_CTX_T *)shared_malloc(sizeof(SMAX_LYR_CTX_T));
				p_smax_ctx = (SMAX_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_smax_ctx->no_inputs = m_h * m_w * n_maps;
				break;
			default:
				REL_INFO("Unsupported layer\n");
		}
	}
}

// Internal parameter initialization apart from caffe prototxt config parameters.
unsigned long  cnn_layer_internal_param_init(void) {
	int lyr, conv_lyr, ip_lyr, bnorm_lyr;
	CONV_LYR_CTX_T *p_conv_ctx;
	POOL_LYR_CTX_T *p_pool_ctx;
	ACT_LYR_CTX_T *p_act_ctx;
	IP_LYR_CTX_T * p_ip_ctx;
	SMAX_LYR_CTX_T *p_smax_ctx;
	BNORM_LYR_CTX_T *p_bnorm_ctx;
	LYR_ARITH_MODE_E common_arith_mode;
	int common_frac_bits, ker_frac_bits, map_frac_bits, o_h, o_w;
	unsigned long buff_req, max_buff_req;
	conv_lyr = 0;
	max_buff_req = 0;
	buff_req = 0;
	ip_lyr = 0;
	bnorm_lyr = 0;
	if(APP_ARITHMETIC_MODE) {
		common_arith_mode = FIXED_POINT;
	} else {
		common_arith_mode = FLOAT_POINT;
	}

	common_frac_bits = 12;
	ker_frac_bits = 15;
	map_frac_bits = 8;
	for( lyr = 0; lyr < NO_DEEP_LAYERS; lyr++) {
		switch(g_cnn_layer_nodes[lyr].lyr_type) {

			case CONV:
				p_conv_ctx = (CONV_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				// TODO: These should come from user after analyzing the dynamic range of the weights and activations after training.
				p_conv_ctx->conv_info.no_ker_frac_bits = ker_frac_bits;
				p_conv_ctx->conv_info.no_map_frac_bits = map_frac_bits;
				p_conv_ctx->lyr_arith_mode = common_arith_mode;
				o_h = (p_conv_ctx->conv_info.map_h + 2*p_conv_ctx->conv_info.pad -
					p_conv_ctx->conv_info.ker_size + 1 + p_conv_ctx->conv_info.stride - 1) / p_conv_ctx->conv_info.stride;
				o_w = (p_conv_ctx->conv_info.map_w + 2*p_conv_ctx->conv_info.pad -
					p_conv_ctx->conv_info.ker_size + 1 + p_conv_ctx->conv_info.stride - 1) / p_conv_ctx->conv_info.stride;
				buff_req = p_conv_ctx->conv_info.no_outputs * o_h * o_w;
#ifndef USE_RANDOM_MODEL
				// Pointer to conv weights and biases, taken from the big network model arrays.
				p_conv_ctx->p_flt_ker = conv_w_ptrs[conv_lyr];
				p_conv_ctx->p_flt_bias = conv_b_ptrs[conv_lyr];
#endif // USE_RANDOM_MODEL
				conv_lyr++;
				break;
			case POOL:
				p_pool_ctx = (POOL_LYR_CTX_T *) g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_pool_ctx->lyr_arith_mode = common_arith_mode;
				o_h = (p_pool_ctx->pool_info.map_h + 2*p_pool_ctx->pool_info.pad -
					p_pool_ctx->pool_info.win_size + 1 + p_pool_ctx->pool_info.stride - 1) / p_pool_ctx->pool_info.stride;
				o_w = (p_pool_ctx->pool_info.map_w + 2*p_pool_ctx->pool_info.pad -
					p_pool_ctx->pool_info.win_size + 1 + p_pool_ctx->pool_info.stride - 1) / p_pool_ctx->pool_info.stride;
				buff_req = p_pool_ctx->pool_info.no_inputs * o_h * o_w;
				break;
			case ACT:
				p_act_ctx = (ACT_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_act_ctx->lyr_arith_mode = common_arith_mode;
				buff_req = p_act_ctx->act_info.map_h * p_act_ctx->act_info.map_w *p_act_ctx->act_info.no_outputs;
				break;
			case BATCH_NORM:
				p_bnorm_ctx = (BNORM_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_bnorm_ctx->lyr_arith_mode = common_arith_mode;
#ifndef USE_RANDOM_MODEL
				//p_bnorm_ctx->p_flt_scale = bnorm_scale_ptrs[bnorm_lyr];
				//p_bnorm_ctx->p_flt_offset = bnorm_offset_ptrs[bnorm_lyr];
#endif
				p_bnorm_ctx->bnorm_info.no_ker_frac_bits = ker_frac_bits;
				p_bnorm_ctx->bnorm_info.no_map_frac_bits = map_frac_bits;
				buff_req = p_bnorm_ctx->bnorm_info.map_h * p_bnorm_ctx->bnorm_info.map_w * p_bnorm_ctx->bnorm_info.no_outputs;
				bnorm_lyr++;
				break;
			case INNER_PROD:
				p_ip_ctx = (IP_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				// TODO: These should come from user after analyzing the dynamic range of the weights and activations after training.
				p_ip_ctx->ip_info.no_ker_frac_bits = ker_frac_bits;
				p_ip_ctx->ip_info.no_map_frac_bits = map_frac_bits;
				p_ip_ctx->lyr_arith_mode = common_arith_mode;
				buff_req = p_ip_ctx->ip_info.map_h * p_ip_ctx->ip_info.map_w * p_ip_ctx->ip_info.no_outputs;
#ifndef USE_RANDOM_MODEL
				// Pointer to FC layer weights and biases, taken from the big network model arrays.
				p_ip_ctx->p_flt_weight = ip_w_ptrs[ip_lyr];
				p_ip_ctx->p_flt_bias = ip_b_ptrs[ip_lyr];
#endif // USE_RANDOM_MODEL
				ip_lyr++;
				break;
			case SOFTMAX:
				p_smax_ctx = (SMAX_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_smax_ctx->lyr_arith_mode = FLOAT_POINT; // No fixed point mode supported for SOFTMAX as of now.
				buff_req = p_smax_ctx->no_inputs;
				break;
			default:
				REL_INFO("Invalid layer type");
		}
		if(buff_req > max_buff_req) {
			max_buff_req = buff_req;
		}
	}
	return max_buff_req;
}


