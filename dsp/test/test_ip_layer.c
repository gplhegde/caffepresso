#include <stdio.h>
#include "unit_test.h"
#include "inner_prod_layer.h"
#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_cacheAux.h>
#include "misc_utils.h"
#include "mem_manager.h"
#include <math.h>

extern unsigned int core_id;

extern FLT_MAP far *p_ref_flt_output;

extern unsigned int far completion_cnt[2];

extern FLT_MAP far *p_flt_input;

extern FIX_MAP far *p_fix_input;

extern FLT_MAP far *p_ref_flt_output;

#pragma DATA_SECTION(ip_ctx, ".shared_ocm")
IP_LYR_CTX_T far ip_ctx;

void compute_ip_ref(IP_LYR_CTX_T *p_ctx, FLT_MAP *p_flt_input) {
	int out, in;
	FLT_MAP sum;
	for(out = 0; out < p_ctx->ip_info.no_outputs; out++) {
		sum = 0.0;
		for(in = 0; in < p_ctx->ip_info.no_inputs; in++) {
			sum += p_ctx->p_flt_weight[out * p_ctx->ip_info.no_inputs + in] * p_flt_input[in];
		}
		p_ref_flt_output[out] = sum + p_ctx->p_flt_bias[out];
	}
}

CMP_STATUS_T compare_ip_out(IP_LYR_CTX_T *p_ctx, FLT_MAP *pOutput) {
	int out;
	CMP_STATUS_T status;
	status.mis_map = -1;
	status.mis_row = -1;
	status.mis_col = -1;
	status.flag = TEST_PASS;

	for(out = 0; out < p_ctx->ip_info.no_outputs; out++) {
		if(fabs(pOutput[out] - p_ref_flt_output[out]) > ERR_THRESHOLD) {
			printf("Act: %f\tRef: %f\n", pOutput[out], p_ref_flt_output[out]);
			status.mis_map = 1;
			status.mis_row = 1;
			status.mis_col = out;
			status.flag = TEST_FAIL;
			return status;
		}
	}
	return status;
}

