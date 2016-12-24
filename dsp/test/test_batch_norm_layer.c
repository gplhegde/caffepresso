/*
 * test_batch_norm_layer.c
 *
 *  Created on: 24 Dec 2016
 *      Author: Gopalakrishna Hegde, NTU Singapore
 */
#include "unit_test.h"
#include "batch_norm_layer.h"
#include "misc_utils.h"
#include "mem_manager.h"
#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_cacheAux.h>
#include <ti/csl/csl_tsc.h>
#include <stdio.h>
#include <math.h>
#include "app_profile.h"

#pragma DATA_SECTION(bnorm_ctx, ".shared_ocm")
BNORM_LYR_CTX_T far bnorm_ctx;

extern unsigned int core_id;

extern unsigned int far completion_cnt[2];

extern FLT_MAP far *p_flt_input;

extern FIX_MAP far *p_fix_input;

extern FLT_MAP far *p_ref_flt_output;


void compute_bnorm_ref(BNORM_LYR_CTX_T *p_ctx, FLT_MAP *p_flt_input) {
	int map, p;
	FLT_MAP *p_ref = p_ref_flt_output;
	for(map = 0; map < p_ctx->bnorm_info.no_inputs; map++) {
		for(p = 0; p < p_ctx->bnorm_info.map_h * p_ctx->bnorm_info.map_w; p++) {
			*p_ref++ = (*p_flt_input++) * p_ctx->p_flt_scale[map] + p_ctx->p_flt_offset[map];
		}
	}
}

CMP_STATUS_T compare_bnorm_out(BNORM_LYR_CTX_T *p_ctx, FLT_MAP *p_output) {
	int map, row, col;
	CMP_STATUS_T status;
	status.mis_map = -1;
	status.mis_row = -1;
	status.mis_col = -1;
	status.flag = TEST_PASS;
	for(map = 0; map < p_ctx->bnorm_info.no_inputs; map++) {
		for(row = 0; row < p_ctx->bnorm_info.map_h; row++) {
			for(col = 0; col < p_ctx->bnorm_info.map_w; col++) {
				if(fabs(p_output[(map * p_ctx->bnorm_info.map_h + row) * p_ctx->bnorm_info.map_w + col] -
					p_ref_flt_output[(map * p_ctx->bnorm_info.map_h + row) * p_ctx->bnorm_info.map_w + col]) > ERR_THRESHOLD) {
					printf("Ref:%f\tAct:%f\n",
						p_ref_flt_output[(map * p_ctx->bnorm_info.map_h + row) * p_ctx->bnorm_info.map_w + col],
						p_output[(map * p_ctx->bnorm_info.map_h + row) * p_ctx->bnorm_info.map_w + col]);
					status.mis_map = map;
					status.mis_row = row;
					status.mis_col = col;
					status.flag = TEST_FAIL;
					return status;
				}

			}
		}
	}
	return status;
}

