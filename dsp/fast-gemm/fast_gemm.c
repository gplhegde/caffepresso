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
#include "config.h"
#include "data_sync.h"
#include "gemm_conv.h"
#include "conv_gemm_params.h"
#include "caffe_proto_params.h"

//---------------------------------------------------------------------------------------------------------------------

#define MAT_R1_SIZE				(256)
#define MAT_C1_SIZE				(MAX_L2_FIX_PANEL_WIDTH)
#define MAT_C2_SIZE				(21000)
#define MAX_INPUT_MAP_BUFF_SIZE	(384 * 512)
//---------------------------------------------------------------------------------------------------------------------


extern unsigned int core_id;

// IDMA channel objects
CSL_IDMA_IDMA1CONFIG idma_ch1_obj;
CSL_IDMA_STATUS idma_ch1_status;


//---------------------------------------------------------------------------------------------------------------------
// All input and output matrices are in DDR
#pragma DATA_SECTION(mat_a, ".ddr_data");
#pragma DATA_ALIGN(mat_a, 8);
uint8_t mat_a[MAT_R1_SIZE * MAT_C1_SIZE * ELEMENT_SIZE];
#pragma DATA_SECTION(mat_b, ".ddr_data");
#pragma DATA_ALIGN(mat_b, 8);
uint8_t mat_b[MAT_C1_SIZE * MAT_C2_SIZE * ELEMENT_SIZE];
// Since the output matrix in the normal conv based approach, mimic the same here
#pragma DATA_SECTION(mat_c, ".ddr_data");
#pragma DATA_ALIGN(mat_c, 8);
uint8_t mat_c[MAT_R1_SIZE * MAT_C2_SIZE * ELEMENT_SIZE];

#pragma DATA_SECTION(mat_c_ref, ".ddr_data");
#pragma DATA_ALIGN(mat_c_ref, 8);
uint8_t mat_c_ref[MAT_R1_SIZE * MAT_C2_SIZE * ELEMENT_SIZE];

#pragma DATA_SECTION(bias_array, ".ddr_data");
#pragma DATA_ALIGN(bias_array, 8);
uint8_t bias_array[512 * ELEMENT_SIZE];

// Place to store input maps. Since we store input maps and output maps on the MSMC in normal conv based approach,
// mimic the same thing here. However, we cannot afford to store unrolled input matrices on MSMC. It will be
// on DDR (mat_b)
#pragma DATA_SECTION(conv_input_maps, ".ddr_data");
#pragma DATA_ALIGN(conv_input_maps, 8);
uint8_t conv_input_maps[MAX_INPUT_MAP_BUFF_SIZE * ELEMENT_SIZE];
// Used to store a vertical panel of matrix B which is shared by all cores.

#pragma DATA_SECTION(panel_b_msmc_ping, ".msmc_data");
#pragma DATA_ALIGN(panel_b_msmc_ping, 8);
uint8_t panel_b_msmc_ping[MAX_MSMC_PANEL_HEIGHT][MAX_MSMC_PANEL_WIDTH * ELEMENT_SIZE];
#pragma DATA_SECTION(panel_b_msmc_pong, ".msmc_data");
#pragma DATA_ALIGN(panel_b_msmc_pong, 8);
uint8_t panel_b_msmc_pong[MAX_MSMC_PANEL_HEIGHT][MAX_MSMC_PANEL_WIDTH * ELEMENT_SIZE];

// Private L2 SRAM buffers for storing 2 horizontal panel of matrix A
#pragma DATA_SECTION(panel_a_l2_ping, ".l2_data");
#pragma DATA_ALIGN(panel_a_l2_ping, 8);
uint8_t panel_a_l2_ping[MAX_L2_PANEL_HEIGHT][MAX_L2_PANEL_WIDTH * ELEMENT_SIZE];
#pragma DATA_SECTION(panel_a_l2_pong, ".l2_data");
#pragma DATA_ALIGN(panel_a_l2_pong, 8);
uint8_t panel_a_l2_pong[MAX_L2_PANEL_HEIGHT][MAX_L2_PANEL_WIDTH * ELEMENT_SIZE];

