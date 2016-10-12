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

/**
 * @file
 * @defgroup Fix16 Fix16
 * @brief Fix16
 * @ingroup VBXware
 */
///@{

#ifndef __VBW_FIX16_H
#define __VBW_FIX16_H

/** Initial functions:
 * vbw_fix16_abs
 * vbw_fix16_min
 * vbw_fix16_add
 * vbw_fix16_sub
 * vbw_fix16_mul //32-bit and 8-bit
 * vbw_fix16_div //8-bit, require 32/32 divide instr for 32-bit implementaion
 * vbw_fix16_div_hw
 * vbw_fix16_sqrt
 * vbw_fix16_sqrt_hw
 *
 * flags
 * no 64-bit
 * no overflow
 */

#include "vbx.h"
#include "vbw_exit_codes.h"

#define USE_OVERFLOW 1
typedef int32_t fxp_t;

/*
static inline fix16_t fix16_abs(fix16_t x)
	{ return (x < 0 ? -x : x); }
*/
static inline void vbw_fix16_abs( vbx_word_t* v_result, vbx_word_t* v_x)
{
  vbx(SVW, VABSDIFF, v_result, 0, v_x );
}

/*
static inline fix16_t fix16_min(fix16_t x, fix16_t y)
	{ return (x < y ? x : y); }
*/
static inline void vbw_fix16_min( vbx_word_t* v_result, vbx_word_t* v_x, vbx_word_t* v_y, vbx_word_t* v_temp )
{
  vbx(VVW, VSUB, v_temp, v_y, v_x );
  vbx(VVW, VCMV_GTZ, v_result, v_y, v_temp);
  vbx(VVW, VCMV_LEZ, v_result, v_x, v_temp);
}

static inline void vbw_fix16_max( vbx_word_t* v_result, vbx_word_t* v_x, vbx_word_t* v_y, vbx_word_t* v_temp )
{
  vbx(VVW, VSUB, v_temp, v_y, v_x );
  vbx(VVW, VCMV_LTZ, v_result, v_y, v_temp);
  vbx(VVW, VCMV_GEZ, v_result, v_x, v_temp);
}


//#ifdef FIXMATH_NO_OVERFLOW
/*
static inline fix16_t fix16_add(fix16_t inArg0, fix16_t inArg1) { return (inArg0 + inArg1); }
*/
static inline void vbw_fix16_add( vbx_word_t* v_result, vbx_word_t* v_x, vbx_word_t* v_y)
{
  vbx(VVW, VADD, v_result, v_x, v_y );
}

static inline void vbw_fix16_add_s( vbx_word_t* v_result, fxp_t num, vbx_word_t* v_y)
{
  vbx(SVW, VADD, v_result, num, v_y );
}
/*
static inline fix16_t fix16_sub(fix16_t inArg0, fix16_t inArg1) { return (inArg0 - inArg1); }
*/
static inline void vbw_fix16_sub( vbx_word_t* v_result, vbx_word_t* v_x, vbx_word_t* v_y)
{
  vbx(VVW, VSUB, v_result, v_x, v_y );
}

static inline void vbw_fix16_sub_s( vbx_word_t* v_result, fxp_t num, vbx_word_t* v_y)
{
  vbx(SVW, VSUB, v_result, num, v_y );
}
//#endif


/* 64-bit implementation for fix16_mul. Fastest version for e.g. ARM Cortex M3.
 * Performs a 32*32 -> 64bit multiplication. The middle 32 bits are the result,
 * bottom 16 bits are used for rounding, and upper 16 bits are used for overflow
 * detection.
 */
