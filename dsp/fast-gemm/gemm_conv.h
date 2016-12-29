/*
 * gemm_conv.h
 *
 *  Created on: 13 Dec 2016
 *      Author: Gopalakrishna Hegde
 */

#ifndef BMARK_INC_GEMM_CONV_H_
#define BMARK_INC_GEMM_CONV_H_
#include <stdint.h>
#include "config.h"

#define GEMM_ARITHMETIC_MODE	0
// Output matrix row must be multiple of 2
#define GEMM_CONV_IN_ROW_FACTOR	(2)

#define GEMM_CONV_IN_COL_FACTOR	(2)

#define GEMM_CONV_OUT_COL_FACTOR	(4)

typedef struct {
	FIX_MAP *p_input_1;
	FIX_MAP *p_input_2;
	FIX_MAP *p_output;
	FIX_MAP *p_bias;
	uint32_t r1;
	uint32_t c1;
	uint32_t c2;
	int no_map_frac_bits;
	int no_ker_frac_bits;
} GEMM_CTX_T;

void run_conv_bmark();

void run_gemm_bmark();

#endif /* BMARK_INC_GEMM_CONV_H_ */
