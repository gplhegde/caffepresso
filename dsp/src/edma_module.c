/*
 * edma_module.c
 *
 *  Created on: 18 Dec 2016
 *      Author: Gopalakrishna Hegde, NTU Singapore
 */
#include <ti/csl/csl_chip.h>
#include "edma_module.h"
#include "edma_config.h"
#include "data_sync.h"
#include "debug_control.h"
#include <stdlib.h>
#include <stdio.h>
 extern unsigned int core_id;

#define HALT(args...) exit(args)
// This is for shared data transfer in case of GEMM
EDMA_OBJ_T master_edma_obj;

// EDMA objects for all cores. Each core will only use from channel core_id * NO_CHANNELS_PER_CORE to core_id * NO_CHANNELS_PER_CORE + 2
#pragma DATA_ALIGN   (shared_edma_obj, L1_CACHE_LINE_SIZE)
#pragma DATA_SECTION (shared_edma_obj, ".shared_ocm")
EDMA_OBJ_T shared_edma_obj[NO_CHANNELS_PER_CORE * NO_CORES];

// EDMA NO, START_CH_NO, QUE_NO, START_PARAM_NO, START_INTR_NO/TCC_TAG
const uint8_t edma_channel_mappings[NO_CORES][5] = {
		{0, 1, 1, 1, 1},
		{1, 0, 0, 0, 3},
		{1, 3, 1, 16, 6},
		{1, 6, 2, 32, 9},
		{1, 9, 3, 48, 12},
		{2, 0, 0, 0, 0},
		{2, 3, 1, 16, 3},
		{2, 6, 2, 32, 6}
};

static void edma_hw_init() {
	// This does nothing as of now.
	CSL_edma3Init(NULL);
}

static CSL_Edma3Handle create_edma(CSL_InstNum edma_no, CSL_Edma3Obj *p_edma_obj ) {

	CSL_Edma3Handle  handle;
	CSL_Status status;

	// Open EDMA Module and return the handle
	handle = CSL_edma3Open(p_edma_obj, edma_no, NULL, &status);

	if(status != CSL_SOK) {
		printf("Could not get the EDMA module\n");
		HALT(0);
	}

	// This handle = p_edma_obj if the allocation is successful.
	return handle;
}

static CSL_Edma3ChannelHandle setup_edma_channel(CSL_InstNum edma_no,
	Int ch_no,
	Int region_no,
	CSL_Edma3Que edma_que,
	Uint16 param_set,
	CSL_Edma3ChannelObj *p_edma_chanel,
	Bool enable) {

	CSL_Edma3ChannelHandle ch_handle;
	CSL_Edma3ChannelAttr ch_attr;
	CSL_Status status;

	ch_attr.chaNum = ch_no;
	ch_attr.regionNum = region_no;
	ch_handle = CSL_edma3ChannelOpen(p_edma_chanel, edma_no, &ch_attr, &status);

	if(status != CSL_SOK) {
		printf("Could not setup the EDMA channel\n");
		HALT(0);
	}

	// Hook the PaRAM set for the channel
	CSL_edma3HwChannelSetupParam(ch_handle, param_set);

	// Setup the event queue for the channel.
    CSL_edma3HwChannelSetupQue(ch_handle, edma_que);

   	// TODO: clear only if SER bits are set
   	// Clear secondary event set reg for this channel
	CSL_edma3ClearDMAChannelSecondaryEvents((CSL_Edma3Handle)ch_handle, ch_no); // FIXME: need to pass handle to the edma object instead of channel

   	// clear any missed events for this channel
   	// TODO: clear only if set
	CSL_edma3ClearDMAMissedEvent((CSL_Edma3Handle)ch_handle, ch_no);

	// Enable the channel for external events. DSP triggered events bypass the enable register.
	if(enable) {
		CSL_edma3HwChannelControl(ch_handle, CSL_EDMA3_CMD_CHANNEL_ENABLE, NULL);
	}
	// This handle = p_edma_chanel if the allocation is successful.
	return ch_handle;
}

// Interrupts need to be enabled only if they are going to be routed to CPU and handled.
// Setting of IPR register is independent of enabling interrupts.
static void enable_edma_interrupt(CSL_Edma3Handle edma_handle, CSL_Edma3CmdIntr *p_intr, Int region_no, Uint8 intr_no) {

	if(intr_no > 31) {
		p_intr->intrh = p_intr->intrh | (1 << (32 - intr_no));
	} else {
		p_intr->intr = p_intr->intr | (1 << intr_no);
	}

	p_intr->region = region_no;

	CSL_edma3HwControl(edma_handle, CSL_EDMA3_CMD_INTR_ENABLE, p_intr);
}

