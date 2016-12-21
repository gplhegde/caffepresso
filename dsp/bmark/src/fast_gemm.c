/*
 * fast_gemm.c
 *
 *  Created on: 19 Dec 2016
 *      Author: hgashok
 */
//---------------------------------------------------------------------------------------------------------------------
#include "fast_gemm.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ti/dsplib/dsplib.h>
#include <ti/csl/csl_tsc.h>
#include <ti/csl/csl_idmaAux.h>
#include "edma_module.h"
//---------------------------------------------------------------------------------------------------------------------
#define MAT_R1_SCALE_FACTOR		(16)
#define MAT_C1_SCALE_FACTOR		(16)
#define MAT_C2_SCALE_FACTOR		(16)
#define MAT_R1_SIZE				(L1_FLT_BLOCK_SIZE * MAT_R1_SCALE_FACTOR)
#define MAT_C1_SIZE				(L1_FLT_BLOCK_SIZE * MAT_C1_SCALE_FACTOR)
#define MAT_C2_SIZE				(L1_FLT_BLOCK_SIZE * MAT_C2_SCALE_FACTOR)
//---------------------------------------------------------------------------------------------------------------------
#define EDMA_INSTANCE_NO		0
#define EDMA_CH_A_NO			0
#define EDMA_CH_B_NO			1
#define EDMA_CH_C_NO			2
#define EDMA_CH_BBLK_NO			3
#define EDMA_INTR_A_NO			0	// this same as TCC code
#define EDMA_INTR_B_NO			1	// this same as TCC code
#define EDMA_INTR_C_NO			2	// this same as TCC code
#define EDMA_INTR_BBLK_NO		3	// this same as TCC code
#define EDMA_CH_A_PARAM_NO 		0
#define EDMA_CH_B_PARAM_NO 		1
#define EDMA_CH_C_PARAM_NO 		2
#define EDMA_CH_BBLK_PARAM_NO 	3
#define EDMA_REGION				CSL_EDMA3_REGION_GLOBAL
// IDMA channel objects
CSL_IDMA_IDMA1CONFIG idma_ch1_obj;
CSL_IDMA_STATUS idma_ch1_status;

// EDMA objects
CSL_Edma3Handle edma_handle;
CSL_Edma3Obj edma_obj;
CSL_Edma3ChannelHandle edma_ch_a_handle, edma_ch_b_handle, edma_ch_c_handle, edma_ch_bblk_handle;
CSL_Edma3ChannelObj edma_ch_a_obj, edma_ch_b_obj, edma_ch_c_obj, edma_ch_bblk_obj;
CSL_Edma3CmdIntr edma_intr_a_obj, edma_intr_b_obj, edma_intr_c_obj, edma_intr_bblk_obj;
CSL_Edma3ParamSetup edma_param_a_obj, edma_param_b_obj, edma_param_c_obj, edma_param_bblk_obj;
//---------------------------------------------------------------------------------------------------------------------
// All input and output matrices are in DDR
#pragma DATA_SECTION(mat_a, ".ddr_data");
float mat_a[MAT_R1_SIZE][MAT_C1_SIZE];
#pragma DATA_SECTION(mat_b, ".ddr_data");
float mat_b[MAT_C1_SIZE][MAT_C2_SIZE];
#pragma DATA_SECTION(mat_c, ".ddr_data");
float mat_c[MAT_R1_SIZE][MAT_C2_SIZE];
#pragma DATA_SECTION(mat_c_ref, ".ddr_data");
float mat_c_ref[MAT_R1_SIZE][MAT_C2_SIZE];

// Used to store a vertical panel of matrix B which is shared by all cores.
#pragma DATA_SECTION(panel_b_msmc_ping, ".msmc_data");
float panel_b_msmc_ping[MAX_MSMC_FLT_PANEL_HEIGHT][MAX_MSMC_FLT_PANEL_WIDTH];
#pragma DATA_SECTION(panel_b_msmc_pong, ".msmc_data");
float panel_b_msmc_pong[MAX_MSMC_FLT_PANEL_HEIGHT][MAX_MSMC_FLT_PANEL_WIDTH];