TEST_STATUS_E test_bnorm_layer() {
	int no_inputs, input_height, input_width;
	int no_map_frac_bits, no_ker_frac_bits;
	uint64_t start_time, end_time;
	CMP_STATUS_T status;

	status.flag = TEST_PASS;
	if(core_id == 0) {
		completion_cnt[0] = 0;
		completion_cnt[1] = 0;
		no_inputs = 1;
		input_height = 54;
		input_width = 54;
		no_map_frac_bits = 14;
		no_ker_frac_bits = 14;


		bnorm_ctx.bnorm_info = (BNORM_INFO_T){input_height, input_width, no_inputs, no_inputs, no_ker_frac_bits, no_map_frac_bits};
		bnorm_ctx.lyr_arith_mode = FLOAT_POINT;

		// input and output buffer allocation
		bnorm_ctx.p_flt_output = shared_malloc(input_height * input_width * no_inputs * sizeof(FLT_MAP));
		bnorm_ctx.p_fix_output = shared_malloc(input_height * input_width * no_inputs * sizeof(FIX_MAP));

		bnorm_ctx.p_flt_scale = shared_malloc(no_inputs *  sizeof(FLT_KER));
		bnorm_ctx.p_fix_scale = shared_malloc(no_inputs * sizeof(FIX_KER));
		bnorm_ctx.p_flt_offset = shared_malloc(no_inputs * sizeof(FLT_KER));
		bnorm_ctx.p_fix_offset = shared_malloc(no_inputs * sizeof(FIX_KER));

#ifdef TEST_MULTICORE
		int quo, rem, core, map;
		quo = bnorm_ctx.bnorm_info.no_inputs / NO_CORES;
		rem = bnorm_ctx.bnorm_info.no_inputs % NO_CORES;
		map = 0;
		for(core = 0; core < NO_CORES; core++) {
			bnorm_ctx.start_map[core] = map;
			if(rem == 0) {
				bnorm_ctx.no_maps[core] = quo;
				map += quo;
			} else if(core < rem) {
				bnorm_ctx.no_maps[core] = quo + 1;
				map += (quo + 1);
			} else {
				bnorm_ctx.no_maps[core] = quo;
				map += quo;
			}
		}
#else
		bnorm_ctx.no_maps[MASTER_CORE_ID] = bnorm_ctx.bnorm_info.no_outputs;
		bnorm_ctx.start_map[MASTER_CORE_ID] = 0;
#endif
		p_ref_flt_output = shared_malloc(input_height * input_width * no_inputs * sizeof(FLT_MAP));
		p_flt_input = shared_malloc(input_height * input_width * no_inputs * sizeof(FLT_MAP));
		p_fix_input = shared_malloc(input_height * input_width * no_inputs * sizeof(FIX_MAP));

		// random input
		generate_random_data(p_flt_input, input_height * input_width * no_inputs, 123);
		generate_random_data(bnorm_ctx.p_flt_scale, no_inputs, 345);
		generate_random_data(bnorm_ctx.p_flt_offset, no_inputs, 321);

		float_to_fix_data(p_flt_input, input_height * input_width * no_inputs, no_map_frac_bits, p_fix_input);
		float_to_fix_data(bnorm_ctx.p_flt_scale, no_inputs, no_ker_frac_bits, bnorm_ctx.p_fix_scale);
		float_to_fix_data(bnorm_ctx.p_flt_offset, no_inputs, no_map_frac_bits, bnorm_ctx.p_fix_offset);

		// Write the updated structure back to MSMC since it is cached. and release the semaphore.
		CACHE_wbAllL1d(CACHE_WAIT);
		CSL_semReleaseSemaphore(INIT_DONE_SEM);
	} else {
		// wait for all init to get over.
		while(!CSL_semIsFree(INIT_DONE_SEM));
		CACHE_wbInvAllL1d(CACHE_WAIT);
	}

#ifdef TEST_MULTICORE
	dsp_batch_norm_layer(&bnorm_ctx, p_flt_input, p_fix_input);
#else
	start_time = CSL_tscRead();
	if(core_id == 0) {
		dsp_batch_norm_layer(&bnorm_ctx, p_flt_input, p_fix_input);
	}
	end_time = CSL_tscRead();
	printf("FLOATING POINT RUNTIME = %.4fus\n", (float)(end_time - start_time)/ DSP_FREQ_IN_MHZ);
#endif

	while(!CSL_semAcquireDirect(SHARED_MEM_SEM));
	CACHE_invAllL1d(CACHE_WAIT);
	completion_cnt[0]++;
	CSL_semReleaseSemaphore(SHARED_MEM_SEM);

#ifdef TEST_MULTICORE
	// wait for all cores to complete computations
	do {
		CACHE_invAllL1d(CACHE_WAIT);
	}while(completion_cnt[0] != NO_CORES);
#endif

	if(core_id == MASTER_CORE_ID) {
		bnorm_ctx.lyr_arith_mode = FIXED_POINT;
	} else {
		do {
			// invalidate cache to synch with the above data struct update by core 0
			CACHE_invAllL1d(CACHE_WAIT);
		}
		while(bnorm_ctx.lyr_arith_mode != FIXED_POINT);
	}

#ifdef TEST_MULTICORE
	dsp_batch_norm_layer(&bnorm_ctx, p_flt_input, p_fix_input);
#else
	start_time = CSL_tscRead();
	if(core_id == 0) {
		dsp_batch_norm_layer(&bnorm_ctx, p_flt_input, p_fix_input);
	}
	end_time = CSL_tscRead();
	printf("FIXED POINT RUNTIME = %.4fus\n", (float)(end_time - start_time)/ DSP_FREQ_IN_MHZ);
#endif

	while(!CSL_semAcquireDirect(SHARED_MEM_SEM));
	CACHE_invAllL1d(CACHE_WAIT);
	completion_cnt[1]++;
	CSL_semReleaseSemaphore(SHARED_MEM_SEM);

#ifdef TEST_MULTICORE
	do {
		CACHE_invAllL1d(CACHE_WAIT);
	}while(completion_cnt[1] != NO_CORES);
#endif

	if(core_id == MASTER_CORE_ID) {
		CACHE_invAllL1d(CACHE_WAIT);

		compute_bnorm_ref(&bnorm_ctx, p_flt_input);
		//print_float_img(p_ref_flt_output, out_height, out_width);

		status = compare_bnorm_out(&bnorm_ctx, bnorm_ctx.p_flt_output);
		check_cmp_status(&status);

		fix16_to_float_data(bnorm_ctx.p_fix_output, input_height * input_width * no_inputs, no_map_frac_bits, bnorm_ctx.p_flt_output);
		status = compare_bnorm_out(&bnorm_ctx, bnorm_ctx.p_flt_output);
		check_cmp_status(&status);

		shared_free(bnorm_ctx.p_flt_output);
		shared_free(bnorm_ctx.p_fix_output);
		shared_free(bnorm_ctx.p_flt_scale);
		shared_free(bnorm_ctx.p_fix_scale);
		shared_free(bnorm_ctx.p_flt_offset);
		shared_free(bnorm_ctx.p_fix_offset);
		shared_free(p_ref_flt_output);
		shared_free(p_fix_input);
		shared_free(p_flt_input);
		reset_mem_manager();
		completion_cnt[0] = 0;
		completion_cnt[1] = 0;
	}

	return status.flag;
}
