/*
 * main.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ti/csl/csl_idmaAux.h>
#include <ti/csl/csl_cacheAux.h>
#include <ti/csl/csl_tsc.h>
#include <ti/csl/csl_chip.h>

#define MAX_TIMER_CNT	(0xFFFFFFFFFFFFFFFF)
#define DSP_FREQ_IN_MHZ			1400

#define BUFF_SIZE	256
#define IDMA_CH_NO	1

#pragma DATA_SECTION(src_buff, ".l1_data")
Uint32 src_buff[BUFF_SIZE];

#pragma DATA_SECTION(dst_buff, ".l2_data")
Uint32 dst_buff[BUFF_SIZE];

CSL_IDMA_IDMA1CONFIG idma_ch1_ctx;
CSL_IDMA_STATUS idma_ch1_status;

uint32_t core_id;

void inline GET_TIME(uint64_t *time) {
	*time = CSL_tscRead();
}

void PRINT_RUNTIME(uint64_t start_time) {
	uint64_t end_time, total_time;
	end_time = CSL_tscRead();

	// check for single overflow condition. This is not possible since the counter will not overflow for ~417 years with 1.4GHz clock rate!
	if(end_time < start_time) {
		total_time = MAX_TIMER_CNT - start_time + end_time;
	} else {
		total_time = end_time - start_time;
	}

	printf("Runtime = %f usec\n", (float)total_time / DSP_FREQ_IN_MHZ);
}
void init_buffers() {
	int n;

	for(n = 0; n < BUFF_SIZE; n++) {
		src_buff[n] = (uint32_t)rand();
		dst_buff[n] = 0xDEADBEEF;
	}
}

Bool test_tx_result() {
	int n;
	Bool status = TRUE;

	for(n = 0; n < BUFF_SIZE; n++) {
		if(dst_buff[n] != src_buff[n]) {
			status = FALSE;
			break;
		}
	}
	return status;
}

void dsp_core_setup() {

	core_id = CSL_chipReadReg(CSL_CHIP_DNUM);

	// Enable timers for profiling.
	CSL_tscEnable();

	// We will not use L2 cache. We will manage L2 RAM to keep local variables specific to the core.
	CACHE_setL2Size (CACHE_0KCACHE);

	// Use L1 SRAM in 16kB + 16kB mode, 16kB -> SRAM and 16kB cache.
	CACHE_setL1DSize(CACHE_L1_16KCACHE);
}

void start_idma(uint32_t wait) {
	idma_ch1_ctx.source = (Uint32 *)src_buff;
	idma_ch1_ctx.destn = (Uint32 *)dst_buff;
	idma_ch1_ctx.count = BUFF_SIZE * sizeof(Uint32);
	idma_ch1_ctx.intEnable = 0;
	idma_ch1_ctx.priority = 0;

	CSL_IDMA_chan1TransferData(&idma_ch1_ctx, wait);
}

void wait_for_completion() {
	CSL_IDMA_chan1Wait();
}

int main(void) {
	uint64_t start_time;
	dsp_core_setup();

	init_buffers();
	
	GET_TIME(&start_time);
	start_idma(FALSE);

	wait_for_completion();

	PRINT_RUNTIME(start_time);

	if(!test_tx_result()) {
		printf("DATA MISMATCH : TEST FAILED\n");
	} else {
		printf("TEST PASS\n");
	}

	return 0;
}