// Used to store output of all cores - 1 block per core and then DMA back to DDR
#pragma DATA_SECTION(nblk_c_msmc_ping, ".msmc_data");
float nblk_c_msmc_ping[NO_GEMM_CORES][L1_FLT_BLOCK_SIZE][L1_FLT_BLOCK_SIZE];
#pragma DATA_SECTION(nblk_c_msmc_pong, ".msmc_data");
float nblk_c_msmc_pong[NO_GEMM_CORES][L1_FLT_BLOCK_SIZE][L1_FLT_BLOCK_SIZE];

// Private L2 SRAM buffers for storing 2 horizontal panel of matrix A
#pragma DATA_SECTION(panel_a_l2_ping, ".l2_data");
float panel_a_l2_ping[MAX_L2_FLT_PANEL_HEIGHT][MAX_L2_FLT_PANEL_WIDTH];
#pragma DATA_SECTION(panel_a_l2_pong, ".l2_data");
float panel_a_l2_pong[MAX_L2_FLT_PANEL_HEIGHT][MAX_L2_FLT_PANEL_WIDTH];

// L2 SRAM buffer for buffering 2 blocks of matrix B from MSMC - on the way to L1 SRAM
#pragma DATA_SECTION(blk_b_l2_ping, ".l2_data");
float blk_b_l2_ping[MAX_L1_FLT_BLOCK_SIZE][MAX_L1_FLT_BLOCK_SIZE];
#pragma DATA_SECTION(blk_b_l2_pong, ".l2_data");
float blk_b_l2_pong[MAX_L1_FLT_BLOCK_SIZE][MAX_L1_FLT_BLOCK_SIZE];

// Private L1 SRAM buffers to store 2-blocks of mat a,b,c
#pragma DATA_SECTION(blk_a_l1_ping, ".l1_data");
float blk_a_l1_ping[MAX_L1_FLT_BLOCK_SIZE][MAX_L1_FLT_BLOCK_SIZE];
#pragma DATA_SECTION(blk_a_l1_pong, ".l1_data");
float blk_a_l1_pong[MAX_L1_FLT_BLOCK_SIZE][MAX_L1_FLT_BLOCK_SIZE];
#pragma DATA_SECTION(blk_b_l1_ping, ".l1_data");
float blk_b_l1_ping[MAX_L1_FLT_BLOCK_SIZE][MAX_L1_FLT_BLOCK_SIZE];
#pragma DATA_SECTION(blk_b_l1_pong, ".l1_data");
float blk_b_l1_pong[MAX_L1_FLT_BLOCK_SIZE][MAX_L1_FLT_BLOCK_SIZE];
#pragma DATA_SECTION(blk_c_l1_ping, ".l1_data");
float blk_c_l1_ping[MAX_L1_FLT_BLOCK_SIZE][MAX_L1_FLT_BLOCK_SIZE];
#pragma DATA_SECTION(blk_c_l1_pong, ".l1_data");
float blk_c_l1_pong[MAX_L1_FLT_BLOCK_SIZE][MAX_L1_FLT_BLOCK_SIZE];
// Buffer of partial block - block mult
#pragma DATA_SECTION(blk_par_prod, ".l1_data");
float blk_par_prod[MAX_L1_FLT_BLOCK_SIZE][MAX_L1_FLT_BLOCK_SIZE];
//---------------------------------------------------------------------------------------------------------------------

void edma_setup() {

	edma_hw_init();

	edma_handle = create_edma(EDMA_INSTANCE_NO, &edma_obj);

	edma_ch_a_handle = setup_edma_channel(EDMA_INSTANCE_NO, EDMA_CH_A_NO,
			EDMA_REGION, CSL_EDMA3_QUE_0, EDMA_CH_A_PARAM_NO, &edma_ch_a_obj, TRUE);

	edma_ch_b_handle = setup_edma_channel(EDMA_INSTANCE_NO, EDMA_CH_B_NO,
			EDMA_REGION, CSL_EDMA3_QUE_1, EDMA_CH_B_PARAM_NO, &edma_ch_b_obj, TRUE);

	edma_ch_c_handle = setup_edma_channel(EDMA_INSTANCE_NO, EDMA_CH_C_NO,
			EDMA_REGION, CSL_EDMA3_QUE_0, EDMA_CH_C_PARAM_NO, &edma_ch_c_obj, TRUE);

	edma_ch_bblk_handle = setup_edma_channel(EDMA_INSTANCE_NO, EDMA_CH_BBLK_NO,
			EDMA_REGION, CSL_EDMA3_QUE_1, EDMA_CH_BBLK_PARAM_NO, &edma_ch_bblk_obj, TRUE);

	enable_edma_interrupt(edma_handle, &edma_intr_a_obj, EDMA_REGION, EDMA_INTR_A_NO);
	enable_edma_interrupt(edma_handle, &edma_intr_b_obj, EDMA_REGION, EDMA_INTR_B_NO);
	enable_edma_interrupt(edma_handle, &edma_intr_c_obj, EDMA_REGION, EDMA_INTR_C_NO);
	enable_edma_interrupt(edma_handle, &edma_intr_bblk_obj, EDMA_REGION, EDMA_INTR_BBLK_NO);
}

