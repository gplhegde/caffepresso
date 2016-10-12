/* VECTORBLOX MXP SOFTWARE DEVELOPMENT KIT
 *
 * Copyright (C) 2012-2015 VectorBlox Computing Inc., Vancouver, British Columbia, Canada.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of VectorBlox Computing Inc. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This agreement shall be governed in all respects by the laws of the Province
 * of British Columbia and by the laws of Canada.
 *
 * This file is part of the VectorBlox MXP Software Development Kit.
 *
 */


#ifndef __VBX_COUNTERS_H
#define __VBX_COUNTERS_H


#define VBX_STATUS_RESET                  0
#define VBX_STATUS_TOTAL_CYCLES           1
#define VBX_STATUS_WRITEBACK_CYCLES       2
#define VBX_STATUS_INSTRUCTIONS           3
#define VBX_STATUS_DMA_CYCLES             4
#define VBX_STATUS_DMAS                   5
#define VBX_STATUS_INSTR_HAZARD_CYCLES    6
#define VBX_STATUS_DMA_HAZARD_CYCLES      7
#define VBX_STATUS_DMA_QUEUE_STALL_CYCLES 8

#define VBX_STATUS_SNAPSHOT                        16
#define VBX_STATUS_SNAPSHOT_TOTAL_CYCLES           17
#define VBX_STATUS_SNAPSHOT_WRITEBACK_CYCLES       18
#define VBX_STATUS_SNAPSHOT_INSTRUCTIONS           19
#define VBX_STATUS_SNAPSHOT_DMA_CYCLES             20
#define VBX_STATUS_SNAPSHOT_DMAS                   21
#define VBX_STATUS_SNAPSHOT_INSTR_HAZARD_CYCLES    22
#define VBX_STATUS_SNAPSHOT_DMA_HAZARD_CYCLES      23
#define VBX_STATUS_SNAPSHOT_DMA_QUEUE_STALL_CYCLES 24
#ifdef __NIOS2__
#  include "io.h"
#  define _GET_COUNTER(cntr)  		IORD(VBX1_STATUS_SLAVE_BASE,cntr)
#  define VBX_GET_TOTAL_CYCLES()            _GET_COUNTER( VBX_STATUS_TOTAL_CYCLES)
#  define VBX_GET_WRITEBACK_CYCLES()        _GET_COUNTER( VBX_STATUS_WRITEBACK_CYCLES)
#  define VBX_GET_INSTRUCTIONS()            _GET_COUNTER( VBX_STATUS_INSTRUCTIONS)
#  define VBX_GET_DMA_CYCLES()              _GET_COUNTER( VBX_STATUS_DMA_CYCLES)
#  define VBX_GET_DMAS()                    _GET_COUNTER( VBX_STATUS_DMAS)
#  define VBX_GET_INSTR_HAZARD_CYCLES()     _GET_COUNTER( VBX_STATUS_INSTR_HAZARD_CYCLES)
#  define VBX_GET_DMA_HAZARD_CYCLES()       _GET_COUNTER( VBX_STATUS_DMA_HAZARD_CYCLES)
#  define VBX_GET_DMA_QUEUE_STALL_CYCLES()  _GET_COUNTER( VBX_STATUS_DMA_QUEUE_STALL_CYCLES)
#  define VBX_COUNTER_RESET()               _GET_COUNTER( VBX_STATUS_RESET)
#elif VBX_SIMULATOR
#  define VBX_GET_TOTAL_CYCLES()            0
#  define VBX_GET_WRITEBACK_CYCLES()        vbxsim_get_instr_cycles(-1,VBX_GET_THIS_MXP()->vector_lanes)
#  define VBX_GET_INSTRUCTIONS()            vbxsim_get_instr_count(-1)
#  define VBX_GET_DMA_CYCLES()              vbxsim_get_dma_cycles(VBX_GET_THIS_MXP()->dma_alignment_bytes /4)
#  define VBX_GET_DMAS()                    vbxsim_get_dma_count()
#  define VBX_GET_INSTR_HAZARD_CYCLES()     0
#  define VBX_GET_DMA_HAZARD_CYCLES()       0
#  define VBX_GET_DMA_QUEUE_STALL_CYCLES()  0
#  define VBX_COUNTER_RESET()               vbxsim_reset_counts()
#endif

