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
#pragma DATA_SECTION(p_shared_dbuff1, ".shared_ocm")
uint32_t *p_shared_dbuff1;
#pragma DATA_SECTION(p_shared_dbuff2, ".shared_ocm")
uint32_t *p_shared_dbuff2;


STATUS_E init_conv_kernels(CONV_LYR_CTX_T *p_conv_ctx) {

	int omap, imap;
#ifdef USE_RANDOM_MODEL
	generate_random_data(p_conv_ctx->p_flt_ker,
		p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size *
		p_conv_ctx->conv_info.no_inputs * p_conv_ctx->conv_info.no_outputs,
		345);
	generate_random_data(p_conv_ctx->p_flt_bias,
		p_conv_ctx->conv_info.no_outputs,
		3456);
#endif // USE_RANDOM_MODEL

	// convert kernels to fix point
	for (omap = 0; omap < p_conv_ctx->conv_info.no_outputs; omap++) {
		for (imap = 0; imap < p_conv_ctx->conv_info.no_inputs; imap++) {
			float_to_fix_data(p_conv_ctx->p_flt_ker + (omap * p_conv_ctx->conv_info.no_inputs + imap) * p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size,
				p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size,
				p_conv_ctx->conv_info.no_ker_frac_bits,
				p_conv_ctx->p_fix_ker + (omap * p_conv_ctx->conv_info.no_inputs + imap) * p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size);
		}
	}
	// rotate the fixed point kernel to compensate for the 180 deg rotation performed by the IMGLIB APIs
	for(omap = 0; omap < p_conv_ctx->conv_info.no_outputs; omap++) {
		for(imap = 0; imap < p_conv_ctx->conv_info.no_inputs; imap++) {
			rotate_180(p_conv_ctx->p_fix_ker + (omap * p_conv_ctx->conv_info.no_inputs + imap) * p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size,
				p_conv_ctx->conv_info.ker_size, p_conv_ctx->conv_info.ker_size);
		}
	}
	// Init bias of conv layer.
	float_to_fix_data(p_conv_ctx->p_flt_bias,
		p_conv_ctx->conv_info.no_outputs,
		p_conv_ctx->conv_info.no_map_frac_bits,
		p_conv_ctx->p_fix_bias);
	return SUCCESS;
}

STATUS_E init_ip_layer_params(IP_LYR_CTX_T *p_ip_ctx) {
	int n;
#ifdef USE_RANDOM_MODEL
	generate_random_data(p_ip_ctx->p_flt_weight,
		p_ip_ctx->ip_info.map_h * p_ip_ctx->ip_info.map_w * p_ip_ctx->ip_info.no_inputs * p_ip_ctx->ip_info.no_outputs,
		p_ip_ctx->ip_info.no_outputs
		);
	generate_random_data(p_ip_ctx->p_flt_bias, p_ip_ctx->ip_info.no_outputs, 123);
#endif // USE_RANDOM_MODEL
	float_to_fix_data(p_ip_ctx->p_flt_weight,
		p_ip_ctx->ip_info.map_h * p_ip_ctx->ip_info.map_w * p_ip_ctx->ip_info.no_inputs * p_ip_ctx->ip_info.no_outputs,
		p_ip_ctx->ip_info.no_ker_frac_bits, p_ip_ctx->p_fix_weight);
	float_to_fix_data(p_ip_ctx->p_flt_bias, p_ip_ctx->ip_info.no_outputs, p_ip_ctx->ip_info.no_map_frac_bits, p_ip_ctx->p_fix_bias);
	return SUCCESS;
}

