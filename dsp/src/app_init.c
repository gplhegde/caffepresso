#include "app_init.h"
#include "debug_control.h"
#include "misc_utils.h"
#include "mem_manager.h"
#include <string.h>

/*Pointers to shared data buffers allocated on MSMC RAM. These 2 buffers are used alternately buy
 *	all layers. Different cores may access these buffers at different offsets depending upon the type of
 *	layers of CNN are being computed. The access to buffer across different data dependent layers is
 *	synchronised using HW semaphores.
 *
 */
uint32_t *p_shared_dbuff1;
uint32_t *p_shared_dbuff2;


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

// returns total output buffer space(no of elements, not bytes) required buy this layer
unsigned long caffe_cnn_layer_malloc(void *p_lyr_ctx, CNN_LAYER_TYPE_E lyr_type) {
	int o_h, o_w;
	unsigned long no_elements = 0;
	switch(lyr_type) {
		case CONV:
		{
			CONV_LYR_CTX_T *p_conv_ctx = (CONV_LYR_CTX_T *)p_lyr_ctx;
			o_h = (p_conv_ctx->conv_info.map_h + 2*p_conv_ctx->conv_info.pad -
				p_conv_ctx->conv_info.ker_size + 1 + p_conv_ctx->conv_info.stride - 1) / p_conv_ctx->conv_info.stride;
			o_w = (p_conv_ctx->conv_info.map_w + 2*p_conv_ctx->conv_info.pad -
				p_conv_ctx->conv_info.ker_size + 1 + p_conv_ctx->conv_info.stride - 1) / p_conv_ctx->conv_info.stride;

			if ((NULL == (p_conv_ctx->p_fix_bias = (FIX_KER *)shared_malloc(p_conv_ctx->conv_info.no_outputs * sizeof (FIX_KER)))) ||
				(NULL == (p_conv_ctx->p_fix_ker = (FIX_KER *)shared_malloc(p_conv_ctx->conv_info.no_inputs *
				p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.no_outputs * sizeof (FIX_KER))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			no_elements = p_conv_ctx->conv_info.no_outputs * o_h * o_w;
			break;
		}

		case POOL:
		{
			POOL_LYR_CTX_T *p_pool_ctx = (POOL_LYR_CTX_T *)p_lyr_ctx;
			o_h = (p_pool_ctx->pool_info.map_h + 2*p_pool_ctx->pool_info.pad -
				p_pool_ctx->pool_info.win_size + 1 + p_pool_ctx->pool_info.stride - 1) / p_pool_ctx->pool_info.stride;
			o_w = (p_pool_ctx->pool_info.map_w + 2*p_pool_ctx->pool_info.pad -
				p_pool_ctx->pool_info.win_size + 1 + p_pool_ctx->pool_info.stride - 1) / p_pool_ctx->pool_info.stride;
			no_elements = p_pool_ctx->pool_info.no_inputs * o_h * o_w;
			break;
		}

		case ACT:	
		{
			ACT_LYR_CTX_T *p_act_ctx = (ACT_LYR_CTX_T *)p_lyr_ctx;
			no_elements = p_act_ctx->act_info.map_h * p_act_ctx->act_info.map_w *p_act_ctx->act_info.no_outputs;
			break;
		}

		case INNER_PROD:
		{
			IP_LYR_CTX_T *p_ip_ctx = (IP_LYR_CTX_T *)p_lyr_ctx;
			// FIXME: replace malloc with custom malloc to allocate memory in on-chip memory.
			if ((NULL == (p_ip_ctx->p_fix_weight = (FIX_KER *)ext_malloc(p_ip_ctx->ip_info.no_outputs *
				p_ip_ctx->ip_info.no_inputs * sizeof(FIX_KER)))) ||
				(NULL == (p_ip_ctx->p_fix_bias = (FIX_KER *)ext_malloc(p_ip_ctx->ip_info.no_outputs * sizeof(FIX_KER))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			no_elements = p_ip_ctx->ip_info.map_h * p_ip_ctx->ip_info.map_w * p_ip_ctx->ip_info.no_outputs;
			break;
		}
		case SOFTMAX:
		{
			SMAX_LYR_CTX_T *p_smax_ctx = (SMAX_LYR_CTX_T *)p_lyr_ctx;
			no_elements = p_smax_ctx->no_inputs;
			break;
		}
		default:
			REL_INFO("Unsupported layer\n");
			break;
	}
	return no_elements;
}

STATUS_E caffe_cnn_layer_mem_free(void *p_lyr_ctx, CNN_LAYER_TYPE_E lyrType) {

	switch(lyrType) {
		case CONV:
		{
			CONV_LYR_CTX_T *p_conv_ctx = (CONV_LYR_CTX_T *)p_lyr_ctx;
			// FIXME: use custom free()
			shared_free(p_conv_ctx->p_fix_bias);
			shared_free(p_conv_ctx->p_fix_ker);
			break;
		}
		case INNER_PROD:
		{
			IP_LYR_CTX_T *p_ip_ctx = (IP_LYR_CTX_T *)p_lyr_ctx;
			// FIXME: use custom free()
			ext_free(p_ip_ctx->p_fix_weight);
			ext_free(p_ip_ctx->p_fix_bias);
			break;
		}
		default:
			break;
	}
	return SUCCESS;
}

STATUS_E cnn_app_malloc(CNN_LYR_NODE_T *p_lyr_nodes, int n_layers) {


	unsigned long max_buff_elements, buff_elements;
	max_buff_elements = 0;
	int lyr;
	CNN_LYR_NODE_T *p_node = p_lyr_nodes;

	for (lyr = 0; lyr < n_layers; lyr++) {
		printf("Layer = %d\n", lyr);
		buff_elements = caffe_cnn_layer_malloc(p_node->p_lyr_ctx, p_node->lyr_type);
		if(max_buff_elements < buff_elements) {
			max_buff_elements = buff_elements;
		}
		p_node++;
	}

	DBG_INFO("Max buffer size requirement = %d elements\n", max_buff_elements);
	// shared ping-pong buffer allocation on MSMC RAM
	DBG_INFO("Allocating 2 buffers with max buffer size requirement\n");
	p_shared_dbuff1 = (uint32_t*)shared_malloc(sizeof(FLT_MAP) * max_buff_elements);
	p_shared_dbuff2 = (uint32_t*)shared_malloc(sizeof(FLT_MAP) * max_buff_elements);
	return SUCCESS;
}

STATUS_E cnn_app_memfree() {
	
	int lyr;
	CNN_LYR_NODE_T *p_node = g_cnn_layer_nodes;

	for (lyr = 0; lyr < NO_DEEP_LAYERS; lyr++) {
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
	return SUCCESS;
}

STATUS_E main_cnn_app_init() {
	STATUS_E status = SUCCESS;

	caffe_layer_ctx_init();


	cnn_layer_internal_param_init();



	cnn_app_malloc(g_cnn_layer_nodes, NO_DEEP_LAYERS);


	cnn_app_model_init(g_cnn_layer_nodes, NO_DEEP_LAYERS);


	// split the computation of maps across all cores.
	workload_sharing_config(g_cnn_layer_nodes, NO_DEEP_LAYERS);
	return status;
}
