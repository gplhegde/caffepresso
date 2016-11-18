#include "app_init.h"
#include "debug_control.h"
#include "misc_utils.h"
#include <string.h>

STATUS_E init_conv_kernels(CONV_LYR_CTX_T *p_conv_ctx) {

	int k, i;

	// convert kernels to fix point
	for ( i = 0; i < p_conv_ctx->conv_info.no_inputs; i++) {
		for (k = 0; k < p_conv_ctx->conv_info.no_outputs; k++) {
			float_to_fix_data(p_conv_ctx->p_flt_ker + (i * p_conv_ctx->conv_info.no_outputs + k) * p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size,
				p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size,
				p_conv_ctx->conv_info.no_ker_frac_bits,
				p_conv_ctx->p_fix_ker + (i * p_conv_ctx->conv_info.no_outputs + k) * p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size);
		}
	}
	// Init bias of conv layer.
	float_to_fix_data(p_conv_ctx->p_flt_bias,
		p_conv_ctx->conv_info.no_outputs,
		p_conv_ctx->conv_info.no_ker_frac_bits,
		p_conv_ctx->p_fix_bias);
	return SUCCESS;
}

STATUS_E init_ip_layer_params(IP_LYR_CTX_T *p_ip_ctx) {
	
	return SUCCESS;
}

