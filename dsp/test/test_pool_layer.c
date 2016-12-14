#include <stdio.h>
#include "unit_test.h"
#include "pool_layer.h"
#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_cacheAux.h>
#include <float.h>
#include "misc_utils.h"
#include "mem_manager.h"
#include <math.h>
#include "c6x.h"

extern unsigned int core_id;
extern unsigned int far completion_cnt[2];
extern FLT_MAP far *p_ref_flt_output;
extern FLT_MAP far *p_flt_input;
extern FIX_MAP far *p_fix_input;
extern FLT_MAP far *p_ref_flt_output;

#pragma DATA_SECTION(pool_ctx, ".shared_ocm")
POOL_LYR_CTX_T far pool_ctx;

void compute_pool_ref(POOL_LYR_CTX_T *p_ctx, FLT_MAP *p_flt_input) {
	int map, row, col, o_h, o_w, i_h, i_w, r ,c;
	i_h = p_ctx->pool_info.map_h + 2*p_ctx->pool_info.pad;
	i_w = p_ctx->pool_info.map_w + 2*p_ctx->pool_info.pad;

	o_h = (p_ctx->pool_info.map_h + 2*p_ctx->pool_info.pad - p_ctx->pool_info.win_size + 1 + p_ctx->pool_info.stride - 1)/ p_ctx->pool_info.stride;
	o_w = (p_ctx->pool_info.map_w + 2*p_ctx->pool_info.pad - p_ctx->pool_info.win_size + 1 + p_ctx->pool_info.stride - 1)/ p_ctx->pool_info.stride;
	for(map = 0; map < p_ctx->pool_info.no_outputs; map++) {
		for(row = 0; row < o_h; row++) {
			for(col = 0; col < o_w; col++) {
				int hstart = row * p_ctx->pool_info.stride;
				int wstart = col * p_ctx->pool_info.stride;
				int hend = hstart + p_ctx->pool_info.win_size;
				int wend = wstart + p_ctx->pool_info.win_size;
				FLT_MAP max_flt_val = -FLT_MAX;
				for(r = hstart; r < hend; r++) {
					for(c = wstart; c < wend; c++) {
						max_flt_val = MAX(max_flt_val , p_flt_input[(map*i_h + r)*i_w + c]);
					}
				}
				// TODO: case for AVG pool
				p_ref_flt_output[(map*o_h + row)*o_w + col] = max_flt_val;
			}
		}
	}
}

