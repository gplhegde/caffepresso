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


// 32-bit instruction encoding used by MicroBlaze and ARM.

#ifndef __VBX_ASM_ENC32_H
#define __VBX_ASM_ENC32_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __VBX_ASM_OR_SIM_H
#error "This header file should not be included directly. Instead, include \"vbx_asm_or_sim.h\""
#else

#if VBX_ASSEMBLER

#include "vbx_macros.h"

// -------------------------------------

// bits 16:11  3d, 2d, acc, masked, sv, ve
// bits 10:9   in_size
// bits 8:7    out_size
// bit  6      signed/unsigned
// bits 5:0    opcode
//
//     sv ve
// VV:  0  0
// VE:  0  1
// SV:  1  0
// SE:  1  1

#define THREED_BIT       16
#define TWOD_BIT         15
#define ACCUM_BIT        14
#define MASKED_BIT       13
#define SCALAR_BIT       12
#define ENUM_BIT         11
#define IN_SIZE_RIGHT    9
#define OUT_SIZE_RIGHT   7
#define SIGNED_BIT       6
#define VBX_OPCODE_SHIFT 0

#define VBX_MODE_SIGNED  (1 << SIGNED_BIT)


#define VBX_OPSIZE_BYTE  0
#define VBX_OPSIZE_HALF  1
#define VBX_OPSIZE_WORD  2

#define VBX_BYTE_IN      (VBX_OPSIZE_BYTE << IN_SIZE_RIGHT)
#define VBX_HALF_IN      (VBX_OPSIZE_HALF << IN_SIZE_RIGHT)
#define VBX_WORD_IN      (VBX_OPSIZE_WORD << IN_SIZE_RIGHT)

#define VBX_BYTE_OUT     (VBX_OPSIZE_BYTE << OUT_SIZE_RIGHT)
#define VBX_HALF_OUT     (VBX_OPSIZE_HALF << OUT_SIZE_RIGHT)
#define VBX_WORD_OUT     (VBX_OPSIZE_WORD << OUT_SIZE_RIGHT)

#define VBX_MODE_VV      0
#define VBX_MODE_VE      (1 << ENUM_BIT)
#define VBX_MODE_SV      (1 << SCALAR_BIT)
#define VBX_MODE_SE      ((1 << ENUM_BIT) | (1 << SCALAR_BIT))

#define MOD_NONE         0
#define MOD_MASKED       (1 << MASKED_BIT)
#define MOD_ACC          (1 << ACCUM_BIT)
#define MOD_2D           (1 << TWOD_BIT)
// The 2d and 3d flags must both be set for 3d operations.
#define MOD_3D           ((1 << THREED_BIT) | (1 << TWOD_BIT))
// A fake '4D' mode (3D bit w/o 2D bit) is used in mask setup instructions
#define MOD_4D           (1 << THREED_BIT)

#define MOD_MASKED_ACC    (MOD_MASKED | MOD_ACC)
#define MOD_MASKED_2D     (MOD_MASKED | MOD_2D)
#define MOD_MASKED_3D     (MOD_MASKED | MOD_3D)
#define MOD_MASKED_4D     (MOD_MASKED | MOD_4D)
#define MOD_ACC_2D        (MOD_ACC | MOD_2D)
#define MOD_ACC_3D        (MOD_ACC | MOD_3D)
#define MOD_ACC_4D        (MOD_ACC | MOD_4D)
#define MOD_MASKED_ACC_2D (MOD_MASKED | MOD_ACC_2D)
#define MOD_MASKED_ACC_3D (MOD_MASKED | MOD_ACC_3D)
#define MOD_MASKED_ACC_4D (MOD_MASKED | MOD_ACC_4D)

// -------------------------------------
// Vector API: Operand Modes