TEST_STATUS_E test_ip_layer() {
	int no_inputs, no_outputs;
	int no_map_frac_bits, no_ker_frac_bits;
	CMP_STATUS_T status;

	status.flag = TEST_PASS;

	if(core_id == MASTER_CORE_ID) {
		no_inputs = 32;
		no_outputs = 120;
		no_map_frac_bits = 11;
		no_ker_frac_bits = 11;
		completion_cnt[0] = 0;
		completion_cnt[1] = 0;
		ip_ctx.ip_info = (IP_INFO_T){1, 1, no_inputs, no_outputs, no_ker_frac_bits, no_map_frac_bits};
		ip_ctx.lyr_arith_mode = FLOAT_POINT;
	
		// input and output buffer allocation	
		ip_ctx.p_flt_output = shared_malloc(no_outputs * sizeof(FLT_MAP));
		ip_ctx.p_fix_output = shared_malloc(no_outputs * sizeof(FIX_MAP));
		memset(ip_ctx.p_flt_output, 0, no_outputs * sizeof(FLT_MAP));
		memset(ip_ctx.p_fix_output, 0, no_outputs * sizeof(FIX_MAP));
		ip_ctx.p_flt_weight = shared_malloc(no_inputs * no_outputs * sizeof(FLT_KER));
		ip_ctx.p_fix_weight = shared_malloc(no_inputs * no_outputs * sizeof(FIX_KER));

		ip_ctx.p_flt_bias = shared_malloc(no_outputs *sizeof(FLT_KER));
		ip_ctx.p_fix_bias = shared_malloc(no_outputs *sizeof(FIX_KER));

#ifdef TEST_MULTICORE
		int quo, rem, core, map;
		quo = ip_ctx.ip_info.no_outputs / NO_CORES;
		rem = ip_ctx.ip_info.no_outputs % NO_CORES;
		map = 0;
		for(core = 0; core < NO_CORES; core++) {
			ip_ctx.start_map[core] = map;
			if(rem == 0) {
				ip_ctx.no_maps[core] = quo;
				map += quo;
			} else if(core < rem) {
				ip_ctx.no_maps[core] = quo + 1;
				map += (quo + 1);
			} else {
				ip_ctx.no_maps[core] = quo;
				map += quo;
			}
		}
#else
		ip_ctx.no_maps[MASTER_CORE_ID] = ip_ctx.ip_info.no_outputs;
		ip_ctx.start_map[MASTER_CORE_ID] = 0;
#endif	
		p_ref_flt_output = shared_malloc(no_outputs * sizeof(FLT_MAP));
		p_flt_input = shared_malloc(no_inputs * sizeof(FLT_MAP));
		p_fix_input = shared_malloc(no_inputs * sizeof(FIX_MAP));
	
		// random input
		generate_random_data(p_flt_input, no_inputs, 123);
		generate_random_data(ip_ctx.p_flt_weight, no_inputs * no_outputs, 345);
		generate_random_data(ip_ctx.p_flt_bias, no_outputs, 321);
	
		float_to_fix_data(p_flt_input, no_inputs, no_map_frac_bits, p_fix_input);
		float_to_fix_data(ip_ctx.p_flt_weight, no_inputs * no_outputs, no_ker_frac_bits, ip_ctx.p_fix_weight);
		float_to_fix_data(ip_ctx.p_flt_bias, no_outputs, no_map_frac_bits, ip_ctx.p_fix_bias);

		// Write back the cache contents. Even though the MSMC region is set to be write-through, we need to flush out the stack
		// contents because entire cache is invalidated later (this slows down, need to align data structures to cache line and invalidate
		// required cache lines in the future)
		CACHE_wbAllL1d(CACHE_WAIT);
		CSL_semReleaseSemaphore(INIT_DONE_SEM);
	} else {
		while(!CSL_semIsFree(INIT_DONE_SEM));
		// sync with MSMC if the data is already cached.
		CACHE_wbInvAllL1d(CACHE_WAIT);
	}

#ifdef TEST_MULTICORE
	dsp_ip_layer(&ip_ctx, p_flt_input, p_fix_input);
#else
	if(core_id == 0) {
		dsp_ip_layer(&ip_ctx, p_flt_input, p_fix_input);
	}
#endif

	while(!CSL_semAcquireDirect(DATA_SYNC_SEM));
	// make sure that all cores read updated count value. So we need invalidation here.
	CACHE_invAllL1d(CACHE_WAIT);
	completion_cnt[0]++;
	CSL_semReleaseSemaphore(DATA_SYNC_SEM);
	
#ifdef TEST_MULTICORE
	// wait for all cores to complete computations
	do {
		CACHE_invAllL1d(CACHE_WAIT);
	}while(completion_cnt[0] != NO_CORES);
#endif

	if(core_id == MASTER_CORE_ID) {
		ip_ctx.lyr_arith_mode = FIXED_POINT;
	} else {
		do {
			// invalidate cache to synch with the above data struct update by core 0
			CACHE_invAllL1d(CACHE_WAIT);
		}
		while(ip_ctx.lyr_arith_mode != FIXED_POINT);
	}
	
#ifdef TEST_MULTICORE	
	dsp_ip_layer(&ip_ctx, p_flt_input, p_fix_input);
#else
	if(core_id == 0) {
		dsp_ip_layer(&ip_ctx, p_flt_input, p_fix_input);
	}
#endif

	while(!CSL_semAcquireDirect(DATA_SYNC_SEM));
	CACHE_invAllL1d(CACHE_WAIT);
	completion_cnt[1]++;
	CSL_semReleaseSemaphore(DATA_SYNC_SEM);
	
#ifdef TEST_MULTICORE
	do {
		CACHE_invAllL1d(CACHE_WAIT);
	}while(completion_cnt[1] != NO_CORES);
#endif

	if(core_id == MASTER_CORE_ID) {
		CACHE_invAllL1d(CACHE_WAIT);

		compute_ip_ref(&ip_ctx, p_flt_input);
		//print_float_img(p_ref_flt_output, 1, no_outputs);
		//print_float_img(ip_ctx.p_flt_output, 1, no_outputs);
	
		status = compare_ip_out(&ip_ctx, ip_ctx.p_flt_output);
		check_cmp_status(&status);
	
		fix16_to_float_data(ip_ctx.p_fix_output, no_outputs, no_map_frac_bits, ip_ctx.p_flt_weight);
		//print_float_img(ip_ctx.p_flt_weight, 1, no_outputs);
		//print_fix_img(ip_ctx.p_fix_output, 1, no_outputs);

		status = compare_ip_out(&ip_ctx, ip_ctx.p_flt_weight);
		check_cmp_status(&status);
	
		shared_free(ip_ctx.p_flt_output);
		shared_free(ip_ctx.p_fix_output);
		shared_free(ip_ctx.p_flt_weight);
		shared_free(ip_ctx.p_fix_weight);
		shared_free(ip_ctx.p_flt_bias);
		shared_free(ip_ctx.p_fix_bias);
		shared_free(p_ref_flt_output);
		shared_free(p_fix_input);
		shared_free(p_flt_input);

		// MSMC buffers are no more needed. Reset the free pointer to the base.
		reset_mem_manager();
		completion_cnt[0] = 0;
		completion_cnt[1] = 0;
	}

	return status.flag;
}
