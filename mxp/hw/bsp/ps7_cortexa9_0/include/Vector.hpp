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

#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__
#include "vbx.h"

#include "fwd_declaration.hpp"
#define INLINE inline __attribute__((always_inline))
//#define INLINE __attribute__((noinline))
extern "C" size_t __old_vl__;
namespace VBX{
#include "vinstr.hpp"
	namespace _internal{
		INLINE void set_vl(size_t len)
		{
			//WARNING, not threadsafe,
			//also doesn't play well with vbx_set_vl()
			size_t& vl=__old_vl__;
			if(len!=vl){
				vbx_set_vl(len);
				vl=len;
			}
		}
		template<typename T,typename U,vinstr_t instr,typename btype,int dim,acc_choice acc>
		struct bin_op{
			const T& lhs;
			const U& rhs;
			bin_op(const T& lhs,const U& rhs)
				:lhs(lhs),rhs(rhs){}
			template<typename new_btype>
			bin_op<T,U,instr,new_btype,dim,acc> cast() const
			{
				return bin_op<T,U,instr,new_btype,dim,acc>(lhs,rhs);
			}
			template<typename new_btype>
			bin_op<T,U,instr,new_btype,dim,acc> cast_to_typeof(const Vector<new_btype>& v) const
			{
				return cast<new_btype>();
			}
		};
		template<typename T,vinstr_t instr,typename btype,int dim,acc_choice acc>
		struct bin_op<vbx_word_t,T,instr,btype,dim,acc>{
			const vbx_word_t lhs;
			const T& rhs;
			bin_op(const vbx_word_t lhs,const T& rhs)
				:lhs(lhs),rhs(rhs){}
			template<typename new_btype>
			bin_op<vbx_word_t,T,instr,new_btype,dim,acc> cast() const
			{
				return bin_op<vbx_word_t,T,instr,new_btype,dim,acc>(lhs,rhs);
			}
			template<typename new_btype>
			bin_op<vbx_word_t,T,instr,new_btype,dim,acc> cast_to_typeof(const Vector<new_btype>& v) const
			{
				return cast<new_btype>();
			}


		};
		template<typename T>
		struct accum_op{
			const T& op;
			accum_op(const T& op):op(op){}
		};
#include "convert_vinstr.hpp"

	};//_internal
	class enum_t{};
	static enum_t ENUM __attribute__((unused));
}//namespace VBX
#include "type_manipulation.hpp"
#include "vbx_func.hpp"
#include "vector_mask_obj.hpp"
#include "Logical_op.hpp"
#include "resolve.hpp"
#include "assign.hpp"
#include "range.hpp"


namespace VBX{


	template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim>
	INLINE _internal::bin_op<lhs_t,rhs_t,instr,btype,dim-1,IS_ACC>
		accumulate(const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim,NO_ACC>& src)
	{
		return _internal::bin_op<lhs_t,rhs_t,instr,btype,dim-1,IS_ACC>(src.lhs,src.rhs);
	}
	template<typename T,int dim>
	INLINE _internal::bin_op<vbx_word_t,Vector<T,dim>,VOR,T,dim-1,IS_ACC>
		accumulate(const Vector<T,dim>& v){
		return _internal::bin_op<vbx_word_t,Vector<T,dim>,VOR,T,dim-1,IS_ACC>(0,v);
	}


#define IS_MASKED_BLOCK ( ( dim==1 || dim == 0 ) && vector_mask_obj::nested)
	template<typename T,int dim>
	class Vector{
		const bool dont_pop;
	public:
		size_t size;//vector length
		vinstr_t cmv;
		T*	data;
		size_t rows;
		ssize_t increment2; //stored in bytes
		size_t mats;
		ssize_t increment3; //stored in bytes
		Vector():dont_pop(true)
		{}
		INLINE Vector(int sz,int rows=0,int incr2=0/*elements*/,int mats=0,int incr3=0/*elements*/):
			dont_pop(false),size(sz),cmv(VCMV_NZ),rows(rows),increment2(incr2*sizeof(T)),
			mats(mats),increment3(incr3*sizeof(T))
		{
			vbx_sp_push();
			switch(dim){
			case 0:
			case 1:
				data=(T*)vbx_sp_malloc(sizeof(T)*size);
				break;
			case 2:
				if(incr2>=0){
					int data_size=(rows-1)*incr2+size;
					data=(T*)vbx_sp_malloc(sizeof(T)*data_size);
				}else{
					assert("Constructing Matrices with negative strides is not supported\n"&&0);
					int data_size=-incr2*rows;
					data=(T*)vbx_sp_malloc(sizeof(T)*data_size);
					if(data){
						data+=-incr2*(rows-1);
					}
				}
				break;
			case 3:
				if(incr2>=0 && incr3>=0){
					int data_size=(mats-1)*incr3+(rows-1)*incr2+size;
					data=(T*)vbx_sp_malloc(sizeof(T)*data_size);
				}else{
					assert("Constructing Matrices with negative strides is not supported\n"&&0);
				}
				break;
			}
			assert(data!=NULL);
		}
		INLINE Vector(T* sp_ptr,int sz,size_t rows=0,ssize_t increment2=0/*elements*/,
		              size_t mats=0,ssize_t increment3=0/*elements*/):
			dont_pop(true),size(sz),cmv(VCMV_NZ),data(sp_ptr)
		{
			if(dim>=2){
				this->rows=rows;
				this->increment2=increment2*sizeof(T);
			}
			if(dim==3){
				this->mats=mats;
				this->increment3=increment3*sizeof(T);
			}
			//conspicuous lack of push, because we are not allocating space on sp
		}
		enum clone_or_alias{CLONE,ALIAS};
		INLINE Vector(const Vector& cp,clone_or_alias ca)
			:dont_pop(ca==ALIAS),size(cp.size)
		{
			if(dim>=2){
				this->rows=cp.rows;
				this->increment2=cp.increment2;
			}
			if(dim==3){
				this->mats=mats;
				this->increment3=cp.increment3;
			}

			if(ca==CLONE){
				vbx_sp_push();
				data=(T*)vbx_sp_malloc(sizeof(T)*size);
				operator=(cp);
			}else{
				data=cp.data;
			}

		}
		INLINE  Vector(const Vector& cp)
			:dont_pop(false),size(cp.size)
		{
			vbx_sp_push();

			if(dim>=2){
				this->rows=cp.rows;
				this->increment2=cp.increment2;
			}
			if(dim==3){
				this->mats=mats;
				this->increment3=cp.increment3;
			}
			data=(T*)vbx_sp_malloc(sizeof(T)*size);
			operator=(cp);
		}