CMP_STATUS_T compare_pool_out(POOL_LYR_CTX_T *p_ctx, FLT_MAP *pOutput) {
	int o_w, o_h, row, col, map;
	CMP_STATUS_T status;
	status.mis_map = -1;
	status.mis_row = -1;
	status.mis_col = -1;
	status.flag = TEST_PASS;

	o_h = (p_ctx->pool_info.map_h + 2*p_ctx->pool_info.pad - p_ctx->pool_info.win_size + 1 + p_ctx->pool_info.stride - 1)/ p_ctx->pool_info.stride;
	o_w = (p_ctx->pool_info.map_w + 2*p_ctx->pool_info.pad - p_ctx->pool_info.win_size + 1 + p_ctx->pool_info.stride - 1)/ p_ctx->pool_info.stride;
	for(map = 0; map < p_ctx->pool_info.no_outputs; map++) {
		for(row = 0; row < o_h; row++) {
			for(col = 0; col < o_w; col++) {
				if(fabs(pOutput[(map*o_h + row)*o_w + col] - p_ref_flt_output[(map*o_h + row)*o_w + col]) > ERR_THRESHOLD) {
					//printf("Mismatch: Map : %d\tRow : %d\tCol : %d\tOrg val: %f\tGot val: %f\n", map, row, col, p_ref_flt_output[(map*o_h + row)*o_w + col], pOutput[(map*o_h + row)*o_w + col]);
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

TEST_STATUS_E test_pool_layer() {
	int no_inputs, input_height, input_width, win_size, stride, pad;
	int out_width, out_height, no_frac_bits;
	POOL_TYPE_E pool_type;
	CMP_STATUS_T status;
	status.flag = TEST_PASS;

	if(core_id == MASTER_CORE_ID) {
		completion_cnt[0] = 0;
		completion_cnt[1] = 0;
		no_inputs = 10;
		no_frac_bits = 14;
		input_height = 17;
		input_width = 17;
		win_size = 2;
		stride = 2;
		pad = 0;
		pool_type = MAX_POOL;
	
		out_height = (input_height + 2*pad - win_size + 1 + stride - 1)/ stride;
		out_width = (input_width + 2*pad - win_size + 1 + stride - 1)/ stride;
		// populate pool layer context
		pool_ctx.pool_info = (POOL_INFO_T){input_height, input_width, no_inputs, no_inputs, win_size, stride, pad, pool_type};

		pool_ctx.lyr_arith_mode = FLOAT_POINT;
#ifdef TEST_MULTICORE
		int quo, rem, core, map;
		quo = pool_ctx.pool_info.no_outputs / NO_CORES;
		rem = pool_ctx.pool_info.no_outputs % NO_CORES;
		map = 0;
		for(core = 0; core < NO_CORES; core++) {
			pool_ctx.start_map[core] = map;
			if(rem == 0) {
				pool_ctx.no_maps[core] = quo;
				map += quo;
			} else if(core < rem) {
				pool_ctx.no_maps[core] = quo + 1;
				map += (quo + 1);
			} else {
				pool_ctx.no_maps[core] = quo;
				map += quo;
			}
		}
#else
		pool_ctx.no_maps[MASTER_CORE_ID] = pool_ctx.pool_info.no_outputs;
		pool_ctx.start_map[MASTER_CORE_ID] = 0;
#endif	
		// input and output buffer allocation	
		pool_ctx.p_flt_output = shared_malloc(out_height * out_width * no_inputs * sizeof(FLT_MAP));
		pool_ctx.p_fix_output = shared_malloc(out_height * out_width * no_inputs * sizeof(FIX_MAP));
		p_ref_flt_output = shared_malloc(out_height * out_width * no_inputs * sizeof(FLT_MAP));
		p_flt_input = shared_malloc((input_height + 2*pad)*(input_width + 2*pad) * no_inputs * sizeof(FLT_MAP));
		p_fix_input = shared_malloc((input_height + 2*pad)*(input_width + 2*pad) * no_inputs * sizeof(FIX_MAP));
	
		// random input
		generate_random_data(p_flt_input, (input_height + 2*pad)*(input_width + 2*pad) * no_inputs, 123);
		float_to_fix_data(p_flt_input, (input_height + 2*pad)*(input_width + 2*pad) * no_inputs, no_frac_bits, p_fix_input);

		CACHE_wbAllL1d(CACHE_WAIT);
		CSL_semReleaseSemaphore(INIT_DONE_SEM);
	} else {
		while(!CSL_semIsFree(INIT_DONE_SEM));
		CACHE_wbInvAllL1d(CACHE_WAIT);
	}

#ifdef TEST_MULTICORE	
	// compute floating point output
	dsp_pool_layer(&pool_ctx, p_flt_input, p_fix_input);
#else
	if(core_id == MASTER_CORE_ID) {
		dsp_pool_layer(&pool_ctx, p_flt_input, p_fix_input);
	}
#endif

	while(!CSL_semAcquireDirect(SHARED_MEM_SEM));
	// make sure that all cores read updated count value. So we need invalidation here.
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
		pool_ctx.lyr_arith_mode = FIXED_POINT;
	} else {
		do {
			// invalidate cache to synch with the above data struct update by core 0
			CACHE_invAllL1d(CACHE_WAIT);
		}
		while(pool_ctx.lyr_arith_mode != FIXED_POINT);
	}

#ifdef TEST_MULTICORE	
	// compute floating point output
	dsp_pool_layer(&pool_ctx, p_flt_input, p_fix_input);
#else
	if(core_id == MASTER_CORE_ID) {
		dsp_pool_layer(&pool_ctx, p_flt_input, p_fix_input);
	}
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

		// compute reference output from the pooling function defined in this file
		compute_pool_ref(&pool_ctx, p_flt_input);
		//print_float_img(p_ref_flt_output + 1 * out_height * out_width, out_height, out_width);
	
		status = compare_pool_out(&pool_ctx, pool_ctx.p_flt_output);
		check_cmp_status(&status);
	
		fix16_to_float_data(pool_ctx.p_fix_output, out_height * out_width * no_inputs, no_frac_bits, pool_ctx.p_flt_output);
		//print_float_img(pool_ctx.p_flt_output + 1 * out_height * out_width, out_height, out_width);
		status = compare_pool_out(&pool_ctx, pool_ctx.p_flt_output);
		check_cmp_status(&status);
	
		shared_free(pool_ctx.p_flt_output);
		shared_free(pool_ctx.p_fix_output);
		shared_free(p_ref_flt_output);
		shared_free(p_fix_input);
		shared_free(p_flt_input);
		reset_mem_manager();
		completion_cnt[0] = 0;
		completion_cnt[1] = 0;

	}

	return status.flag;
}
