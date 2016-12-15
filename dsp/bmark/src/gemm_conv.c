/*
 * gemm_conv.c
 *
 *  Created on: 13 Dec 2016
 *      Author: Gopalakrishna Hegde
 */
#include "gemm_conv.h"
#include "dsplib.h"
#include "debug_control.h"
#include "mem_manager.h"
#include "lenet_conv_gemm_params.h"
#include "data_sync.h"
#include "app_profile.h"
#include "misc_utils.h"

#define GEMM_BMARK_START_SIZE	(16)
#define GEMM_BMARK_NO_CASES		(10)

extern unsigned int core_id;
extern uint8_t private_temp_buff[PRIVATE_TEMP_BUFF_SIZE];
extern CONV_LYR_CTX_T far conv_ctx;
extern FLT_MAP far *p_flt_input;
extern FIX_MAP far *p_fix_input;

#pragma DATA_SECTION(gemm_ctx, ".shared_ocm")
GEMM_CTX_T far gemm_ctx;


//----------------------------------------------------------------------------------

#pragma CODE_SECTION(DSP_vs_add16_unroll_4, ".text:optimized");
void DSP_vs_add16_unroll_4 (
    short * restrict x,   /* Input array of length nx  */
    short y,   			  /* Scalar value to be added */
    short * restrict r,   /* Output array of length nx. even though the output array can be same as input in this case(because it is pointwise op) using restrict to fool the compiler*/
    int              nx   /* Number of elements.       */
)
{
    int i;

    _nassert(((int)x & 4) ==0);
    _nassert(((int)r & 4) ==0);
    #pragma MUST_ITERATE(4,,4);
    #pragma UNROLL(4);

    for(i=0; i<nx; i++) {
        r[i] = x[i] + y;
    }
}

#pragma CODE_SECTION(DSPF_vs_add_unroll_4, ".text:optimized");
void DSPF_vs_add_unroll_4 (
    float * restrict x,   /* Input array of length nx  */
    float y,   			  /* Scalar to be added */
    float * restrict r,   /* Output array of length nx */
    int              nx   /* Number of elements.       */
)
{
    int i;

    _nassert(((int)x & 7) ==0);
    _nassert(((int)y & 7) ==0);
    _nassert(((int)r & 7) ==0);
    #pragma MUST_ITERATE(4,,4);
    #pragma UNROLL(4);

    for(i=0; i<nx; i++) {
        r[i] = x[i] + y;
    }
}

static inline split_gemm_row(int no_rows, int *subset_size, int *start_row) {
	int quo, rem;

	// Requirement: the total no_rows should be multiple of constraint factor.
	// This
	REL_ASSERT(no_rows % GEMM_CONV_IN_ROW_FACTOR == 0);

	quo = no_rows / (GEMM_CONV_IN_ROW_FACTOR * NO_CORES);
	rem = no_rows % (GEMM_CONV_IN_ROW_FACTOR * NO_CORES);
	//printf("C_%d : No rows = %d\n", core_id, no_rows);

	if(core_id < rem / GEMM_CONV_IN_ROW_FACTOR) {
		// first few cores will have extra rows
		*subset_size = (quo + 1) * GEMM_CONV_IN_ROW_FACTOR;
		*start_row = core_id *  (*subset_size);
	} else {
		*subset_size = quo * GEMM_CONV_IN_ROW_FACTOR;
		*start_row = rem + core_id *  (*subset_size);
	}

}
/*
 * GEMM based convolution using DSPLIB library
 * M : height of matrix a
 * N : width of matrix a == height of matrix b
 * P : width of matrix b
 * NOTE: does not support in-place processing.
 * Requirements
 * M   min = 2, Factor 2
 * N   min = 2, Factor 2
 * P   min = 4, Factor 4
 *
 */
