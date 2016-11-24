#include <stdio.h>
#include "unit_test.h"
#include "pool_layer.h"
#include <float.h>
#include "misc_utils.h"
#include <math.h>

FLT_MAP *p_ref_flt_output;

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
	int out_width, out_height;
	POOL_TYPE_E pool_type;
	FLT_MAP *p_flt_input;
	FIX_MAP *p_fix_input;
	
	CMP_STATUS_T status;

	POOL_LYR_CTX_T pool_ctx;

	printf("Testing POOL Layer\n");
	no_inputs = 3;
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
	pool_ctx.no_maps[0] = pool_ctx.pool_info.no_outputs;
	pool_ctx.start_map[0] = 0;

	// input and output buffer allocation	
	pool_ctx.p_flt_output = ext_malloc(out_height * out_width * no_inputs * sizeof(FLT_MAP));
	pool_ctx.p_fix_output = ext_malloc(out_height * out_width * no_inputs * sizeof(FIX_MAP));
	p_ref_flt_output = ext_malloc(out_height * out_width * no_inputs * sizeof(FLT_MAP));
	p_flt_input = ext_malloc((input_height + 2*pad)*(input_width + 2*pad) * no_inputs * sizeof(FLT_MAP));
	p_fix_input = ext_malloc((input_height + 2*pad)*(input_width + 2*pad) * no_inputs * sizeof(FIX_MAP));

	// random input
	generate_random_data(p_flt_input, (input_height + 2*pad)*(input_width + 2*pad) * no_inputs, 123);
	float_to_fix_data(p_flt_input, (input_height + 2*pad)*(input_width + 2*pad) * no_inputs, 14, p_fix_input);

	// compute floating point output
	dsp_pool_layer(&pool_ctx, p_flt_input, p_fix_input);
	// compute fixed point scalar output
	pool_ctx.lyr_arith_mode = FIXED_POINT;
	dsp_pool_layer(&pool_ctx, p_flt_input, p_fix_input);

	//print_float_img(pool_ctx.p_flt_output, out_height, out_width);

	// compute reference output from the pooling function defined in this file
	compute_pool_ref(&pool_ctx, p_flt_input);
	//print_float_img(p_ref_flt_output + 1 * out_height * out_width, out_height, out_width);

	status = compare_pool_out(&pool_ctx, pool_ctx.p_flt_output);
	check_cmp_status(&status);

	fix16_to_float_data(pool_ctx.p_fix_output, out_height * out_width * no_inputs, 14, pool_ctx.p_flt_output);
	//print_float_img(pool_ctx.p_flt_output + 1 * out_height * out_width, out_height, out_width);
	status = compare_pool_out(&pool_ctx, pool_ctx.p_flt_output);
	check_cmp_status(&status);


	ext_free(pool_ctx.p_flt_output);
	ext_free(pool_ctx.p_fix_output);
	ext_free(p_ref_flt_output);
	ext_free(p_fix_input);
	ext_free(p_flt_input);
	return status.flag;
}
