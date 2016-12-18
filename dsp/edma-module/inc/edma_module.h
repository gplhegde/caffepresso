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

void edma_hw_init();

CSL_Edma3Handle create_edma(CSL_InstNum edma_no, CSL_Edma3Obj *p_edma_obj);

CSL_Edma3ChannelHandle setup_edma_channel(CSL_InstNum edma_no,
	Int ch_no,
	Int region_no,
	CSL_Edma3Que edma_que,
	Uint16 param_set,
	CSL_Edma3ChannelObj *p_edma_chanel,
	Bool enable);

void enable_edma_interrupt(CSL_Edma3Handle edma_handle, CSL_Edma3CmdIntr *p_intr, Int region_no, Uint8 intr_no);

void setup_edma_array_param(CSL_Edma3ChannelHandle ch_handle,
	CSL_Edma3ParamSetup *p_param,
	Uint16 param_no,	// PaRAM set number to be written.
	Uint32 src_addr,	// word aligned source address(global)
	Uint32 dst_addr,	// word aligned destination address(global)
	Uint32 no_bytes,	// multiple of 4
	Uint8 tcc_tag);		// TCC tag to be used. The interrupt no = TCC tag will be set after transfer completion

void setup_edma_mat_param(CSL_Edma3ChannelHandle ch_handle,
	CSL_Edma3ParamSetup *p_param,
	Uint16 param_no,	// PaRAM set number to be written.
	Uint32 src_addr,	// word aligned source address(global)
	Uint32 dst_addr,	// word aligned destination address(global)
	Uint32 row_bytes,	// multiple of 4
	Uint32 no_rows,		// no of rows each containing row_bytes to transfer
	Uint32 src_row_size,// no of bytes in the full source matrix row.
	Uint32 dst_row_size,// no of bytes in the destination matrix row.
	Uint8 tcc_tag);		// TCC tag to be used. The interrupt no = TCC tag will be set after transfer completion

void trigger_edma_channel_events(CSL_Edma3ChannelHandle ch_handle, CSL_Edma3Que edma_que, Uint32 event_cnt);

void trigger_edma_channel_event(CSL_Edma3ChannelHandle ch_handle);

void wait_for_transfer(CSL_Edma3Handle edma_handle, CSL_Edma3CmdIntr *p_intr, Uint8 intr_no);

void close_edma(CSL_Edma3Handle edma_handle, CSL_Edma3ChannelHandle ch_handle);

#endif /* INC_EDMA_MODULE_H_ */