#define VBX_MODE_BU  (VBX_BYTE_IN | VBX_BYTE_OUT)
#define VBX_MODE_BBU (VBX_BYTE_IN | VBX_BYTE_OUT)
#define VBX_MODE_BS  (VBX_BYTE_IN | VBX_BYTE_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_BBS (VBX_BYTE_IN | VBX_BYTE_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_B   (VBX_BYTE_IN | VBX_BYTE_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_BB  (VBX_BYTE_IN | VBX_BYTE_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_BHU (VBX_BYTE_IN | VBX_HALF_OUT)
#define VBX_MODE_BHS (VBX_BYTE_IN | VBX_HALF_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_BH  (VBX_BYTE_IN | VBX_HALF_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_BWU (VBX_BYTE_IN | VBX_WORD_OUT)
#define VBX_MODE_BWS (VBX_BYTE_IN | VBX_WORD_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_BW  (VBX_BYTE_IN | VBX_WORD_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_HBU (VBX_HALF_IN | VBX_BYTE_OUT)
#define VBX_MODE_HBS (VBX_HALF_IN | VBX_BYTE_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_HB  (VBX_HALF_IN | VBX_BYTE_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_HU  (VBX_HALF_IN | VBX_HALF_OUT)
#define VBX_MODE_HHU (VBX_HALF_IN | VBX_HALF_OUT)
#define VBX_MODE_HS  (VBX_HALF_IN | VBX_HALF_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_HHS (VBX_HALF_IN | VBX_HALF_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_H   (VBX_HALF_IN | VBX_HALF_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_HH  (VBX_HALF_IN | VBX_HALF_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_HWU (VBX_HALF_IN | VBX_WORD_OUT)
#define VBX_MODE_HWS (VBX_HALF_IN | VBX_WORD_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_HW  (VBX_HALF_IN | VBX_WORD_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_WBU (VBX_WORD_IN | VBX_BYTE_OUT)
#define VBX_MODE_WBS (VBX_WORD_IN | VBX_BYTE_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_WB  (VBX_WORD_IN | VBX_BYTE_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_WHU (VBX_WORD_IN | VBX_HALF_OUT)
#define VBX_MODE_WHS (VBX_WORD_IN | VBX_HALF_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_WH  (VBX_WORD_IN | VBX_HALF_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_WU  (VBX_WORD_IN | VBX_WORD_OUT)
#define VBX_MODE_WWU (VBX_WORD_IN | VBX_WORD_OUT)
#define VBX_MODE_WS  (VBX_WORD_IN | VBX_WORD_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_WWS (VBX_WORD_IN | VBX_WORD_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_W   (VBX_WORD_IN | VBX_WORD_OUT | VBX_MODE_SIGNED)
#define VBX_MODE_WW  (VBX_WORD_IN | VBX_WORD_OUT | VBX_MODE_SIGNED)

//VV type
#define M_VVBU  (VBX_MODE_VV | VBX_MODE_BU)
#define M_VVBBU (VBX_MODE_VV | VBX_MODE_BBU)
#define M_VVBS  (VBX_MODE_VV | VBX_MODE_BS)
#define M_VVBBS (VBX_MODE_VV | VBX_MODE_BBS)
#define M_VVB   (VBX_MODE_VV | VBX_MODE_B)
#define M_VVBB  (VBX_MODE_VV | VBX_MODE_BB)
#define M_VVBHU (VBX_MODE_VV | VBX_MODE_BHU)
#define M_VVBHS (VBX_MODE_VV | VBX_MODE_BHS)
#define M_VVBH  (VBX_MODE_VV | VBX_MODE_BH)
#define M_VVBWU (VBX_MODE_VV | VBX_MODE_BWU)
#define M_VVBWS (VBX_MODE_VV | VBX_MODE_BWS)
#define M_VVBW  (VBX_MODE_VV | VBX_MODE_BW)
#define M_VVHBU (VBX_MODE_VV | VBX_MODE_HBU)
#define M_VVHBS (VBX_MODE_VV | VBX_MODE_HBS)
#define M_VVHB  (VBX_MODE_VV | VBX_MODE_HB)
#define M_VVHU  (VBX_MODE_VV | VBX_MODE_HU)
#define M_VVHHU (VBX_MODE_VV | VBX_MODE_HHU)
#define M_VVHS  (VBX_MODE_VV | VBX_MODE_HS)
#define M_VVHHS (VBX_MODE_VV | VBX_MODE_HHS)
#define M_VVH   (VBX_MODE_VV | VBX_MODE_H)
#define M_VVHH  (VBX_MODE_VV | VBX_MODE_HH)
#define M_VVHWU (VBX_MODE_VV | VBX_MODE_HWU)
#define M_VVHWS (VBX_MODE_VV | VBX_MODE_HWS)
#define M_VVHW  (VBX_MODE_VV | VBX_MODE_HW)
#define M_VVWBU (VBX_MODE_VV | VBX_MODE_WBU)
#define M_VVWBS (VBX_MODE_VV | VBX_MODE_WBS)
#define M_VVWB  (VBX_MODE_VV | VBX_MODE_WB)
#define M_VVWHU (VBX_MODE_VV | VBX_MODE_WHU)
#define M_VVWHS (VBX_MODE_VV | VBX_MODE_WHS)
#define M_VVWH  (VBX_MODE_VV | VBX_MODE_WH)
#define M_VVWU  (VBX_MODE_VV | VBX_MODE_WU)
#define M_VVWWU (VBX_MODE_VV | VBX_MODE_WWU)
#define M_VVWS  (VBX_MODE_VV | VBX_MODE_WS)
#define M_VVWWS (VBX_MODE_VV | VBX_MODE_WWS)
#define M_VVW   (VBX_MODE_VV | VBX_MODE_W)
#define M_VVWW  (VBX_MODE_VV | VBX_MODE_WW)

