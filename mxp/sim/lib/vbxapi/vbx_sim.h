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

/**
 * @file
 * @defgroup VBX_sim VBX Simulator
 * @brief VBX simulator
 *
 * @ingroup VBXapi
 */
/**@{*/

#ifndef __VBX_SIM_H
#define __VBX_SIM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __VBX_ASM_OR_SIM_H
#error "This header file should not be included directly.\
 Instead, include \"vbx_asm_or_sim.h\""
#endif

#if !VBX_SIMULATOR
#error "Something went wrong, this file should not be included if VBX_SIMULATOR is not defined"
#endif
// -------------------------------------
// Vector API: Valid Mode Settings

#define MOD_NONE       1
#define MOD_ACC        2
#define MOD_2D         3
#define MOD_ACC_2D     4

// ----------------------
// Primary modes

typedef enum {
    V_SEBBS ,
    V_SEBBU,
    V_SVBBS,
    V_SVBBU,
    V_VEBBS,
    V_VEBBU,
    V_VVBBS,
    V_VVBBU,

    V_SEBHS,
    V_SEBHU,
    V_SVBHS,
    V_SVBHU,
    V_VEBHS,
    V_VEBHU,
    V_VVBHS,
    V_VVBHU,

    V_SEBWS,
    V_SEBWU,
    V_SVBWS,
    V_SVBWU,
    V_VEBWS,
    V_VEBWU,
    V_VVBWS,
    V_VVBWU,

    V_SEHBS,
    V_SEHBU,
    V_SVHBS,
    V_SVHBU,
    V_VEHBS,
    V_VEHBU,
    V_VVHBS,
    V_VVHBU,

    V_SEHHS,
    V_SEHHU,
    V_SVHHS,
    V_SVHHU,
    V_VEHHS,
    V_VEHHU,
    V_VVHHS,
    V_VVHHU,

    V_SEHWS,
    V_SEHWU,
    V_SVHWS,
    V_SVHWU,
    V_VEHWS,
    V_VEHWU,
    V_VVHWS,
    V_VVHWU,

    V_SEWBS,
    V_SEWBU,
    V_SVWBS,
    V_SVWBU,
    V_VEWBS,
    V_VEWBU,
    V_VVWBS,
    V_VVWBU,

    V_SEWHS,
    V_SEWHU,
    V_SVWHS,
    V_SVWHU,
    V_VEWHS,
    V_VEWHU,
    V_VVWHS,
    V_VVWHU,

    V_SEWWS,
    V_SEWWU,
    V_SVWWS,
    V_SVWWU,
    V_VEWWS,
    V_VEWWU,
    V_VVWWS,
    V_VVWWU
}vbxsim_vmode_t;

// ----------------------

// byte-to-byte ops
#define SEB  SEBBS
#define SEBS SEBBS
#define SEBU SEBBU
#define SVV  SVBBS
#define SVBS SVBBS
#define SVBU SVBBU
#define SVB  SVBBS
#define VEB  VEBBS
#define VEBS VEBBS
#define VEBU VEBBU
#define VVB  VVBBS
#define VVBS VVBBS
#define VVBU VVBBU
//
#define SEBB  SEBBS
#define SEBBS SEBBS
#define SEBBU SEBBU
#define SVBB  SVBBS
#define SVBBS SVBBS
#define SVBBU SVBBU
#define VEBB  VEBBS
#define VEBBS VEBBS
#define VEBBU VEBBU
#define VVBB  VVBBS
#define VVBBS VVBBS
#define VVBBU VVBBU

// byte-to-half ops
#define SEBH  SEBHS
#define SEBHS SEBHS
#define SEBHU SEBHU
#define SVBH  SVBHS
#define SVBHS SVBHS
#define SVBHU SVBHU
#define VEBH  VEBHS
#define VEBHS VEBHS
#define VEBHU VEBHU
#define VVBH  VVBHS
#define VVBHS VVBHS
#define VVBHU VVBHU

// byte-to-word ops
#define SEBW  SEBWS
#define SEBWS SEBWS
#define SEBWU SEBWU
#define SVBW  SVBWS
#define SVBWS SVBWS
#define SVBWU SVBWU
#define VEBW  VEBWS
#define VEBWS VEBWS
#define VEBWU VEBWU
#define VVBW  VVBWS
#define VVBWS VVBWS
#define VVBWU VVBWU