void dsp_fix_gemm_conv(FIX_MAP * restrict p_a,
	FIX_MAP *restrict p_b,
	FIX_MAP * restrict p_bias,
	int M,
	int N,
	int P,
	int shift,
	FIX_MAP *restrict p_c) {

	int r, b_m, start_row;


	// take care of padding 0 if N is odd during map unrolling. Invalid read from the unrolled weight matrix is OK
	// since the corresponding pixel in unrolled feature map matrix is 0
	REL_ASSERT((N >= GEMM_CONV_IN_COL_FACTOR) && (N % GEMM_CONV_IN_COL_FACTOR == 0));
	REL_ASSERT((P >= GEMM_CONV_OUT_COL_FACTOR) && (P % GEMM_CONV_OUT_COL_FACTOR == 0));

	split_gemm_row(M, &b_m, &start_row);
	//printf("C_%d : start_row = %d\tno_rows = %d\n", core_id, start_row, b_m);
	// perform GEMM
	DSP_mat_mul(p_a + start_row * N, b_m, N, p_b, P, p_c + start_row * P, shift);

	// add bias
	for(r = start_row; r < start_row+ b_m; r++) {
		// NOTE: Both input and output are pointing to same location even though they are restrict pointers.
		// This should be fine since it is point-wise operation.
		DSP_vs_add16_unroll_4(p_c + r * P, p_bias[r], p_c + r * P, P);
	}
}

// GEMM based floating point convolution with multi-core support
void dsp_flt_gemm_conv(FLT_MAP *restrict p_a,
	FLT_MAP *restrict p_b,
	FLT_MAP *restrict p_bias,
	int M,
	int N,
	int P,
	FLT_MAP *restrict p_c) {

	int r, b_m, start_row;

	REL_ASSERT((N >= 2) && (N % 2 == 0));
	REL_ASSERT((P >= 4) && (P % 4 == 0));

	split_gemm_row(M, &b_m, &start_row);

	//printf("C_%d : start_row = %d\tno_rows = %d\n", core_id, start_row, b_m);
	DSPF_sp_mat_mul(p_a + start_row * N, b_m, N, p_b, P, p_c + start_row * P);

	// add bias
	for(r = start_row; r < start_row + b_m; r++) {
		// NOTE: Both input and output are pointing to same location even though they are restrict pointers.
		// This should be fine since it is point-wise operation.
		DSPF_vs_add_unroll_4(p_c + r * P, p_bias[r], p_c + r * P, P);
	}
}

/*
 * General matrix-matrix multiplication using DSPLIB library with multi-core support
 * M : height of matrix a
 * N : width of matrix a == height of matrix b
 * P : width of matrix b
 * NOTE: does not support in-place processing.
 * Requirements
 * M   min = 2, Factor 2
 * N   min = 2, Factor 2
 * P   min = 4, Factor 4
 *
 */
void dsp_fix_gemm(FIX_MAP * restrict p_a,
	FIX_MAP *restrict p_b,
	int M,
	int N,
	int P,
	int shift,
	FIX_MAP *restrict p_c) {

	int b_m, start_row;


	// take care of padding 0 if N is odd during map unrolling. Invalid read from the unrolled weight matrix is OK
	// since the corresponding pixel in unrolled feature map matrix is 0
	REL_ASSERT((N >= GEMM_CONV_IN_COL_FACTOR) && (N % GEMM_CONV_IN_COL_FACTOR == 0));
	REL_ASSERT((P >= GEMM_CONV_OUT_COL_FACTOR) && (P % GEMM_CONV_OUT_COL_FACTOR == 0));

	split_gemm_row(M, &b_m, &start_row);
	//printf("C_%d : start_row = %d\tno_rows = %d\n", core_id, start_row, b_m);
	// perform GEMM
	DSP_mat_mul(p_a + start_row * N, b_m, N, p_b, P, p_c + start_row * P, shift);
}