//SV type
#define M_SVBU  (VBX_MODE_SV | VBX_MODE_BU)
#define M_SVBBU (VBX_MODE_SV | VBX_MODE_BBU)
#define M_SVBS  (VBX_MODE_SV | VBX_MODE_BS)
#define M_SVBBS (VBX_MODE_SV | VBX_MODE_BBS)
#define M_SVB   (VBX_MODE_SV | VBX_MODE_B)
#define M_SVBB  (VBX_MODE_SV | VBX_MODE_BB)
#define M_SVBHU (VBX_MODE_SV | VBX_MODE_BHU)
#define M_SVBHS (VBX_MODE_SV | VBX_MODE_BHS)
#define M_SVBH  (VBX_MODE_SV | VBX_MODE_BH)
#define M_SVBWU (VBX_MODE_SV | VBX_MODE_BWU)
#define M_SVBWS (VBX_MODE_SV | VBX_MODE_BWS)
#define M_SVBW  (VBX_MODE_SV | VBX_MODE_BW)
#define M_SVHBU (VBX_MODE_SV | VBX_MODE_HBU)
#define M_SVHBS (VBX_MODE_SV | VBX_MODE_HBS)
#define M_SVHB  (VBX_MODE_SV | VBX_MODE_HB)
#define M_SVHU  (VBX_MODE_SV | VBX_MODE_HU)
#define M_SVHHU (VBX_MODE_SV | VBX_MODE_HHU)
#define M_SVHS  (VBX_MODE_SV | VBX_MODE_HS)
#define M_SVHHS (VBX_MODE_SV | VBX_MODE_HHS)
#define M_SVH   (VBX_MODE_SV | VBX_MODE_H)
#define M_SVHH  (VBX_MODE_SV | VBX_MODE_HH)
#define M_SVHWU (VBX_MODE_SV | VBX_MODE_HWU)
#define M_SVHWS (VBX_MODE_SV | VBX_MODE_HWS)
#define M_SVHW  (VBX_MODE_SV | VBX_MODE_HW)
#define M_SVWBU (VBX_MODE_SV | VBX_MODE_WBU)
#define M_SVWBS (VBX_MODE_SV | VBX_MODE_WBS)
#define M_SVWB  (VBX_MODE_SV | VBX_MODE_WB)
#define M_SVWHU (VBX_MODE_SV | VBX_MODE_WHU)
#define M_SVWHS (VBX_MODE_SV | VBX_MODE_WHS)
#define M_SVWH  (VBX_MODE_SV | VBX_MODE_WH)
#define M_SVWU  (VBX_MODE_SV | VBX_MODE_WU)
#define M_SVWWU (VBX_MODE_SV | VBX_MODE_WWU)
#define M_SVWS  (VBX_MODE_SV | VBX_MODE_WS)
#define M_SVWWS (VBX_MODE_SV | VBX_MODE_WWS)
#define M_SVW   (VBX_MODE_SV | VBX_MODE_W)
#define M_SVWW  (VBX_MODE_SV | VBX_MODE_WW)