// half-to-half ops
#define SEH  SEHHS
#define SEHS SEHHS
#define SEHU SEHHU
#define SVH  SVHHS
#define SVHS SVHHS
#define SVHU SVHHU
#define VEH  VEHHS
#define VEHS VEHHS
#define VEHU VEHHU
#define VVH  VVHHS
#define VVHS VVHHS
#define VVHU VVHHU
//
#define SEHH  SEHHS
#define SEHHS SEHHS
#define SEHHU SEHHU
#define SVHH  SVHHS
#define SVHHS SVHHS
#define SVHHU SVHHU
#define VEHH  VEHHS
#define VEHHS VEHHS
#define VEHHU VEHHU
#define VVHH  VVHHS
#define VVHHS VVHHS
#define VVHHU VVHHU

// half-to-byte ops
#define SEHB  SEHBS
#define SEHBS SEHBS
#define SEHBU SEHBU
#define SVHB  SVHBS
#define SVHBS SVHBS
#define SVHBU SVHBU
#define VEHB  VEHBS
#define VEHBS VEHBS
#define VEHBU VEHBU
#define VVHB  VVHBS
#define VVHBS VVHBS
#define VVHBU VVHBU

// half-to-word ops
#define SEHW  SEHWS
#define SEHWS SEHWS
#define SEHWU SEHWU
#define SVHW  SVHWS
#define SVHWS SVHWS
#define SVHWU SVHWU
#define VEHW  VEHWS
#define VEHWS VEHWS
#define VEHWU VEHWU
#define VVHW  VVHWS
#define VVHWS VVHWS
#define VVHWU VVHWU

// word-to-word ops
#define SEW  SEWWS
#define SEWS SEWWS
#define SEWU SEWWU
#define SVW  SVWWS
#define SVWS SVWWS
#define SVWU SVWWU
#define VEW  VEWWS
#define VEWS VEWWS
#define VEWU VEWWU
#define VVW  VVWWS
#define VVWS VVWWS
#define VVWU VVWWU
//
#define SEWW  SEWWS
#define SEWWS SEWWS
#define SEWWU SEWWU
#define SVWW  SVWWS
#define SVWWS SVWWS
#define SVWWU SVWWU
#define VEWW  VEWWS
#define VEWWS VEWWS
#define VEWWU VEWWU
#define VVWW  VVWWS
#define VVWWS VVWWS
#define VVWWU VVWWU

// word-to-byte ops
#define SEWB  SEWBS
#define SEWBS SEWBS
#define SEWBU SEWBU
#define SVWB  SVWBS
#define SVWBS SVWBS
#define SVWBU SVWBU
#define VEWB  VEWBS
#define VEWBS VEWBS
#define VEWBU VEWBU
#define VVWB  VVWBS
#define VVWBS VVWBS
#define VVWBU VVWBU

// word-to-half ops
#define SEWH  SEWHS
#define SEWHS SEWHS
#define SEWHU SEWHU
#define SVWH  SVWHS
#define SVWHS SVWHS
#define SVWHU SVWHU
#define VEWH  VEWHS
#define VEWHS VEWHS
#define VEWHU VEWHU
#define VVWH  VVWHS
#define VVWHS VVWHS
#define VVWHU VVWHU

void vbxsim_setup_mask(vbxsim_vmode_t vmode,vinstr_t vinstr,void *src);
void vbxsim_setup_mask_masked(vbxsim_vmode_t vmode,vinstr_t vinstr,void *src);


// -------------------------------------
// Vector API: Macros