void dsp_flt_gemm(FLT_MAP *restrict p_a,
	FLT_MAP *restrict p_b,
	int M,
	int N,
	int P,
	FLT_MAP *restrict p_c) {

	int b_m, start_row;

	REL_ASSERT((N >= 2) && (N % 2 == 0));
	REL_ASSERT((P >= 4) && (P % 4 == 0));

	split_gemm_row(M, &b_m, &start_row);

	//printf("C_%d : start_row = %d\tno_rows = %d\n", core_id, start_row, b_m);
	DSPF_sp_mat_mul(p_a + start_row * N, b_m, N, p_b, P, p_c + start_row * P);

}
// Only called by master core
void setup_gemm_ctx(int r1, int c1, int c2) {
	int element_size, rem;

	// make sure that the final matrix dims are abiding constraints from low level API
	rem = r1 % GEMM_CONV_IN_ROW_FACTOR;
	if(rem != 0) {
		r1 += (GEMM_CONV_IN_ROW_FACTOR - rem);
	}

	rem = c1 % GEMM_CONV_IN_COL_FACTOR;
	if(rem != 0) {
		c1 += (GEMM_CONV_IN_COL_FACTOR - rem);
	}
	rem = c2 % GEMM_CONV_OUT_COL_FACTOR;
	if(rem != 0) {
		c2 += (GEMM_CONV_OUT_COL_FACTOR - rem);
	}

	gemm_ctx.r1 = r1;
	gemm_ctx.c1 = c1;
	gemm_ctx.c2 = c2;

	gemm_ctx.no_map_frac_bits = 8;
	gemm_ctx.no_ker_frac_bits = 0;

	element_size = sizeof(FLT_MAP);	// allocate extra buffer to initially generate random data

	gemm_ctx.p_input_1 = shared_malloc(gemm_ctx.r1 * gemm_ctx.c1 * element_size);
	gemm_ctx.p_input_2 = shared_malloc(gemm_ctx.c1 * gemm_ctx.c2 * element_size);
	gemm_ctx.p_output = shared_malloc(gemm_ctx.r1 * gemm_ctx.c2 * element_size);


}

// Only called by master core
void setup_conv_ctx(int input_height,
	int input_width, int no_inputs, int no_outputs,
	int K, int pad, int stride,
	int no_ker_frac_bits, int no_map_frac_bits) {

	int out_height, out_width, quo, rem, core, map, omap;

	out_height = (input_height + 2 * pad - K + 1 + stride - 1)/ stride;
	out_width = (input_width + 2 * pad - K + 1 + stride - 1)/ stride;
	conv_ctx.conv_info = (CONV_INFO_T){input_height, input_width, K, no_inputs, no_outputs, pad, stride, no_ker_frac_bits, no_map_frac_bits};

	conv_ctx.lyr_arith_mode = APP_ARITHMETIC_MODE == 1? FIXED_POINT : FLOAT_POINT;

	// input and output buffer allocation. Using same float buffer for fixed point.
	conv_ctx.p_flt_output = shared_malloc(out_height * out_width * no_outputs * sizeof(FLT_MAP));
	conv_ctx.p_fix_output = (FIX_MAP *)conv_ctx.p_flt_output;
	conv_ctx.p_flt_ker = shared_malloc(K * K * no_inputs * no_outputs * sizeof(FLT_KER));
	conv_ctx.p_fix_ker = (FIX_KER *)conv_ctx.p_flt_ker;
	conv_ctx.p_flt_bias = shared_malloc(no_outputs * sizeof(FLT_KER));
	conv_ctx.p_fix_bias = (FIX_KER *)conv_ctx.p_flt_bias;

	quo = conv_ctx.conv_info.no_outputs / NO_CORES;
	rem = conv_ctx.conv_info.no_outputs % NO_CORES;

	for(core = 0; core < NO_CORES; core++) {
		if(core < rem) {
			conv_ctx.no_maps[core] = quo + 1;
			conv_ctx.start_map[core]  = core * conv_ctx.no_maps[core];
		} else {
			conv_ctx.no_maps[core] = quo;
			conv_ctx.start_map[core]  = rem + core * conv_ctx.no_maps[core];
		}
	}

	p_flt_input = shared_malloc((input_height + 2*pad)*(input_width + 2*pad) * no_inputs * sizeof(FLT_MAP));
	p_fix_input = (FIX_MAP *)p_flt_input;

	// random input
	generate_random_data(p_flt_input, (input_height + 2*pad)*(input_width + 2*pad) * no_inputs, 123);
	generate_random_data(conv_ctx.p_flt_ker, K * K * no_inputs * no_outputs, 345);
	generate_random_data(conv_ctx.p_flt_bias, no_outputs, 321);

	if(conv_ctx.lyr_arith_mode == FIXED_POINT) {
		float_to_fix_data(p_flt_input, (input_height + 2*pad)*(input_width + 2*pad) * no_inputs, no_map_frac_bits, p_fix_input);
		float_to_fix_data(conv_ctx.p_flt_ker, K * K * no_inputs * no_outputs, no_ker_frac_bits, conv_ctx.p_fix_ker);
		float_to_fix_data(conv_ctx.p_flt_bias, no_outputs, no_map_frac_bits, conv_ctx.p_fix_bias);

		// rotate the fixed point kernel to cmpensate for the 180 deg rotation performed by the IMGLIB APIs
		for(omap = 0; omap < no_outputs; omap++) {
			for(map = 0; map < no_inputs; map++) {
				rotate_180(conv_ctx.p_fix_ker + omap * no_inputs * K * K + map * K * K, K, K);
			}
		}
	}
}