//VE type
#define M_VEBU  (VBX_MODE_VE | VBX_MODE_BU)
#define M_VEBBU (VBX_MODE_VE | VBX_MODE_BBU)
#define M_VEBS  (VBX_MODE_VE | VBX_MODE_BS)
#define M_VEBBS (VBX_MODE_VE | VBX_MODE_BBS)
#define M_VEB   (VBX_MODE_VE | VBX_MODE_B)
#define M_VEBB  (VBX_MODE_VE | VBX_MODE_BB)
#define M_VEBHU (VBX_MODE_VE | VBX_MODE_BHU)
#define M_VEBHS (VBX_MODE_VE | VBX_MODE_BHS)
#define M_VEBH  (VBX_MODE_VE | VBX_MODE_BH)
#define M_VEBWU (VBX_MODE_VE | VBX_MODE_BWU)
#define M_VEBWS (VBX_MODE_VE | VBX_MODE_BWS)
#define M_VEBW  (VBX_MODE_VE | VBX_MODE_BW)
#define M_VEHBU (VBX_MODE_VE | VBX_MODE_HBU)
#define M_VEHBS (VBX_MODE_VE | VBX_MODE_HBS)
#define M_VEHB  (VBX_MODE_VE | VBX_MODE_HB)
#define M_VEHU  (VBX_MODE_VE | VBX_MODE_HU)
#define M_VEHHU (VBX_MODE_VE | VBX_MODE_HHU)
#define M_VEHS  (VBX_MODE_VE | VBX_MODE_HS)
#define M_VEHHS (VBX_MODE_VE | VBX_MODE_HHS)
#define M_VEH   (VBX_MODE_VE | VBX_MODE_H)
#define M_VEHH  (VBX_MODE_VE | VBX_MODE_HH)
#define M_VEHWU (VBX_MODE_VE | VBX_MODE_HWU)
#define M_VEHWS (VBX_MODE_VE | VBX_MODE_HWS)
#define M_VEHW  (VBX_MODE_VE | VBX_MODE_HW)
#define M_VEWBU (VBX_MODE_VE | VBX_MODE_WBU)
#define M_VEWBS (VBX_MODE_VE | VBX_MODE_WBS)
#define M_VEWB  (VBX_MODE_VE | VBX_MODE_WB)
#define M_VEWHU (VBX_MODE_VE | VBX_MODE_WHU)
#define M_VEWHS (VBX_MODE_VE | VBX_MODE_WHS)
#define M_VEWH  (VBX_MODE_VE | VBX_MODE_WH)
#define M_VEWU  (VBX_MODE_VE | VBX_MODE_WU)
#define M_VEWWU (VBX_MODE_VE | VBX_MODE_WWU)
#define M_VEWS  (VBX_MODE_VE | VBX_MODE_WS)
#define M_VEWWS (VBX_MODE_VE | VBX_MODE_WWS)
#define M_VEW   (VBX_MODE_VE | VBX_MODE_W)
#define M_VEWW  (VBX_MODE_VE | VBX_MODE_WW)