void generate_random_data(float *p_data, int N, int seed) {
	int k, n;
	srand(seed);
	for (k = 0; k < N; k++) {
		n = rand() % 100;
		p_data[k] = (float)n;
	}
}



Bool verify_result(float *p_mat, float *p_ref, int R, int C) {
	int r, c;
	Bool flag = TRUE;

	for(r = 0; r < R; r++) {
		for(c = 0; c < C; c++) {
			if(p_mat[r * C + c] != p_ref[r * C + c]) {
				printf("Data mismatch at (%d, %d) : Act : %f\tRef : %f\n", r, c, p_mat[r * C + c], p_ref[r * C + c]);
				flag = FALSE;
				return flag;
			}
		}
	}
	return flag;
}

void flt_ref_gemm(float *p_mat_a, float *p_mat_b, int r1, int c1, int c2, float *p_mat_c) {
	int m, k, n;
	float sum;
	assert(r1 % L1_FLT_BLOCK_SIZE == 0);
	assert(c1 % L1_FLT_BLOCK_SIZE == 0);
	assert(c2 % L1_FLT_BLOCK_SIZE == 0);

	for(m = 0; m < r1; m++) {
		for(n = 0; n < c2; n++) {
			sum = 0.0;
			for(k = 0; k < c1; k++) {
				sum += p_mat_a[m * c1 + k] * p_mat_b[k * c2 + n];
			}
			p_mat_c[m * c2 + n] = sum;
		}
	}
}

void flt_mat_acc(float *p_mat, float *p_mat_acc, int R, int C) {
	int r, c;
	for(r = 0; r < R; r++) {
		for(c = 0; c < C; c++) {
			p_mat_acc[r * C + c] += p_mat[r * C + c];
		}
	}
}
void flt_blk_panel_gemm(float *p_mat_a, float *p_mat_b, int r1, int c1, int c2, float *p_mat_c) {
	int m, k, n, br, bc;
	float sum;
	assert(r1 % L1_FLT_BLOCK_SIZE == 0);
	assert(c1 % L1_FLT_BLOCK_SIZE == 0);
	assert(c2 % L1_FLT_BLOCK_SIZE == 0);

	for(m = 0; m < r1; m += L1_FLT_BLOCK_SIZE) {
		for(n = 0; n < c2; n += L1_FLT_BLOCK_SIZE) {
			// compute one block of output matrix
			// This is panel-panel multiplication
			for(br = 0; br < L1_FLT_BLOCK_SIZE; br++) {
				for(bc = 0; bc < L1_FLT_BLOCK_SIZE; bc++) {
					sum = 0;
					for(k = 0; k < c1; k++) {
						sum += p_mat_a[(m + br) * c1 + k] * p_mat_b[k * c2 + n + bc];
					}
					p_mat_c[(m + br) * c2 + n + bc]  = sum;
				}
			}
		}
	}
}