/*
#if !defined(FIXMATH_NO_64BIT) && !defined(FIXMATH_OPTIMIZE_8BIT)
fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1)
{
	int64_t product = (int64_t)inArg0 * inArg1;

	#ifndef FIXMATH_NO_OVERFLOW
	// The upper 17 bits should all be the same (the sign).
	uint32_t upper = (product >> 47);
	#endif

	if (product < 0)
	{
		#ifndef FIXMATH_NO_OVERFLOW
		if (~upper)
				return fix16_overflow;
		#endif

		#ifndef FIXMATH_NO_ROUNDING
		// This adjustment is required in order to round -1/2 correctly
		product--;
		#endif
	}
	else
	{
		#ifndef FIXMATH_NO_OVERFLOW
		if (upper)
				return fix16_overflow;
		#endif
	}

	#ifdef FIXMATH_NO_ROUNDING
	return product >> 16;
	#else
	fix16_t result = product >> 16;
	result += (product & 0x8000) >> 15;

	return result;
	#endif
}
#endif
*/
static inline void vbw_fix16_mul( vbx_word_t* v_result, vbx_word_t* v_x, vbx_word_t* v_y)
{
  vbx(VVW, VMULFXP, v_result, v_x, v_y );
}

static inline void vbw_fix16_mul_s( vbx_word_t* v_result, fxp_t num, vbx_word_t* v_y)
{
  vbx(SVW, VMULFXP, v_result, num, v_y );
}

static inline void vbw_fix16_smul( vbx_word_t* v_result, vbx_word_t* v_x, vbx_word_t* v_y, vbx_uword_t * v_tmp)
{
  vbx(VVW, VMULFXP, v_result, v_x, v_y );
#if USE_OVERFLOW
  vbx(SVWU, VSHR, v_tmp, 31, (vbx_uword_t*)v_result);
  vbx(SVWU, VADD, v_tmp, 0x7FFFFFFF,  v_tmp);
  vbx(VVWU, VCMV_LTZ, (vbx_uword_t*)v_result, v_tmp, (vbx_uword_t*)v_result);
#endif
}

static inline void vbw_fix16_smul_s( vbx_word_t* v_result, fxp_t num, vbx_word_t* v_y, vbx_uword_t* v_tmp)
{
  vbx(SVW, VMULFXP, v_result, num, v_y );
#if USE_OVERFLOW
  vbx(SVWU, VSHR, v_tmp, 31, (vbx_uword_t*)v_result);
  vbx(SVWU, VADD, v_tmp, 0x7FFFFFFF,  v_tmp);
  vbx(VVWU, VCMV_LTZ, (vbx_uword_t*)v_result, v_tmp, (vbx_uword_t*)v_result);
#endif
}

//#if defined(FIXMATH_OPTIMIZE_8BIT)
static inline void vbw_fix16_div_hw( vbx_word_t* v_result, vbx_word_t* v_a, vbx_word_t* v_b, int length, int hw_div_offset )
{
  vbx_set_vl(length + hw_div_offset);
  vbx(VVW, VCUSTOM0, v_result, v_b, v_a );
  vbx_set_vl(length);
}

void vbw_fix16_div( vbx_word_t* v_result, vbx_word_t* v_a, vbx_word_t* v_b, int length );


static inline void vbw_fix16_sqrt_hw( vbx_word_t* v_out, vbx_word_t* v_x, int length, int hw_sqrt_offset)
{
  vbx_set_vl(length + hw_sqrt_offset);
  vbx(VVW, VCUSTOM1, v_out, v_x, 0 );
  vbx_set_vl(length);
}


void vbw_fix16_sqrt( vbx_word_t* v_out, vbx_word_t* v_x, int length);


static inline void vbw_fix16_clamp(vbx_word_t* v_out, vbx_word_t* v_x, fxp_t lo, fxp_t hi, vbx_word_t* v_tmp)
{
	//{ return fix16_min(fix16_max(x, lo), hi); }
  vbx(VVW, VMOV, v_out, v_x, 0); //cap overflow
  vbw_fix16_sub_s(v_tmp, lo, v_x);
  vbx(SVW, VCMV_GTZ, v_out, lo, v_tmp); //cap overflow
  vbw_fix16_sub_s(v_tmp, hi, v_x);
  vbx(SVW, VCMV_LTZ, v_out, hi, v_tmp); //cap @ 10
}

#endif // __VBW_FIX16_H
///@}
