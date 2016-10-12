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

#ifndef __SCALAR_VEC_DIVIDE_H
#define __SCALAR_VEC_DIVIDE_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define RANGE  UINT_MAX
#ifdef __cplusplus
extern "C"{
#endif
void scalar_vec_divide_byte(int8_t *output, int8_t *remainder, int8_t *input1, int8_t *input2, const int N);
void scalar_vec_divide_half(int16_t *output, int16_t *remainder, int16_t *input1, int16_t *input2, const int N);
void scalar_vec_divide_word(int32_t *output, int32_t *remainder, int32_t *input1, int32_t *input2, const int N);
void scalar_vec_divide_ubyte(uint8_t *output, uint8_t *remainder, uint8_t *input1, uint8_t *input2, const int N);
void scalar_vec_divide_uhalf(uint16_t *output, uint16_t *remainder,  uint16_t *input1, uint16_t *input2, const int N);
void scalar_vec_divide_uword(uint32_t *output, uint32_t *remainder, uint32_t *input1, uint32_t *input2, const int N);

#ifdef __cplusplus
}
#endif

#endif //__SCALAR_VEC_DIVIDE_H
