/*
 * edma_module.c
 *
 *  Created on: 18 Dec 2016
 *      Author: Gopalakrishna Hegde, NTU Singapore
 */
#include "edma_module.h"
#include <stdlib.h>
#include <stdio.h>


#define HALT(args...) exit(args)



void edma_hw_init() {
	// This does nothing as of now.
	CSL_edma3Init(NULL);
}

CSL_Edma3Handle create_edma(CSL_InstNum edma_no, CSL_Edma3Obj *p_edma_obj ) {

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

CSL_Edma3ChannelHandle setup_edma_channel(CSL_InstNum edma_no,
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
void enable_edma_interrupt(CSL_Edma3Handle edma_handle, CSL_Edma3CmdIntr *p_intr, Int region_no, Uint8 intr_no) {

	if(intr_no > 31) {
		p_intr->intrh = p_intr->intrh | (1 << (32 - intr_no));
	} else {
		p_intr->intr = p_intr->intr | (1 << intr_no);
	}

	p_intr->region = region_no;

	CSL_edma3HwControl(edma_handle, CSL_EDMA3_CMD_INTR_ENABLE, p_intr);
}

// Sets up parameters for a 1D array transfer
void setup_edma_array_param(CSL_Edma3ChannelHandle ch_handle,
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
void setup_edma_mat_param(CSL_Edma3ChannelHandle ch_handle,
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

// Use this trigger only when sync mode is A and BCNT > 1
void trigger_edma_channel_events(CSL_Edma3ChannelHandle ch_handle, CSL_Edma3Que edma_que, Uint32 event_cnt){
	int bcnt;
	CSL_Edma3QueStat que_status;

	que_status.que = edma_que;

	// Need to send BCNT no of events if the transfer mode is A-synchronous.
	for(bcnt = 0; bcnt < event_cnt; bcnt++) {
		do {
		// TODO: Check the no of entries in the event queue, make sure that the FIFO is not full before sending event
		CSL_edma3GetHwStatus((CSL_Edma3Handle)ch_handle, CSL_EDMA3_QUERY_QUESTATUS, &que_status);
		} while(que_status.numVal > 15);	// 16 is the queue depth

		// set the DMA event to start the transfer
		CSL_edma3HwChannelControl(ch_handle, CSL_EDMA3_CMD_CHANNEL_SET, NULL);
	}
}

// trigger single channel event. Use this for AB-sync mode
void trigger_edma_channel_event(CSL_Edma3ChannelHandle ch_handle) {
	CSL_edma3HwChannelControl(ch_handle, CSL_EDMA3_CMD_CHANNEL_SET, NULL);
}

// Wait for the interrupt pending register to be set
// The interrupt number must be the TCC code used for DMA params.
void wait_for_transfer(CSL_Edma3Handle edma_handle, CSL_Edma3CmdIntr *p_intr, Uint8 intr_no) {
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

void close_edma(CSL_Edma3Handle edma_handle, CSL_Edma3ChannelHandle ch_handle) {
	CSL_FINST(edma_handle->regs->TPCC_SECR,TPCC_TPCC_SECR_SECR2,RESETVAL);
	CSL_FINST(edma_handle->regs->TPCC_SECR,TPCC_TPCC_SECR_SECR3,RESETVAL);

	CSL_edma3ChannelClose(ch_handle);
	CSL_edma3Close(edma_handle);
}