// NOTE: the double-macro calling is required to ensure macro arguments are fully expanded.
#define _vbxasm(VMODE,VINSTR,DEST,SRCA,SRCB)            vbxsim_1D_##VMODE(V_##VMODE,VINSTR,DEST,SRCA,SRCB)
#define _vbxasm_acc(VMODE,VINSTR,DEST,SRCA,SRCB)        vbxsim_acc_1D_##VMODE(V_##VMODE,VINSTR,DEST,SRCA,SRCB)
#define _vbxasm_2D(VMODE,VINSTR,DEST,SRCA,SRCB)         vbxsim_2D_##VMODE(V_##VMODE,VINSTR,DEST,SRCA,SRCB)
#define _vbxasm_acc_2D(VMODE,VINSTR,DEST,SRCA,SRCB)     vbxsim_acc_2D_##VMODE(V_##VMODE,VINSTR,DEST,SRCA,SRCB)
#define _vbxasm_3D(VMODE,VINSTR,DEST,SRCA,SRCB)         vbxsim_3D_##VMODE(V_##VMODE,VINSTR,DEST,SRCA,SRCB)
#define _vbxasm_acc_3D(VMODE,VINSTR,DEST,SRCA,SRCB)     vbxsim_acc_3D_##VMODE(V_##VMODE,VINSTR,DEST,SRCA,SRCB)
#define _vbxasm_masked(VMODE,VINSTR,DEST,SRCA,SRCB)     vbxsim_masked_1D_##VMODE(V_##VMODE,VINSTR,DEST,SRCA,SRCB)
#define _vbxasm_masked_acc(VMODE,VINSTR,DEST,SRCA,SRCB) vbxsim_masked_acc_1D_##VMODE(V_##VMODE,VINSTR,DEST,SRCA,SRCB)
#define _vbxasm_setup_mask(VMODE,VINSTR,SRC)            vbxsim_setup_mask(V_##VMODE,VINSTR,SRC)
#define _vbxasm_setup_mask_masked(VMODE,VINSTR,SRC)     vbxsim_setup_mask_masked(V_##VMODE,VINSTR,SRC)
/**@name VBX Assembly Macros*/
/**@{*/
#define vbxasm(VMODE,VINSTR,DEST,SRCA,SRCB)             _vbxasm(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_acc(VMODE,VINSTR,DEST,SRCA,SRCB)         _vbxasm_acc(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_2D(VMODE,VINSTR,DEST,SRCA,SRCB)          _vbxasm_2D(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_acc_2D(VMODE,VINSTR,DEST,SRCA,SRCB)      _vbxasm_acc_2D(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_3D(VMODE,VINSTR,DEST,SRCA,SRCB)          _vbxasm_3D(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_acc_3D(VMODE,VINSTR,DEST,SRCA,SRCB)      _vbxasm_acc_3D(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_masked(VMODE,VINSTR,DEST,SRCA,SRCB)      _vbxasm_masked(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_masked_acc(VMODE,VINSTR,DEST,SRCA,SRCB)  _vbxasm_masked_acc(VMODE,VINSTR,DEST,SRCA,SRCB)

#define vbxasm_setup_mask(VMODE,VINSTR,SRC)             _vbxasm_setup_mask(VMODE,VINSTR,SRC)
#define vbxasm_setup_mask_masked(VMODE,VINSTR,SRC)      _vbxasm_setup_mask_masked(VMODE,VINSTR,SRC)
/**@}*/

#include "vbx_simproto.h"

// -------------------------------------
// Vector API: Prototypes

void vbxsim_init( int num_lanes,
                  int scratchpad_capacity_kb ,
                  int max_masked_waves,
                  int fxp_word_frac_bits,
                  int fxp_half_frac_bits,
                  int fxp_byte_frac_bits);

void vbxsim_destroy();

typedef struct{
	//bool reset;         ///< Global (hard) synchronous reset
	uint16_t valid;       ///< Current wavefront contains valid data
	char vector_start;	 ///< First cycle of vector operation
	char vector_end;	    ///< last cycle of vector operation
	void* dest_addr_in;	 ///< Destination (writeback) address from address generation
	void* dest_addr_out;	 ///< Destination (writeback) address to be written  (OUTPUT)
	char sign;				 ///< Signed operation
	int opsize;           ///< Datasize (00=Byte, 01=Halfword, 10=Word)
	void* byte_valid;	    ///< Bytes containing valid data
	void* byte_enable;    ///< Bytes to be written to scratchpad              (OUTPUT)
	void* data_a;	       ///< Source A input data
	void* flag_a;			 ///< Source A input flags
	void* data_b;	       ///< Source B input data
	void* flag_b;			 ///< Source B input flags
	void* data_out;       ///< Destination (writeback) data          (OUTPUT)
	void* flag_out;       ///< Destination (writeback) flags         (OUTPUT)
}vbxsim_custom_instr_t;

