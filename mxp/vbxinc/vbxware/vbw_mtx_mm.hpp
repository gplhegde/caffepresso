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

#ifndef VBW_MTX_MM_TEMPLATE_HPP
#define VBW_MTX_MM_TEMPLATE_HPP
#include "vbx.h"
#include "vbx_port.h"
#include "vbw_exit_codes.h"

#include "vbw_mtx_xp.h"
#include "vbw_mtx_mm.h"
#include "vbw_buffer.h"
#include "vbw_mtx_xp.h"
#include <assert.h>
#include "vbx_common.h"

#ifndef min
#define min(x,y) ((x)<(y) ? (x) : (y))
#endif

//simple scalar version for testing. should be moved to libscalar.a
template<typename vbx_mm_t>
int vbw_mtx_mul_scalar_ext( vbx_mm_t *out, vbx_mm_t *in1, const int IN1ROWS, const int IN1COLS, vbx_mm_t *in2pre, const int IN2ROWS, const int IN2COLS )
{
	int i,j,k;
	int out_rows=IN1ROWS;
	int out_cols=IN2COLS;
	for(i=0; i<out_rows; i++) {
		for(j=0; j<out_cols; j++){
			int index=j+out_rows*i;
			out[index] = 0;
			for(k=0; k<IN1COLS; k++) {
				out[index] += in1[k+i*IN1COLS] * in2pre[j+k*IN2COLS];
			}
		}
	}
	return VBW_SUCCESS;
}

/** VBX matrix multiply. Multiplies two matrices allocated *in the scratchpad*.
 *
 * @param[out] v_out Scratchpad pointer to the output matrix
 * @param[in] v_in1 Scratchpatd pointer to the left side matrix of the multiplication
 * @param[in] rows1 Number of rows in v_in1
 * @param[in] cols1 Number of columns in v_in1
 * @param[in] v_in2  Scratchpatd pointer to the right side matrix of the multiplication
 * @param[in] rows2 Number of rows in v_in2
 * @param[in] cols2 Number of columns in v_in2
 */
template<typename vbx_sp_t>
inline int vbw_mtx_mul( vbx_sp_t *v_out, vbx_sp_t *v_in1, const int rows1, const int cols1, vbx_sp_t *v_in2, const int rows2, const int cols2 )
{
	// TODO check for overlapping.

	const int OUTROWS = rows1;
	const int OUTCOLS = cols2;

	vbx_sp_push();

	int in2_bytes = cols2*rows2*sizeof(vbx_sp_t);
	vbx_sp_t *v_in2_trans = (vbx_sp_t *)vbx_sp_malloc(in2_bytes);
	if (v_in2_trans==NULL){
		return  VBW_ERROR_SP_ALLOC_FAILED;
	}
	vbw_mtx_xp(v_in2_trans, v_in2, rows2, cols2);
	vbx_set_vl(cols1);
	vbx_set_2D(cols2, sizeof(vbx_sp_t), 0, cols2*sizeof(vbx_sp_t));
	vbx_set_3D(OUTROWS, OUTCOLS*sizeof(vbx_sp_t), cols1*sizeof(vbx_sp_t), 0);
	vbxx_acc_3D(VMUL,v_out,v_in1,v_in2_trans);
	vbx_sp_pop();
	return VBW_SUCCESS;
}



/** VBX matrix multiply (Main Memory)
 *
 * This function does a transpose of in2 and then does a 3d accumulate
 * for the multiplication. broken for large matrix
 * it is hard to tile properly
 * @param[out] out Pointer to the output matrix
 * @param[in] in1 Pointer to the left side matrix of the multiplication
 * @param[in] rows1 Number of rows in v_in1
 * @param[in] cols1 Number of columns in v_in1
 * @param[in] in2  Pointer to the right side matrix of the multiplication
 * @param[in] rows2 Number of rows in v_in2
 * @param[in] cols2 Number of columns in v_in2
 */