		template<typename U>
		INLINE  Vector(const Vector<U,dim>& cp)
			:dont_pop(false),size(cp.size)
		{
			if(dim>=2){
				this->rows=cp.rows;
				this->increment2=cp.increment2;
			}
			if(dim==3){
				this->mats=mats;
				this->increment3=cp.increment3;
			}
			vbx_sp_push();
			data=(T*)vbx_sp_malloc(sizeof(T)*size);
			operator=(cp);
		}

		INLINE ~Vector(){
			if(!this->dont_pop){
				vbx_sp_pop();
			}
		}
		INLINE Vector& operator=(const Vector& rhs)
		{
			if(IS_MASKED_BLOCK){
				_internal::assignment<T,IS_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}else{
				_internal::assignment<T,NO_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}
			cmv=rhs.cmv;
			return *this;
		}
		template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
		INLINE Vector& operator=(const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
		{
			dimensions_match<dim,dim1>();
			if(IS_MASKED_BLOCK){
				_internal::assignment<T,IS_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}else{
				_internal::assignment<T,NO_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}
			cmv=_internal::get_cmv_t<instr>::instr;
			return *this;
		}
		template<typename U>
		INLINE Vector& operator=(const Vector<U,dim>& rhs)
		{
			if(IS_MASKED_BLOCK){
				_internal::assignment<T,IS_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}else{
				_internal::assignment<T,NO_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}
			cmv=rhs.cmv;
			return *this;
		}
		INLINE Vector& operator=( vbx_word_t rhs)
		{
			if(IS_MASKED_BLOCK){
				_internal::assignment<T,IS_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}else{
				_internal::assignment<T,NO_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}
			cmv=VCMV_NZ;
			return *this;
		}
		INLINE Vector& operator=( const enum_t& rhs)
		{
			if(IS_MASKED_BLOCK){
				_internal::assignment<T,IS_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}else{
				_internal::assignment<T,NO_MASK,dim>::assign(data,rows,increment2,mats,increment3,rhs,size);
			}
			cmv=VCMV_NZ;
			return *this;
		}
		template<typename lhs_t,typename rhs_t,_internal::log_op_t lop,bool negate>
		INLINE Vector& operator=( const _internal::Logical_vop<lhs_t,rhs_t,lop,negate> lvo)
		{
			T* data_ptr=data;// make copy, so it is not changed by reference
			if(IS_MASKED_BLOCK){
				_internal::resolve_logical<T,IS_MASK,_internal::NONE>::resolve(lvo,data_ptr,size);
			}else{
				_internal::resolve_logical<T,NO_MASK,_internal::NONE>::resolve(lvo,data_ptr,size);
			}
			cmv=VCMV_NZ;
			return *this;
		}
		template<typename U>
		INLINE Vector<U> cast() const
		{
			//only do a copy if
			if(sizeof(T)==sizeof(U)){
				return Vector<U>((U*)data,size);
			}else{
				return Vector<U>(*this);
			}
		}
		template<typename U,int dim1>
		INLINE Vector<U> cast_to_typeof(const Vector<U,dim1>&) const{
			return cast<U>();
		}
		Vector<T>
		INLINE operator[](const range_t& range) const
		{
			dimensions_match<1,dim>();
			return Vector<T>(this->data + range.from, /*data*/
			                 range.to - range.from);/*size*/

		}
		Vector<T,2>
		INLINE operator[](const range2D_t& range) const
		{
			dimensions_match<2,dim>();
			Vector<T,2> toret(this->data + range.rows.from*this->increment2/sizeof(T)+range.cols.from , /*data*/
			                  range.cols.to- range.cols.from,/*columns*/
			                  range.rows.to- range.rows.from,/*rows*/
			                  this->increment2/sizeof(T));/*increment*/
			return toret;
		}

		INLINE accum_t<T> operator[](int index) const
		{
			return accum_t<T>(this->data+index);

		}
		template<typename if_t,typename then_t>
		INLINE void cond_move(const if_t& v_if,const then_t& v_then)
		{
			if(IS_MASKED_BLOCK){
				_internal::assignment<T,IS_MASK,dim>::cond_move(data,v_if,v_then,mats,increment3,rows,increment2,size);
			}else{
				_internal::assignment<T,NO_MASK,dim>::cond_move(data,v_if,v_then,mats,increment3,rows,increment2,size);
			}
		}

		void dma_write(T* to,int host_incr=0){
			if(dim==1){
				vbx_dma_to_host(to,data,sizeof(T)*size);
			}else if(dim==2){
				vbx_dma_to_host_2D(to,data,sizeof(T)*size,rows,host_incr*sizeof(T),increment2/*already in bytes*/);
			}
		}
		void dma_read(T* from,int host_incr=0){
			if(dim==1){
				vbx_dma_to_vector(data,from,sizeof(T)*size);
			}else if(dim==2){
				vbx_dma_to_vector_2D(data,from,sizeof(T)*size,rows,increment2/*already in bytes*/,host_incr*sizeof(T));
			}
		}
		//hope for RVO for these next functions.
		Vector<T> fs(){
			Vector<T> to_ret(this->data,this->size);
			to_ret.cmv=VCMV_FS;
			return to_ret;
		}
		Vector<T> fc(){
			Vector<T> to_ret(this->data,this->size);
			to_ret.cmv=VCMV_FC;
			return to_ret;
		}
		Vector<T> overflow(){
			return fs();
		}
		Vector<T> carry(){
			return fs();
		}

		static void* operator new(size_t sz){
			return malloc(sz);
		}
		static void operator delete(void* ptr){
			free(ptr);
		}

		void printVec() const{
			vbx_sync();
			if(dim==1){
				for(size_t i=0;i<size;i++){
					printf("%8d,",(int)((volatile T*)data)[i]);
				}
				printf("\b \b\n");
			}else if (dim==2){
				for(unsigned i=0;i<rows;i++){
					for(unsigned j=0;j<size;j++){
						int d=(int)((volatile T*)data)[j+increment2/sizeof(T)*i];
						printf("%8d,",d);
					}
					printf("\b \b\n");
				}
			}
		}
		Vector<T,1> to1D(int vl) const {
			return Vector<T,1>(data,vl);
		}
		Vector<T,1> get_row(int row_sel) const {
			return Vector<T,1>(data+row_sel*increment2/sizeof(T),size);
		}
		Vector<T,2> to2D(int vl,int rows,ssize_t increment2) const {
			return Vector<T,2>(data,vl,rows,increment2);
		}
		Vector<T,3> to3D(int vl,int rows,ssize_t increment2,int mats,ssize_t increment3) const {
			return Vector<T,3>(data,vl,rows,increment2,mats,increment3);
		}

	};
#undef IS_MASKED_BLOCK
	template<typename T>
	class accum_t{
		Vector<T,0> v;
	public:
		accum_t()
			:v(1){}
		accum_t(T* sp_ptr)
			:v(sp_ptr,1)
		{}
		accum_t(int init)
			:v(1){
			v=init;
			vbx_sync();
		}

		template<typename U>
		accum_t& operator=( const U& rhs){
			v=rhs;
			return *this;
		}
		accum_t& operator=( T rhs){
			v=rhs;
			vbx_sync();
			return *this;
		}

		T async_read()
		{return *(volatile T*)v.data;}
		T sync_read()
		{vbx_sync();return *(volatile T*)v.data;}
		operator T(){return sync_read();}

	};
#include "operators.hpp"
}//namesapce VBX
#include "masked_vector.hpp"
#undef INLINE
#endif //__VECTOR_HPP__