//SE type
#define M_SEBU  (VBX_MODE_SE | VBX_MODE_BU)
#define M_SEBBU (VBX_MODE_SE | VBX_MODE_BBU)
#define M_SEBS  (VBX_MODE_SE | VBX_MODE_BS)
#define M_SEBBS (VBX_MODE_SE | VBX_MODE_BBS)
#define M_SEB   (VBX_MODE_SE | VBX_MODE_B)
#define M_SEBB  (VBX_MODE_SE | VBX_MODE_BB)
#define M_SEBHU (VBX_MODE_SE | VBX_MODE_BHU)
#define M_SEBHS (VBX_MODE_SE | VBX_MODE_BHS)
#define M_SEBH  (VBX_MODE_SE | VBX_MODE_BH)
#define M_SEBWU (VBX_MODE_SE | VBX_MODE_BWU)
#define M_SEBWS (VBX_MODE_SE | VBX_MODE_BWS)
#define M_SEBW  (VBX_MODE_SE | VBX_MODE_BW)
#define M_SEHBU (VBX_MODE_SE | VBX_MODE_HBU)
#define M_SEHBS (VBX_MODE_SE | VBX_MODE_HBS)
#define M_SEHB  (VBX_MODE_SE | VBX_MODE_HB)
#define M_SEHU  (VBX_MODE_SE | VBX_MODE_HU)
#define M_SEHHU (VBX_MODE_SE | VBX_MODE_HHU)
#define M_SEHS  (VBX_MODE_SE | VBX_MODE_HS)
#define M_SEHHS (VBX_MODE_SE | VBX_MODE_HHS)
#define M_SEH   (VBX_MODE_SE | VBX_MODE_H)
#define M_SEHH  (VBX_MODE_SE | VBX_MODE_HH)
#define M_SEHWU (VBX_MODE_SE | VBX_MODE_HWU)
#define M_SEHWS (VBX_MODE_SE | VBX_MODE_HWS)
#define M_SEHW  (VBX_MODE_SE | VBX_MODE_HW)
#define M_SEWBU (VBX_MODE_SE | VBX_MODE_WBU)
#define M_SEWBS (VBX_MODE_SE | VBX_MODE_WBS)
#define M_SEWB  (VBX_MODE_SE | VBX_MODE_WB)
#define M_SEWHU (VBX_MODE_SE | VBX_MODE_WHU)
#define M_SEWHS (VBX_MODE_SE | VBX_MODE_WHS)
#define M_SEWH  (VBX_MODE_SE | VBX_MODE_WH)
#define M_SEWU  (VBX_MODE_SE | VBX_MODE_WU)
#define M_SEWWU (VBX_MODE_SE | VBX_MODE_WWU)
#define M_SEWS  (VBX_MODE_SE | VBX_MODE_WS)
#define M_SEWWS (VBX_MODE_SE | VBX_MODE_WWS)
#define M_SEW   (VBX_MODE_SE | VBX_MODE_W)
#define M_SEWW  (VBX_MODE_SE | VBX_MODE_WW)

// -------------------------------------
// Vector API: Instructions

#define VMOV_I       4
#define VAND_I       5
#define VOR_I        6
#define VXOR_I       7

#define VADD_I       8
#define VSUB_I       9
#define VADDC_I     10
#define VSUBB_I     11

#define VMUL_I      12
#define VMULLO_I    12
#define VMULHI_I    13
#define VMULFXP_I   14

#define VSHL_I      16
#define VSHR_I      17
#define VROTL_I     18
#define VROTR_I     19

#define VCMV_LEZ_I  24 // N | Z
#define VCMV_GTZ_I  25 // ~N & ~Z
#define VCMV_LTZ_I  26 // N
#define VCMV_FS_I   26 // N
#define VCMV_GEZ_I  27 // ~N
#define VCMV_FC_I   27 // ~N
#define VCMV_Z_I    28 // Z
#define VCMV_NZ_I   29 // ~Z

#define VABSDIFF_I  31

#define VCUSTOM_I    48
#define VCUSTOM0_I   48
#define VCUSTOM1_I   49
#define VCUSTOM2_I   50
#define VCUSTOM3_I   51
#define VCUSTOM4_I   52
#define VCUSTOM5_I   53
#define VCUSTOM6_I   54
#define VCUSTOM7_I   55
#define VCUSTOM8_I   56
#define VCUSTOM9_I   57
#define VCUSTOM10_I  58
#define VCUSTOM11_I  59
#define VCUSTOM12_I  60
#define VCUSTOM13_I  61
#define VCUSTOM14_I  62
#define VCUSTOM15_I  63


// -------------------------------------
// Vector API: Prototypes