void flt_blk_blk_gemm(float *p_mat_a, float *p_mat_b, int r1, int c1, int c2, float *p_mat_c) {
	int m, k, n, br, bc, e;

	float temp_acc[L1_FLT_BLOCK_SIZE][L1_FLT_BLOCK_SIZE];

	assert(r1 % L1_FLT_BLOCK_SIZE == 0);
	assert(c1 % L1_FLT_BLOCK_SIZE == 0);
	assert(c2 % L1_FLT_BLOCK_SIZE == 0);

	for(m = 0; m < r1; m += L1_FLT_BLOCK_SIZE) {
		for(n = 0; n < c2; n += L1_FLT_BLOCK_SIZE) {
			// reset the block output buffer
			memset(temp_acc[0], 0, L1_FLT_BLOCK_SIZE * L1_FLT_BLOCK_SIZE * sizeof(float));
			for(k = 0; k < c1; k += L1_FLT_BLOCK_SIZE) {
				for(br = 0; br < L1_FLT_BLOCK_SIZE; br++) {
					for(bc = 0; bc < L1_FLT_BLOCK_SIZE; bc++) {
						for(e = 0; e < L1_FLT_BLOCK_SIZE; e++) {
							temp_acc[br][bc] += p_mat_a[(m + br) * c1 + k + e] * p_mat_b[(k + e) * c2 + n + bc];
						}
					}
				}
			}
			for(br = 0; br < L1_FLT_BLOCK_SIZE; br++) {
				for(bc = 0; bc < L1_FLT_BLOCK_SIZE; bc++) {
					p_mat_c[(m + br) * c2 + n + bc] = temp_acc[br][bc];
				}
			}
		}
	}
}

void transfer_data(float *p_src, float *p_dst, int no_rows, int no_cols, int src_ptr_inc, int dst_ptr_inc) {
	int row, col;
	//return;
	for(row = 0; row < no_rows; row++) {
		for(col = 0; col < no_cols; col++) {
			p_dst[col] = p_src[col];
		}
		p_src += src_ptr_inc;
		p_dst += dst_ptr_inc;
	}
}
void transfer_b_panel(float *p_src, float *p_dst, int no_rows, int no_cols, int src_ptr_inc, int dst_ptr_inc) {
	setup_edma_mat_param(edma_ch_b_handle,
		&edma_param_b_obj,
		EDMA_CH_B_PARAM_NO,
		(Uint32)p_src,
		(Uint32)p_dst,
		no_cols * sizeof(float),
		no_rows,
		src_ptr_inc * sizeof(float),
		dst_ptr_inc * sizeof(float),
		EDMA_INTR_B_NO);
	trigger_edma_channel_event(edma_ch_b_handle);
}
void transfer_a_panel(float *p_src, float *p_dst, int no_rows, int no_cols) {
	setup_edma_array_param(edma_ch_a_handle,
		&edma_param_a_obj,
		EDMA_CH_A_PARAM_NO,
		(Uint32)p_src,
		(Uint32)p_dst,
		no_rows * no_cols * sizeof(Uint32),
		EDMA_INTR_A_NO);
	trigger_edma_channel_event(edma_ch_a_handle);
}

void transfer_b_block(float *p_src, float *p_dst, int no_rows, int no_cols) {
	setup_edma_array_param(edma_ch_bblk_handle,
		&edma_param_bblk_obj,
		EDMA_CH_BBLK_PARAM_NO,
		(Uint32)p_src,
		(Uint32)p_dst,
		no_rows * no_cols * sizeof(float),
		EDMA_INTR_BBLK_NO);
	trigger_edma_channel_event(edma_ch_bblk_handle);
}
void transfer_c_block(float *p_src, float *p_dst, int no_rows, int no_cols, int src_ptr_inc, int dst_ptr_inc) {
	setup_edma_mat_param(edma_ch_c_handle,
		&edma_param_c_obj,
		EDMA_CH_C_PARAM_NO,
		(Uint32)p_src,
		(Uint32)p_dst,
		no_cols * sizeof(float),
		no_rows,
		src_ptr_inc * sizeof(float),
		dst_ptr_inc * sizeof(float),
		EDMA_INTR_C_NO);
	trigger_edma_channel_event(edma_ch_c_handle);
}

