/*
 * gemm_conv.c
 *
 *  Created on: 13 Dec 2016
 *      Author: Gopalakrishna Hegde
 */
#include "gemm_conv.h"
#include <ti/dsplib/dsplib.h>
#include <ti/csl/csl_tsc.h>
#include "data_sync.h"
#include <stdio.h>
#include "config.h"
#include "mem_manager.h"


#define GEMM_BMARK_START_SIZE	(16)
#define GEMM_BMARK_NO_CASES		(7)
#define MAX_MAT_SIZE			(1024)

extern unsigned int core_id;

#pragma DATA_SECTION(gemm_ctx, ".msmc_data")
GEMM_CTX_T gemm_ctx;


//----------------------------------------------------------------------------------
static inline split_gemm_row(int no_rows, int *subset_size, int *start_row) {
	int quo, rem;

	// Requirement: the total no_rows should be multiple of constraint factor.
	// This
	REL_ASSERT(no_rows % GEMM_CONV_IN_ROW_FACTOR == 0);

	quo = no_rows / (GEMM_CONV_IN_ROW_FACTOR * NO_GEMM_CORES);
	rem = no_rows % (GEMM_CONV_IN_ROW_FACTOR * NO_GEMM_CORES);
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
	L1_CACHE_WB(p_c + start_row * P, b_m * P * sizeof(FIX_MAP), CACHE_WAIT);
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
	L1_CACHE_WB(p_c + start_row * P, b_m * P * sizeof(FLT_MAP), CACHE_WAIT);
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
	L1_CACHE_WB(p_c + start_row * P, b_m * P * sizeof(FIX_MAP), CACHE_WAIT);
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

	L1_CACHE_WB(p_c + start_row * P, b_m * P * sizeof(FLT_MAP), CACHE_WAIT);

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

	element_size = sizeof(DTYPE);	// allocate extra buffer to initially generate random data

	gemm_ctx.p_input_1 = ext_malloc(gemm_ctx.r1 * gemm_ctx.c1 * element_size);
	gemm_ctx.p_input_2 = ext_malloc(gemm_ctx.c1 * gemm_ctx.c2 * element_size);
	gemm_ctx.p_output = ext_malloc(gemm_ctx.r1 * gemm_ctx.c2 * element_size);
	gemm_ctx.p_bias = ext_malloc(gemm_ctx.r1  * element_size);


}

#define COMPARE_RESULT 0
void run_gemm_bmark() {
	int gemm, i, common_mat_size;
	uint64_t start_time, end_time, cycles;
	double gops, runtime;
	FLT_MAP *p_out, *p_ref;

	gemm = 0;
	common_mat_size = GEMM_BMARK_START_SIZE;
	printf("R1, C1, C2, CYCLES, RUNTIME(us), GOPS\n");
	while (common_mat_size <= MAX_MAT_SIZE) {
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

#if FIXED_POINT_GEMM
				// set matrix 1 to be identity matrix so that we can compare the output to be same as matrix 2
				get_fix_eye_matrix(gemm_ctx.p_input_1, gemm_ctx.r1, gemm_ctx.c1);
				float_to_fix_data((FLT_MAP *)gemm_ctx.p_input_2, gemm_ctx.c1 * gemm_ctx.c2, gemm_ctx.no_map_frac_bits, (FIX_MAP *)gemm_ctx.p_input_2);
#else

				get_flt_eye_matrix((FLT_MAP *)gemm_ctx.p_input_1, gemm_ctx.r1, gemm_ctx.c1);
#endif

			memset(gemm_ctx.p_output, 0, gemm_ctx.r1 * gemm_ctx.c2 * sizeof(DTYPE));

			toggle_image_init_flag(gemm);
			start_time = CSL_tscRead();
		}

		// perform GEMM
#if FIXED_POINT_GEMM
			dsp_fix_gemm_conv((FIX_MAP*)gemm_ctx.p_input_1,
				(FIX_MAP*)gemm_ctx.p_input_2,
				(FIX_MAP *)gemm_ctx.p_bias,
				gemm_ctx.r1,
				gemm_ctx.c1,
				gemm_ctx.c2,
				gemm_ctx.no_ker_frac_bits,
				(FIX_MAP*)gemm_ctx.p_output
				);
#else
			dsp_flt_gemm_conv((FLT_MAP*)gemm_ctx.p_input_1,
				(FLT_MAP*)gemm_ctx.p_input_2,
				(FLT_MAP *)gemm_ctx.p_bias,
				gemm_ctx.r1,
				gemm_ctx.c1,
				gemm_ctx.c2,
				(FLT_MAP*)gemm_ctx.p_output
				);

#endif
		// using same flags as the CNN app for data sync
		signal_lyr_completion(0);

		// wait for all cores to complete their portion of GEMM
		wait_for_maps(0);

		if(core_id == MASTER_CORE_ID) {
			end_time = CSL_tscRead();
			// get the runtime
			L1_CACHE_INV(gemm_ctx.p_output, gemm_ctx.r1 * gemm_ctx.c2 * sizeof(FLT_MAP), CACHE_WAIT);
			cycles = end_time - start_time;
			runtime = (float)cycles / DSP_FREQ_MHZ;
			//gops = ((gemm_ctx.r1 * gemm_ctx.c1 * gemm_ctx.c2 * 2) * DSP_FREQ_GHZ)/((float)(end_time - start_time));
			gops = gemm_ctx.r1/(float)cycles;
			gops *= gemm_ctx.c1;
			gops *= (gemm_ctx.c2 * 2 * DSP_FREQ_GHZ);

			printf("%d, %d, %d, %lld, %.4f, %.4f\n", gemm_ctx.r1, gemm_ctx.c1, gemm_ctx.c2, cycles, runtime, gops);
#if COMPARE_RESULT
			// Compare the result. Since one matrix is identity matrix, the output must be = second matrix(only for square matrix case)
#if FIXED_POINT_GEMM
				for(i = 0; i < gemm_ctx.r1 * gemm_ctx.c2; i++) {
					if(gemm_ctx.p_output[i] != gemm_ctx.p_input_2[i]) {
						printf("Data mismatch at Row = %d, col = %d : Org : %d\tAct : %d\n",
							i/gemm_ctx.c2, i%gemm_ctx.c2, gemm_ctx.p_input_2[i], gemm_ctx.p_output[i]);
						return;
					}
				}
#else
				p_out = (FLT_MAP *)gemm_ctx.p_output;
				p_ref = (FLT_MAP *)gemm_ctx.p_input_2;
				for(i = 0; i < gemm_ctx.r1 * gemm_ctx.c2; i++) {
					if(p_out[i] != p_ref[i]) {
						printf("Data mismatch at Row = %d, col = %d : Org : %f\tAct : %f\n",
							i/gemm_ctx.c2, i%gemm_ctx.c2, p_ref[i], p_out[i]);
						return;
					}
				}
#endif
#endif
			// reset the memory manager
			reset_mem_manager();
			reset_layer_sync_cntr();
			toggle_image_init_flag(gemm);
		}
		common_mat_size += (GEMM_CONV_OUT_COL_FACTOR*2);
		gemm++;
	}
}