#define _vbxasm(VMODE,VINSTR,DEST,SRCA,SRCB)               VBX_ASM(MOD_NONE,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
#define _vbxasm_masked(VMODE,VINSTR,DEST,SRCA,SRCB)        VBX_ASM(MOD_MASKED,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
#define _vbxasm_acc(VMODE,VINSTR,DEST,SRCA,SRCB)           VBX_ASM(MOD_ACC,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
#define _vbxasm_masked_acc(VMODE,VINSTR,DEST,SRCA,SRCB)    VBX_ASM(MOD_MASKED_ACC,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
#define _vbxasm_2D(VMODE,VINSTR,DEST,SRCA,SRCB)            VBX_ASM(MOD_2D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
//#define _vbxasm_masked_2D(VMODE,VINSTR,DEST,SRCA,SRCB)     VBX_ASM(MOD_MASKED_2D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
#define _vbxasm_acc_2D(VMODE,VINSTR,DEST,SRCA,SRCB)        VBX_ASM(MOD_ACC_2D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
//#define _vbxasm_masked_acc_2D(VMODE,VINSTR,DEST,SRCA,SRCB) VBX_ASM(MOD_MASKED_ACC_2D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
#define _vbxasm_3D(VMODE,VINSTR,DEST,SRCA,SRCB)            VBX_ASM(MOD_3D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
//#define _vbxasm_masked_3D(VMODE,VINSTR,DEST,SRCA,SRCB)     VBX_ASM(MOD_MASKED_3D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
#define _vbxasm_acc_3D(VMODE,VINSTR,DEST,SRCA,SRCB)        VBX_ASM(MOD_ACC_3D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
//#define _vbxasm_masked_acc_3D(VMODE,VINSTR,DEST,SRCA,SRCB) VBX_ASM(MOD_MASKED_ACC_3D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
#define _vbxasm_masked_4D(VMODE,VINSTR,DEST,SRCA,SRCB)     VBX_ASM(MOD_MASKED_4D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)
#define _vbxasm_masked_acc_4D(VMODE,VINSTR,DEST,SRCA,SRCB) VBX_ASM(MOD_MASKED_ACC_4D,M_##VMODE,VINSTR##_I,DEST,SRCA,SRCB)

// NOTE: the double-macro calling is required to ensure macro arguments are fully expanded.
#define vbxasm(VMODE,VINSTR,DEST,SRCA,SRCB)               _vbxasm(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_masked(VMODE,VINSTR,DEST,SRCA,SRCB)        _vbxasm_masked(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_acc(VMODE,VINSTR,DEST,SRCA,SRCB)           _vbxasm_acc(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_masked_acc(VMODE,VINSTR,DEST,SRCA,SRCB)    _vbxasm_masked_acc(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_2D(VMODE,VINSTR,DEST,SRCA,SRCB)            _vbxasm_2D(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_acc_2D(VMODE,VINSTR,DEST,SRCA,SRCB)        _vbxasm_acc_2D(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_3D(VMODE,VINSTR,DEST,SRCA,SRCB)            _vbxasm_3D(VMODE,VINSTR,DEST,SRCA,SRCB)
#define vbxasm_acc_3D(VMODE,VINSTR,DEST,SRCA,SRCB)        _vbxasm_acc_3D(VMODE,VINSTR,DEST,SRCA,SRCB)

//These make no sense; 2D/3D during masked.
//#define vbxasm_masked_2D(VMODE,VINSTR,DEST,SRCA,SRCB)     _vbxasm_masked_2D(VMODE,VINSTR,DEST,SRCA,SRCB)
//#define vbxasm_masked_acc_2D(VMODE,VINSTR,DEST,SRCA,SRCB) _vbxasm_masked_acc_2D(VMODE,VINSTR,DEST,SRCA,SRCB)
//#define vbxasm_masked_3D(VMODE,VINSTR,DEST,SRCA,SRCB)     _vbxasm_masked_3D(VMODE,VINSTR,DEST,SRCA,SRCB)
//#define vbxasm_masked_acc_3D(VMODE,VINSTR,DEST,SRCA,SRCB) _vbxasm_masked_acc_3D(VMODE,VINSTR,DEST,SRCA,SRCB)

//2D/3D masked setup will take extra BRAMs to implement and so are not currently implemented
//Use '4D' mode bit to indicate mask set 
//Setup + acc does not make sense, so it's used as a way to do a masked mask setup
#define vbxasm_setup_mask(VMODE,VINSTR,SRC)        _vbxasm_masked_4D(VMODE,VINSTR,SRC,SRC,SRC)
#define vbxasm_setup_mask_masked(VMODE,VINSTR,SRC) _vbxasm_masked_acc_4D(VMODE,VINSTR,SRC,SRC,SRC)

#endif // VBX_ASSEMBLER
#endif // __VBX_ASM_OR_SIM_H

#ifdef __cplusplus
}
#endif

#endif // __VBX_ASM_ENC32_H