void idma_ch1_tx_non_blocking(Uint8 *src_buff, Uint8 *dst_buff, Uint16 no_bytes) {
	assert((Uint32)src_buff % 4 == 0);
	assert((Uint32)dst_buff % 4 == 0);

	idma_ch1_obj.source = (Uint32 *)src_buff;
	idma_ch1_obj.destn = (Uint32 *)dst_buff;
	idma_ch1_obj.count = no_bytes;
	idma_ch1_obj.intEnable = 0;
	idma_ch1_obj.priority = 0;

	CSL_IDMA_chan1TransferData(&idma_ch1_obj, FALSE);
}
void idma_ch1_tx_blocking(Uint8 *src_buff, Uint8 *dst_buff, Uint16 no_bytes) {
	assert((Uint32)src_buff % 4 == 0);
	assert((Uint32)dst_buff % 4 == 0);

	idma_ch1_obj.source = (Uint32 *)src_buff;
	idma_ch1_obj.destn = (Uint32 *)dst_buff;
	idma_ch1_obj.count = no_bytes;
	idma_ch1_obj.intEnable = 0;
	idma_ch1_obj.priority = 0;

	CSL_IDMA_chan1TransferData(&idma_ch1_obj, TRUE);
}

inline void wait_for_a_panel_tx() {
	wait_for_transfer(edma_handle, &edma_intr_a_obj, EDMA_INTR_A_NO);
}

void wait_for_b_panel_tx() {
	wait_for_transfer(edma_handle, &edma_intr_b_obj, EDMA_INTR_B_NO);
}

void wait_for_blk_a_tx() {
	return;
}
void wait_for_blk_b_tx() {
	wait_for_transfer(edma_handle, &edma_intr_bblk_obj, EDMA_INTR_BBLK_NO);
}
void wait_for_blk_c_tx() {
	wait_for_transfer(edma_handle, &edma_intr_c_obj, EDMA_INTR_C_NO);
}

void print_mat(float *p_mat, int R, int C) {
	int r, c;
	printf("--------------------------------------------\n");
	for(r = 0; r < R; r++) {
		for(c = 0; c < C; c++) {
			printf("%.0f\t", p_mat[r * C +c]);
		}
		printf("\n");
	}
	printf("--------------------------------------------\n");
}
#if NO_GEMM_CORES > 1
#error "Fast GEMM is not supported for multi-core as of now"
#endif

