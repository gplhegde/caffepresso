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

#ifndef VBW_MTX_FIR_HPP
#define VBW_MTX_FIR_HPP
#include "Vector.hpp"

namespace vbw{
	template<typename vbx_mm_t>
	void mtx_2Dfir(vbx_mm_t *output, vbx_mm_t *input, int32_t *coeffs,
	               const int num_row, const int num_column, const int ntaps_row, const int ntaps_column )
	{
		using namespace VBX;
		Prefetcher<vbx_mm_t> sample(ntaps_row,num_column,input,input+num_row*num_column,num_column);
		int out_cols=num_column-ntaps_column;
		int out_rows=num_row - ntaps_row;
		typedef typename word_sized<vbx_mm_t>::type word_t;
		Vector<word_t> accum_int(num_column-ntaps_column);
		Vector<vbx_mm_t> accum(num_column-ntaps_column);
		for(int i=0;i<ntaps_row;i++){
			sample.fetch();
		}
		for(int row=0;row<out_rows;row++){
			accum_int=0;
			sample.fetch();
			for(int j=0;j<ntaps_row;j++){
				Vector<vbx_mm_t>& sample_row=sample[j];
				for(int i=0;i<ntaps_column;i++){
					int32_t tap=coeffs[ntaps_row*j +i];
					accum_int+=sample_row[i upto i+out_cols]* tap;
				}
			}
			accum= accum_int>>8;
			accum.dma_write(output+num_column*row);
		}
		vbx_sync();
	}
}
#endif //VBW_MTX_FIR_HPP
