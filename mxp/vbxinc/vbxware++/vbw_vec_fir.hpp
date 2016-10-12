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

/**@file*/
#ifndef VBWPP_VEC_FIR_HPP
#define VBWPP_VEC_FIR_HPP


#include "vbx.h"
#include "vbw_exit_codes.h"
#include "Vector.hpp"
#include "prefetch.hpp"
namespace vbw{


	template<typename vbx_sp_t>
	void vec_fir(VBX::Vector<vbx_sp_t>& v_out,const VBX::Vector<vbx_sp_t>& v_in,const VBX::Vector<vbx_sp_t>& v_coeffs)
	{
		int out_size= v_out.size;
		v_out=accumulate(v_coeffs.to2D(v_coeffs.size,out_size,0)*v_in.to2D(v_coeffs.size,out_size,1));
	}


	/**
	 * This function takes care of tiling both the transpose and the non-transpose versions
	 * of the fir filter.
	 */

	template<typename vbx_mm_t,bool transpose>
	inline int vec_fir_tiler(vbx_mm_t *output, vbx_mm_t *input, vbx_mm_t *coeffs,  int sample_size,  int num_taps)
	{
		typedef vbx_mm_t vbx_sp_t;

		//use 1/8 of scratchpad, only really need 1/4, but lets be safe
		int chunk_size = vbx_sp_getfree()>>3;

		//divide by sizeof vbx_sp_t
		chunk_size >>= (sizeof(vbx_sp_t)==sizeof(vbx_word_t)?2:
		                sizeof(vbx_sp_t)==sizeof(vbx_half_t)?1:0);
		// Note: chunksize is the size of the input chunk, so the output
		// chunk is chunk_size - num_taps.
		if( chunk_size==0 ){
			return VBW_ERROR_SP_ALLOC_FAILED;
		}
		VBX::Vector<vbx_sp_t> v_coeffs(num_taps);
		v_coeffs.dma_read(coeffs);
		VBX::Prefetcher<vbx_sp_t> input_dbl_buf(1,chunk_size+num_taps,input,input+sample_size,chunk_size);


		input_dbl_buf.fetch();
		//if the entire sample ifts in the scratchpad, do that.

		if(chunk_size>sample_size-num_taps){

			//do in sp fir filter
			VBX::Vector<vbx_sp_t>& v_in=input_dbl_buf[0];
			VBX::Vector<vbx_sp_t> v_out(sample_size-num_taps);
			vec_fir(v_out,v_in,v_coeffs);
			v_out.dma_write(output);
			vbx_sync();
			return VBW_SUCCESS;
		}
		VBX::Vector<vbx_sp_t> v_out(chunk_size);
		int num_chunks=(sample_size + chunk_size-num_taps-1)/(chunk_size-num_taps);
		for(int chunk=0;chunk<num_chunks;chunk++){
			input_dbl_buf.fetch();
			VBX::Vector<vbx_sp_t>& v_in=input_dbl_buf[0];
			vec_fir(v_out,v_in,v_coeffs);
			v_out[0 upto v_in.size-num_taps].dma_write(output+chunk*chunk_size);
		}
		vbx_sync();
		return VBW_SUCCESS;
	}

	template<typename vbx_mm_t>
	inline int vec_fir_ext(vbx_mm_t *output, vbx_mm_t *input, vbx_mm_t *coeffs,  int sample_size,  int num_taps)
	{
		vbx_sp_push();
		int ret = vec_fir_tiler<vbx_mm_t,false>(output,input,coeffs,sample_size,num_taps);
		vbx_sp_pop();
		return ret;
	}
}

#endif //VBW_VEC_FIR_HPP
