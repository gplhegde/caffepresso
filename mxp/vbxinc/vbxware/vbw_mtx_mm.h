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

#ifndef VBW_MTX_MM_H
#define VBW_MTX_MM_H
#ifdef __cplusplus
#include "vbw_mtx_mm.hpp"
extern "C" {
#endif
/* this family of functions does a transpose of src2, this is faster for smaller matrices*/
int vbw_mtx_mm_trans_ext_word( vbx_word_t *out, vbx_word_t *in1,const int IN1ROWS, const int IN1COLS,
                          vbx_word_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mm_trans_ext_uword( vbx_uword_t *out, vbx_uword_t *in1,const int IN1ROWS, const int IN1COLS,
                           vbx_uword_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mm_trans_ext_half( vbx_half_t *out, vbx_half_t *in1,const int IN1ROWS, const int IN1COLS,
                          vbx_half_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mm_trans_ext_uhalf( vbx_uhalf_t *out, vbx_uhalf_t *in1,const int IN1ROWS, const int IN1COLS,
                           vbx_uhalf_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mm_trans_ext_byte( vbx_byte_t *out, vbx_byte_t *in1,const int IN1ROWS, const int IN1COLS,
                          vbx_byte_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mm_trans_ext_ubyte( vbx_ubyte_t *out, vbx_ubyte_t *in1,const int IN1ROWS, const int IN1COLS,
                           vbx_ubyte_t *in2pre, const int IN2ROWS, const int IN2COLS );

int vbw_mtx_mm_ext_word( vbx_word_t *out, vbx_word_t *in1, const int rows1, const int cols1,
                         vbx_word_t *in2, const int rows2, const int cols2 );
int vbw_mtx_mm_ext_uword( vbx_uword_t *out, vbx_uword_t *in1, const int rows1, const int cols1,
                          vbx_uword_t *in2, const int rows2, const int cols2 );
int vbw_mtx_mm_ext_half( vbx_half_t *out, vbx_half_t *in1, const int rows1, const int cols1,
                         vbx_half_t *in2, const int rows2, const int cols2 );
int vbw_mtx_mm_ext_uhalf( vbx_uhalf_t *out, vbx_uhalf_t *in1, const int rows1, const int cols1,
                          vbx_uhalf_t *in2, const int rows2, const int cols2 );
int vbw_mtx_mm_ext_byte( vbx_byte_t *out, vbx_byte_t *in1, const int rows1, const int cols1,
                         vbx_byte_t *in2, const int rows2, const int cols2 );
int vbw_mtx_mm_ext_ubyte( vbx_ubyte_t *out, vbx_ubyte_t *in1, const int rows1, const int cols1,
                          vbx_ubyte_t *in2, const int rows2, const int cols2 );


int vbw_mtx_mul_scalar_ext_word( vbx_word_t *out, vbx_word_t *in1, const int IN1ROWS, const int IN1COLS,
                                 vbx_word_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mul_scalar_ext_uword( vbx_uword_t *out, vbx_uword_t *in1, const int IN1ROWS, const int IN1COLS,
                                  vbx_uword_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mul_scalar_ext_half( vbx_half_t *out, vbx_half_t *in1, const int IN1ROWS, const int IN1COLS,
                                 vbx_half_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mul_scalar_ext_uhalf( vbx_uhalf_t *out, vbx_uhalf_t *in1, const int IN1ROWS, const int IN1COLS,
                                  vbx_uhalf_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mul_scalar_ext_byte( vbx_byte_t *out, vbx_byte_t *in1, const int IN1ROWS, const int IN1COLS,
                                 vbx_byte_t *in2pre, const int IN2ROWS, const int IN2COLS );
int vbw_mtx_mul_scalar_ext_ubyte( vbx_ubyte_t *out, vbx_ubyte_t *in1, const int IN1ROWS, const int IN1COLS,
                                  vbx_ubyte_t *in2pre, const int IN2ROWS, const int IN2COLS );


#ifdef __cplusplus
}
#endif

#endif //VBW_MTX_MM_H
