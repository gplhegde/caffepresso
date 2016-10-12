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

#ifndef VBW_VEC_REV_H
#define VBW_VEC_REV_H

#ifdef __cplusplus
template<typename vbx_mm_t>
int vbw_vec_reverse_ext( vbx_mm_t *dst, vbx_mm_t *src, const unsigned int N );
template<typename vbx_sp_t>
int vbw_vec_reverse( vbx_sp_t *v_dst, vbx_sp_t *v_src, const unsigned int N );
extern "C" {
#endif

int vbw_vec_reverse_word( vbx_word_t *v_dst, vbx_word_t *v_src, const unsigned int N );
int vbw_vec_reverse_uword( vbx_uword_t *v_dst, vbx_uword_t *v_src, const unsigned int N );
int vbw_vec_reverse_half( vbx_half_t *v_dst, vbx_half_t *v_src, const unsigned int N );
int vbw_vec_reverse_uhalf( vbx_uhalf_t *v_dst, vbx_uhalf_t *v_src, const unsigned int N );
int vbw_vec_reverse_byte( vbx_byte_t *v_dst, vbx_byte_t *v_src, const unsigned int N );
int vbw_vec_reverse_ubyte( vbx_ubyte_t *v_dst, vbx_ubyte_t *v_src, const unsigned int N );

int vbw_vec_reverse_ext_word( vbx_word_t *dst, vbx_word_t *src, const unsigned int N );
int vbw_vec_reverse_ext_uword( vbx_uword_t *dst, vbx_uword_t *src, const unsigned int N );
int vbw_vec_reverse_ext_half( vbx_half_t *dst, vbx_half_t *src, const unsigned int N );
int vbw_vec_reverse_ext_uhalf( vbx_uhalf_t *dst, vbx_uhalf_t *src, const unsigned int N );
int vbw_vec_reverse_ext_byte( vbx_byte_t *dst, vbx_byte_t *src, const unsigned int N );
int vbw_vec_reverse_ext_ubyte( vbx_ubyte_t *dst, vbx_ubyte_t *src, const unsigned int N );
#ifdef __cplusplus
}
#endif
#endif //VBW_VEC_REV_H