//----------------------------
// Private L1 SRAM buffers to store 2-blocks of mat a,b,c
#pragma DATA_SECTION(blk_a_l1_ping, ".l1_data");
#pragma DATA_ALIGN(blk_a_l1_ping, 8);
uint8_t blk_a_l1_ping[MAX_L1_BLOCK_SIZE][MAX_L1_BLOCK_SIZE  *ELEMENT_SIZE];
#pragma DATA_SECTION(blk_a_l1_pong, ".l1_data");
#pragma DATA_ALIGN(blk_a_l1_pong, 8);
uint8_t blk_a_l1_pong[MAX_L1_BLOCK_SIZE][MAX_L1_BLOCK_SIZE * ELEMENT_SIZE];
#pragma DATA_SECTION(blk_b_l1_ping, ".l1_data");
#pragma DATA_ALIGN(blk_b_l1_ping, 8);
uint8_t blk_b_l1_ping[MAX_L1_BLOCK_SIZE][MAX_L1_BLOCK_SIZE * ELEMENT_SIZE];
#pragma DATA_SECTION(blk_b_l1_pong, ".l1_data");
#pragma DATA_ALIGN(blk_b_l1_pong, 8);
uint8_t blk_b_l1_pong[MAX_L1_BLOCK_SIZE][MAX_L1_BLOCK_SIZE * ELEMENT_SIZE];
#pragma DATA_SECTION(blk_c_l1_ping, ".l1_data");
#pragma DATA_ALIGN(blk_c_l1_ping, 8);
uint8_t blk_c_l1_ping[MAX_L1_BLOCK_SIZE][MAX_L1_BLOCK_SIZE * ELEMENT_SIZE];
#pragma DATA_SECTION(blk_c_l1_pong, ".l1_data");
#pragma DATA_ALIGN(blk_c_l1_pong, 8);
uint8_t blk_c_l1_pong[MAX_L1_BLOCK_SIZE][MAX_L1_BLOCK_SIZE * ELEMENT_SIZE];
// Buffer of partial block - block mult
#pragma DATA_SECTION(blk_par_prod, ".l1_data");
#pragma DATA_ALIGN(blk_par_prod, 8);
uint8_t blk_par_prod[MAX_L1_BLOCK_SIZE][MAX_L1_BLOCK_SIZE * ELEMENT_SIZE];

//---------------------------------------------------------------------------------------------------------------------
#pragma DATA_SECTION(gemm_conv_ctx, ".msmc_data");
GEMM_CTX_T gemm_conv_ctx;

Bool verify_result(DTYPE *p_mat, DTYPE *p_ref, int R, int C) {
	int r, c;
	Bool flag = TRUE;

	for(r = 0; r < R; r++) {
		for(c = 0; c < C; c++) {
#if FIXED_POINT_GEMM
			if(abs(p_mat[r * C + c] - p_ref[r * C + c]) > 1) {
				printf("Data mismatch at (%d, %d) : Act : %d\tRef : %d\n", r, c, p_mat[r * C + c], p_ref[r * C + c]);
				flag = FALSE;
				return flag;
			}
#else
			if(fabs(p_mat[r * C + c] - p_ref[r * C + c]) > 1e-4) {
				printf("Data mismatch at (%d, %d) : Act : %f\tRef : %f\n", r, c, p_mat[r * C + c], p_ref[r * C + c]);
				flag = FALSE;
				return flag;
			}
#endif
		}
	}
	return flag;
}

