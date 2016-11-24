#include "unit_test.h"
#include "inner_prod_layer.h"
#include "misc_utils.h"
#include <math.h>

FLT_MAP *p_ref_flt_output;

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
	FLT_MAP *p_flt_input;
	FIX_MAP *p_fix_input;
	CMP_STATUS_T status;

	IP_LYR_CTX_T ip_ctx;

	printf("Testing Inner product Layer\n");
	no_inputs = 64;
	no_outputs = 160;
	no_map_frac_bits = 11;
	no_ker_frac_bits = 11;

	ip_ctx.ip_info = (IP_INFO_T){1, 1, no_inputs, no_outputs, no_ker_frac_bits, no_map_frac_bits};
	ip_ctx.lyr_arith_mode = FLOAT_POINT;
	ip_ctx.no_maps[0] = ip_ctx.ip_info.no_outputs;
	ip_ctx.start_map[0] = 0;

	// input and output buffer allocation	
	ip_ctx.p_flt_output = ext_malloc(no_outputs * sizeof(FLT_MAP));
	ip_ctx.p_fix_output = ext_malloc(no_outputs * sizeof(FIX_MAP));
	ip_ctx.p_flt_weight = ext_malloc(no_inputs * no_outputs * sizeof(FLT_KER));
	ip_ctx.p_fix_weight = ext_malloc(no_inputs * no_outputs * sizeof(FIX_KER));
	ip_ctx.p_flt_bias = ext_malloc(no_outputs *sizeof(FLT_KER));
	ip_ctx.p_fix_bias = ext_malloc(no_outputs *sizeof(FIX_KER));

	p_ref_flt_output = ext_malloc(no_outputs * sizeof(FLT_MAP));
	p_flt_input = ext_malloc(no_inputs * sizeof(FLT_MAP));
	p_fix_input = ext_malloc(no_inputs * sizeof(FIX_MAP));

	// random input
	generate_random_data(p_flt_input, no_inputs, 123);
	generate_random_data(ip_ctx.p_flt_weight, no_inputs * no_outputs, 345);
	generate_random_data(ip_ctx.p_flt_bias, no_outputs, 321);

	float_to_fix_data(p_flt_input, no_inputs, no_map_frac_bits, p_fix_input);
	float_to_fix_data(ip_ctx.p_flt_weight, no_inputs * no_outputs, no_ker_frac_bits, ip_ctx.p_fix_weight);
	float_to_fix_data(ip_ctx.p_flt_bias, no_outputs, no_map_frac_bits, ip_ctx.p_fix_bias);
	dsp_ip_layer(&ip_ctx, p_flt_input, p_fix_input);

	ip_ctx.lyr_arith_mode = FIXED_POINT;
	dsp_ip_layer(&ip_ctx, p_flt_input, p_fix_input);

	//print_float_img(ip_ctx.p_flt_output, 1, no_outputs);

	compute_ip_ref(&ip_ctx, p_flt_input);
	printf("Reference feature map\n");
	//print_float_img(p_ref_flt_output, 1, no_outputs);

	status = compare_ip_out(&ip_ctx, ip_ctx.p_flt_output);
	printf("Comparing floating point output\n");
	check_cmp_status(&status);

	fix16_to_float_data(ip_ctx.p_fix_output, no_outputs, no_map_frac_bits, ip_ctx.p_flt_output);
    printf("Fixed point output map: scalar version\n");
	//print_float_img(ip_ctx.p_flt_output, 1, no_outputs);
	//print_fix_img(ip_ctx.p_fix_output, 1, no_outputs);
	status = compare_ip_out(&ip_ctx, ip_ctx.p_flt_output);
	check_cmp_status(&status);



	ext_free(ip_ctx.p_flt_output);
	ext_free(ip_ctx.p_fix_output);
	ext_free(ip_ctx.p_flt_weight);
	ext_free(ip_ctx.p_fix_weight);
	ext_free(ip_ctx.p_flt_bias);
	ext_free(ip_ctx.p_fix_bias);

	ext_free(p_ref_flt_output);
	ext_free(p_fix_input);
	ext_free(p_flt_input);
	return status.flag;
}
