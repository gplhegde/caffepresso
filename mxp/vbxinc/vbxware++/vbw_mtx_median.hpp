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

#ifndef VBW_MTX_MEDIAN_HPP
#define VBW_MTX_MEDIAN_HPP
#include "Vector.hpp"

namespace vbw{
	template<typename T>
	int mtx_median_ext( T *output, T *input, const int filter_height, const int filter_width,
	                    const int image_height, const int image_width, const int image_pitch )
	{
		using namespace VBX;
		Vector<T,2> in_rows(image_width,filter_height,image_width);

		int num_vecs=filter_height*filter_width;
		int working_vec_size=image_width-filter_width;

		Vector<T> **working_rows = (Vector<T>**)malloc(sizeof(Vector<T>*)*num_vecs);
		for(int i=0;i<num_vecs;i++){
			working_rows[i]=new Vector<T>(working_vec_size);
		}

		Vector<T> tmpa(working_vec_size);
		Vector<T> tmpb(working_vec_size);
		Vector<T> comparison(working_vec_size);


		for(int row=0;row<image_height-filter_height;row++){
			in_rows.dma_read(input+row*image_pitch,image_pitch);
			for(int i=0;i<filter_height;i++){
				for(int j=0;j<filter_width;j++){
					*working_rows[i*filter_height+j]=in_rows.get_row(i)[j upto working_vec_size+j];
				}
			}

			//bubble sort
			for(int i=0;i<num_vecs;i++){
				for(int j=0;j<num_vecs-1-i;j++){
					tmpa=*working_rows[j];
					tmpb=*working_rows[j+1];
					comparison=tmpa>tmpb;
					working_rows[j]->cond_move(comparison,tmpb);
					working_rows[j+1]->cond_move(comparison,tmpa);
				}
			}
			working_rows[num_vecs/2]->dma_write(output+row*image_pitch);
		}
		//clean up vectors

		for(int i=0;i<num_vecs;i++){
			delete working_rows[i];
		}
		free(working_rows);
		vbx_sync();
		return 0;
	}

}
#endif //VBW_MTX_MEDIAN_HPP
