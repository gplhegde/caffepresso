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

#ifndef VBW_MTX_XP_HPP
#define VBW_MTX_XP_HPP

/** VBX Matrix Transpose *in the scratchpad*.
 *  Transposes entirely within scratchpad (height,width decribe v_src layout)
 *  @pre  v_src must not overlap v_dst.
 *  @pre  v_src contains the data to transpose.
 *  @pre  v_dst points to preallocated scratchpad space
 *  @post v_dst contains the transposed data.
 *
 *  @param[out] v_dst Scratchpad pointer to output matrix
 *  @param[in] v_src Scratchpad pointer to input matrix
 *  @param[in] INROWS Number of rows in v_src.
 *  @param[in] INCOLS Number of cols in v_src.
 *  @returns negative on error condition. See vbw_exit_codes.h
 */
namespace vbw{
	template<typename T>
	int mtx_xp(VBX::Vector<T,2> &v_dst, VBX::Vector<T,2> &v_src)
	{
		//This could be done with .to3D() on the vectors, but it seems
		//clearer to do it explicitly
		int INCOLS=v_src.size;
		int INROWS=v_src.rows;

		vbx_set_vl( 1 );
		vbx_set_2D(  INCOLS, INROWS*sizeof(T),       sizeof(T), 0 );
		vbx_set_3D( INROWS,        sizeof(T), INCOLS*sizeof(T), 0 );
		vbxx_3D( VMOV, v_dst.data, v_src.data);

		return VBW_SUCCESS;
	}
	int inline ceil_div(int a,int b){
		return (a + (b>>1))/b;
	}

	template<typename T>
	int mtx_xp_ext(T* output, T* input , int incols,int inrows)
	{
		size_t free_sp=vbx_sp_getfree();
		using namespace VBX;
		if(free_sp/sizeof(T) >(size_t)(incols*inrows)*2){
			//do entire matrix in scratchpad
			Vector<T,2> in_row(incols,inrows,incols);
			Vector<T,2> out_row(inrows,incols,inrows);

			in_row.dma_read(input,incols);
			mtx_xp(out_row,in_row);
			out_row.dma_write(output,inrows);
		}
		//do one tile at a time
#if 0
		else{
			//NOT implemented yet
			int rows_at_time=VBX_GET_THIS_MXP()->dma_alignment_bytes / sizeof(T);
			int cols_at_time=free_sp/vectors_needed /rows_at_time /sizeof(T);

			Vector<T,2> *input_dbl_buffer[2];
			input_dbl_buffer[0]=Vector<T,2>(cols_at_a_time,rows_at_time,cols_at_time);
			input_dbl_buffer[1]=Vector<T,2>(cols_at_a_time,rows_at_time,cols_at_time);

			int tile_rows=ceil_div(inrows,rows_at_time);
			int tile_cols=ceil_div(incols,cols_at_time);
			for(int tile_row=0;tile_row<tile_rows;tile_row++){

				bool last_tile_in_col=tile_row+1 == til_rows;
				int rows_in_tile=last_tile_in_col?inrows-tile_row*rows_at_time:rows_at_time;

				for(int tile_col=0;tile_col<tile_cols;tile_col++){

					bool last_tile_in_row=tile_col+1 == til_cols;
					int cols_in_tile=last_tile_in_row?incols-tile_col*cols_at_time:cols_at_time;

					Vector<T,2> input(cols_in_tile,rows_in_tile,cols_in_tile);
					Vector<T,2> output(rows_in_tile,cols_in_tile,rows_in_tile);
					input_offset=tile_row*rows_at_time*inrows+tile_col*cols_at_time;
					output_offset=tile_row*rows_at_time*inrows+tile_col*cols_at_time;
					input.dma_read(input+input_offset,inrows);

					mtx_xp(output,input);
					output.dma_write(output,incols);
				}
			}
		}
#else
		else{
			Vector<T> in_row(incols);
			for(int row=0;row<inrows;row++){
				in_row.dma_read(input+row*incols);
				in_row.to2D(1,incols,1).dma_write(output+row,inrows);
			}
		}
#endif
		vbx_sync();
		return 0;

	}
}
#endif //VBW_MTX_XP_HPP