template<typename vbx_mm_t>
int vbw_mtx_mm_trans_ext( vbx_mm_t *out, vbx_mm_t *in1,  int rows1, int cols1, vbx_mm_t *in2, int rows2, int cols2 )
{
	typedef vbx_mm_t vbx_sp_t;
	const int outrows = rows1;
	const int outcols = cols2;

	//check to see if we can fit everything in scratchpad
	int sp_free = vbx_sp_getfree();
	int double_to_to_make_space_for_transpose=2;
	if(sp_free >(rows1*cols1 +
	             double_to_to_make_space_for_transpose*rows2*cols2 +
	             outcols*outrows )){
		//do it all in scratchpad
		vbx_sp_push();
		vbx_sp_t* v_in1=(vbx_sp_t*)vbx_sp_malloc(sizeof(vbx_sp_t)*cols1*rows1);
		vbx_sp_t* v_in2=(vbx_sp_t*)vbx_sp_malloc(sizeof(vbx_sp_t)*cols2*rows2);
		vbx_sp_t* v_out=(vbx_sp_t*)vbx_sp_malloc(sizeof(vbx_sp_t)*outcols*outrows);
		assert(v_out!=NULL);
		vbx_dma_to_vector(v_in1,in1,cols1*rows1*sizeof(vbx_mm_t));
		vbx_dma_to_vector(v_in2,in2,cols2*rows2*sizeof(vbx_mm_t));
		vbw_mtx_mul( v_out, v_in1,  rows1, cols1,v_in2, rows2, cols2 );
		vbx_dma_to_host(out,v_out,outcols*outrows*sizeof(vbx_mm_t));
		vbx_sp_pop();
		vbx_sync();
		return VBW_SUCCESS;
	}

	//can't do it all in scratchapd
	vbx_mm_t* in2_trans=(vbx_mm_t*)vbx_shared_malloc(sizeof(vbx_mm_t*)*cols2*rows2);
	if( in2_trans==NULL ){
		printf("Malloc failed");
		exit(1);
	}
	vbw_mtx_xp_ext(in2_trans,in2,rows2,cols2);
	//flip rows and cols
	{
		int tmp=rows2;
		rows2=cols2;
		cols2=tmp;
	}
	vbx_sp_push();
	vbx_sp_t* v_row1=(vbx_sp_t*)vbx_sp_malloc(sizeof(vbx_sp_t)*cols1);
	vbx_sp_t* v_out =(vbx_sp_t*)vbx_sp_malloc(sizeof(vbx_sp_t)*cols1);
	//how many rows of in2_trans can we fit?
	sp_free = vbx_sp_getfree();
	int rows_of_2=sp_free/(sizeof(vbx_sp_t)*cols2*2); //multiply by two is for double buffering
	rows_of_2=min(rows_of_2,rows2);
	if(rows_of_2 == 0){//cant fit any rows
		vbx_sp_pop();
		return VBW_ERROR_SP_ALLOC_FAILED;
	}
	rotating_prefetcher_t rowsb=rotating_prefetcher(1,rows_of_2*rows2*sizeof(vbx_sp_t),
	                                                in2_trans,in2_trans+cols2*rows2,
	                                                rows_of_2*rows2*sizeof(vbx_sp_t));

	vbx_set_vl(cols1);
	for(int row=0;row<rows1;row++){
		//read in next row of in1
		vbx_dma_to_vector(v_row1,in1+cols1*row,sizeof(vbx_sp_t)*cols1);

		//read in first bunch of rows from in2
		rp_fetch(&rowsb);
		vbx_set_2D(rows_of_2,sizeof(vbx_sp_t),0,cols2*sizeof(vbx_sp_t));

		for(int out_index=0;out_index<rows2;out_index+=rows_of_2){
			rp_fetch(&rowsb);
			vbx_sp_t* v_row2=(vbx_sp_t*)rp_get_buffer(&rowsb,0);
			vbxx_acc_2D(VMUL,v_out+out_index,v_row1,v_row2);
		}
		vbx_dma_to_host(out+row*outcols,v_out,outcols*sizeof(vbx_mm_t));
	}

	vbx_sp_pop();
	vbx_sync();
	vbx_shared_free(in2_trans);
	return VBW_SUCCESS;
}


