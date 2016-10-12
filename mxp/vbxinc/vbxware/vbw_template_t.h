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
 * @defgroup Template Template
 * @brief Template system for C
 * @ingroup VBXware 
 */
/**
 * @file
 * @defgroup Template_header Template header
 * @brief Template header
 * @ingroup Template 
 */
/**@{*/

// DO NOT ADD #ifdef WRAPPER TO THIS HEADER FILE
// This file should NOT be protected from multiple #includes.

#ifndef VBX_TEMPLATE_T
#error The C file must define VBX_TEMPLATE_T as one of \
 VBX_BYTESIZE_DEF, VBX_HALFSIZE_DEF, VBX_WORDSIZE_DEF, \
 VBX_UBYTESIZE_DEF, VBX_UHALFSIZE_DEF, or VBX_UWORDSIZE_DEF \
 before including this header.
#endif

#ifndef __VBX_TEMPLATE1_H
#define __VBX_TEMPLATE1_H
	#define VBX_BYTESIZE_DEF   1
	#define VBX_HALFSIZE_DEF   2
	#define VBX_WORDSIZE_DEF   4
	#define VBX_UBYTESIZE_DEF 11
	#define VBX_UHALFSIZE_DEF 12
	#define VBX_UWORDSIZE_DEF 14
#endif // __VBX_TEMPLATE1_H

#ifdef BYTEHALFWORD
#undef BYTEHALFWORD
#endif

#ifdef T
#undef T
#endif

#ifdef B
#undef B
#endif

#ifdef H
#undef H
#endif

#ifdef W
#undef W
#endif

#ifdef TB
#undef TB
#endif

#ifdef TH
#undef TH
#endif

#ifdef TW
#undef TW
#endif

#ifdef BT
#undef BT
#endif

#ifdef HT
#undef HT
#endif

#ifdef WT
#undef WT
#endif

#ifdef vbx_sp_t
#undef vbx_sp_t
#endif

#ifdef vbx_mm_t
#undef vbx_mm_t
#endif

#if ((VBX_TEMPLATE_T == VBX_BYTESIZE_DEF) | (VBX_TEMPLATE_T == VBX_HALFSIZE_DEF ) | (VBX_TEMPLATE_T == VBX_WORDSIZE_DEF))
	#define B B
	#define H H
	#define W W
#else
	#define B BU
	#define H HU
	#define W WU
#endif


#if VBX_TEMPLATE_T == VBX_BYTESIZE_DEF
	#define BYTEHALFWORD byte
	#define T B
	#define TB B
	#define TH BH
	#define TW BW
	#define BT B
	#define HT HB
	#define WT WB
	#define vbx_sp_t vbx_byte_t 
	#define vbx_mm_t int8_t

#elif VBX_TEMPLATE_T == VBX_HALFSIZE_DEF
	#define BYTEHALFWORD half
	#define T H
	#define TB HB
	#define TH H
	#define TW HW
	#define BT BH
	#define HT H
	#define WT WH
	#define vbx_sp_t vbx_half_t 
	#define vbx_mm_t int16_t

#elif VBX_TEMPLATE_T == VBX_WORDSIZE_DEF
	#define BYTEHALFWORD word
	#define T W
	#define TB WB
	#define TH WH
	#define TW W
	#define BT BW
	#define HT HW
	#define WT W
	#define vbx_sp_t vbx_word_t 
	#define vbx_mm_t int32_t

#elif VBX_TEMPLATE_T == VBX_UBYTESIZE_DEF
	#define BYTEHALFWORD ubyte
	#define T BU
	#define TB BU
	#define TH BHU
	#define TW BWU
	#define BT BU
	#define HT HBU
	#define WT WBU
	#define vbx_sp_t vbx_ubyte_t 
	#define vbx_mm_t uint8_t

#elif VBX_TEMPLATE_T == VBX_UHALFSIZE_DEF
	#define BYTEHALFWORD uhalf
	#define T HU
	#define TB HBU
	#define TH HU
	#define TW HWU
	#define BT BHU
	#define HT HU
	#define WT WHU
	#define vbx_sp_t vbx_uhalf_t 
	#define vbx_mm_t uint16_t

#elif VBX_TEMPLATE_T == VBX_UWORDSIZE_DEF
	#define BYTEHALFWORD uword
	#define T WU
	#define TB WBU
	#define TH WHU
	#define TW WU
	#define BT BWU
	#define HT HWU
	#define WT WU
	#define vbx_sp_t vbx_uword_t 
	#define vbx_mm_t uint32_t
#endif

// --------------------------
// START: This part should be protected from multiple #includes.
#ifndef __VBX_TEMPLATE2_H
#define __VBX_TEMPLATE2_H

// --
#define _VBXPROTO2(fname,type)  fname ## _ ## type 
#define _VBXPROTO1(fname,type)  _VBXPROTO2(fname,type)
#define VBX_T(fname)            _VBXPROTO1(fname,BYTEHALFWORD)

// --
#define _VBXMODE2(mode,type)  mode ## type 
#define _VBXMODE1(mode,type) _VBXMODE2(mode,type)
#define VV(type)	_VBXMODE1(VV,type)
#define SV(type)	_VBXMODE1(SV,type)
#define VE(type)	_VBXMODE1(VE,type)
#define SE(type)	_VBXMODE1(SE,type)

// --
#endif // __VBX_TEMPLATE2_H
// END: This part should be protected from multiple #includes.
// --------------------------


// DO NOT ADD #ifdef WRAPPER TO THIS HEADER FILE
// This file should NOT be protected from multiple #includes.
/**@}*/
