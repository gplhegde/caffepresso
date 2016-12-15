/*
 * dsp_bmarks.c
 *
 *  Created on: 13 Dec 2016
 *      Author: Gopalakrishna Hegde
 */
#include <stdio.h>
#include "gemm_conv.h"

void run_dsp_bmarks() {

	run_gemm_bmark();

	//run_conv_bmark();
	printf("Benchmarks run is complete\n");
}