void flt_blk_blk_fgemm(float *p_mat_a, float *p_mat_b, int r1, int c1, int c2, float *p_mat_c) {
	int blk, blk_x, blk_y, no_blk_rows, no_blk_cols, no_patches;
	uint32_t core_id;
	float *p_l1_blk_a[2], *p_l1_blk_b[2], *p_l1_blk_c[2];
	float *p_l2_blk_b[2], *p_l2_panel_a[2], *p_blk_par_prod;
	float *p_msmc_nblk_c[2], *p_msmc_panel_b[2];
	float *p_a, *p_b, *p_c;
	core_id = 0;
	assert(r1 % L1_FLT_BLOCK_SIZE == 0);
	assert(c1 % L1_FLT_BLOCK_SIZE == 0);
	assert(c2 % L1_FLT_BLOCK_SIZE == 0);
	assert(c1 <= MAX_L2_FLT_PANEL_WIDTH);

	p_l2_panel_a[0] = panel_a_l2_ping[0]; p_l2_panel_a[1] = panel_a_l2_pong[0];
	p_msmc_panel_b[0] = panel_b_msmc_ping[0]; p_msmc_panel_b[1] = panel_b_msmc_pong[0];
	p_l1_blk_a[0] = blk_a_l1_ping[0]; p_l1_blk_a[1] = blk_a_l1_pong[0];
	p_l1_blk_b[0] = blk_b_l1_ping[0]; p_l1_blk_b[1] = blk_b_l1_pong[0];
	p_l1_blk_c[0] = blk_c_l1_ping[0]; p_l1_blk_c[1] = blk_c_l1_pong[0];
	p_l2_blk_b[0] = blk_b_l2_ping[0]; p_l2_blk_b[1] = blk_b_l2_pong[0];
	p_msmc_nblk_c[0] = nblk_c_msmc_ping[core_id][0]; p_msmc_nblk_c[1] = nblk_c_msmc_pong[core_id][0];
	p_blk_par_prod = blk_par_prod[0];
	// NOTE: take care of cache sync every time after DMA and shared data access.

	// Pre-transfer data for double buffering purpose.
	no_blk_rows = r1 / L1_FLT_BLOCK_SIZE;
	no_blk_cols = c2 / L1_FLT_BLOCK_SIZE;
	no_patches = c1 / L1_FLT_BLOCK_SIZE;
	p_c = p_mat_c;

	p_a = p_mat_a;
	// All : Transfer 1 horizontal panel of A into L2 SRAM buffer
	transfer_a_panel(p_a, p_l2_panel_a[0], L1_FLT_BLOCK_SIZE, c1);
	p_a += (L1_FLT_BLOCK_SIZE * c1);
	// All : Wait for transfer of panel A into L2 SRAM
	wait_for_a_panel_tx();

	/*printf("MAt A\n");
	print_mat(p_mat_a, r1, c1);
	printf("MAt B\n");
	print_mat(p_mat_b, c1, c2);*/
	// Enter compute loop
	// Loop over all blocks in output row dimension.
	for(blk_y = 0; blk_y < no_blk_rows; blk_y++) {
		p_b = p_mat_b;
		p_c = p_mat_c +  blk_y * c2 * L1_FLT_BLOCK_SIZE;
		// M : Transfer 1 vertical panel of B into MSMC RAM buffer.
		if(core_id == MASTER_CORE_ID) {
			transfer_b_panel(p_b, p_msmc_panel_b[0], c1, L1_FLT_BLOCK_SIZE, c2, L1_FLT_BLOCK_SIZE);
		}
		p_b += L1_FLT_BLOCK_SIZE;

		// All: Initiate transfer of panel A into ping buffer if it is not the last horizontal panel for the core.
		if( blk_y < no_blk_rows - 1) {
			transfer_a_panel(p_a, p_l2_panel_a[(blk_y + 1) % 2], L1_FLT_BLOCK_SIZE, c1);
		}
		// Wait for panel B transfer
		wait_for_b_panel_tx();

		// Loop over all blocks in output row dimension.
		for(blk_x = 0; blk_x < no_blk_cols; blk_x++) {
			// M : Initiate transfer of panel B into pong buffer if it is not the last vertical panel of the matrix B/C
			if(blk_x < no_blk_cols - 1 && core_id == MASTER_CORE_ID) {
				transfer_b_panel(p_b, p_msmc_panel_b[(blk_x + 1) % 2], c1, L1_FLT_BLOCK_SIZE, c2, L1_FLT_BLOCK_SIZE);
			}
			//print_mat(p_msmc_panel_b[blk_x % 2], c1, L1_FLT_BLOCK_SIZE);
			// All : Tx 1 block of matrix A into L1 from L2 using IDMA - ping


			// All : Tx 1 block of matrix B into L1 from MSMC via L2 - into ping
			transfer_b_block(p_msmc_panel_b[blk_x % 2], p_l2_blk_b[0], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE);
			transfer_data(p_l2_panel_a[blk_y % 2], p_l1_blk_a[0], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, c1, L1_FLT_BLOCK_SIZE);
			wait_for_blk_b_tx();
			//transfer_data(p_l2_blk_b[0], p_l1_blk_b[0], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE);
			idma_ch1_tx_blocking(p_l2_blk_b[0], p_l1_blk_b[0], L1_FLT_BLOCK_SIZE * L1_FLT_BLOCK_SIZE * sizeof(float));

			// reset the block accum C
			memset(p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2], 0, L1_FLT_BLOCK_SIZE * L1_FLT_BLOCK_SIZE * sizeof(float));

			// wait for block A and B transfer
			wait_for_blk_a_tx();


			// All : loop over the input row dimension of A / col dim of B with block size step.
			//printf("Computing output block (%d, %d)\n", blk_y, blk_x);
			for(blk = 0; blk < no_patches; blk++) {
				if(blk < no_patches - 1) {
					// All: Init Tx of 1 block of B from MSMC to L1 via L2 if not the last block - EDMA + IDMA -- there is single IDMA channel, how to deal with it?
					transfer_b_block(p_msmc_panel_b[blk_x % 2] + (blk + 1) * L1_FLT_BLOCK_SIZE * L1_FLT_BLOCK_SIZE,
						p_l2_blk_b[(blk + 1) % 2], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE);
					// All: Init Tx of 1 block of A from L2 to L1 pong buffer if not the last block. - IDMA
					transfer_data(p_l2_panel_a[blk_y % 2] + (blk + 1) * L1_FLT_BLOCK_SIZE,
						p_l1_blk_a[(blk + 1) % 2], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, c1, L1_FLT_BLOCK_SIZE);
				}
				//print_mat(p_l1_blk_a[blk % 2], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE);
				//print_mat(p_l1_blk_b[blk % 2], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE);

				//=============Core compute block using DSPLIB==============
				// Compute block X block using DSPLIB API - all inputs and output into buffer in L1
				//flt_ref_gemm(p_l1_blk_a[blk % 2], p_l1_blk_b[blk % 2], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, p_blk_par_prod);
				DSPF_sp_mat_mul(p_l1_blk_a[blk % 2], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, p_l1_blk_b[blk % 2], L1_FLT_BLOCK_SIZE, p_blk_par_prod);
				// Add the partial result to the block accumulator C
				flt_mat_acc(p_blk_par_prod, p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2], L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE);

				/*DSPF_sp_mat_mul_gemm(p_l1_blk_a[blk % 2], 1, L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE,
					p_l1_blk_b[blk % 2], L1_FLT_BLOCK_SIZE, p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2]);*/
				//==========================================================

				// Wait for block of A and B to be transferred into L1
				// OR
				// Wait for block of A to be transferred into L1 and block of B to be into L2 and then IDMA block B into L1 since there is 1 IDMA channel
				if(blk < no_patches - 1) {
					wait_for_blk_a_tx();
					wait_for_blk_b_tx();
					idma_ch1_tx_blocking(p_l2_blk_b[(blk + 1) % 2], p_l1_blk_b[(blk + 1) % 2], L1_FLT_BLOCK_SIZE * L1_FLT_BLOCK_SIZE * sizeof(float));
					CSL_IDMA_chan1Wait();
				}
			}
			// write the block of result into MSMC - may be via L2 using IDMA + EDMA
			transfer_data(p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2], p_msmc_nblk_c[(blk_y * no_blk_cols + blk_x) % 2],
				L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE);

			// wait for all the blocks to transfer results into MSMC
			// setup NO_CORES DMA channels to transfer n blocks of C into different locations / setup a chained DMA
			if(core_id == MASTER_CORE_ID) {
				transfer_data(p_msmc_nblk_c[(blk_y * no_blk_cols + blk_x) % 2], p_c, L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, L1_FLT_BLOCK_SIZE, c2);
			}
			p_c += L1_FLT_BLOCK_SIZE;

			// wait for completion of panel B into MSMC
			if(blk_x < no_blk_cols - 1 && core_id == MASTER_CORE_ID) {
				wait_for_b_panel_tx();
			}
			p_b += L1_FLT_BLOCK_SIZE;
		}
		if(blk_y < no_blk_rows - 1) {
			// wait for completion of panel A into L2
			wait_for_a_panel_tx();
		}
		p_a += (L1_FLT_BLOCK_SIZE * c1);
	}
}

