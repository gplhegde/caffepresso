/* VECTORBLOX MXP SOFTWARE DEVELOPMENT KIT
 *
 * Copyright (C) 2012-2016 VectorBlox Computing Inc., Vancouver, British Columbia, Canada.
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


// Cortex-A9 / ARMv7 version - uses memory-mapped instruction port.

#ifndef __VBX_ASM_ARM_H
#define __VBX_ASM_ARM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __VBX_ASM_OR_SIM_H
#error "This header file should not be included directly. Instead, include \"vbx_asm_or_sim.h\""
#else

#if VBX_ASSEMBLER

#include "vbx_macros.h"

#include "vbx_asm_enc32.h"


// -------------------------------------

// Convert a possibly uncached external memory address from the CPU
// to a physical address that the DMA engine can use. (This happens to be
// the same address that the CPU would use for a cached access.)


#if ARM_XIL_STANDALONE

// Assume that cacheable shared memory in the 1GB range 0x0-0x3fff_ffff
// can also be accessed in an uncached manner by the ARM CPU via the
// in the range 0x8000_0000-0xbfff_ffff (i.e. by setting address bit 31 to 1).
#define VBX_DCACHE_BYPASS_MASK  0x7fffffff
#define VBX_DCACHE_BYPASS_VAL   0x80000000
#define VBX_DCACHE_NOBYPASS_VAL 0x00000000

#define VBX_DMA_ADDR(x,len)	  \
	(( ((uint32_t) (x)) & VBX_DCACHE_BYPASS_MASK) | VBX_DCACHE_NOBYPASS_VAL)

#define VBX_UNCACHED_ADDR(x) \
	(( ((uint32_t) (x)) & VBX_DCACHE_BYPASS_MASK) | VBX_DCACHE_BYPASS_VAL)
#define VBX_CACHED_ADDR(x)   \
	(( ((uint32_t) (x)) & VBX_DCACHE_BYPASS_MASK) | VBX_DCACHE_NOBYPASS_VAL)
#elif ARM_ALT_STANDALONE
#define VBX_DCACHE_BYPASS_MASK  0x3FFFFFFF
#define VBX_DCACHE_NOBYPASS_VAL 0x00000000
#define VBX_DMA_ADDR(x,len)	  \
	(( ((uint32_t) (x)) & VBX_DCACHE_BYPASS_MASK) | VBX_DCACHE_NOBYPASS_VAL)

#else
//linux
//use this macro to 	translate to  physical address
	void* translate_for_dma(void* virt_addr, size_t len);
#define VBX_DMA_ADDR(x,len)      ((uint32_t)translate_for_dma(x,len))



#endif

// -------------------------------------

// Address of the AXI4 memory-mapped instruction port.
// This is XPAR_VECTORBLOX_MXP_ARM_0_S_AXI_INSTR_BASEADDR in xparameters.h,
// but we can't use xparameters.h if we want to be able to compile vbxlib
// without a BSP.
#if VBX_USE_GLOBAL_MXP_PTR
#define VBX_INSTR_PORT_ADDR (vbx_mxp_ptr->instr_port_addr)
#else
#include "xparameters.h"
#define VBX_INSTR_PORT_ADDR XPAR_VECTORBLOX_MXP_ARM_0_S_AXI_INSTR_BASEADDR
#endif

//
// load/store ASM macros from standalone_v3_09_a/src/xpseudo_asm_gcc.h
//
#define vbx_getw(val) __asm__ __volatile__(\
                          "ldr        %0,[%1]"\
                          : "=r" (val) : "r" (VBX_INSTR_PORT_ADDR)\
                      )

#define vbx_getw_dummy() ({unsigned long rval;\
                           __asm__ __volatile__(\
                             "ldr        %0,[%1]"\
                             : "=r" (rval) : "r" (VBX_INSTR_PORT_ADDR)\
                           );\
                          })

#define vbx_putw(val) __asm__ __volatile__(\
                        "str  %0,[%1]\n"\
                        : : "r" (val), "r" (VBX_INSTR_PORT_ADDR)\
                      )

#if VBX_USE_AXI_INSTR_PORT_ADDR_INCR && VBX_USE_GLOBAL_MXP_PTR

// Write instruction words to consecutive addresses to allow the Cortex-A9's
// store buffer to merge writes into bursts, provided the instruction port
// is mapped to normal, cacheable memory.
// XXX Seems that store buffer in A9 or in L2 Cache Controller can re-order
// and/or repeat writes for normal, cacheable memory.

// Update INSTR_P to the incremented pointer value P such that INSTR_P remains
// within a 256-byte offset of the instruction port base address. (i.e. the
// the address wraps around after 256 bytes.) The 256-byte window is larger
// than the A9's 32-byte ("4-entry, 64-bit") merging store buffer and the
// L2 Cache Controller's 96-byte (3 slots, 256-bits each) store buffer in an
// effort to prevent unwanted merging of instruction words that are written to
// the same address when the memory region type is set to "normal".
#define VBX_AXI_INSTR_P_UPDATE(INSTR_P,P)	  \
	(INSTR_P) = (uint32_t *) (((uint32_t) vbx_mxp_ptr->instr_port_addr) + \
	                          (((uint32_t) (P)) & 0xff))

#if VBX_USE_AXI_INSTR_PORT_VST

// Use NEON vector store instruction (VST) to write MXP instructions to
// instruction port.
#include "arm_neon.h"

#define VBX_INSTR_QUAD(W0, W1, W2, W3) \
	VBX_S{ \
		uint32x4_t __v__; \
		volatile uint32_t *__p__ = (volatile uint32_t *) (vbx_mxp_ptr->instr_p); \
		__v__ = vdupq_n_u32(0); \
		__v__ = vsetq_lane_u32((uint32_t) (W0), __v__, 0); \
		__v__ = vsetq_lane_u32((uint32_t) (W1), __v__, 1); \
		__v__ = vsetq_lane_u32((uint32_t) (W2), __v__, 2); \
		__v__ = vsetq_lane_u32((uint32_t) (W3), __v__, 3); \
		vst1q_u32((uint32_t *) __p__, __v__); \
		VBX_AXI_INSTR_P_UPDATE(vbx_mxp_ptr->instr_p, __p__+4); \
	}VBX_E

#define VBX_INSTR_DOUBLE(W0, W1) \
	VBX_S{ \
		uint32x2_t __v__; \
		volatile uint32_t *__p__ = (volatile uint32_t *) (vbx_mxp_ptr->instr_p); \
		__v__ = vdup_n_u32(0); \
		__v__ = vset_lane_u32((uint32_t) (W0), __v__, 0); \
		__v__ = vset_lane_u32((uint32_t) (W1), __v__, 1); \
		vst1_u32((uint32_t *) __p__, __v__); \
		VBX_AXI_INSTR_P_UPDATE(vbx_mxp_ptr->instr_p, __p__+4); \
	}VBX_E

#else // !USE_VST

#define VBX_INSTR_QUAD(W0, W1, W2, W3) \
	VBX_S{ \
		volatile uint32_t *__p__ = (volatile uint32_t *) (vbx_mxp_ptr->instr_p); \
		*__p__++ = (uint32_t) (W0); \
		*__p__++ = (uint32_t) (W1); \
		*__p__++ = (uint32_t) (W2); \
		*__p__++ = (uint32_t) (W3); \
		VBX_AXI_INSTR_P_UPDATE(vbx_mxp_ptr->instr_p, __p__); \
	}VBX_E

#define VBX_INSTR_DOUBLE(W0, W1) \
	VBX_S{
		volatile uint32_t *__p__ = (volatile uint32_t *) (vbx_mxp_ptr->instr_p); \
		*__p__++ = (uint32_t) (W0); \
		*__p__++ = (uint32_t) (W1); \
		__p__ += 2; \
		VBX_AXI_INSTR_P_UPDATE(vbx_mxp_ptr->instr_p, __p__); \
	}VBX_E

#endif // !USE_VST

#define VBX_INSTR_SINGLE(W0, RETURN_VAR) \
	VBX_S{ \
		volatile uint32_t *__p__ = (volatile uint32_t *) (vbx_mxp_ptr->instr_p); \
		*__p__++ = (uint32_t) (W0); \
		(RETURN_VAR) = *__p__++; \
		__p__ += 2; \
		VBX_AXI_INSTR_P_UPDATE(vbx_mxp_ptr->instr_p, __p__); \
	}VBX_E

#else // !(VBX_USE_AXI_INSTR_PORT_ADDR_INCR && VBX_USE_GLOBAL_MXP_PTR)

// Write instruction words to the same address.
// Instruction port must be mapped to device or strongly-ordered memory
// (to prevent merging of writes to the same address).

#if VBX_USE_AXI_INSTR_PORT_VST

#include "arm_neon.h"

#define VBX_INSTR_QUAD(W0, W1, W2, W3) \
	VBX_S{ \
		uint32x4_t __v__; \
		volatile uint32_t *__p__ = (volatile uint32_t *) (VBX_INSTR_PORT_ADDR); \
		__v__ = vdupq_n_u32(0); \
		__v__ = vsetq_lane_u32((uint32_t) (W0), __v__, 0); \
		__v__ = vsetq_lane_u32((uint32_t) (W1), __v__, 1); \
		__v__ = vsetq_lane_u32((uint32_t) (W2), __v__, 2); \
		__v__ = vsetq_lane_u32((uint32_t) (W3), __v__, 3); \
		vst1q_u32((uint32_t *) __p__, __v__); \
	}VBX_E

#define VBX_INSTR_DOUBLE(W0, W1) \
	VBX_S{ \
		uint32x2_t __v__; \
		volatile uint32_t *__p__ = (volatile uint32_t *) (VBX_INSTR_PORT_ADDR); \
		__v__ = vdup_n_u32(0); \
		__v__ = vset_lane_u32((uint32_t) (W0), __v__, 0); \
		__v__ = vset_lane_u32((uint32_t) (W1), __v__, 1); \
		vst1_u32((uint32_t *) __p__, __v__); \
	}VBX_E

#else // !USE_VST

#define VBX_INSTR_QUAD(W0, W1, W2, W3) \
	VBX_S{ \
		vbx_putw((W0)); \
		vbx_putw((W1)); \
		vbx_putw((W2)); \
		vbx_putw((W3)); \
	}VBX_E

#define VBX_INSTR_DOUBLE(W0, W1) \
	VBX_S{ \
		vbx_putw((W0)); \
		vbx_putw((W1)); \
	}VBX_E

#endif // !USE_VST

#define VBX_INSTR_SINGLE(W0, RETURN_VAR) \
	VBX_S{ \
		vbx_putw((W0)); \
		vbx_getw((RETURN_VAR)); \
	}VBX_E

#endif // !(VBX_USE_AXI_INSTR_PORT_ADDR_INCR && VBX_USE_GLOBAL_MXP_PTR)

#define VBX_ASM(MODIFIERS,VMODE,VINSTR,DEST,SRCA,SRCB) \
	VBX_INSTR_QUAD((((VINSTR) << (VBX_OPCODE_SHIFT)) | (VMODE) | (MODIFIERS)), \
	               (SRCA), (SRCB), (DEST))

// -------------------------------------

#endif // VBX_ASSEMBLER
#endif // __VBX_ASM_OR_SIM_H

#ifdef __cplusplus
}
#endif

#endif // __VBX_ASM_ARM_H