struct vbx_counters{
	uint32_t total_cycles;
	uint32_t writeback_cycles;
	uint32_t instructions;
	uint32_t dma_cycles;
	uint32_t dmas;
	uint32_t instr_hazard_cycles;
	uint32_t dma_hazard_cycles;
	uint32_t dma_queue_stall_cycles;
};

#ifdef  __NIOS2__
#  include "io.h"
static inline struct vbx_counters get_counter_snapshot() {
	//set snapshot
	int base = VBX1_STATUS_SLAVE_BASE;
	IORD(base, VBX_STATUS_SNAPSHOT);
	struct vbx_counters to_ret;
	to_ret.total_cycles          = IORD(base, VBX_STATUS_SNAPSHOT_TOTAL_CYCLES);
	to_ret.writeback_cycles      = IORD(base, VBX_STATUS_SNAPSHOT_WRITEBACK_CYCLES);
	to_ret.instructions          = IORD(base, VBX_STATUS_SNAPSHOT_INSTRUCTIONS);
	to_ret.dma_cycles            = IORD(base, VBX_STATUS_SNAPSHOT_DMA_CYCLES);
	to_ret.dmas                  = IORD(base, VBX_STATUS_SNAPSHOT_DMAS);
	to_ret.instr_hazard_cycles   = IORD(base, VBX_STATUS_SNAPSHOT_INSTR_HAZARD_CYCLES);
	to_ret.dma_hazard_cycles     = IORD(base, VBX_STATUS_SNAPSHOT_DMA_HAZARD_CYCLES);
	to_ret.dma_queue_stall_cycles= IORD(base, VBX_STATUS_SNAPSHOT_DMA_QUEUE_STALL_CYCLES);
	return to_ret;
}
#elif VBX_SIMULATOR
static inline struct vbx_counters get_counter_snapshot() {
	struct vbx_counters to_ret;
	to_ret.total_cycles          = 0;
	to_ret.writeback_cycles      = vbxsim_get_instr_cycles(-1,VBX_GET_THIS_MXP()->vector_lanes);
	to_ret.instructions          = vbxsim_get_instr_count(-1);
	to_ret.dma_cycles            = vbxsim_get_dma_cycles(VBX_GET_THIS_MXP()->dma_alignment_bytes /4);
	to_ret.dmas                  = vbxsim_get_dma_count();
	to_ret.instr_hazard_cycles   = 0;
	to_ret.dma_hazard_cycles     = 0;
	to_ret.dma_queue_stall_cycles= 0;
	return to_ret;
}
#endif


#define PRINT_COUNTERS()	  \
	do { \
	struct vbx_counters cnt=get_counter_snapshot(); \
	printf("TOTAL_CYCLES           %d\n",cnt.total_cycles); \
	printf("WRITEBACK_CYCLES       %d\n",cnt.writeback_cycles); \
	printf("INSTRUCTIONS           %d\n",cnt.instructions); \
	printf("DMA_CYCLES             %d\n",cnt.dma_cycles); \
	printf("DMAS                   %d\n",cnt.dmas); \
	printf("INSTR_HAZARD_CYCLES    %d\n",cnt.instr_hazard_cycles); \
	printf("DMA_HAZARD_CYCLES      %d\n",cnt.dma_hazard_cycles); \
	printf("DMA_QUEUE_STALL_CYCLES %d\n",cnt.dma_queue_stall_cycles);	\
	} while(0)

#endif //n__VBX_COUNTERS_H