typedef void (*custom_instr_func)(vbxsim_custom_instr_t*);
void vbxsim_set_custom_instruction(int opcode_start,
                                   int internal_functions,
                                   int lanes,
                                   custom_instr_func fun);


#define MAX_VEC_LANE  /*2^9*/ 9
struct simulator_statistics{
	union{
		struct {
			unsigned VMOV[MAX_VEC_LANE];
			unsigned VAND[MAX_VEC_LANE];
			unsigned VOR[MAX_VEC_LANE];
			unsigned VXOR[MAX_VEC_LANE];
			unsigned VADD[MAX_VEC_LANE];
			unsigned VSUB[MAX_VEC_LANE];
			unsigned VADDC[MAX_VEC_LANE];
			unsigned VSUBB[MAX_VEC_LANE];
			unsigned VMUL[MAX_VEC_LANE];
			unsigned VMULHI[MAX_VEC_LANE];
			unsigned VMULFXP[MAX_VEC_LANE];
			unsigned VSHL[MAX_VEC_LANE];
			unsigned VSHR[MAX_VEC_LANE];
			unsigned VROTL[MAX_VEC_LANE];
			unsigned VROTR[MAX_VEC_LANE];
			unsigned VCMV_LEZ[MAX_VEC_LANE];
			unsigned VCMV_GTZ[MAX_VEC_LANE];
			unsigned VCMV_LTZ[MAX_VEC_LANE];
			unsigned VCMV_GEZ[MAX_VEC_LANE];
			unsigned VCMV_Z[MAX_VEC_LANE];
			unsigned VCMV_NZ[MAX_VEC_LANE];
			unsigned VABSDIFF[MAX_VEC_LANE];
			unsigned VCUSTOM0[MAX_VEC_LANE];
			unsigned VCUSTOM1[MAX_VEC_LANE];
			unsigned VCUSTOM2[MAX_VEC_LANE];
			unsigned VCUSTOM3[MAX_VEC_LANE];
			unsigned VCUSTOM4[MAX_VEC_LANE];
			unsigned VCUSTOM5[MAX_VEC_LANE];
			unsigned VCUSTOM6[MAX_VEC_LANE];
			unsigned VCUSTOM7[MAX_VEC_LANE];
			unsigned VCUSTOM8[MAX_VEC_LANE];
			unsigned VCUSTOM9[MAX_VEC_LANE];
			unsigned VCUSTOM10[MAX_VEC_LANE];
			unsigned VCUSTOM11[MAX_VEC_LANE];
			unsigned VCUSTOM12[MAX_VEC_LANE];
			unsigned VCUSTOM13[MAX_VEC_LANE];
			unsigned VCUSTOM14[MAX_VEC_LANE];
			unsigned VCUSTOM15[MAX_VEC_LANE];
		}as_name;
		unsigned as_array[MAX_INSTR_VAL+1][MAX_VEC_LANE];
	}instruction_cycles;
	unsigned int instruction_count[MAX_INSTR_VAL+1];
	unsigned int set_vl;
	unsigned int set_2D;
	unsigned int set_3D;
	unsigned int dma_bytes;
	unsigned int dma_calls;
	unsigned int dma_cycles[MAX_VEC_LANE];
};
struct simulator_statistics vbxsim_get_stats();
//deferred is default because it is zero, static variables are initialized to zero
enum dma_type {DEFERRED=0,	IMMEDIATE=1};
void vbxsim_set_dma_type(enum dma_type);
//reset all statistics to zero
void vbxsim_reset_stats();
//print out all the wave counts for all the instructions for all the
//lane sizes;
void vbxsim_print_stats();
void vbxsim_print_stats_extended();
//disable simulator specific warnings
void vbxsim_disable_warnings();
//enable simulator specific warnings
void vbxsim_enable_warnings();
#ifdef __cplusplus
}
#endif

#endif // __VBX_SIM_H
/**@}*/