void run_gemm_bmark() {
	int gemm, i, common_mat_size;
	uint64_t start_time, end_time;
	float gops;
	FLT_MAP *p_out, *p_ref;

	gemm = 0;
	common_mat_size = GEMM_BMARK_START_SIZE;
	while (gemm < GEMM_BMARK_NO_CASES) {
		if(core_id != MASTER_CORE_ID) {
			// other cores wait for the gemm context initialized by the master
			// Making use of same global counter which is used for inference of images.
			wait_for_image_init(gemm);
			L1_CACHE_INV((void *)&gemm_ctx, L1_CACHE_LINE_SIZE, CACHE_WAIT);
		} else {
			// init the context and allocate buffers
			setup_gemm_ctx(common_mat_size, common_mat_size, common_mat_size);

			// random matrix for testing purposes
			generate_random_data((FLT_MAP *)gemm_ctx.p_input_2, gemm_ctx.c1 * gemm_ctx.c2, 123);

			if(APP_ARITHMETIC_MODE) {
				// set matrix 1 to be identity matrix so that we can compare the output to be same as matrix 2
				get_fix_eye_matrix(gemm_ctx.p_input_1, gemm_ctx.r1, gemm_ctx.c1);
				float_to_fix_data((FLT_MAP *)gemm_ctx.p_input_2, gemm_ctx.c1 * gemm_ctx.c2, gemm_ctx.no_map_frac_bits, (FIX_MAP *)gemm_ctx.p_input_2);
			} else {
				get_flt_eye_matrix((FLT_MAP *)gemm_ctx.p_input_1, gemm_ctx.r1, gemm_ctx.c1);
			}

			memset(gemm_ctx.p_output, 0, gemm_ctx.r1 * gemm_ctx.c2 * sizeof(FLT_MAP));
			printf("Matrix dim : MxNxP = %d x %d x %d\n", gemm_ctx.r1, gemm_ctx.c1, gemm_ctx.c2);
			toggle_image_init_flag(gemm);
			GET_TIME(&start_time);
		}

		// perform GEMM
		if(APP_ARITHMETIC_MODE) { // fixed point
			dsp_fix_gemm((FIX_MAP*)gemm_ctx.p_input_1,
				(FIX_MAP*)gemm_ctx.p_input_2,
				gemm_ctx.r1,
				gemm_ctx.c1,
				gemm_ctx.c2,
				gemm_ctx.no_ker_frac_bits,
				(FIX_MAP*)gemm_ctx.p_output
				);
		} else {
			dsp_flt_gemm((FLT_MAP*)gemm_ctx.p_input_1,
				(FLT_MAP*)gemm_ctx.p_input_2,
				gemm_ctx.r1,
				gemm_ctx.c1,
				gemm_ctx.c2,
				(FLT_MAP*)gemm_ctx.p_output
				);

		}
		// using same flags as the CNN app for data sync
		signal_lyr_completion(0);

		// wait for all cores to complete their portion of GEMM
		wait_for_maps(0);

		if(core_id == MASTER_CORE_ID) {
			GET_TIME(&end_time);
			// get the runtime
			PRINT_RUNTIME(start_time);
			gops = ((gemm_ctx.r1 * gemm_ctx.c1 * gemm_ctx.c2 * 2) * DSP_FREQ_IN_MHZ)/(1000.0 * (float)(end_time - start_time));
			printf("GOPS = %f\n", gops);

			// Compare the result. Since one matrix is identity matrix, the output must be = second matrix(only for square matrix case)
			if(APP_ARITHMETIC_MODE) {
				for(i = 0; i < gemm_ctx.r1 * gemm_ctx.c2; i++) {
					if(gemm_ctx.p_output[i] != gemm_ctx.p_input_2[i]) {
						printf("Data mismatch at Row = %d, col = %d : Org : %d\tAct : %d\n",
							i/gemm_ctx.c2, i%gemm_ctx.c2, gemm_ctx.p_input_2[i], gemm_ctx.p_output[i]);
						return;
					}
				}
			} else {
				p_out = (FLT_MAP *)gemm_ctx.p_output;
				p_ref = (FLT_MAP *)gemm_ctx.p_input_2;
				for(i = 0; i < gemm_ctx.r1 * gemm_ctx.c2; i++) {
					if(p_out[i] != p_ref[i]) {
						printf("Data mismatch at Row = %d, col = %d : Org : %f\tAct : %f\n",
							i/gemm_ctx.c2, i%gemm_ctx.c2, p_ref[i], p_out[i]);
						return;
					}
				}
			}
			// reset the memory manager
			reset_mem_manager();
			reset_layer_sync_cntr();
			toggle_image_init_flag(gemm);
		}
		common_mat_size *= 2;
		gemm++;
	}
}

