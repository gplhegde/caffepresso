/*
 * edma_module.h
 *
 *  Created on: 18 Dec 2016
 *      Author: Gopalakrishna Hegde, NTU Singapore
 */

#ifndef INC_EDMA_MODULE_H_
#define INC_EDMA_MODULE_H_

#include <ti/csl/tistdtypes.h>
#include <ti/csl/csl_edma3Aux.h>
#include "edma_config.h"
#include "config.h"

typedef struct {
	//EDMA object
	CSL_Edma3Obj edma_obj;
	// EDMA object handle
	CSL_Edma3Handle edma_handle;
	// EDMA channel object
	CSL_Edma3ChannelObj ch_obj;
	// EDMA channel handle
	CSL_Edma3ChannelHandle ch_handle;
	// Event que number for this channel
	CSL_Edma3Que event_que;
	// Interrupt and tags info
	CSL_Edma3CmdIntr intr_obj;
	// Interrupt no / tcc tag
	Uint32 intr_no;
	// Chanel param object  & no
	CSL_Edma3ParamSetup param;
	Uint32 param_no;
} EDMA_OBJ_T;

extern EDMA_OBJ_T shared_edma_obj[NO_CHANNELS_PER_CORE * NO_GEMM_CORES];
extern EDMA_OBJ_T master_edma_obj;
void all_edma_init();

Uint32 global_address (Uint32 addr);

void dma_array(EDMA_OBJ_T *p_edma, void *p_src, void *p_dst, Uint16 no_bytes);

void dma_matrix(EDMA_OBJ_T *p_edma, void *p_src, void *p_dst, int no_rows, int no_cols, int ele_size, int src_ptr_inc, int dst_ptr_inc);

void wait_for_dma_tx(EDMA_OBJ_T *p_edma, Bool cache_wb, Bool cache_inv);
#endif /* INC_EDMA_MODULE_H_ */