#include "Vector.hpp"
#include "prefetch.hpp"
template<typename vbx_mm_t>
int vbw_mtx_mm_helper( vbx_mm_t *out, vbx_mm_t *in1, int rows1, int cols1,
                       vbx_mm_t *in2, int rows2, int cols2 ,int pitch)
{

	const int row_blk=8; //magic number I (JDV) pulled out of thin air
	VBX::Vector<vbx_mm_t> *vout[row_blk];
	for(int i=0;i<row_blk;i++){
		vout[i] = new VBX::Vector<vbx_mm_t>(cols2);
	}

	if(vout[row_blk-1]->data==NULL)
		return VBW_ERROR_SP_ALLOC_FAILED;
	for(int i=0;i<rows1;i+=row_blk){
		for(int k=0;k<row_blk && i+k < rows1 ;k++){
			*vout[k]=0;
		}

		VBX::Prefetcher<vbx_mm_t> vin2_db(1,cols2,
		                                  in2,in2+rows2*pitch,pitch);
		vin2_db.fetch();

		for(int j=0;j<rows2;j++){

			vin2_db.fetch();

			//refence to avoid VMOV
			VBX::Vector<vbx_mm_t>& vin2=vin2_db[0];
			for(int k=0;k<row_blk && i+k < rows1;k++){
				*vout[k]+=vin2*in1[j+(i+k)*rows2];
			}
		}
		for(int k=0;k<row_blk && i+k < rows1;k++){
			vout[k]->printVec();
			vout[k]->dma_write(out+(i+k)*pitch);
		}
	}
	for(int i=0;i<row_blk;i++){
		delete vout[i];
	}

	return VBW_SUCCESS;
}

/** VBX matrix multiply (Main Memory)
 *
 * This function does a matric multiply for any size of matrix, does not use transform.
 *
 * @param[out] out Pointer to the output matrix
 * @param[in] in1 Pointer to the left side matrix of the multiplication
 * @param[in] rows1 Number of rows in v_in1
 * @param[in] cols1 Number of columns in v_in1
 * @param[in] in2  Pointer to the right side matrix of the multiplication
 * @param[in] rows2 Number of rows in v_in2
 * @param[in] cols2 Number of columns in v_in2
 */
template<typename vbx_mm_t>
int vbw_mtx_mm_ext( vbx_mm_t *out, vbx_mm_t *in1, int rows1, int cols1,
                    vbx_mm_t *in2, int rows2, int cols2)
{
	int ret=VBW_SUCCESS;
	int sp_avail=vbx_sp_getfree();
	int sp_needed=cols2*sizeof(vbx_mm_t)*16/*guess at number of vectors needed*/;
	if (sp_avail<sp_needed){
		int slices= sp_needed/sp_avail +1;
		int cols_per_slice=cols2/slices;
		for(int slice=0;slice<slices;slice++){
			if(slice== (slices-1)){//last slice
				int last_slice=cols2-(slices-1)*cols_per_slice;
				ret=vbw_mtx_mm_helper(out+cols_per_slice*slice,in1,rows1,cols1,in2+cols_per_slice*slice,rows2,last_slice,cols2);
			}else{
				ret=vbw_mtx_mm_helper(out+cols_per_slice*slice,in1,rows1,cols1,in2+cols_per_slice*slice,rows2,cols_per_slice,cols2);
			}
			if (ret!=VBW_SUCCESS)return ret;
		}
	}else{
		ret=vbw_mtx_mm_helper(out,in1,rows1,cols1,in2,rows2,cols2,cols2);
		if (ret!=VBW_SUCCESS)return ret;
	}
	vbx_sync();
	return VBW_SUCCESS;
}


#endif //VBW_MTX_MM_TEMPLATE_HPP
