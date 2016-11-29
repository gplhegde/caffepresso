#include "unit_test.h"
#include "conv_layer.h"
#include "misc_utils.h"
#include "mem_manager.h"
#include <ti/csl/csl_semAux.h>
#include <stdio.h>
#include <math.h>


#pragma DATA_SECTION(completion_cnt, ".sharedram")
unsigned int completion_cnt;
#pragma DATA_SECTION(p_flt_input, ".sharedram")
FLT_MAP *p_flt_input;
#pragma DATA_SECTION(p_fix_input, ".sharedram")
FIX_MAP *p_fix_input;
#pragma DATA_SECTION(conv_ctx, ".sharedram")
CONV_LYR_CTX_T conv_ctx;

FLT_MAP *p_conv_ref_flt_output;
extern unsigned int core_id;

void compute_conv_ref(CONV_LYR_CTX_T *p_ctx, FLT_MAP *p_flt_input) {
	int row, col, o_h, o_w, i_h, i_w, omap, imap, hstart, wstart, kr, kc;
	i_h = p_ctx->conv_info.map_h + 2*p_ctx->conv_info.pad;
	i_w = p_ctx->conv_info.map_w + 2*p_ctx->conv_info.pad;
	o_h = (p_ctx->conv_info.map_h + 2 * p_ctx->conv_info.pad - p_ctx->conv_info.ker_size + 1 + p_ctx->conv_info.stride - 1) / p_ctx->conv_info.stride;
	o_w = (p_ctx->conv_info.map_w + 2 * p_ctx->conv_info.pad - p_ctx->conv_info.ker_size + 1 + p_ctx->conv_info.stride - 1) / p_ctx->conv_info.stride;

	for(omap = 0; omap < p_ctx->conv_info.no_outputs; omap++) {
		for(row = 0; row < o_h; row++) {
			hstart = row * p_ctx->conv_info.stride;
			for(col = 0; col < o_w; col++) {
				wstart = col * p_ctx->conv_info.stride;
				float sum = 0.0f;
				for(imap = 0; imap < p_ctx->conv_info.no_inputs; imap++) {
					for(kr = 0; kr < p_ctx->conv_info.ker_size; kr++) {
						for(kc = 0; kc < p_ctx->conv_info.ker_size; kc++) {
							sum += 	p_ctx->p_flt_ker[((omap * p_ctx->conv_info.no_inputs +imap) * p_ctx->conv_info.ker_size + kr)* p_ctx->conv_info.ker_size + kc] *
								p_flt_input[imap * i_w * i_h + (hstart + kr) * i_w + wstart + kc];
						}
					}
				}
				sum += p_ctx->p_flt_bias[omap];
				p_conv_ref_flt_output[(omap * o_h + row) * o_w + col] = sum;
			}	
		}
	}

}