void ref_gemm(DTYPE *p_mat_a, DTYPE *p_mat_b, int r1, int c1, int c2, DTYPE *p_mat_c, int shift) {
	int m, k, n;
#if FIXED_POINT_GEMM
	int32_t sum;
#else
	float sum;
#endif
	assert(r1 % L1_BLOCK_SIZE == 0);
	assert(c1 % L1_BLOCK_SIZE == 0);
	assert(c2 % L1_BLOCK_SIZE == 0);

	for(m = 0; m < r1; m++) {
		for(n = 0; n < c2; n++) {
			sum = 0;
			for(k = 0; k < c1; k++) {
				sum += p_mat_a[m * c1 + k] * p_mat_b[k * c2 + n];
			}
#if FIXED_POINT_GEMM
			p_mat_c[m * c2 + n] = (DTYPE)(sum >> shift);
#else
			p_mat_c[m * c2 + n] = sum;
#endif
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
	assert(r1 % L1_BLOCK_SIZE == 0);
	assert(c1 % L1_BLOCK_SIZE == 0);
	assert(c2 % L1_BLOCK_SIZE == 0);

	for(m = 0; m < r1; m += L1_BLOCK_SIZE) {
		for(n = 0; n < c2; n += L1_BLOCK_SIZE) {
			// compute one block of output matrix
			// This is panel-panel multiplication
			for(br = 0; br < L1_BLOCK_SIZE; br++) {
				for(bc = 0; bc < L1_BLOCK_SIZE; bc++) {
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

	float temp_acc[L1_BLOCK_SIZE][L1_BLOCK_SIZE];

	assert(r1 % L1_BLOCK_SIZE == 0);
	assert(c1 % L1_BLOCK_SIZE == 0);
	assert(c2 % L1_BLOCK_SIZE == 0);

	for(m = 0; m < r1; m += L1_BLOCK_SIZE) {
		for(n = 0; n < c2; n += L1_BLOCK_SIZE) {
			// reset the block output buffer
			memset(temp_acc[0], 0, L1_BLOCK_SIZE * L1_BLOCK_SIZE * sizeof(float));
			for(k = 0; k < c1; k += L1_BLOCK_SIZE) {
				for(br = 0; br < L1_BLOCK_SIZE; br++) {
					for(bc = 0; bc < L1_BLOCK_SIZE; bc++) {
						for(e = 0; e < L1_BLOCK_SIZE; e++) {
							temp_acc[br][bc] += p_mat_a[(m + br) * c1 + k + e] * p_mat_b[(k + e) * c2 + n + bc];
						}
					}
				}
			}
			for(br = 0; br < L1_BLOCK_SIZE; br++) {
				for(bc = 0; bc < L1_BLOCK_SIZE; bc++) {
					p_mat_c[(m + br) * c2 + n + bc] = temp_acc[br][bc];
				}
			}
		}
	}
}

void transfer_data(DTYPE *p_src, DTYPE *p_dst, int no_rows, int no_cols, int src_ptr_inc, int dst_ptr_inc) {
	int row;
	for(row = 0; row < no_rows; row++) {
		memcpy(p_dst, p_src, no_cols * sizeof(DTYPE));
		p_src += src_ptr_inc;
		p_dst += dst_ptr_inc;
	}
}

inline void transfer_b_panel(EDMA_OBJ_T *p_edma, DTYPE *p_src, DTYPE *p_dst, int no_rows, int no_cols, int src_ptr_inc, int dst_ptr_inc) {
	dma_matrix(p_edma, p_src, p_dst, no_rows, no_cols, sizeof(DTYPE), src_ptr_inc, dst_ptr_inc);

}

inline void transfer_a_panel(EDMA_OBJ_T *p_edma, DTYPE *p_src, DTYPE *p_dst, int no_rows, int no_cols) {
	dma_array(p_edma, p_src, p_dst, no_rows * no_cols * sizeof(DTYPE) );
}

inline void transfer_b_block(DTYPE *p_src, DTYPE *p_dst, int no_rows, int no_cols) {
	memcpy(p_dst, p_src, no_rows * no_cols * sizeof(DTYPE));
}

inline void transfer_c_block(EDMA_OBJ_T *p_edma, DTYPE *p_src, DTYPE *p_dst, int no_rows, int no_cols, int src_ptr_inc, int dst_ptr_inc) {
	dma_matrix(p_edma, p_src, p_dst, no_rows, no_cols, sizeof(DTYPE), src_ptr_inc, dst_ptr_inc);
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

void transfer_a_block(DTYPE *p_src, DTYPE *p_dst, int no_rows, int no_cols, int src_ptr_inc, int dst_ptr_inc) {
	int r;
	for(r = 0; r < no_rows; r++) {
		idma_ch1_tx_blocking(p_src, p_dst, no_cols * sizeof(DTYPE));
		p_src += src_ptr_inc;
		p_dst += dst_ptr_inc;
	}
}

inline void fill_data(DTYPE *p_buff, Uint32 val, int no_bytes) {
	idma_ch1_obj.source = val;
	idma_ch1_obj.destn = (Uint32 *)p_buff;
	idma_ch1_obj.intEnable = 0;
	idma_ch1_obj.priority = 0;
	idma_ch1_obj.count = no_bytes;

    CSL_IDMA_chan1FillData (&idma_ch1_obj, TRUE);
}

inline void wait_for_a_panel_tx(EDMA_OBJ_T *p_edma, void *addr, int size) {
	//printf("C_%d : waiting for panel A tx\n", core_id);
	wait_for_dma_tx(p_edma, FALSE, FALSE);
	//printf("C_%d : Done panel A tx\n", core_id);
	//L1_CACHE_INV((void *)addr, size, CACHE_WAIT);
}

void wait_for_b_panel_tx(EDMA_OBJ_T *p_edma, int blk_x_no, void *addr, int size) {
	//printf("C_%d : waiting for panel B tx\n", core_id);
	if(core_id == MASTER_CORE_ID) {
		wait_for_dma_tx(p_edma, FALSE, FALSE);
		toggle_panel_copy_flag(blk_x_no);
	} else {
		wait_for_panel_copy(blk_x_no);
	}
	//printf("C_%d : Done panel B tx\n", core_id);
	// invalidate the L1 cache since EDMA has updated the MSMC region
	L1_CACHE_INV((void *)addr, size, CACHE_WAIT);
}

void wait_for_blk_a_tx() {
	return;
}
void wait_for_blk_b_tx() {
	//wait_for_transfer(edma_handle, &edma_intr_bblk_obj, EDMA_INTR_BBLK_NO);
}
void wait_for_blk_c_tx(EDMA_OBJ_T *p_edma) {
	wait_for_dma_tx(p_edma, FALSE, FALSE);
}

void print_mat(DTYPE *p_mat, int R, int C) {
	int r, c;
	printf("--------------------------------------------\n");
	for(r = 0; r < R; r++) {
		for(c = 0; c < C; c++) {
#if FIXED_POINT_GEMM
			printf("%d\t", p_mat[r * C +c]);
#else
			printf("%.0f\t", p_mat[r * C +c]);
#endif
		}
		printf("\n");
	}
	printf("--------------------------------------------\n");
}



static inline split_fgemm_row(int no_rows, int *subset_size, int *start_row) {
	int quo, rem;

	// Requirement: the total no_rows should be multiple of constraint factor.
	// This
	REL_ASSERT(no_rows % L1_BLOCK_SIZE == 0);

	quo = no_rows / (L1_BLOCK_SIZE * NO_GEMM_CORES);
	rem = no_rows % (L1_BLOCK_SIZE * NO_GEMM_CORES);
	//printf("C_%d : No rows = %d\n", core_id, no_rows);

	if(core_id < rem / L1_BLOCK_SIZE) {
		// first few cores will have extra rows
		*subset_size = (quo + 1) * L1_BLOCK_SIZE;
		*start_row = core_id *  (*subset_size);
	} else {
		*subset_size = quo * L1_BLOCK_SIZE;
		*start_row = rem + core_id *  (*subset_size);
	}

}

void blk_blk_fgemm(DTYPE *p_mat_a, DTYPE *p_mat_b, int r1, int c1, int c2, DTYPE *p_mat_c) {
	int blk, blk_x, blk_y, no_blk_rows, no_blk_cols, no_patches;
	int R1, C1, C2, start_row;

	DTYPE *p_l1_blk_a[2], *p_l1_blk_b[2], *p_l1_blk_c[2];
	DTYPE *p_l2_panel_a[2], *p_blk_par_prod;
	DTYPE *p_msmc_panel_b[2];
	DTYPE *p_a, *p_b, *p_c;
	EDMA_OBJ_T *p_edma_a_panel, *p_edma_b_panel, *p_edma_c_blk;

	assert(r1 % L1_BLOCK_SIZE == 0);
	assert(c1 % L1_BLOCK_SIZE == 0);
	assert(c2 % L1_BLOCK_SIZE == 0);
	assert(c1 <= MAX_L2_PANEL_WIDTH);

	// EDMA channels to transfer A, B, C for this core.
	p_edma_a_panel = &shared_edma_obj[core_id * NO_CHANNELS_PER_CORE + 0];
	p_edma_b_panel = &shared_edma_obj[core_id * NO_CHANNELS_PER_CORE + 1];
	p_edma_c_blk = &shared_edma_obj[core_id * NO_CHANNELS_PER_CORE + 2];

	p_l2_panel_a[0] = (DTYPE *)global_address(panel_a_l2_ping[0]); p_l2_panel_a[1] = (DTYPE *)global_address(panel_a_l2_pong[0]);
	p_msmc_panel_b[0] = (DTYPE *)panel_b_msmc_ping[0]; p_msmc_panel_b[1] = (DTYPE *)panel_b_msmc_pong[0];
	p_l1_blk_a[0] = (DTYPE *)global_address(blk_a_l1_ping[0]); p_l1_blk_a[1] = (DTYPE *)global_address(blk_a_l1_pong[0]);
	p_l1_blk_b[0] = (DTYPE *)global_address(blk_b_l1_ping[0]); p_l1_blk_b[1] = (DTYPE *)global_address(blk_b_l1_pong[0]);
	p_l1_blk_c[0] = (DTYPE *)global_address(blk_c_l1_ping[0]); p_l1_blk_c[1] = (DTYPE *)global_address(blk_c_l1_pong[0]);
	p_blk_par_prod = (DTYPE *)global_address(blk_par_prod[0]);

	// workload distribution. Currently the output matrix is horizontally distributed among different cores.
	// Granularity of this split = L1_BLOCK_SIZE
	split_fgemm_row(r1, &R1, &start_row);

	C1 = c1; C2 = c2;		// no change in other 2 dim
	// Pre-transfer data for double buffering purpose.
	no_blk_rows = R1 / L1_BLOCK_SIZE;
	no_blk_cols = C2 / L1_BLOCK_SIZE;
	no_patches = C1 / L1_BLOCK_SIZE;

	//printf("C_%d : Start row = %d No rows = %d\n", core_id, start_row, R1);
	if(R1 == 0) {
		return;
	}
	p_a = p_mat_a + start_row * C1;

	// All : Transfer 1 horizontal panel of A into L2 SRAM buffer
	transfer_a_panel(p_edma_a_panel, p_a, p_l2_panel_a[0], L1_BLOCK_SIZE, C1);
	p_a += (L1_BLOCK_SIZE * C1);
	// All : Wait for transfer of panel A into L2 SRAM
	wait_for_a_panel_tx(p_edma_a_panel, p_l2_panel_a[0], L1_BLOCK_SIZE * C1);

	// Enter compute loop
	// Loop over all blocks in output row dimension.
	for(blk_y = 0; blk_y < no_blk_rows; blk_y++) {
		p_b = p_mat_b;
		p_c = p_mat_c + start_row * C2 + blk_y * C2 * L1_BLOCK_SIZE;
		// M : Transfer 1 vertical panel of B into MSMC RAM buffer.
		if(core_id == MASTER_CORE_ID) {
			transfer_b_panel(p_edma_b_panel, p_b, p_msmc_panel_b[0], C1, L1_BLOCK_SIZE, C2, L1_BLOCK_SIZE);
		}
		p_b += L1_BLOCK_SIZE;
		// All: Initiate transfer of panel A into ping buffer if it is not the last horizontal panel for the core.
		if( blk_y < no_blk_rows - 1) {
			transfer_a_panel(p_edma_a_panel, p_a, p_l2_panel_a[(blk_y + 1) % 2], L1_BLOCK_SIZE, C1);
		}
		// Wait for panel B transfer
		wait_for_b_panel_tx(p_edma_b_panel, 1, p_msmc_panel_b[0], C1 * L1_BLOCK_SIZE * sizeof(DTYPE));

		// Loop over all blocks in output column dimension.
		for(blk_x = 0; blk_x < no_blk_cols; blk_x++) {
			// M : Initiate transfer of panel B into pong buffer if it is not the last vertical panel of the matrix B/C
			if(blk_x < no_blk_cols - 1 && core_id == MASTER_CORE_ID) {
				transfer_b_panel(p_edma_b_panel, p_b, p_msmc_panel_b[(blk_x + 1) % 2], C1, L1_BLOCK_SIZE, C2, L1_BLOCK_SIZE);
			}
			// All : Tx 1 block of matrix A into L1 from L2 using IDMA - ping
			// All : Tx 1 block of matrix B into L1 from MSMC using EDMA
			transfer_b_block(p_msmc_panel_b[blk_x % 2], p_l1_blk_b[0], L1_BLOCK_SIZE, L1_BLOCK_SIZE);
			transfer_data(p_l2_panel_a[blk_y % 2], p_l1_blk_a[0], L1_BLOCK_SIZE, L1_BLOCK_SIZE, C1, L1_BLOCK_SIZE);
			wait_for_blk_b_tx();

			// reset the block accum C - memset is faster thatn IDMA1 fill
			memset(p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2], 0, L1_BLOCK_SIZE * L1_BLOCK_SIZE * sizeof(DTYPE));

			// wait for block A and B transfer
			wait_for_blk_a_tx();

			// All : loop over the input row dimension of A / col dim of B with block size step.
			for(blk = 0; blk < no_patches; blk++) {
				if(blk < no_patches - 1) {
					// All: Init Tx of 1 block of B from MSMC to L1 via L2 if not the last block - using EDMA
					transfer_b_block(p_msmc_panel_b[blk_x % 2] + (blk + 1) * L1_BLOCK_SIZE * L1_BLOCK_SIZE,
						p_l1_blk_b[(blk + 1) % 2], L1_BLOCK_SIZE, L1_BLOCK_SIZE);
					// All: Init Tx of 1 block of A from L2 to L1 pong buffer if not the last block. - since we need 16 IDMA calls here, it is slower that the normal read!
					transfer_data(p_l2_panel_a[blk_y % 2] + (blk + 1) * L1_BLOCK_SIZE,
						p_l1_blk_a[(blk + 1) % 2], L1_BLOCK_SIZE, L1_BLOCK_SIZE, C1, L1_BLOCK_SIZE);
				}

				//=============Core compute block using DSPLIB==============
#if FIXED_POINT_GEMM
				// Compute block X block using DSPLIB API - all inputs and output into buffer in L1
				DSP_mat_mul(p_l1_blk_a[blk % 2], L1_BLOCK_SIZE, L1_BLOCK_SIZE, p_l1_blk_b[blk % 2], L1_BLOCK_SIZE, p_blk_par_prod, 8);
				// Add the partial result to the block accumulator C
				DSP_add16(p_blk_par_prod, p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2],
					p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2], L1_BLOCK_SIZE * L1_BLOCK_SIZE);
#else
				//print_mat(p_l1_blk_a[blk % 2], L1_BLOCK_SIZE, L1_BLOCK_SIZE);
				//print_mat(p_l1_blk_b[blk % 2], L1_BLOCK_SIZE, L1_BLOCK_SIZE);
				// Compute block X block using DSPLIB API - all inputs and output into buffer in L1
				DSPF_sp_mat_mul(p_l1_blk_a[blk % 2], L1_BLOCK_SIZE, L1_BLOCK_SIZE, p_l1_blk_b[blk % 2], L1_BLOCK_SIZE, p_blk_par_prod);
				// Add the partial result to the block accumulator C
				DSPF_sp_vecadd(p_blk_par_prod, p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2],
					p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2], L1_BLOCK_SIZE * L1_BLOCK_SIZE);
#endif // FIXED_POINT_GEMM
				//==========================================================

				// Wait for block of A and B to be transferred into L1
				// OR
				// Wait for block of A to be transferred into L1 and block of B to be into L2 and then IDMA block B into L1 since there is 1 IDMA channel
				if(blk < no_patches - 1) {
					wait_for_blk_a_tx();
					wait_for_blk_b_tx();
				}
			}
			// if it is not the first block, we need to wait for the previous transfer of the result back to DDR.
			if(blk_y * no_blk_cols + blk_x != 0) {
				wait_for_blk_c_tx(p_edma_c_blk);
			}
			//print_mat(p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2], L1_BLOCK_SIZE, L1_BLOCK_SIZE);
			transfer_c_block(p_edma_c_blk, p_l1_blk_c[(blk_y * no_blk_cols + blk_x) % 2], p_c, L1_BLOCK_SIZE, L1_BLOCK_SIZE, L1_BLOCK_SIZE, C2);

			p_c += L1_BLOCK_SIZE;

			// tell the master that the panel B is used
			signal_panel_use(blk_x);

			// if some cores do  not get the output matrix share, they will immediately return and hence
			// do not increment this counter. In such case the cores which are given the actual work keeps waiting!
			// Disabling this for time being
			//wait_for_panel_use(blk_x);
			// wait for completion of panel B into MSMC
			if(blk_x < no_blk_cols - 1) { // TODO : every core has to wait here when using more than one core.
				wait_for_b_panel_tx(p_edma_b_panel, blk_x, p_msmc_panel_b[(blk_x + 1) % 2], C1 * L1_BLOCK_SIZE * sizeof(DTYPE));
			}
			// FIXME : wait for all cores to make use of the panel B
			if(core_id == MASTER_CORE_ID) {
				reset_panel_use_cntr(blk_x + 1);
				toggle_panel_copy_flag(blk_x + 1);
			}
			p_b += L1_BLOCK_SIZE;
		}
		if(blk_y < no_blk_rows - 1) {
			// wait for completion of panel A into L2
			wait_for_a_panel_tx(p_edma_a_panel, p_l2_panel_a[(blk_y + 1) % 2], C1 * L1_BLOCK_SIZE);
		}
		p_a += (L1_BLOCK_SIZE * C1);
	}
	// wait for the transfer completion of the last output block
	wait_for_blk_c_tx(p_edma_c_blk);
	L1_CACHE_WB(p_c + start_row * R1, R1 * C2 * sizeof(DTYPE), CACHE_WAIT);
}



#define VERIFY_RESULT 0
void run_flt_fast_gemm_bmark() {
	int size, blk_size, base_blk_size, factor, no_steps;
	int step;
	volatile DTYPE read_back;
	double gops;
	uint64_t start_time, end_time, runtime;


	base_blk_size = L1_BLOCK_SIZE;
	factor = 2;
	no_steps = 9;
	printf("==============================================================\n");
	printf("BLK_SIZE,R1,C1,C2,CYCLES,GOPS\n");

	for(blk_size = base_blk_size; blk_size <= L1_BLOCK_SIZE; blk_size += 4) {
		size = blk_size;
		step = 0;
		while(size <= MAT_R1_SIZE) {
			//printf("C_%d : Step = %d\n", core_id, step);
			if (core_id != MASTER_CORE_ID) {
				wait_for_image_init(step);
			} else {
#if VERIFY_RESULT
				generate_random_data((DTYPE *)mat_a, size * size, size+123);
				generate_random_data((DTYPE *)mat_b, size * size, size+456);
#if 	FIXED_POINT_GEMM
				ref_gemm((DTYPE *)mat_a, (DTYPE *)mat_b, size, size, size, (DTYPE *)mat_c_ref, 8);
#else
				ref_gemm((DTYPE *)mat_a, (DTYPE *)mat_b, size, size, size, (DTYPE *)mat_c_ref, 0);

#endif // FIXED_POINT_GEMM
#endif // VERIFY_RESULT
				reset_panel_copy_flags();
				reset_panel_use_cntr(0);
				reset_panel_use_cntr(1);
				//print_mat((DTYPE *)mat_c_ref, size, size);
				toggle_image_init_flag(step);
				start_time = CSL_tscRead();
			}
			//printf("C_%d :  Starting gemm\n", core_id);
			blk_blk_fgemm((DTYPE *)mat_a, (DTYPE *)mat_b, size, size, size, (DTYPE *)mat_c);

			// signal the completion of portion of  the output
			//printf("C_%d : Step done\n", core_id);
			signal_lyr_completion(step);

			// wait for all cores / portions of output
			wait_for_maps(step);

			read_back = mat_c[size * size - 1];
			if(core_id == MASTER_CORE_ID) {
				end_time = CSL_tscRead();

				// Since DMA has written into DDR, invalidate matrix C from L1D before reading during result verification.
				L1_CACHE_INV(mat_c, size * size * sizeof(DTYPE), CACHE_WAIT);
				//print_mat((DTYPE *)mat_c, size, size);
				runtime = end_time - start_time;
				gops = size/(float)runtime;
				gops *= size;
				gops *= (size * 2 * DSP_FREQ_GHZ);

				printf("%d,%d,%d,%d,%lld,%E\n", blk_size, size, size, size, runtime, gops);
#if VERIFY_RESULT
				if(verify_result((DTYPE *)mat_c, (DTYPE *)mat_c_ref, size, size)) {
					printf("TEST SUCCESS\n");
				} else {
					printf("TEST FAIL\n");
				}
#endif // VERIFY_RESULT
				toggle_image_init_flag(step);
				reset_layer_sync_cntr();
			}
			step++;
			size += L1_BLOCK_SIZE;
		}
	}
	printf("C_%d : GEMM benchmark complete----------------\n", core_id);
}

void gemm_conv_ctx_init(int r1, int c1, int c2) {
	int rem;

	// make sure that the final matrix dims are abiding constraints from low level API
	rem = r1 % L1_BLOCK_SIZE;
	if(rem != 0) {
		r1 += (L1_BLOCK_SIZE - rem);
	}

	rem = c1 % L1_BLOCK_SIZE;
	if(rem != 0) {
		c1 += (L1_BLOCK_SIZE - rem);
	}
	rem = c2 % L1_BLOCK_SIZE;
	if(rem != 0) {
		c2 += (L1_BLOCK_SIZE - rem);
	}

	REL_ASSERT(r1 <= MAT_R1_SIZE);
	REL_ASSERT(c1 <= MAT_C1_SIZE);
	REL_ASSERT(c2 <= MAT_C2_SIZE);

	gemm_conv_ctx.r1 = r1;
	gemm_conv_ctx.c1 = c1;
	gemm_conv_ctx.c2 = c2;

	gemm_conv_ctx.no_map_frac_bits = 8;
	gemm_conv_ctx.no_ker_frac_bits = 0;


	gemm_conv_ctx.p_input_1 = (DTYPE *)mat_a;
	gemm_conv_ctx.p_input_2 = (DTYPE *)mat_b;
	gemm_conv_ctx.p_output = (DTYPE *)mat_c;
	gemm_conv_ctx.p_bias = (DTYPE *)bias_array;
}

void get_output_dim(CAFFE_LYR_PARAM_T *p_lyr, int n_imaps, int in_h, int in_w, int *n_omaps, int *o_h, int *o_w) {
	switch(p_lyr->lyrType) {
		case CONV:
			*o_h = ( in_h + 2*p_lyr->pad - p_lyr->K) / p_lyr->stride + 1;
			*o_w = ( in_w + 2*p_lyr->pad - p_lyr->K) / p_lyr->stride + 1;
			*n_omaps = p_lyr->nOutMaps;
			break;
		case POOL:
			*o_h = ( in_h + 2*p_lyr->pad - p_lyr->winSize) / p_lyr->stride + 1;
			*o_w = ( in_w + 2*p_lyr->pad - p_lyr->winSize) / p_lyr->stride + 1;
			*n_omaps = n_imaps;
			break;
		case ACT:
			*n_omaps = n_imaps;
			*o_w = in_w;
			*o_h = in_h;
			break;
		case BATCH_NORM:
			*n_omaps = n_imaps;
			*o_w = in_w;
			*o_h = in_h;
			break;
		case INNER_PROD:
			*n_omaps = 1;
			*o_w = in_h * in_w;
			*o_h = 1;
			break;
		default:
	}
}

void run_fast_gemm_conv_bmark() {
	int gemm, n_ch, in_h, in_w, K, stride, pad, n_omaps, out_w, out_h;
	volatile DTYPE read_back;
	double gops, time_us;
	uint64_t start_time, end_time, runtime, unroll_start, unroll_end;
	CAFFE_LYR_PARAM_T *p_conv_param;

	printf("==============================================================\n");
	printf("BLK_SIZE,R1(original r1),C1(original c1),C2(original c2),IMG2COL-CYCLES, IMG2COL-RUNTIME, GEMM-CYCLES, GEMM-RUNTIME(us), GOPS\n");

	gemm = 0;
	n_ch = NO_INPUT_MAPS;
	in_h = INPUT_IMG_HEIGHT;
	in_w = INPUT_IMG_WIDTH;
	p_conv_param = cnn_param_table;

	get_output_dim(p_conv_param, n_ch, in_h, in_w, &n_omaps, &out_h, &out_w);
	while(gemm < NO_GEMMS) {
		if (core_id != MASTER_CORE_ID) {
			wait_for_image_init(gemm);
			L1_CACHE_INV((void *)&gemm_conv_ctx, L1_CACHE_LINE_SIZE, CACHE_WAIT);
		} else {
			// init the matrix size, make them multiple of block sizes
			gemm_conv_ctx_init(caltech101_A_height[gemm], caltech101_A_width[gemm], caltech101_B_width[gemm]);

			while(p_conv_param->lyrType != CONV) {
				get_output_dim(p_conv_param, n_ch, in_h, in_w, &n_omaps, &out_h, &out_w);
				p_conv_param++;
				n_ch = n_omaps;
				in_h = out_h;
				in_w = out_w;
			}
			K = p_conv_param->K;
			pad = p_conv_param->pad;
			stride = p_conv_param->stride;
			get_output_dim(p_conv_param, n_ch, in_h, in_w, &n_omaps, &out_h, &out_w);

			REL_ASSERT(n_ch * in_h * in_w <= MAX_INPUT_MAP_BUFF_SIZE);

			// input map unroll
			unroll_start =  CSL_tscRead();
			img2col((DTYPE *)conv_input_maps, gemm_conv_ctx.p_input_2,
				n_ch,
				in_h,
				in_w,
				K,
				stride,
				pad,
				gemm_conv_ctx.c1 - caltech101_A_width[gemm],	// extra zero rows of B
				gemm_conv_ctx.c2 - caltech101_B_width[gemm]	// extra zero cols of B
				);
			unroll_end =  CSL_tscRead();

			// this is not needed if the MSMC region is set to write through.
			L1_CACHE_WB((void *)&gemm_conv_ctx, L1_CACHE_LINE_SIZE, CACHE_WAIT);
			reset_panel_copy_flags();
			reset_panel_use_cntr(0);
			reset_panel_use_cntr(1);
			toggle_image_init_flag(gemm);
			start_time = CSL_tscRead();
		}

		blk_blk_fgemm((DTYPE *)gemm_conv_ctx.p_input_1, (DTYPE *)gemm_conv_ctx.p_input_2,
			gemm_conv_ctx.r1, gemm_conv_ctx.c1, gemm_conv_ctx.c2, (DTYPE *)gemm_conv_ctx.p_output);

		// signal the completion of portion of  the output
		signal_lyr_completion(gemm);

		// wait for all cores / portions of output
		wait_for_maps(gemm);

		read_back = mat_c[gemm_conv_ctx.r1 * gemm_conv_ctx.c2 - 1];
		if(core_id == MASTER_CORE_ID) {
			end_time = CSL_tscRead();

			// Since DMA has written into DDR, invalidate matrix C from L1D before reading during result verification.
			L1_CACHE_INV(mat_c, gemm_conv_ctx.r1 * gemm_conv_ctx.c2 * sizeof(DTYPE), CACHE_WAIT);
			//print_mat((DTYPE *)mat_c, size, size);
			runtime = end_time - start_time;
			gops = gemm_conv_ctx.r1/(float)runtime;
			gops *= gemm_conv_ctx.c1;
			gops *= (gemm_conv_ctx.c2 * 2 * DSP_FREQ_GHZ);
			time_us = (double)runtime / DSP_FREQ_MHZ;
			printf("%d, %d(%d), %d(%d), %d(%d), %lld, %.4f, %lld, %.4f, %E\n", L1_BLOCK_SIZE,
				gemm_conv_ctx.r1, caltech101_A_height[gemm],
				gemm_conv_ctx.c1, caltech101_A_width[gemm],
				gemm_conv_ctx.c2, caltech101_B_width[gemm],
				(unroll_end - unroll_start),
				(double)(unroll_end - unroll_start) / DSP_FREQ_MHZ,
				runtime, time_us, gops);

			toggle_image_init_flag(gemm);
			reset_layer_sync_cntr();

			p_conv_param++;
			in_h = out_h;
			in_w = out_w;
			n_ch = n_omaps;
		}
		gemm++;

	}
	//printf("C_%d : GEMM based convolution benchmark complete----------------\n", core_id);
}


void run_fast_gemm_bmark() {
	//run_flt_fgemm_compute_bmark();
	//run_flt_gemm_compute_bmark();
	//run_flt_fast_gemm_bmark();
	run_fast_gemm_conv_bmark();
}

