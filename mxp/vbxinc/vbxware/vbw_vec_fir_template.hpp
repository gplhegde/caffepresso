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
#ifndef VBW_VEC_FIR_HPP
#define VBW_VEC_FIR_HPP

#include "vbx.h"
#include "vbw_exit_codes.h"
#include "vbw_buffer.h"

template<typename vbx_sp_t>
static inline void vec_fir_transpose_helper(vbx_sp_t* v_out,vbx_sp_t* v_in,vbx_sp_t* coeffs,int sample_size,int num_taps,vbx_sp_t* v_mul)
{
	//input:   a  b  c  d  e  f  g  h  i  j  k   l  m  n  o  p  q  r  s  t
	//coeff:   1  2  3  4
	//coeff:      1  2  3  4
	//coeff:         1  2  3  4
	//coeff:            1  2  3  4
	//coeff:               1  2  3  4
	//
	//      a1b2c3d4 b1c2d3e4 ...
	vbx_set_vl(sample_size-num_taps);
	vbxx( VMUL, v_out, coeffs[0], v_in);
	for(int j=1;j<num_taps;j++){
		vbxx( VMUL, v_mul, coeffs[j], v_in+j);
		vbxx( VADD, v_out,v_out,v_mul);
	}
}
template<typename vbx_sp_t>
static inline void vec_fir_helper(vbx_sp_t* v_out,vbx_sp_t* v_in,vbx_sp_t* v_coeffs,int sample_size,int num_taps)
{
	vbx_set_vl(num_taps);
	vbx_set_2D(sample_size, sizeof(vbx_sp_t), 0, sizeof(vbx_sp_t));
	vbxx_acc_2D( VMUL, v_out, v_coeffs, v_in);
}

template<typename vbx_sp_t>
int vbw_vec_fir_transpose(vbx_sp_t* output,vbx_sp_t* input,vbx_sp_t* coeffs,int sample_size,int num_taps)
{
	vbx_sp_t* v_mul=(vbx_sp_t*)vbx_sp_malloc(sizeof(vbx_sp_t)*sample_size);
	if(v_mul==NULL){
		return VBW_ERROR_SP_ALLOC_FAILED;
	}
	vec_fir_transpose_helper(output,input,coeffs,sample_size,num_taps,v_mul);
	vbx_sync();
	return VBW_SUCCESS;
}
template<typename vbx_sp_t>
int vbw_vec_fir(vbx_sp_t* output,vbx_sp_t* input,vbx_sp_t* coeffs,int sample_size,int num_taps)
{
	vec_fir_helper(output,input,coeffs,sample_size,num_taps);
	vbx_sync();
	return VBW_SUCCESS;
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

	vbx_sp_t* v_coeffs=NULL;
	if(!transpose){
		//allocate coeffs on scratchpad right away.
		v_coeffs=(vbx_sp_t*)vbx_sp_malloc(sizeof(vbx_mm_t)*num_taps);
		vbx_dma_to_vector(v_coeffs,coeffs,sizeof(vbx_mm_t)*num_taps);
	}


	rotating_prefetcher_t input_dbl_buf=rotating_prefetcher(1,(chunk_size+num_taps)*sizeof(vbx_sp_t),
	                                                        input,
	                                                        input+sample_size,chunk_size*sizeof(vbx_sp_t));
	vbx_sp_t *v_mul=NULL;
	if(transpose){
		v_mul = (vbx_sp_t *)vbx_sp_malloc(chunk_size*sizeof(vbx_sp_t));
	}
	vbx_sp_t *v_out = (vbx_sp_t *)vbx_sp_malloc(chunk_size*sizeof(vbx_sp_t));
	vbx_sp_t *v_in;
	if( v_out==NULL ){
		vbx_sp_pop();
		return VBW_ERROR_SP_ALLOC_FAILED;
	}
	rp_fetch(&input_dbl_buf);
	//if the entire sample ifts in the scratchpad, do that.

	if(chunk_size>sample_size){
		//do in sp fir filter
		v_in=(vbx_sp_t*)rp_get_buffer(&input_dbl_buf,0);
		if(transpose){
			vec_fir_transpose_helper(v_out,v_in,coeffs,sample_size,num_taps,v_mul);
		}else{
			vec_fir_helper(v_out,v_in,v_coeffs,sample_size,num_taps);
		}
		vbx_dma_to_host(output,v_out,(sample_size-num_taps)*sizeof(vbx_sp_t));
		vbx_sync();
		return VBW_SUCCESS;
	}

	//Loop explanation:
	// one chunk has already been transfered in, so the in_start
	// pointer points to the start of the next chunk, and is
	// incremented by (chunksize-numtaps) every iteration.
	// The output chunk follows the in_start pointer delayed
	// by one iteration.
	int num_chunks=(sample_size + chunk_size-num_taps-1)/(chunk_size-num_taps);
	for(int chunk=0;chunk<num_chunks;chunk++){
		int current_chunksize=input_dbl_buf.chunk_size/sizeof(vbx_sp_t);
		rp_fetch(&input_dbl_buf);
		v_in=(vbx_sp_t*)rp_get_buffer(&input_dbl_buf,0);

		if(transpose){
			vec_fir_transpose_helper(v_out,v_in,coeffs,current_chunksize,num_taps,v_mul);
		}else{
			vec_fir_helper(v_out,v_in,v_coeffs,current_chunksize,num_taps);

		}
		vbx_dma_to_host(output+chunk*chunk_size,v_out,current_chunksize-num_taps);
	}

	vbx_sync();
	return VBW_SUCCESS;
}

/** VBX 1D Transposed Fir Filter.
 * Uses the vbx length to scan over the
 * entire sample for each coefficient, as this allows for longer vbx
 * lengths than multiplying and using the built in accumulators
 *
 *  @param[out] output Pointer to start of output array
 *  @param[in] input Pointer to start of input array
 *  @param[in] coeffs Pointer to start of array of coefficients
 *  @param[in] sample_size Length of input array
 *  @param[in] num_taps Length of coefficient array
 *  @retval negative on error condition. See vbw_exit_codes.h
 */
template<typename vbx_mm_t>
inline int vbw_vec_fir_transpose_ext(vbx_mm_t *output, vbx_mm_t *input, vbx_mm_t *coeffs,  int sample_size,  int num_taps)
{
	vbx_sp_push();
	int ret = vec_fir_tiler<vbx_mm_t,true>(output,input,coeffs,sample_size,num_taps);
	vbx_sp_pop();
	return ret;
}

template<typename vbx_mm_t>
inline int vbw_vec_fir_ext(vbx_mm_t *output, vbx_mm_t *input, vbx_mm_t *coeffs,  int sample_size,  int num_taps)
{
	vbx_sp_push();
	int ret = vec_fir_tiler<vbx_mm_t,false>(output,input,coeffs,sample_size,num_taps);
	vbx_sp_pop();
	return ret;
}

#endif //VBW_VEC_FIR_HPP