void run_conv_bmark() {
	int conv, i, ops, out_h, out_w;
	uint64_t start_time, end_time;
	float gops;


	conv = 0;

	while (conv < 0) {
		if(core_id != MASTER_CORE_ID) {
			// other cores wait for the conv context initialized by the master
			// Making use of same global counter which is used for inference of images.
			wait_for_image_init(conv);
			L1_CACHE_INV((void *)&conv_ctx, L1_CACHE_LINE_SIZE, CACHE_WAIT);
		} else {
			// conv context and buffer init

			// signal init complete
			toggle_image_init_flag(conv);
			GET_TIME(&start_time);
		}

		// perform GEMM
		if(APP_ARITHMETIC_MODE) { // fixed point
			// fix conv layer
		} else {
			// float conv layer.
		}
		// using same flags as the CNN app for data sync
		signal_lyr_completion(0);

		// wait for all cores to complete their portion of GEMM
		wait_for_maps(0);

		if(core_id == MASTER_CORE_ID) {
			GET_TIME(&end_time);
			// get the runtime
			PRINT_RUNTIME(start_time);
			out_h = (conv_ctx.conv_info.map_h + 2 * conv_ctx.conv_info.pad - conv_ctx.conv_info.ker_size + 1 + conv_ctx.conv_info.stride - 1) / conv_ctx.conv_info.stride;
			out_w = (conv_ctx.conv_info.map_w + 2 * conv_ctx.conv_info.pad - conv_ctx.conv_info.ker_size + 1 + conv_ctx.conv_info.stride - 1) / conv_ctx.conv_info.stride;
			ops = out_h * out_w * conv_ctx.conv_info.no_outputs;
			gops = (ops * DSP_FREQ_IN_MHZ)/(1000.0 * (float)(end_time - start_time));
			printf("GOPS = %f\n", gops);

			// reset the memory manager
			reset_mem_manager();
			reset_layer_sync_cntr();
			toggle_image_init_flag(conv);
		}
		conv++;
	}
}