init_bnorm_layer_params(BNORM_LYR_CTX_T *p_bnorm_ctx) {

#ifdef USE_RANDOM_MODEL
	generate_random_data(p_bnorm_ctx->p_flt_scale, p_bnorm_ctx->bnorm_info.no_inputs, 123);
	generate_random_data(p_bnorm_ctx->p_flt_offset, p_bnorm_ctx->bnorm_info.no_inputs, 123);
#endif // USE_RANDOM_MODEL

	float_to_fix_data(p_bnorm_ctx->p_flt_offset, p_bnorm_ctx->bnorm_info.no_inputs, p_bnorm_ctx->bnorm_info.no_map_frac_bits, p_bnorm_ctx->p_fix_offset);
	float_to_fix_data(p_bnorm_ctx->p_flt_scale, p_bnorm_ctx->bnorm_info.no_inputs, p_bnorm_ctx->bnorm_info.no_ker_frac_bits, p_bnorm_ctx->p_fix_scale);
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
#ifdef USE_RANDOM_MODEL
			if ((NULL == (p_conv_ctx->p_flt_bias = (FLT_KER *)ext_malloc(p_conv_ctx->conv_info.no_outputs * sizeof (FLT_KER)))) ||
				(NULL == (p_conv_ctx->p_flt_ker = (FLT_KER *)ext_malloc(p_conv_ctx->conv_info.no_inputs *
				p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.no_outputs * sizeof (FLT_KER))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
#endif // USE_RANDOM_MODEL
			if ((NULL == (p_conv_ctx->p_fix_bias = (FIX_KER *)shared_malloc(p_conv_ctx->conv_info.no_outputs * sizeof (FIX_KER)))) ||
				(NULL == (p_conv_ctx->p_fix_ker = (FIX_KER *)shared_malloc(p_conv_ctx->conv_info.no_inputs *
				p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.ker_size * p_conv_ctx->conv_info.no_outputs * sizeof (FIX_KER))))) {
				REL_INFO("Malloc failed : cannot fit conv params on MSMC\n");
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
		case BATCH_NORM:
		{
			BNORM_LYR_CTX_T *p_bnorm_ctx = (BNORM_LYR_CTX_T *)p_lyr_ctx;
#ifdef USE_RANDOM_MODEL
			if ((NULL == (p_bnorm_ctx->p_flt_scale = (FLT_KER *)ext_malloc(p_bnorm_ctx->bnorm_info.no_inputs * sizeof (FLT_KER)))) ||
				(NULL == (p_bnorm_ctx->p_flt_offset = (FIX_KER *)ext_malloc(p_bnorm_ctx->bnorm_info.no_inputs * sizeof (FLT_KER))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
#endif // USE_RANDOM_MODEL
			if ((NULL == (p_bnorm_ctx->p_fix_scale = (FIX_KER *)shared_malloc(p_bnorm_ctx->bnorm_info.no_outputs * sizeof (FIX_KER)))) ||
				(NULL == (p_bnorm_ctx->p_fix_offset = (FIX_KER *)shared_malloc(p_bnorm_ctx->bnorm_info.no_inputs * sizeof (FIX_KER))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
			no_elements = p_bnorm_ctx->bnorm_info.map_h * p_bnorm_ctx->bnorm_info.map_w * p_bnorm_ctx->bnorm_info.no_outputs;
			break;
		}
		case INNER_PROD:
		{
			IP_LYR_CTX_T *p_ip_ctx = (IP_LYR_CTX_T *)p_lyr_ctx;
#ifdef USE_RANDOM_MODEL
			if ((NULL == (p_ip_ctx->p_flt_weight = (FLT_KER *)ext_malloc(p_ip_ctx->ip_info.no_outputs *
				p_ip_ctx->ip_info.no_inputs * sizeof(FLT_KER)))) ||
				(NULL == (p_ip_ctx->p_flt_bias = (FLT_KER *)ext_malloc(p_ip_ctx->ip_info.no_outputs * sizeof(FLT_KER))))) {
				REL_INFO("Malloc failed\n");
				return MALLOC_FAIL;
			}
#endif // USE_RANDOM_MODEL
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

			shared_free(p_conv_ctx->p_fix_bias);
			shared_free(p_conv_ctx->p_fix_ker);
			break;
		}
		case BATCH_NORM:
		{
			BNORM_LYR_CTX_T *p_bnorm_ctx = (BNORM_LYR_CTX_T *)p_lyr_ctx;

			shared_free(p_bnorm_ctx->p_fix_scale);
			shared_free(p_bnorm_ctx->p_fix_offset);
			break;

		}
		case INNER_PROD:
		{
			IP_LYR_CTX_T *p_ip_ctx = (IP_LYR_CTX_T *)p_lyr_ctx;

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

	//===============
	CONV_LYR_CTX_T *p_conv_ctx;
	POOL_LYR_CTX_T *p_pool_ctx;
	ACT_LYR_CTX_T *p_act_ctx;
	IP_LYR_CTX_T * p_ip_ctx;
	SMAX_LYR_CTX_T *p_smax_ctx;
	BNORM_LYR_CTX_T *p_bnorm_ctx;

	for (lyr = 0; lyr < n_layers; lyr++) {
		switch(g_cnn_layer_nodes[lyr].lyr_type) {
			case CONV:
				p_conv_ctx = (CONV_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				// set the base address of the shared output buffer for this layer. The offset for this core is
				// taken care while calling the layer APIs
				// The buff1 and buff2 are used alternatively for input and output of layers in a ping-pong manner.
				// Even numbered layers will use buff2 as output, odd layers will use buff1 for output.
				// Use same buffer for fixed and floating point since both are not needed simultaneously.
				p_conv_ctx->p_fix_output = (lyr % 2) == 0 ? (FIX_MAP*)p_shared_dbuff2 : (FIX_MAP*)p_shared_dbuff1;
				p_conv_ctx->p_flt_output = (lyr % 2) == 0 ? (FLT_MAP*)p_shared_dbuff2 : (FLT_MAP*)p_shared_dbuff1;
				break;
			case POOL:
				p_pool_ctx = (POOL_LYR_CTX_T *) g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_pool_ctx->p_fix_output = (lyr % 2) == 0 ? (FIX_MAP*)p_shared_dbuff2 : (FIX_MAP*)p_shared_dbuff1;
				p_pool_ctx->p_flt_output = (lyr % 2) == 0 ? (FLT_MAP*)p_shared_dbuff2 : (FLT_MAP*)p_shared_dbuff1;
				break;
			case ACT:
				p_act_ctx = (ACT_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_act_ctx->p_fix_output = (lyr % 2) == 0 ? (FIX_MAP*)p_shared_dbuff2 : (FIX_MAP*)p_shared_dbuff1;
				p_act_ctx->p_flt_output = (lyr % 2) == 0 ? (FLT_MAP*)p_shared_dbuff2 : (FLT_MAP*)p_shared_dbuff1;
				break;
			case BATCH_NORM:
				p_bnorm_ctx = (BNORM_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_bnorm_ctx->p_fix_output = (lyr % 2) == 0 ? (FIX_MAP*)p_shared_dbuff2 : (FIX_MAP*)p_shared_dbuff1;
				p_bnorm_ctx->p_flt_output = (lyr % 2) == 0 ? (FLT_MAP*)p_shared_dbuff2 : (FLT_MAP*)p_shared_dbuff1;
				break;
			case INNER_PROD:
				p_ip_ctx = (IP_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_ip_ctx->p_fix_output = (lyr % 2) == 0 ? (FIX_MAP*)p_shared_dbuff2 : (FIX_MAP*)p_shared_dbuff1;
				p_ip_ctx->p_flt_output = (lyr % 2) == 0 ? (FLT_MAP*)p_shared_dbuff2 : (FLT_MAP*)p_shared_dbuff1;
				break;
			case SOFTMAX:
				p_smax_ctx = (SMAX_LYR_CTX_T *)g_cnn_layer_nodes[lyr].p_lyr_ctx;
				p_smax_ctx->p_float_output = (lyr % 2) == 0 ? (FLT_MAP*)p_shared_dbuff2 : (FLT_MAP*)p_shared_dbuff1;
				break;
			default:
				REL_INFO("Unsupported layer\n");
		}
	}
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
	BNORM_LYR_CTX_T *p_bnorm_ctx;
	CNN_LYR_NODE_T *p_node = p_lyr_nodes;


	for (lyr = 0; lyr < n_layers; lyr++) {
		if(p_node->lyr_type == CONV) {
			p_conv_ctx = (CONV_LYR_CTX_T *)p_node->p_lyr_ctx;
			init_conv_kernels(p_conv_ctx);
		} else if(p_node->lyr_type == INNER_PROD) {
			p_ip_ctx = (IP_LYR_CTX_T *)p_node->p_lyr_ctx;
			init_ip_layer_params(p_ip_ctx);
		} else if(p_node->lyr_type == BATCH_NORM) {
			p_bnorm_ctx = (BNORM_LYR_CTX_T *)p_node->p_lyr_ctx;
			init_bnorm_layer_params(p_bnorm_ctx);
		}
		p_node++;
	}
	return SUCCESS;
}

STATUS_E workload_sharing_config(CNN_LYR_NODE_T *p_lyr_nodes, int n_layers) {
	int lyr, core, quo, rem, map;
	IP_LYR_CTX_T *p_ip_ctx;
	CONV_LYR_CTX_T *p_conv_ctx;
	POOL_LYR_CTX_T *p_pool_ctx;
	ACT_LYR_CTX_T *p_act_ctx;
	BNORM_LYR_CTX_T *p_bnorm_ctx;
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
			case BATCH_NORM:
				p_bnorm_ctx = (BNORM_LYR_CTX_T *)p_node->p_lyr_ctx;
				quo = p_bnorm_ctx->bnorm_info.no_outputs / NO_CORES;
				rem = p_bnorm_ctx->bnorm_info.no_outputs % NO_CORES;
				map = 0;
				for(core = 0; core < NO_CORES; core++) {
					p_bnorm_ctx->start_map[core] = map;
					if(rem == 0) {
						p_bnorm_ctx->no_maps[core] = quo;
						map += quo;
					} else if(core < rem) {
						p_bnorm_ctx->no_maps[core] = quo + 1;
						map += (quo + 1);
					} else {
						p_bnorm_ctx->no_maps[core] = quo;
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

	REL_INFO("Initializing network context...\n");
	caffe_layer_ctx_init();

	REL_INFO("Initializing internal context...\n");
	cnn_layer_internal_param_init();

	REL_INFO("Allocating buffers...\n");
	cnn_app_malloc(g_cnn_layer_nodes, NO_DEEP_LAYERS);

	REL_INFO("Initializing CNN model weights and biases\n");
	cnn_app_model_init(g_cnn_layer_nodes, NO_DEEP_LAYERS);

	// split the computation of maps across all cores.
	REL_INFO("Performing workload distributing among %d cores\n", NO_CORES);
	workload_sharing_config(g_cnn_layer_nodes, NO_DEEP_LAYERS);

	REL_INFO("--------Init Complete--------\n");
	return status;
}
