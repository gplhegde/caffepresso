#include "inner_prod_layer.h"
#include "struct_defs.h"
#include <stdio.h>
#include <string.h>
#include "dsplib.h"
#include "ext_dsplib.h"
#include "debug_control.h"
#include "edma_module.h"
#include "mem_manager.h"

extern unsigned int core_id;
extern uint8_t far private_temp_buff[PRIVATE_TEMP_BUFF_SIZE];
extern uint8_t far private_conv_buff[PRIVATE_TEMP_BUFF_SIZE];
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

static inline void fix16_vect_add(FIX_MAP *p_vect1, FIX_MAP *p_vect2, int len, FIX_MAP *p_sum) {
    int e;
    for ( e = 0; e < len; e++) {
        p_sum[e] = p_vect1[e] + p_vect2[e];
    }
}

/* Use EDMA only if the input and/or weights are in DDR.
 * If they are in MSMC, just use memcpy which is found to be faster than DMA.	 *
 */
#define USE_DMA_IP_LYR
STATUS_E dsp_fix_ip_layer(FIX_MAP *p_input,	// pointer to input features
	FIX_KER *p_weight,	// pointer to weight matrix stored in [no_outputs][no_inputs] manner
	FIX_KER *p_bias,	// pointer to bias units
	int no_inputs,		// number of input units to this layer
	int no_outputs,		// number of output units
	int shift,			// shift used to convert the dot product to 16b. Perform conversion before adding bias
	FIX_MAP *p_output	// pointer to output features.
	) {

	int n, sop, n_x4;
	STATUS_E status = SUCCESS;
	REL_ASSERT((int)p_input % 8 == 0);
	REL_ASSERT((int)p_weight % 8 == 0);

	// no of elements must be multiple of 4 for the DSPLIB dot-product API
	n_x4 = ((no_inputs >> 2) << 2) + 4;
#ifdef USE_DMA_IP_LYR
	FIX_KER *p_w[2];
	EDMA_OBJ_T * p_edma;

	p_w[0] = global_address((Uint32)private_temp_buff);
	p_w[1] = global_address((Uint32)private_conv_buff);

	p_edma = &shared_edma_obj[core_id * NO_CHANNELS_PER_CORE + 0];
	memset(p_w[0], 0, n_x4 * sizeof(FIX_KER));
	dma_array(p_edma, p_weight, p_w[0], no_inputs * sizeof(FIX_KER));
	// This will make sure that the extra multiplications are done with 0.
	memset(p_w[1], 0, n_x4 * sizeof(FIX_KER));
	wait_for_dma_tx(p_edma, FALSE, FALSE);
	for (n = 0; n < no_outputs; n++) {
		// Prefetch the next set of weights into different buffer
		if(n < no_outputs - 1) {
			dma_array(p_edma, p_weight + (n + 1) * no_inputs, p_w[(n + 1) % 2], no_inputs * sizeof(FIX_KER));
		}

		sop = DSP_dotprod(p_input, p_w[n % 2], n_x4);
		p_output[n] = (FIX_MAP)(sop >> shift);

		// Wait for the previously initiated DMA transfer.
		if(n < no_outputs - 1) {
			wait_for_dma_tx(p_edma, FALSE, FALSE);
		}
	}
#else
	FIX_KER *p_w;

	p_w = (FIX_KER *)private_temp_buff;

	memset(p_w, 0, n_x4 * sizeof(FIX_KER));

	for (n = 0; n < no_outputs; n++) {
		// Copy weights into first no_inputs locations. Rest are already set to 0.
		memcpy(p_w, p_weight + n * no_inputs, no_inputs * sizeof(FIX_KER));
		sop = DSP_dotprod(p_input, p_w, n_x4);
		p_output[n] = (FIX_MAP)(sop >> shift);
	}
#endif // USE_DMA_IP_LYR

	fix16_vect_add(p_output, p_bias, no_outputs, p_output);

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
		// TODO: Use DSPLIB APIs like DSPF_sp_dotprod for this
		p_output[n] = float_dot_prod(p_input, p_weight + n * no_inputs, no_inputs);
	}
	float_vect_add(p_output, p_bias, no_outputs, p_output);

	return status;
}

STATUS_E dsp_ip_layer(IP_LYR_CTX_T *p_ip_ctx, FLT_MAP *p_flt_input, FIX_MAP *p_fix_input) {
	STATUS_E status = FAILED;

	if(p_ip_ctx->lyr_arith_mode == FIXED_POINT) {
		status = dsp_fix_ip_layer(p_fix_input,
			p_ip_ctx->p_fix_weight + p_ip_ctx->start_map[core_id] * p_ip_ctx->ip_info.no_inputs,
			p_ip_ctx->p_fix_bias + p_ip_ctx->start_map[core_id],
			p_ip_ctx->ip_info.no_inputs,
			p_ip_ctx->no_maps[core_id],
			p_ip_ctx->ip_info.no_ker_frac_bits,
			p_ip_ctx->p_fix_output + p_ip_ctx->start_map[core_id]
			);
	} else {
		status = dsp_flt_ip_layer(p_flt_input,
			p_ip_ctx->p_flt_weight + p_ip_ctx->start_map[core_id] * p_ip_ctx->ip_info.no_inputs,
			p_ip_ctx->p_flt_bias +  p_ip_ctx->start_map[core_id],
			p_ip_ctx->ip_info.no_inputs,
			p_ip_ctx->no_maps[core_id],
			p_ip_ctx->p_flt_output + p_ip_ctx->start_map[core_id]
			);
	}
	return status;
}
