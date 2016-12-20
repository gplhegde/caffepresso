/*
 * fast_gemm.h
 *
 *  Created on: 18 Dec 2016
 *      Author: Gopalakrishna Hegde
 */

#ifndef _FAST_GEMM_H_
#define _FAST_GEMM_H_

#define NO_GEMM_CORES	1
#define L1_SCRATCHPAD_SIZE		(16 * 1024)
#define L2_SCRATCHPAD_SIZE		(256 * 1024)

// we need 6 buffers of size 32x32 - ping pong buffering case
// max possible block sizes which are powers of 2 with above scratchpad size constraints
#define TOTAL_NO_ON_CHIP_BUFFERS	(6)

#define MAX_L1_FIX_BLOCK_SIZE		(32)
#define MAX_L1_FLT_BLOCK_SIZE		(16)
#define MAX_L2_FIX_BLOCK_SIZE		(128)
#define MAX_L2_FLT_BLOCK_SIZE		(64)

#define MAX_MSMC_FLT_PANEL_WIDTH	(16)
#define MAX_MSMC_FLT_PANEL_HEIGHT	(4096)
#define MAX_MSMC_FIX_PANEL_WIDTH	(MAX_MSMC_FLT_PANEL_WIDTH * 2)
#define MAX_MSMC_FIX_PANEL_HEIGHT	(MAX_MSMC_FLT_PANEL_HEIGHT * 2)

#define MAX_L2_FLT_PANEL_WIDTH	(MAX_MSMC_FLT_PANEL_HEIGHT)
#define MAX_L2_FLT_PANEL_HEIGHT	(MAX_MSMC_FLT_PANEL_WIDTH)
#define MAX_L2_FIX_PANEL_WIDTH	(MAX_MSMC_FIX_PANEL_HEIGHT)
#define MAX_L2_FIX_PANEL_HEIGHT	(MAX_MSMC_FIX_PANEL_WIDTH)

#define L1_FIX_BLOCK_SIZE		(32)
#define L1_FLT_BLOCK_SIZE		(16)
#define L2_FIX_BLOCK_SIZE		(64)
#define L2_FLT_BLOCK_SIZE		(32)



void run_fast_gemm_bmark();
#endif /* _FAST_GEMM_H_ */