// Sets up parameters for a 1D array transfer
static inline void setup_edma_array_param(CSL_Edma3ChannelHandle ch_handle,
	CSL_Edma3ParamSetup *p_param,
	Uint16 param_no,	// PaRAM set number to be written.
	Uint32 src_addr,	// word aligned source address(global)
	Uint32 dst_addr,	// word aligned destination address(global)
	Uint32 no_bytes,	// multiple of 4
	Uint8 tcc_tag) {	// TCC tag to be used. The interrupt no = TCC tag will be set after transfer completion

	CSL_Edma3ParamHandle param_handle;
	CSL_Status status;

	// Get the handle to the parameter set(address to the PaRAM with proper offset)
	param_handle            = CSL_edma3GetParamHandle(ch_handle, param_no, &status);

	// Populate the EDMA transfer parameters
	p_param->aCntbCnt     = CSL_EDMA3_CNT_MAKE(no_bytes, 1);
	p_param->srcDstBidx   = CSL_EDMA3_BIDX_MAKE(1, 1);
	p_param->srcDstCidx   = CSL_EDMA3_CIDX_MAKE(1, 1);
	p_param->cCnt         = 1;
	p_param->option       = CSL_EDMA3_OPT_MAKE(FALSE,FALSE,FALSE,TRUE, tcc_tag ,CSL_EDMA3_TCC_NORMAL, \
	      CSL_EDMA3_FIFOWIDTH_NONE, FALSE, CSL_EDMA3_SYNC_A, CSL_EDMA3_ADDRMODE_INCR, CSL_EDMA3_ADDRMODE_INCR);
	p_param->srcAddr      = src_addr;
	p_param->dstAddr      = dst_addr;
	p_param->linkBcntrld  = CSL_EDMA3_LINKBCNTRLD_MAKE(CSL_EDMA3_LINK_NULL, 1);

	// Write the transfer parameters to the PaRAM
	CSL_edma3ParamSetup(param_handle, p_param);
}

// Sets up parameters for a 2D array transfer
static inline void setup_edma_mat_param(CSL_Edma3ChannelHandle ch_handle,
	CSL_Edma3ParamSetup *p_param,
	Uint16 param_no,	// PaRAM set number to be written.
	Uint32 src_addr,	// word aligned source address(global)
	Uint32 dst_addr,	// word aligned destination address(global)
	Uint32 row_bytes,	// multiple of 4
	Uint32 no_rows,		// no of rows each containing row_bytes to transfer
	Uint32 src_row_size,// no of bytes in the full source matrix row.
	Uint32 dst_row_size,// no of bytes in the destination matrix row.
	Uint8 tcc_tag) {	// TCC tag to be used. The interrupt no = TCC tag will be set after transfer completion

	CSL_Edma3ParamHandle param_handle;
	CSL_Status status;

	// Get the handle to the parameter set(address to the PaRAM with proper offset)
	param_handle            = CSL_edma3GetParamHandle(ch_handle, param_no, &status);

	// Populate the EDMA transfer parameters
	p_param->aCntbCnt     = CSL_EDMA3_CNT_MAKE(row_bytes, no_rows);
	p_param->srcDstBidx   = CSL_EDMA3_BIDX_MAKE(src_row_size, dst_row_size);
	p_param->srcDstCidx   = CSL_EDMA3_CIDX_MAKE(1, 1);
	p_param->cCnt         = 1;
	p_param->option       = CSL_EDMA3_OPT_MAKE(FALSE,FALSE,FALSE,TRUE, tcc_tag ,CSL_EDMA3_TCC_NORMAL, \
	      CSL_EDMA3_FIFOWIDTH_NONE, FALSE, CSL_EDMA3_SYNC_AB, CSL_EDMA3_ADDRMODE_INCR, CSL_EDMA3_ADDRMODE_INCR);
	p_param->srcAddr      = src_addr;
	p_param->dstAddr      = dst_addr;
	p_param->linkBcntrld  = CSL_EDMA3_LINKBCNTRLD_MAKE(CSL_EDMA3_LINK_NULL, 1);

	// Write the transfer parameters to the PaRAM
	CSL_edma3ParamSetup(param_handle, p_param);
}


// trigger single channel event. Use this for AB-sync mode
static void trigger_edma_channel_event(CSL_Edma3ChannelHandle ch_handle) {
	CSL_edma3HwChannelControl(ch_handle, CSL_EDMA3_CMD_CHANNEL_SET, NULL);
}

// Wait for the interrupt pending register to be set
// The interrupt number must be the TCC code used for DMA params.
static inline void wait_for_transfer(CSL_Edma3Handle edma_handle, CSL_Edma3CmdIntr *p_intr, Uint8 intr_no) {
	CSL_Edma3CmdIntr intr_ctx;
	Uint32 flag;

	intr_ctx.region = p_intr->region;

	do{
		// waiting for the bit (= TCC number) interrupt pending register to be set.
		CSL_edma3GetHwStatus(edma_handle, CSL_EDMA3_QUERY_INTRPEND, &intr_ctx);
		if(intr_no > 31) {
			flag = intr_ctx.intrh & (1 << (32 - intr_no));
		} else {
			flag = intr_ctx.intr & (1 << intr_no);
		}

	}while (!flag);

	if(intr_no < 32) {
		intr_ctx.intr = 1 << intr_no;
	} else {
		intr_ctx.intrh = 1 << (32 - intr_no);
	}
	// clear interrupt
	CSL_edma3HwControl(edma_handle, CSL_EDMA3_CMD_INTRPEND_CLEAR, &intr_ctx);
}