STATUS_E caffe_cnn_layer_malloc(void *p_lyr_ctx, CNN_LAYER_TYPE_E lyr_type) {

	switch(lyr_type) {
		case CONV:
		{
			CONV_LYR_CTX_T *p_conv_ctx = (CONV_LYR_CTX_T *)p_lyr_ctx;
			// FIXME: replace malloc with custom malloc to allocate memory in on-chip memory.
			if ((NULL == (p_conv_ctx->p_fix_bias = (FIX_KER *)malloc(p_conv_ctx->conv_info.no_outputs * sizeof (FIX_KER)))) ||
				(NULL == (p_conv_ctx->p_fix_ker = (FIX_KER *)malloc(p_conv_ctx->conv_info.no_inputs *
				p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.no_outputs * sizeof (FIX_KER))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			// TODO: assign output buffer for the core with proper offset
			p_conv_ctx->p_flt_output = NULL;
			p_conv_ctx->p_fix_output = NULL;
			break;
		}

		case POOL:
		{
			POOL_LYR_CTX_T *p_pool_ctx = (POOL_LYR_CTX_T *)p_lyr_ctx;
			// TODO: assign output buffer for the core with proper offset
			p_pool_ctx->p_fix_output = NULL;
			p_pool_ctx->p_flt_output = NULL;
			break;
		}

		case ACT:	
		{
			ACT_LYR_CTX_T *p_act_ctx = (ACT_LYR_CTX_T *)p_lyr_ctx;
			// TODO: assign output buffer for the core with proper offset
			p_act_ctx->p_fix_output = NULL;
			p_act_ctx->p_flt_output = NULL;
			break;
		}

		case INNER_PROD:
		{
			IP_LYR_CTX_T *p_ip_ctx = (IP_LYR_CTX_T *)p_lyr_ctx;
			// FIXME: replace malloc with custom malloc to allocate memory in on-chip memory.
			if ((NULL == (p_ip_ctx->p_fix_weight = (FIX_KER *)malloc(p_ip_ctx->ip_info.no_outputs *
				p_ip_ctx->ip_info.no_inputs * sizeof(FIX_KER)))) ||
				(NULL == (p_ip_ctx->p_fix_bias = (FIX_KER *)malloc(p_ip_ctx->ip_info.no_outputs * sizeof(FIX_KER))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			p_ip_ctx->p_fix_output = NULL;
			p_ip_ctx->p_flt_output = NULL;
			break;
		}
		case SOFTMAX:
		{
			SMAX_LYR_CTX_T *p_smax_ctx = (SMAX_LYR_CTX_T *)p_lyr_ctx;
			p_smax_ctx->p_float_output = NULL;
			break;
		}
		default:
			REL_INFO("Unsupported layer\n");
			break;
	}
	return SUCCESS;
}

STATUS_E caffe_cnn_layer_mem_free(void *p_lyr_ctx, CNN_LAYER_TYPE_E lyrType) {

	switch(lyrType) {
		case CONV:
		{
			CONV_LYR_CTX_T *p_conv_ctx = (CONV_LYR_CTX_T *)p_lyr_ctx;
			// FIXME: use custom free()
			free(p_conv_ctx->p_fix_bias);
			free(p_conv_ctx->p_fix_ker);
			break;
		}
		case INNER_PROD:
		{
			IP_LYR_CTX_T *p_ip_ctx = (IP_LYR_CTX_T *)p_lyr_ctx;
			// FIXME: use custom free()
			free(p_ip_ctx->p_fix_weight);
			free(p_ip_ctx->p_fix_bias);
			break;
		}
		default:
			break;
	}
	return SUCCESS;
}

STATUS_E cnn_app_malloc(CNN_LYR_NODE_T *p_lyr_nodes, int n_layers) {

	int lyr;
	CNN_LYR_NODE_T *p_node = p_lyr_nodes;

	for (lyr = 0; lyr < n_layers; lyr++) {
		printf("Layer = %d\n", lyr);
		caffe_cnn_layer_malloc(p_node->p_lyr_ctx, p_node->lyr_type);
		p_node++;
	}
	return SUCCESS;
}

STATUS_E cnn_app_memfree(CNN_LYR_NODE_T *p_lyr_nodes, int n_layers) {
	
	int lyr;
	CNN_LYR_NODE_T *p_node = p_lyr_nodes;

	for (lyr = 0; lyr < n_layers; lyr++) {
		caffe_cnn_layer_mem_free(p_node->p_lyr_ctx, p_node->lyr_type);
		p_node++;
	}
	return SUCCESS;
}

STATUS_E cnn_app_model_init(CNN_LYR_NODE_T *p_lyr_nodes, int n_layers) {

	int lyr;
	IP_LYR_CTX_T *p_ip_ctx;
	CONV_LYR_CTX_T *p_conv_ctx;
	CNN_LYR_NODE_T *p_node = p_lyr_nodes;

	REL_INFO("Initializing CNN model weights and biases\n");
	for (lyr = 0; lyr < n_layers; lyr++) {
		REL_INFO("Layer %d parameters are being initialized\n", lyr);
		if(p_node->lyr_type == CONV) {
			p_conv_ctx = (CONV_LYR_CTX_T *)p_node->p_lyr_ctx;
			init_conv_kernels(p_conv_ctx);
		} else if(p_node->lyr_type == INNER_PROD) {
			p_ip_ctx = (IP_LYR_CTX_T *)p_node->p_lyr_ctx;
			init_ip_layer_params(p_ip_ctx);
		}
		p_node++;
	}
	return SUCCESS;
}

STATUS_E workload_sharing_config(CNN_LYR_NODE_T *p_lyr_nodes, int n_layers) {
	int lyr, core, quo, rem, map, o_h, o_w;
	IP_LYR_CTX_T *p_ip_ctx;
	CONV_LYR_CTX_T *p_conv_ctx;
	POOL_LYR_CTX_T *p_pool_ctx;
	ACT_LYR_CTX_T *p_act_ctx;
	CNN_LYR_NODE_T *p_node = p_lyr_nodes;

	for (lyr = 0; lyr < n_layers; lyr++) {

		switch(p_node->lyr_type) {
			case CONV:
				p_conv_ctx = (CONV_LYR_CTX_T *)p_node->p_lyr_ctx;
				quo = p_conv_ctx->conv_info.no_outputs / NO_CORES;
				rem = p_conv_ctx->conv_info.no_outputs % NO_CORES;
				map = 0;
				for(core = 0; core < NO_CORES; core++) {
					p_conv_ctx->start_map[core] = map;
					if(rem == 0) {
						p_conv_ctx->no_maps[core] = quo;
						map += quo;
					} else if(core < rem) {
						p_conv_ctx->no_maps[core] = quo + 1;
						map += (quo + 1);
					} else {
						p_conv_ctx->no_maps[core] = quo;
						map += quo;
					}
				}
				break;
			case POOL:
				p_pool_ctx = (POOL_LYR_CTX_T *)p_node->p_lyr_ctx;
				quo = p_pool_ctx->pool_info.no_outputs / NO_CORES;
				rem = p_pool_ctx->pool_info.no_outputs % NO_CORES;
				map = 0;
				for(core = 0; core < NO_CORES; core++) {
					p_pool_ctx->start_map[core] = map;
					if(rem == 0) {
						p_pool_ctx->no_maps[core] = quo;
						map += quo;
					} else if(core < rem) {
						p_pool_ctx->no_maps[core] = quo + 1;
						map += (quo + 1);
					} else {
						p_pool_ctx->no_maps[core] = quo;
						map += quo;
					}
				}
				break;
			case ACT:
				p_act_ctx = (ACT_LYR_CTX_T *)p_node->p_lyr_ctx;
				quo = p_act_ctx->act_info.no_outputs / NO_CORES;
				rem = p_act_ctx->act_info.no_outputs % NO_CORES;
				map = 0;
				for(core = 0; core < NO_CORES; core++) {
					p_act_ctx->start_map[core] = map;
					if(rem == 0) {
						p_act_ctx->no_maps[core] = quo;
						map += quo;
					} else if(core < rem) {
						p_act_ctx->no_maps[core] = quo + 1;
						map += (quo + 1);
					} else {
						p_act_ctx->no_maps[core] = quo;
						map += quo;
					}
				}
				break;
			case INNER_PROD:
				p_ip_ctx = (IP_LYR_CTX_T *)p_node->p_lyr_ctx;
				quo = p_ip_ctx->ip_info.no_outputs / NO_CORES;
				rem = p_ip_ctx->ip_info.no_outputs % NO_CORES;
				map = 0;
				for(core = 0; core < NO_CORES; core++) {
					p_ip_ctx->start_map[core] = map;
					if(rem == 0) {
						p_ip_ctx->no_maps[core] = quo;
						map += quo;
					} else if(core < rem) {
						p_ip_ctx->no_maps[core] = quo + 1;
						map += (quo + 1);
					} else {
						p_ip_ctx->no_maps[core] = quo;
						map += quo;
					}
				}
				break;
			default:
				break;
		}
		p_node++;
	}
}
