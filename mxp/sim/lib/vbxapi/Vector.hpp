/* VECTORBLOX MXP SOFTWARE DEVELOPMENT KIT
 *
 * Copyright (C) 2012-2015 VectorBlox Computing Inc., Vancouver, British Columbia, Canada.
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
#define INLINE __attribute__((always_inline)) inline
extern "C" size_t __old_vl__;
namespace VBX{
#include "vinstr.hpp"
	template<typename T>
	class Vector;
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
		template<typename T,typename U,vinstr_t instr,typename btype>
		struct bin_op{
			const T& lhs;
			const U& rhs;
			bin_op(const T& lhs,const U& rhs)
				:lhs(lhs),rhs(rhs){}
			template<typename new_btype>
			bin_op<T,U,instr,new_btype> cast(){
				return bin_op<T,U,instr,new_btype>(lhs,rhs);
			}
		};
		template<typename T,vinstr_t instr,typename btype>
		struct bin_op<vbx_word_t,T,instr,btype>{
			const vbx_word_t lhs;
			const T& rhs;
			bin_op(const vbx_word_t lhs,const T& rhs)
				:lhs(lhs),rhs(rhs){}
			template<typename new_btype>
			bin_op<vbx_word_t,T,instr,new_btype> cast(){
				return bin_op<vbx_word_t,T,instr,new_btype>(lhs,rhs);
			}

		};
		template<typename T>
		struct accum_op{
			const T& op;
			const size_t len;
			accum_op(const T& op,size_t len):op(op),len(len){}
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
template<typename T>
struct accum {
T* data;
bool alloc;
public:
accum(bool alloc=true) {
	this->alloc=alloc;
	if (alloc){
		vbx_sp_push();
		data=(T*)vbx_sp_malloc(sizeof(T));
	}
}
	accum(const accum& s){
		data=s.data;
	}
	accum& operator=(vbx_word_t d){
		_internal::set_vl(1);
		if(vector_mask_obj::nested){
			_internal::vbx_func<true,true>::func(VMOV,data,d);
		}else{
			_internal::vbx_func<false,true>::func(VMOV,data,d);
		}

		return *this;
	}
	accum(int d,bool alloc=true){
		this->alloc=alloc;
		if (alloc){
			vbx_sp_push();
			data=(vbx_word_t*)vbx_sp_malloc(sizeof(T));
		}
		operator=(d);
	}
	template<typename op_t>
	accum& operator=(const _internal::accum_op<op_t>& op)
	{
		_internal::set_vl(op.len);
		if(vector_mask_obj::nested){
			_internal::assignment<T,true,true>::assign(data,op.op,op.len);
		}else{
			_internal::assignment<T,false,true>::assign(data,op.op,op.len);
		}
		return *this;
	}
	~accum(){
		if(alloc)vbx_sp_pop();
	}
	T async_getval(){
		volatile T* ptr=(volatile T*)(data);
		return *ptr;
	}
	T sync_getval(){
		vbx_sync();
		volatile T* ptr=(volatile T*)(data);
		return *ptr;
	}
	operator T(){
		return sync_getval();
	}
};
	template<typename T>
	_internal::accum_op<T> accumulate(const T& op,size_t len)
	{
		return _internal::accum_op<T>(op,len);
	}
	namespace _internal{
		struct default_size_t{
			int default_size;
			void set(size_t s){
				default_size=s;
			}
			size_t get(){
				return default_size;
			}
		};
	}
	extern _internal::default_size_t default_vec_size;
	template<typename T>
	class Vector{
		const bool dont_pop;
	public:
		size_t size;
		T*	data;
		vinstr_t cmv;
		INLINE Vector(int sz):dont_pop(false),size(sz),cmv(VCMV_NZ)
		{
			vbx_sp_push();
			data=(T*)vbx_sp_malloc(sizeof(T)*size);
		}
		INLINE Vector(T* sp_ptr,int sz):dont_pop(true),size(sz),cmv(VCMV_NZ)
		{
			//conspicuous lack of push, because we are not allocating space on sp
			data=sp_ptr;
		}

		INLINE  Vector(const Vector& cp)
			:dont_pop(false),size(cp.size),cmv(cp.cmv)
		{
			vbx_sp_push();
			data=(T*)vbx_sp_malloc(sizeof(T)*size);
			_internal::set_vl(size);
			if(vector_mask_obj::nested){
				_internal::vbx_func<true,false>::func(VMOV,data,cp.data);
			}else{
				_internal::vbx_func<false,false>::func(VMOV,data,cp.data);
			}
		}
		template<typename U>
		INLINE  Vector(const Vector<U>& cp)
			:dont_pop(false),size(cp.size),cmv(cp.cmv)
		{
			vbx_sp_push();
			data=(T*)vbx_sp_malloc(sizeof(T)*size);
			_internal::set_vl(size);
			if(vector_mask_obj::nested){
				_internal::assignment<T,true>::assign(data,cp,size);
			}else{
				_internal::assignment<T,false>::assign(data,cp,size);
			}
		}

		INLINE ~Vector(){
			if(this->dont_pop){
			}else{
				vbx_sp_pop();
			}
		}
		INLINE Vector& operator=(const Vector& rhs)
		{
			_internal::set_vl(size);
			_internal::vector_length::check(size,rhs);
			if(vector_mask_obj::nested){
				_internal::assignment<T,true>::assign(data,rhs,size);
			}else{
				_internal::assignment<T,false>::assign(data,rhs,size);
			}
			cmv=rhs.cmv;
			return *this;
		}
		template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
		INLINE Vector& operator=(const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
		{
			_internal::set_vl(size);
			_internal::vector_length::check(size,rhs);
			if(vector_mask_obj::nested){
				_internal::assignment<T,true>::assign(data,rhs,size);
			}else{
				_internal::assignment<T,false>::assign(data,rhs,size);
			}
			cmv=_internal::get_cmv_t<instr>::instr;
			return *this;
		}
		template<typename U>
		INLINE Vector& operator=(const Vector<U>& rhs)
		{
			_internal::set_vl(size);
			_internal::vector_length::check(size,rhs);
			if(vector_mask_obj::nested){
				_internal::assignment<T,true>::assign(data,rhs,size);
			}else{
				_internal::assignment<T,false>::assign(data,rhs,size);
			}
			cmv=rhs.cmv;
			return *this;
		}
		INLINE Vector& operator=( typename word_sized<T>::type rhs)
		{
			_internal::set_vl(size);
			if(vector_mask_obj::nested){
				_internal::vbx_func<true,false>::func(VMOV,data,rhs);
			}else{
				_internal::vbx_func<false,false>::func(VMOV,data,rhs);
			}
			cmv=VCMV_NZ;
			return *this;
		}
		INLINE Vector& operator=( const enum_t& )
		{
			_internal::set_vl(size);
			if(vector_mask_obj::nested){
				_internal::vbx_func<true,false>::func(VMUL,data,1,(vbx_enum_t*)0);
			}else{
				_internal::vbx_func<false,false>::func(VMUL,data,1,(vbx_enum_t*)0);
			}
			cmv=VCMV_NZ;
			return *this;
		}
		template<typename lhs_t,typename rhs_t,_internal::log_op_t lop,bool negate>
		INLINE Vector& operator=( const _internal::Logical_vop<lhs_t,rhs_t,lop,negate> lvo)
		{
			T* data=this->data;
			if(vector_mask_obj::nested){
				_internal::resolve_logical<T,true,_internal::NONE>::resolve(lvo,data,size);
			}else{
				_internal::resolve_logical<T,false,_internal::NONE>::resolve(lvo,data,size);
			}
			cmv=VCMV_NZ;
			return *this;
		}
		template<typename U>
		INLINE Vector<U> cast()
		{
			//only do a copy if
			if(sizeof(T)==sizeof(U)){
				return Vector<U>((U*)data,size);
			}else{
				return Vector<U>(*this);
			}
		}

		Vector<T>
		INLINE operator[](const range_t& range)
		{
			return Vector<T>(this->data + range.from, /*data*/
			                 range.to - range.from);/*size*/

		}
		INLINE accum<T> operator[](int index)
		{
			accum<T> acc(false);//accum_t without allocating space
			acc.data=this->data+index;
			return acc;
		}
		template<typename if_t,typename then_t>
		INLINE void cond_move(const if_t& _if_,const then_t& _then_)
		{
			typedef typename get_op_size<if_t>::type t1;
			typedef typename get_op_size<then_t>::type t2;
			typedef typename types_are_equivalent<t1,t2>::type btype;
			_internal::set_vl(size);
			if(vector_mask_obj::nested){
				_internal::assignment<T,true>::cond_move(data,_if_,_then_,size,(btype*)0);
			}else{
				_internal::assignment<T,false>::cond_move(data,_if_,_then_,size,(btype*)0);
			}
		}
		template<typename U>
		INLINE void cond_move(const U& _if_,const vbx_word_t _then_)
		{
			typedef typename get_op_size<U>::type btype;
			_internal::set_vl(size);
			_internal::vector_length::check(size,_if_);
			if(vector_mask_obj::nested){
				_internal::assignment<T,true>::cond_move(data,_if_,_then_,size,(btype*)0);
			}else{
				_internal::assignment<T,false>::cond_move(data,_if_,_then_,size,(btype*)0);
			}
		}
		template<typename U>
		INLINE void cond_move(const U& _if_,const vbx_uword_t _then_)
		{
			_internal::set_vl(size);
			_internal::vector_length::check(size,_if_);
			vbx_word_t _then_1=_then_;
			if(vector_mask_obj::nested){
				_internal::assignment<T,true>::cond_move(data,_if_,_then_1,size);
			}else{
				_internal::assignment<T,false>::cond_move(data,_if_,_then_1,size);
			}
		}

		T* get_sp_ptr(){return data;}
		size_t get_size(){return size;}
		void dma_write(T* to){
			vbx_dma_to_host(to,data,sizeof(T)*size);
		}
		void dma_read(T* from){
			vbx_dma_to_vector(data,from,sizeof(T)*size);
		}
		Vector()
			:dont_pop(false),size(default_vec_size.get()),cmv(VCMV_NZ)
		{
			vbx_sp_push();
			data=(T*)vbx_sp_malloc(sizeof(T)*size);
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
		static void* operator new[](size_t sz){
			return malloc(sz);
		}
		static void operator delete(void* ptr){
			free(ptr);
		}
		static void operator delete[](void* ptr){
			free(ptr);
		}

		void printVec() const{
			T* out=(T*)vbx_shared_malloc(sizeof(T)*size);
			vbx_dma_to_host(out,data,sizeof(T)*size);
			vbx_sync();
			for(size_t i=0;i<size;i++){
				printf("%8d,",(int)data[i]);
			}
			printf("\b \b\n");
			vbx_shared_free(out);
		}
	};

#include "operators.hpp"

}//namesapce VBX
#include "masked_vector.hpp"
#undef INLINE
#endif //__VECTOR_HPP__