void run_flt_fast_gemm_bmark() {
	float gops;
	uint64_t start_time, end_time;

	edma_setup();

	generate_random_data(mat_a[0], MAT_R1_SIZE * MAT_C1_SIZE, 123);
	generate_random_data(mat_b[0], MAT_C1_SIZE * MAT_C2_SIZE, 456);

	//flt_ref_gemm(mat_a[0], mat_b[0], MAT_R1_SIZE, MAT_C1_SIZE, MAT_C2_SIZE, mat_c_ref[0]);
	start_time = CSL_tscRead();
	//flt_blk_panel_gemm(mat_a[0], mat_b[0], MAT_R1_SIZE, MAT_C1_SIZE, MAT_C2_SIZE, mat_c[0]);


	flt_blk_blk_fgemm(mat_a[0], mat_b[0], MAT_R1_SIZE, MAT_C1_SIZE, MAT_C2_SIZE, mat_c[0]);
	end_time = CSL_tscRead();

	/*if(verify_result(mat_c[0], mat_c_ref[0], MAT_R1_SIZE, MAT_C2_SIZE)) {
		printf("TEST SUCCESS\n");
	} else {
		printf("TEST FAIL\n");
	}*/
	gops = (MAT_R1_SIZE * MAT_C1_SIZE * MAT_C2_SIZE * 2 * 1.4)/(end_time - start_time);
	printf("GOPS = %.4f\n", gops);

}
void run_fast_gemm_bmark() {
	run_flt_fast_gemm_bmark();
}
