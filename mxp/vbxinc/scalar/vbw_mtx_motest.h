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
 * @file vbw_mtx_motest.h
 * @defgroup Matrix_Motion_Estimation Matrix Motion Estimation
 * @brief Matrix Motion Estimation
 * @ingroup VBXware
 */
/**@{*/

#ifndef __VBX_MTX_MOTEST_H
#define __VBX_MTX_MOTEST_H

//
// Motion Estimation
// Scalar NIOS version and vbx VECTOR version
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "vbx.h"
#ifdef __cplusplus
extern "C"{
#endif
typedef unsigned char  input_type;
typedef unsigned short intermediate_type;
typedef unsigned int   output_type;

typedef struct {
int                image_width;
int                image_height;
int                block_width;
int                block_height;
int                search_width;
int                search_height;
int                result_size;
// 2D and 3D
input_type        *v_block;
input_type        *v_img;
intermediate_type *v_row_sad;
output_type       *v_result;
// 2D
input_type        *v_img_sub;
input_type        *v_absdiff;
} vbw_motest_t;


void vbw_mtx_motest_scalar_byte  (output_type *result, input_type *x, input_type *y, vbw_motest_t *m);

int  vbw_mtx_motest_byte         (output_type *result, input_type *x, input_type *y, vbw_motest_t *m);

int  vbw_mtx_motest_3d_byte      (output_type *result, input_type *x, input_type *y, vbw_motest_t *m);

int  vbw_mtx_motest_byte_setup   ( vbw_motest_t *m );

int  vbw_mtx_motest_3d_byte_setup( vbw_motest_t *m );
#ifdef __cplusplus
}
#endif
#endif // __VBX_MTX_MOTEST_H
/**@}*/
