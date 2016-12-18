/*
 * test_edma_module.c
 *
 *  Created on: 18 Dec 2016
 *      Author: Gopalakrishna Hegde, NTU Singapore
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ti/csl/csl_cacheAux.h>
#include <ti/csl/csl_tsc.h>
#include <ti/csl/csl_chip.h>
#include "edma_module.h"

#define EDMA_INSTANCE_NO		0
#define EDMA_CH_NO				0
#define EDMA_INTR_NO			0	// this same as TCC code
#define EDMA_CHANEL_PARAM_NO 	0
#define EDMA_REGION				CSL_EDMA3_REGION_GLOBAL
#define BUFF_SIZE				256
#define FIXED_PATTERN			255
#define DSP_FREQ_MHZ			1400

CSL_Edma3Handle edma_handle;
CSL_Edma3Obj edma_obj;
CSL_Edma3ChannelHandle edma_ch_handle;
CSL_Edma3ChannelObj edma_ch_obj;
CSL_Edma3CmdIntr edma_intr_obj;
CSL_Edma3ParamSetup edma_param_obj;

#pragma DATA_SECTION(src_buff, ".ddr_data");
Uint32 src_buff[BUFF_SIZE];
#pragma DATA_SECTION(dst_buff, ".msmc_data");
Uint32 dst_buff[BUFF_SIZE];

void edma_setup() {

	edma_hw_init();

	edma_handle = create_edma(EDMA_INSTANCE_NO, &edma_obj);

	edma_ch_handle = setup_edma_channel(EDMA_INSTANCE_NO, EDMA_CH_NO,
			EDMA_REGION, CSL_EDMA3_QUE_1, EDMA_CHANEL_PARAM_NO, &edma_ch_obj, FALSE);

	enable_edma_interrupt(edma_handle, &edma_intr_obj, EDMA_REGION, EDMA_INTR_NO);
}

void init_buffers() {
	int n;
	for(n = 0; n < BUFF_SIZE; n++) {
		dst_buff[n] = FIXED_PATTERN;
		src_buff[n] = n;
	}
}

Bool verify_transfer(int src_h, int src_w, int dst_h, int dst_w, int row_offset, int col_offset) {
	int r, c;
	Bool flag = TRUE;

	for(r = 0; r < dst_h; r++) {
		for(c = 0; c < dst_w; c++) {
			//printf("%d\t%d\n", dst_buff[r * dst_w + c], src_buff[(r + row_offset) * src_w + col_offset + c]);
			if(dst_buff[r * dst_w + c] != src_buff[(r + row_offset) * src_w + col_offset + c]) {
				flag = FALSE;
			}
		}
	}
	printf("Status = %d\n", flag);
	return flag;
}

void test_edma_module() {
	int src_w, dst_w, src_h, dst_h, row_offset, col_offset;
	uint64_t start_time, end_time;
	float bandwidth;

	uint32_t core_id = CSL_chipReadReg(CSL_CHIP_DNUM);

	// Enable timers for profiling.
	CSL_tscEnable();

	// Use full L2 as SRAM
	CACHE_setL2Size (CACHE_0KCACHE);

	// Use L1 as SRAM
	CACHE_setL1DSize(CACHE_L1_0KCACHE);

	edma_setup();

	init_buffers();

	setup_edma_array_param(edma_ch_handle,
		&edma_param_obj,
		EDMA_CHANEL_PARAM_NO,
		(Uint32)src_buff,
		(Uint32)dst_buff,
		BUFF_SIZE * sizeof(Uint32),
		EDMA_INTR_NO);

	start_time = CSL_tscRead();

	trigger_edma_channel_event(edma_ch_handle);

	wait_for_transfer(edma_handle, &edma_intr_obj, EDMA_INTR_NO);

	end_time = CSL_tscRead();

	if(!verify_transfer(1, BUFF_SIZE, 1, BUFF_SIZE, 0, 0)) {
		printf("1D Array EDMA A-sync TEST FAILED\n");
	} else {
		printf("1D Array EDMA A-sync TEST PASS\n");
	}
	bandwidth = (float)(BUFF_SIZE * sizeof(Uint32) * DSP_FREQ_MHZ)/(end_time - start_time);
	printf("Bandwidth = %.4f MB/s\n", bandwidth);

	src_h = 16;
	src_w = 16;
	dst_h = 8;
	dst_w = 8;
	row_offset = 4;
	col_offset = 4;
	init_buffers();
	// 2D matrix transfer
	setup_edma_mat_param(edma_ch_handle,
		&edma_param_obj,
		EDMA_CHANEL_PARAM_NO,
		(Uint32)src_buff + (row_offset  * src_w + col_offset) * sizeof(Uint32),
		(Uint32)dst_buff,
		dst_w * sizeof(Uint32),
		dst_h,
		src_w * sizeof(Uint32),
		dst_w * sizeof(Uint32),
		EDMA_INTR_NO);

	trigger_edma_channel_event(edma_ch_handle);

	wait_for_transfer(edma_handle, &edma_intr_obj, EDMA_INTR_NO);

	if(!verify_transfer(src_h, src_w, dst_h, dst_w, row_offset, col_offset)) {
		printf("2D Array EDMA A-sync TEST FAILED\n");
	} else {
		printf("2D Array EDMA A-sync TEST PASS\n");
	}

	close_edma(edma_handle, edma_ch_handle);
}