CMP_STATUS_T compare_conv_out(CONV_LYR_CTX_T *p_ctx, FLT_MAP *p_output) {
	int o_w, o_h, map, row, col;
	CMP_STATUS_T status;
	status.mis_map = -1;
	status.mis_row = -1;
	status.mis_col = -1;
	status.flag = TEST_PASS;

	o_h = (p_ctx->conv_info.map_h + 2*p_ctx->conv_info.pad - p_ctx->conv_info.ker_size + 1 + p_ctx->conv_info.stride - 1)/ p_ctx->conv_info.stride;
	o_w = (p_ctx->conv_info.map_w + 2*p_ctx->conv_info.pad - p_ctx->conv_info.ker_size + 1 + p_ctx->conv_info.stride - 1)/ p_ctx->conv_info.stride;
	for(map = 0; map < p_ctx->conv_info.no_outputs; map++) {
		for(row = 0; row < o_h; row++) {
			for(col = 0; col < o_w; col++) {
				if(fabs(p_output[(map*o_h + row)*o_w + col] - p_conv_ref_flt_output[(map*o_h + row)*o_w + col]) > ERR_THRESHOLD) {
					printf("Ref:%f\tAct:%f\n", p_conv_ref_flt_output[(map*o_h + row)*o_w + col], p_output[(map*o_h + row)*o_w + col]);
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

TEST_STATUS_E test_conv_layer() {
	int no_inputs, no_outputs, input_height, input_width, K, stride, pad, k, i;
	int out_width, out_height, no_map_frac_bits, no_ker_frac_bits, map, omap;

	
	CMP_STATUS_T status;



	printf("Testing CONV Layer\n");
	if(core_id == 0) {
		completion_cnt = 0;
		no_inputs = 3;
		no_outputs = 10;
		input_height = 9;
		input_width = 10;
		K = 3;
		stride = 2;
		pad = 0;
		no_map_frac_bits = 12;
		no_ker_frac_bits = 12;
	
		out_height = (input_height + 2*pad - K + 1 + stride - 1)/ stride;
		out_width = (input_width + 2*pad - K + 1 + stride - 1)/ stride;
		conv_ctx.conv_info = (CONV_INFO_T){input_height, input_width, K, no_inputs, no_outputs, pad, stride, no_ker_frac_bits, no_map_frac_bits};
	
		conv_ctx.lyr_arith_mode = FLOAT_POINT;
	
		// input and output buffer allocation	
		conv_ctx.p_flt_output = ext_malloc(out_height * out_width * no_outputs * sizeof(FLT_MAP));
		conv_ctx.p_fix_output = ext_malloc(out_height * out_width * no_outputs * sizeof(FIX_MAP));
		conv_ctx.p_flt_ker = ext_malloc(K * K * no_inputs * no_outputs * sizeof(FLT_KER));
		conv_ctx.p_fix_ker = ext_malloc(K * K * no_inputs * no_outputs * sizeof(FIX_KER));
		conv_ctx.p_flt_bias = ext_malloc(no_outputs *sizeof(FLT_KER));
		conv_ctx.p_fix_bias = ext_malloc(no_outputs *sizeof(FIX_KER));

#ifdef TEST_MULTICORE
		int quo, rem, core;
		quo = conv_ctx.conv_info.no_outputs / NO_CORES;
		rem = conv_ctx.conv_info.no_outputs % NO_CORES;
		map = 0;
		for(core = 0; core < NO_CORES; core++) {
			conv_ctx.start_map[core] = map;
			if(rem == 0) {
				conv_ctx.no_maps[core] = quo;
				map += quo;
			} else if(core < rem) {
				conv_ctx.no_maps[core] = quo + 1;
				map += (quo + 1);
			} else {
				conv_ctx.no_maps[core] = quo;
				map += quo;
			}
		}
#else
		conv_ctx.no_maps[0] = conv_ctx.conv_info.no_outputs;
		conv_ctx.start_map[0] = 0;
#endif
		p_conv_ref_flt_output = ext_malloc(out_height * out_width * no_outputs * sizeof(FLT_MAP));
		p_flt_input = ext_malloc((input_height + 2*pad)*(input_width + 2*pad) * no_inputs * sizeof(FLT_MAP));
		p_fix_input = ext_malloc((input_height + 2*pad)*(input_width + 2*pad) * no_inputs * sizeof(FIX_MAP));
	
		// random input
		generate_random_data(p_flt_input, (input_height + 2*pad)*(input_width + 2*pad) * no_inputs, 123);
		generate_random_data(conv_ctx.p_flt_ker, K * K * no_inputs * no_outputs, 345);
		generate_random_data(conv_ctx.p_flt_bias, no_outputs, 321);
	
		float_to_fix_data(p_flt_input, (input_height + 2*pad)*(input_width + 2*pad) * no_inputs, no_map_frac_bits, p_fix_input);
		float_to_fix_data(conv_ctx.p_flt_ker, K * K * no_inputs * no_outputs, no_ker_frac_bits, conv_ctx.p_fix_ker);
		float_to_fix_data(conv_ctx.p_flt_bias, no_outputs, no_map_frac_bits, conv_ctx.p_fix_bias);
		
		// rotate the fixed point kernel to cmpensate for the 180 deg rotation performed by the IMGLIB APIs
		for(omap = 0; omap < no_outputs; omap++) {
			for(map = 0; map < no_inputs; map++) {
				rotate_180(conv_ctx.p_fix_ker + omap * no_inputs * K * K + map * K * K, K, K);
			}
		}
		printf("Init done\n");
		CSL_semReleaseSemaphore(INIT_DONE_SEM);
	}
	// wait for all init to get over.
	while(!CSL_semIsFree(INIT_DONE_SEM));

#ifdef TEST_MULTICORE
	dsp_conv_layer(&conv_ctx, p_flt_input, p_fix_input);
#else
	if(core_id == 0) {
		dsp_conv_layer(&conv_ctx, p_flt_input, p_fix_input);
	}
#endif
	while(!CSL_semAcquireDirect(DATA_SYNC_SEM));
	completion_cnt++;
	CSL_semReleaseSemaphore(DATA_SYNC_SEM);

	//while(!CSL_semIsFree(DATA_SYNC_SEM));
#ifdef TEST_MULTICORE
	while(completion_cnt != NO_CORES);
#endif
	
	if(core_id == 0) {
		while(!CSL_semAcquireDirect(INIT_DONE_SEM));
		printf("Changing the arithmetic mode: %d\n", core_id);
		completion_cnt = 0;
		conv_ctx.lyr_arith_mode = FIXED_POINT;
		CSL_semReleaseSemaphore(INIT_DONE_SEM);
	}
	printf("Waiting for master cores: %d\n", core_id);
	while(!CSL_semIsFree(INIT_DONE_SEM));

#ifdef TEST_MULTICORE
	dsp_conv_layer(&conv_ctx, p_flt_input, p_fix_input);
	printf("Done till here: %d\n", core_id);
#else
	if(core_id == 0) {
		dsp_conv_layer(&conv_ctx, p_flt_input, p_fix_input);
	}
#endif

	
	while(!CSL_semAcquireDirect(DATA_SYNC_SEM));
	completion_cnt++;
	CSL_semReleaseSemaphore(DATA_SYNC_SEM);
	
#ifdef TEST_MULTICORE
	while(completion_cnt != NO_CORES);
#endif

	//print_fix_img(conv_ctx.p_fix_output, out_height, out_width);
	//print_float_img(conv_ctx.p_flt_output, out_height, out_width);
	if(core_id == 0) {
		while(!CSL_semAcquireDirect(DATA_SYNC_SEM));	
	
		compute_conv_ref(&conv_ctx, p_flt_input);
		print_float_img(p_conv_ref_flt_output, out_height, out_width);
	
		status = compare_conv_out(&conv_ctx, conv_ctx.p_flt_output);
		check_cmp_status(&status);
	
		fix16_to_float_data(conv_ctx.p_fix_output, out_height * out_width * no_outputs, no_map_frac_bits, conv_ctx.p_flt_output);
		status = compare_conv_out(&conv_ctx, conv_ctx.p_flt_output);
		print_float_img(conv_ctx.p_flt_output, out_height, out_width);
		check_cmp_status(&status);
	
		ext_free(conv_ctx.p_flt_output);
		ext_free(conv_ctx.p_fix_output);
		ext_free(conv_ctx.p_flt_ker);
		ext_free(conv_ctx.p_fix_ker);
		ext_free(conv_ctx.p_flt_bias);
		ext_free(conv_ctx.p_fix_bias);
		ext_free(p_conv_ref_flt_output);
		ext_free(p_fix_input);
		ext_free(p_flt_input);
		
		CSL_semReleaseSemaphore(DATA_SYNC_SEM);
	}
	while(!CSL_semIsFree(DATA_SYNC_SEM));
	return status.flag;
}