static void close_edma(CSL_Edma3Handle edma_handle, CSL_Edma3ChannelHandle ch_handle) {
	CSL_FINST(edma_handle->regs->TPCC_SECR,TPCC_TPCC_SECR_SECR2,RESETVAL);
	CSL_FINST(edma_handle->regs->TPCC_SECR,TPCC_TPCC_SECR_SECR3,RESETVAL);

	CSL_edma3ChannelClose(ch_handle);
	CSL_edma3Close(edma_handle);
}

// To be called only by master core
void all_edma_init() {
	int core, ch;
	EDMA_OBJ_T *p_edma_array;
	if(core_id != MASTER_CORE_ID) {
		REL_INFO("Only master can do EDMA init\n");
		return;
	}
	memset(&master_edma_obj, 0, sizeof(master_edma_obj));
	memset(shared_edma_obj, 0, sizeof(shared_edma_obj));
	p_edma_array = shared_edma_obj;
	edma_hw_init();
	// Setup EDMA for common data in GEMM
	master_edma_obj.param_no = 0;
	master_edma_obj.intr_no = 0;
	master_edma_obj.edma_handle = create_edma(0, &master_edma_obj.edma_obj);
	master_edma_obj.ch_handle = setup_edma_channel(0, 0,
			CSL_EDMA3_REGION_GLOBAL, CSL_EDMA3_QUE_0, master_edma_obj.param_no, &master_edma_obj.ch_obj, TRUE);
	enable_edma_interrupt(master_edma_obj.edma_handle, &master_edma_obj.intr_obj, CSL_EDMA3_REGION_GLOBAL, master_edma_obj.intr_no);


	// Setup 3 channels per core for the private data transfer.
	for(core = 0; core < NO_CORES; core++) {
		for(ch = 0; ch < NO_CHANNELS_PER_CORE; ch++) {
			p_edma_array->intr_no = edma_channel_mappings[core][4] + ch;
			p_edma_array->param_no = edma_channel_mappings[core][3] + ch;
			p_edma_array->edma_handle = create_edma(edma_channel_mappings[core][0], &p_edma_array->edma_obj);
			p_edma_array->ch_handle = setup_edma_channel(edma_channel_mappings[core][0],
					edma_channel_mappings[core][1] + ch,
					CSL_EDMA3_REGION_GLOBAL,
					edma_channel_mappings[core][2],
					p_edma_array->param_no,
					&p_edma_array->ch_obj,
					TRUE
					);
			enable_edma_interrupt(p_edma_array->edma_handle, &p_edma_array->intr_obj, CSL_EDMA3_REGION_GLOBAL, p_edma_array->intr_no);
			p_edma_array++;
		}
	}
	L1_CACHE_WB((void *)shared_edma_obj, sizeof(shared_edma_obj), CACHE_WAIT);

}
Uint32 global_address (Uint32 addr) {
	Uint32 core;
	core = CSL_chipReadReg(CSL_CHIP_DNUM);

	addr = addr + (0x10000000 + core * 0x1000000);
	return addr;
}

void dma_array(EDMA_OBJ_T *p_edma, void *p_src, void *p_dst, Uint16 no_bytes) {
	setup_edma_array_param(p_edma->ch_handle,
			&p_edma->param,
			p_edma->param_no,
			(Uint32)p_src,
			(Uint32)p_dst,
			no_bytes,
			p_edma->intr_no);
	trigger_edma_channel_event(p_edma->ch_handle);
}

void dma_matrix(EDMA_OBJ_T *p_edma, Uint32 *p_src, Uint32 *p_dst, int no_rows, int no_cols, int ele_size, int src_ptr_inc, int dst_ptr_inc) {

	setup_edma_mat_param(p_edma->ch_handle,
		&p_edma->param,
		p_edma->param_no,
		(Uint32)p_src,
		(Uint32)p_dst,
		no_cols * ele_size,
		no_rows,
		src_ptr_inc * ele_size,
		dst_ptr_inc * ele_size,
		p_edma->intr_no
		);
	trigger_edma_channel_event(p_edma->ch_handle);
}

void wait_for_dma_tx(EDMA_OBJ_T *p_edma, Bool cache_wb, Bool cache_inv) {
	wait_for_transfer(p_edma->edma_handle, &p_edma->intr_obj, p_edma->intr_no);
}
