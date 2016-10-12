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

#ifndef __operators_hpp__
#define __operators_hpp__

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VADD,T,dim1>
 operator+ (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VADD,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VADD,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator+ (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VADD,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VADD,T,dim1>
INLINE operator+ (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VADD,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VADD,vbx_enum_t,-1>
INLINE operator+ (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VADD,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VADD,type,dim1>
INLINE operator+ (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VADD,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VADD,T,dims>
INLINE operator+ (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VADD,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VADD,type,dim1>
INLINE operator+ (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VADD,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VADD,T,dim2>
INLINE operator+ (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VADD,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VADD,T,dim2>
INLINE operator+ (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VADD,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim1>
_internal::bin_op<Vector<T,dim1>,enum_t,VADD,T,dim1>
INLINE operator+ (const enum_t& lhs,const Vector<T,dim1>& rhs)
{
	return operator+(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VADD,btype,dim1>
INLINE operator+ ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator+(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VADD,vbx_enum_t,-1>
INLINE operator+ (const enum_t &lhs,const vbx_word_t& rhs)
{
	return operator+(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VADD,T,dim>
INLINE operator+ (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator+(rhs, lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VADD,btype,dim1>
INLINE operator+ (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return operator+(rhs, lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VSUB,T,dim1>
 operator- (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VSUB,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VSUB,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator- (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VSUB,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VSUB,T,dim1>
INLINE operator- (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VSUB,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VSUB,vbx_enum_t,-1>
INLINE operator- (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VSUB,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VSUB,type,dim1>
INLINE operator- (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VSUB,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VSUB,T,dims>
INLINE operator- (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VSUB,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VSUB,type,dim1>
INLINE operator- (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VSUB,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VSUB,T,dim2>
INLINE operator- (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VSUB,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VSUB,T,dim2>
INLINE operator- (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VSUB,T,dim1>(lhs,rhs);
}

//EV
//TODO: implement a Vector subtracted from an ENUM

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VSUB,btype,dim1>
INLINE operator- ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator-(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VADD,vbx_enum_t,-1>
INLINE operator- (const enum_t &lhs,const vbx_word_t& rhs)
{
	return (-rhs) + lhs;
}

//VS
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VADD,T,dim1>
INLINE operator- (const Vector<T,dim1>& lhs, vbx_word_t rhs)
{
	return (-rhs) + lhs;
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1>,VADD,btype,dim1,acc>
INLINE operator- (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return (-rhs) + lhs;
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VMUL,T,dim1>
 operator* (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VMUL,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VMUL,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator* (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VMUL,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VMUL,T,dim1>
INLINE operator* (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VMUL,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VMUL,vbx_enum_t,-1>
INLINE operator* (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VMUL,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VMUL,type,dim1>
INLINE operator* (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VMUL,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VMUL,T,dims>
INLINE operator* (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VMUL,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VMUL,type,dim1>
INLINE operator* (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VMUL,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VMUL,T,dim2>
INLINE operator* (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VMUL,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VMUL,T,dim2>
INLINE operator* (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VMUL,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim1>
_internal::bin_op<Vector<T,dim1>,enum_t,VMUL,T,dim1>
INLINE operator* (const enum_t& lhs,const Vector<T,dim1>& rhs)
{
	return operator*(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VMUL,btype,dim1>
INLINE operator* ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator*(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VMUL,vbx_enum_t,-1>
INLINE operator* (const enum_t &lhs,const vbx_word_t& rhs)
{
	return operator*(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VMUL,T,dim>
INLINE operator* (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator*(rhs, lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VMUL,btype,dim1>
INLINE operator* (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return operator*(rhs, lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VAND,T,dim1>
 operator& (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VAND,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VAND,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator& (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VAND,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VAND,T,dim1>
INLINE operator& (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VAND,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VAND,vbx_enum_t,-1>
INLINE operator& (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VAND,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VAND,type,dim1>
INLINE operator& (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VAND,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VAND,T,dims>
INLINE operator& (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VAND,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VAND,type,dim1>
INLINE operator& (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VAND,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VAND,T,dim2>
INLINE operator& (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VAND,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VAND,T,dim2>
INLINE operator& (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VAND,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim1>
_internal::bin_op<Vector<T,dim1>,enum_t,VAND,T,dim1>
INLINE operator& (const enum_t& lhs,const Vector<T,dim1>& rhs)
{
	return operator&(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VAND,btype,dim1>
INLINE operator& ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator&(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VAND,vbx_enum_t,-1>
INLINE operator& (const enum_t &lhs,const vbx_word_t& rhs)
{
	return operator&(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VAND,T,dim>
INLINE operator& (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator&(rhs, lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VAND,btype,dim1>
INLINE operator& (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return operator&(rhs, lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VXOR,T,dim1>
 operator^ (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VXOR,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VXOR,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator^ (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VXOR,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VXOR,T,dim1>
INLINE operator^ (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VXOR,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VXOR,vbx_enum_t,-1>
INLINE operator^ (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VXOR,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VXOR,type,dim1>
INLINE operator^ (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VXOR,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VXOR,T,dims>
INLINE operator^ (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VXOR,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VXOR,type,dim1>
INLINE operator^ (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VXOR,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VXOR,T,dim2>
INLINE operator^ (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VXOR,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VXOR,T,dim2>
INLINE operator^ (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VXOR,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim1>
_internal::bin_op<Vector<T,dim1>,enum_t,VXOR,T,dim1>
INLINE operator^ (const enum_t& lhs,const Vector<T,dim1>& rhs)
{
	return operator^(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VXOR,btype,dim1>
INLINE operator^ ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator^(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VXOR,vbx_enum_t,-1>
INLINE operator^ (const enum_t &lhs,const vbx_word_t& rhs)
{
	return operator^(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VXOR,T,dim>
INLINE operator^ (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator^(rhs, lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VXOR,btype,dim1>
INLINE operator^ (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return operator^(rhs, lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VOR,T,dim1>
 operator| (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VOR,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VOR,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator| (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VOR,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VOR,T,dim1>
INLINE operator| (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VOR,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VOR,vbx_enum_t,-1>
INLINE operator| (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VOR,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VOR,type,dim1>
INLINE operator| (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VOR,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VOR,T,dims>
INLINE operator| (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VOR,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VOR,type,dim1>
INLINE operator| (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VOR,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VOR,T,dim2>
INLINE operator| (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VOR,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VOR,T,dim2>
INLINE operator| (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VOR,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim1>
_internal::bin_op<Vector<T,dim1>,enum_t,VOR,T,dim1>
INLINE operator| (const enum_t& lhs,const Vector<T,dim1>& rhs)
{
	return operator|(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VOR,btype,dim1>
INLINE operator| ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator|(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VOR,vbx_enum_t,-1>
INLINE operator| (const enum_t &lhs,const vbx_word_t& rhs)
{
	return operator|(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VOR,T,dim>
INLINE operator| (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator|(rhs, lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VOR,btype,dim1>
INLINE operator| (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return operator|(rhs, lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VABSDIFF,T,dim1>
 absdiff (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VABSDIFF,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VABSDIFF,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE absdiff (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VABSDIFF,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VABSDIFF,T,dim1>
INLINE absdiff (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VABSDIFF,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VABSDIFF,vbx_enum_t,-1>
INLINE absdiff (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VABSDIFF,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VABSDIFF,type,dim1>
INLINE absdiff (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VABSDIFF,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VABSDIFF,T,dims>
INLINE absdiff (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VABSDIFF,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VABSDIFF,type,dim1>
INLINE absdiff (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VABSDIFF,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VABSDIFF,T,dim2>
INLINE absdiff (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VABSDIFF,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VABSDIFF,T,dim2>
INLINE absdiff (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VABSDIFF,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim1>
_internal::bin_op<Vector<T,dim1>,enum_t,VABSDIFF,T,dim1>
INLINE absdiff (const enum_t& lhs,const Vector<T,dim1>& rhs)
{
	return absdiff(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VABSDIFF,btype,dim1>
INLINE absdiff ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return absdiff(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VABSDIFF,vbx_enum_t,-1>
INLINE absdiff (const enum_t &lhs,const vbx_word_t& rhs)
{
	return absdiff(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VABSDIFF,T,dim>
INLINE absdiff (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return absdiff(rhs, lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VABSDIFF,btype,dim1>
INLINE absdiff (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return absdiff(rhs, lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VMULFXP,T,dim1>
 mulfxp (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VMULFXP,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VMULFXP,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE mulfxp (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VMULFXP,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VMULFXP,T,dim1>
INLINE mulfxp (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VMULFXP,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VMULFXP,vbx_enum_t,-1>
INLINE mulfxp (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VMULFXP,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VMULFXP,type,dim1>
INLINE mulfxp (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VMULFXP,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VMULFXP,T,dims>
INLINE mulfxp (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VMULFXP,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VMULFXP,type,dim1>
INLINE mulfxp (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VMULFXP,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VMULFXP,T,dim2>
INLINE mulfxp (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VMULFXP,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VMULFXP,T,dim2>
INLINE mulfxp (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VMULFXP,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim1>
_internal::bin_op<Vector<T,dim1>,enum_t,VMULFXP,T,dim1>
INLINE mulfxp (const enum_t& lhs,const Vector<T,dim1>& rhs)
{
	return mulfxp(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VMULFXP,btype,dim1>
INLINE mulfxp ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return mulfxp(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VMULFXP,vbx_enum_t,-1>
INLINE mulfxp (const enum_t &lhs,const vbx_word_t& rhs)
{
	return mulfxp(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VMULFXP,T,dim>
INLINE mulfxp (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return mulfxp(rhs, lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VMULFXP,btype,dim1>
INLINE mulfxp (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return mulfxp(rhs, lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VMULHI,T,dim1>
 mulhi (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VMULHI,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VMULHI,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE mulhi (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VMULHI,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VMULHI,T,dim1>
INLINE mulhi (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VMULHI,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VMULHI,vbx_enum_t,-1>
INLINE mulhi (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VMULHI,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VMULHI,type,dim1>
INLINE mulhi (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VMULHI,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VMULHI,T,dims>
INLINE mulhi (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VMULHI,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VMULHI,type,dim1>
INLINE mulhi (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VMULHI,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VMULHI,T,dim2>
INLINE mulhi (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VMULHI,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VMULHI,T,dim2>
INLINE mulhi (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VMULHI,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim1>
_internal::bin_op<Vector<T,dim1>,enum_t,VMULHI,T,dim1>
INLINE mulhi (const enum_t& lhs,const Vector<T,dim1>& rhs)
{
	return mulhi(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VMULHI,btype,dim1>
INLINE mulhi ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return mulhi(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VMULHI,vbx_enum_t,-1>
INLINE mulhi (const enum_t &lhs,const vbx_word_t& rhs)
{
	return mulhi(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VMULHI,T,dim>
INLINE mulhi (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return mulhi(rhs, lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VMULHI,btype,dim1>
INLINE mulhi (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return mulhi(rhs, lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<U,dim2>,Vector<T,dim1>,VSHL,T,
	                 dim2>

 operator<< (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<U,dim2>,Vector<T,dim1>,VSHL,T,
	                 dim2>(rhs,lhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2 >
_internal::bin_op<_internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type1,dim1,acc1>,
                  _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type2,dim2,acc2>,VSHL,
                  typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim >
INLINE operator<< (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type2,dim2,acc2>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type1,dim1,acc1>& rhs)
{
	return _internal::bin_op<_internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,VSHL,
	                         typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(rhs,lhs);
}

//SV
//TODO: add scalar shifted by vector

//SE
//TODO: add enum shifted by scalar

//SB
//TODO: add scalar shifted by _internal::bin_op

//VE
//TODO implement Vector shifted by enum

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VSHL,type,dim1>
INLINE operator<< (const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,
	                         enum_t,VSHL,type,dim1>(lhs,rhs);
}


//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VSHL,T,
                  dim1>
INLINE operator<< (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& lhs,const Vector<T,dim1>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,
                  VSHL,T,dim1>(rhs,lhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VSHL,T,dim2>
INLINE operator<< (const Vector<T,dim2>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,
	                         Vector<T,dim2>,VSHL,T,dim2>(rhs,lhs);
}

//EV
//TODO: add Vector shifted by enum

//EB
//TODO: add binop shifted by enum

//ES
_internal::bin_op<vbx_word_t,enum_t,VSHL,vbx_enum_t,-1>
INLINE operator<< (const enum_t& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VSHL,vbx_enum_t,-1>(rhs,lhs);
}
//EV
template<typename T,int dim>
_internal::bin_op<Vector<T>,enum_t,VSHL,T,dim>
INLINE operator<< (const enum_t& lhs,const Vector<T,dim>& rhs)
{
	return _internal::bin_op<Vector<T,dim>,enum_t,VSHL,T,dim>(rhs,lhs);
}

//VS
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VSHL,T,dim1>
INLINE operator<< (const Vector<T,dim1>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VSHL,T,dim1>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VSHL,btype,dim1>
INLINE operator<< (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,
	                         VSHL,btype,dim1>(rhs,lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<U,dim2>,Vector<T,dim1>,VSHR,T,
	                 dim2>

 operator>> (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<U,dim2>,Vector<T,dim1>,VSHR,T,
	                 dim2>(rhs,lhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2 >
_internal::bin_op<_internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type1,dim1,acc1>,
                  _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type2,dim2,acc2>,VSHR,
                  typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim >
INLINE operator>> (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type2,dim2,acc2>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type1,dim1,acc1>& rhs)
{
	return _internal::bin_op<_internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,VSHR,
	                         typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(rhs,lhs);
}

//SV
//TODO: add scalar shifted by vector

//SE
//TODO: add enum shifted by scalar

//SB
//TODO: add scalar shifted by _internal::bin_op

//VE
//TODO implement Vector shifted by enum

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VSHR,type,dim1>
INLINE operator>> (const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,
	                         enum_t,VSHR,type,dim1>(lhs,rhs);
}


//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VSHR,T,
                  dim1>
INLINE operator>> (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& lhs,const Vector<T,dim1>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,
                  VSHR,T,dim1>(rhs,lhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VSHR,T,dim2>
INLINE operator>> (const Vector<T,dim2>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,
	                         Vector<T,dim2>,VSHR,T,dim2>(rhs,lhs);
}

//EV
//TODO: add Vector shifted by enum

//EB
//TODO: add binop shifted by enum

//ES
_internal::bin_op<vbx_word_t,enum_t,VSHR,vbx_enum_t,-1>
INLINE operator>> (const enum_t& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VSHR,vbx_enum_t,-1>(rhs,lhs);
}
//EV
template<typename T,int dim>
_internal::bin_op<Vector<T>,enum_t,VSHR,T,dim>
INLINE operator>> (const enum_t& lhs,const Vector<T,dim>& rhs)
{
	return _internal::bin_op<Vector<T,dim>,enum_t,VSHR,T,dim>(rhs,lhs);
}

//VS
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VSHR,T,dim1>
INLINE operator>> (const Vector<T,dim1>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VSHR,T,dim1>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VSHR,btype,dim1>
INLINE operator>> (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,
	                         VSHR,btype,dim1>(rhs,lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VCMV_LTZ,T,dim1>
 operator< (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VCMV_LTZ,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VCMV_LTZ,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator< (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VCMV_LTZ,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_LTZ,T,dim1>
INLINE operator< (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_LTZ,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_LTZ,vbx_enum_t,-1>
INLINE operator< (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_LTZ,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_LTZ,type,dim1>
INLINE operator< (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_LTZ,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VCMV_LTZ,T,dims>
INLINE operator< (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VCMV_LTZ,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_LTZ,type,dim1>
INLINE operator< (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_LTZ,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_LTZ,T,dim2>
INLINE operator< (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_LTZ,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VCMV_LTZ,T,dim2>
INLINE operator< (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VCMV_LTZ,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim>
_internal::bin_op<Vector<T,dim>,enum_t,VCMV_GTZ,T,dim>
INLINE operator< (const enum_t& lhs,const Vector<T,dim>& rhs)
{
	return _internal::bin_op<Vector<T,dim>,enum_t,VCMV_GTZ,T,dim>(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VCMV_LTZ,btype,dim1>
INLINE operator< ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator<(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_GTZ,vbx_enum_t,-1>
INLINE operator< (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_GTZ,vbx_enum_t,-1>(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_GTZ,T,dim>
INLINE operator< (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_GTZ,T,dim>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>,VCMV_GTZ,btype,dim>
INLINE operator< (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>
	                      ,VCMV_GTZ,btype,dim>(rhs,lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VCMV_GTZ,T,dim1>
 operator> (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VCMV_GTZ,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VCMV_GTZ,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator> (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VCMV_GTZ,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_GTZ,T,dim1>
INLINE operator> (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_GTZ,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_GTZ,vbx_enum_t,-1>
INLINE operator> (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_GTZ,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_GTZ,type,dim1>
INLINE operator> (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_GTZ,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VCMV_GTZ,T,dims>
INLINE operator> (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VCMV_GTZ,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_GTZ,type,dim1>
INLINE operator> (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_GTZ,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_GTZ,T,dim2>
INLINE operator> (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_GTZ,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VCMV_GTZ,T,dim2>
INLINE operator> (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VCMV_GTZ,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim>
_internal::bin_op<Vector<T,dim>,enum_t,VCMV_LTZ,T,dim>
INLINE operator> (const enum_t& lhs,const Vector<T,dim>& rhs)
{
	return _internal::bin_op<Vector<T,dim>,enum_t,VCMV_LTZ,T,dim>(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VCMV_GTZ,btype,dim1>
INLINE operator> ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_LTZ,vbx_enum_t,-1>
INLINE operator> (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_LTZ,vbx_enum_t,-1>(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_LTZ,T,dim>
INLINE operator> (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_LTZ,T,dim>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>,VCMV_LTZ,btype,dim>
INLINE operator> (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>
	                      ,VCMV_LTZ,btype,dim>(rhs,lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VCMV_LEZ,T,dim1>
 operator<= (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VCMV_LEZ,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VCMV_LEZ,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator<= (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VCMV_LEZ,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_LEZ,T,dim1>
INLINE operator<= (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_LEZ,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_LEZ,vbx_enum_t,-1>
INLINE operator<= (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_LEZ,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_LEZ,type,dim1>
INLINE operator<= (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_LEZ,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VCMV_LEZ,T,dims>
INLINE operator<= (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VCMV_LEZ,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_LEZ,type,dim1>
INLINE operator<= (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_LEZ,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_LEZ,T,dim2>
INLINE operator<= (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_LEZ,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VCMV_LEZ,T,dim2>
INLINE operator<= (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VCMV_LEZ,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim>
_internal::bin_op<Vector<T,dim>,enum_t,VCMV_GEZ,T,dim>
INLINE operator<= (const enum_t& lhs,const Vector<T,dim>& rhs)
{
	return _internal::bin_op<Vector<T,dim>,enum_t,VCMV_GEZ,T,dim>(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VCMV_LEZ,btype,dim1>
INLINE operator<= ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator<=(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_GEZ,vbx_enum_t,-1>
INLINE operator<= (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_GEZ,vbx_enum_t,-1>(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_GEZ,T,dim>
INLINE operator<= (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_GEZ,T,dim>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>,VCMV_GEZ,btype,dim>
INLINE operator<= (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>
	                      ,VCMV_GEZ,btype,dim>(rhs,lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VCMV_GEZ,T,dim1>
 operator>= (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VCMV_GEZ,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VCMV_GEZ,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator>= (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VCMV_GEZ,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_GEZ,T,dim1>
INLINE operator>= (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_GEZ,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_GEZ,vbx_enum_t,-1>
INLINE operator>= (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_GEZ,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_GEZ,type,dim1>
INLINE operator>= (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_GEZ,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VCMV_GEZ,T,dims>
INLINE operator>= (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VCMV_GEZ,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_GEZ,type,dim1>
INLINE operator>= (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_GEZ,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_GEZ,T,dim2>
INLINE operator>= (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_GEZ,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VCMV_GEZ,T,dim2>
INLINE operator>= (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VCMV_GEZ,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim>
_internal::bin_op<Vector<T,dim>,enum_t,VCMV_LEZ,T,dim>
INLINE operator>= (const enum_t& lhs,const Vector<T,dim>& rhs)
{
	return _internal::bin_op<Vector<T,dim>,enum_t,VCMV_LEZ,T,dim>(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VCMV_GEZ,btype,dim1>
INLINE operator>= ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator>=(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_LEZ,vbx_enum_t,-1>
INLINE operator>= (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_LEZ,vbx_enum_t,-1>(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_LEZ,T,dim>
INLINE operator>= (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_LEZ,T,dim>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>,VCMV_LEZ,btype,dim>
INLINE operator>= (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>
	                      ,VCMV_LEZ,btype,dim>(rhs,lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VCMV_Z,T,dim1>
 operator== (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VCMV_Z,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VCMV_Z,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator== (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VCMV_Z,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_Z,T,dim1>
INLINE operator== (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_Z,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_Z,vbx_enum_t,-1>
INLINE operator== (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_Z,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_Z,type,dim1>
INLINE operator== (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_Z,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VCMV_Z,T,dims>
INLINE operator== (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VCMV_Z,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_Z,type,dim1>
INLINE operator== (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_Z,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_Z,T,dim2>
INLINE operator== (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_Z,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VCMV_Z,T,dim2>
INLINE operator== (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VCMV_Z,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim>
_internal::bin_op<Vector<T,dim>,enum_t,VCMV_Z,T,dim>
INLINE operator== (const enum_t& lhs,const Vector<T,dim>& rhs)
{
	return _internal::bin_op<Vector<T,dim>,enum_t,VCMV_Z,T,dim>(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VCMV_Z,btype,dim1>
INLINE operator== ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator==(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_Z,vbx_enum_t,-1>
INLINE operator== (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_Z,vbx_enum_t,-1>(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_Z,T,dim>
INLINE operator== (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_Z,T,dim>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>,VCMV_Z,btype,dim>
INLINE operator== (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>
	                      ,VCMV_Z,btype,dim>(rhs,lhs);
}

//VV
template <typename T,typename U,int dim1,int dim2>
INLINE _internal::bin_op<Vector<T,dim1>,Vector<U,dim2>,VCMV_NZ,T,dim1>
 operator!= (const Vector<T,dim1>& lhs,const Vector<U,dim2>& rhs)
{
	types_are_equivalent<T,U>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,Vector<T,dim2>,VCMV_NZ,T,dim1>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,int dim1,acc_choice acc1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2,int dim2,acc_choice acc2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,VCMV_NZ,
	          typename types_are_equivalent<type1,type2>::type,
	          dimensions_match<dim1,dim2>::dim>
INLINE operator!= (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1,dim1,acc1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2,dim2,acc2>,
	                         VCMV_NZ,typename types_are_equivalent<type1,type2>::type,
	                         dimensions_match<dim1,dim2>::dim >(lhs,rhs);
}

//SV
template<typename T,int dim1>
_internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_NZ,T,dim1>
INLINE operator!= (const vbx_word_t& lhs,const Vector<T,dim1>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim1>,VCMV_NZ,T,dim1>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_NZ,vbx_enum_t,-1>
INLINE operator!= (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_NZ,vbx_enum_t,-1>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_NZ,type,dim1>
INLINE operator!= (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,VCMV_NZ,type,dim1>(lhs,rhs);
}

//VE
template<typename T,int dims>
_internal::bin_op<Vector<T,dims>,enum_t,VCMV_NZ,T,dims>
INLINE operator!= (const Vector<T,dims>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T,dims>,enum_t,VCMV_NZ,T,dims>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_NZ,type,dim1>
INLINE operator!= (const _internal::bin_op<lhs_t,rhs_t,instr,type,dim1>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type,dim1,acc>,enum_t,VCMV_NZ,type,dim1>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_NZ,T,dim2>
INLINE operator!= (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& lhs,const Vector<T,dim2>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,Vector<T,dim2>,VCMV_NZ,T,dim2>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,int dim2,acc_choice acc>
_internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,VCMV_NZ,T,dim2>
INLINE operator!= (const Vector<T,dim1>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& rhs)
{
	types_are_equivalent<T,btype>();
	dimensions_match<dim1,dim2>();
	return _internal::bin_op<Vector<T,dim1>,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>,VCMV_NZ,T,dim1>(lhs,rhs);
}

//EV
template<typename T,int dim>
_internal::bin_op<Vector<T,dim>,enum_t,VCMV_NZ,T,dim>
INLINE operator!= (const enum_t& lhs,const Vector<T,dim>& rhs)
{
	return _internal::bin_op<Vector<T,dim>,enum_t,VCMV_NZ,T,dim>(rhs,lhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim1,acc_choice acc>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>,enum_t,VCMV_NZ,btype,dim1>
INLINE operator!= ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim1,acc>& rhs)
{
	return operator!=(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_NZ,vbx_enum_t,-1>
INLINE operator!= (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_NZ,vbx_enum_t,-1>(rhs,lhs);
}

//VS
template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_NZ,T,dim>
INLINE operator!= (const Vector<T,dim>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T,dim>,VCMV_NZ,T,dim>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim,acc_choice acc>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>,VCMV_NZ,btype,dim>
INLINE operator!= (const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype,dim,acc>
	                      ,VCMV_NZ,btype,dim>(rhs,lhs);
}

////////
//compound assignment
//////
template<typename T,int dim1,
         typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim2,acc_choice acc>
INLINE Vector<T> &operator+=(Vector<T,dim1>& a, const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& b){
	a=a + b.cast_to_typeof(a);
	return a;
}

template<typename T,typename U,int dim>
INLINE Vector<T> &operator+=(Vector<T,dim>& a, const Vector<U,dim>& b){
	a=a + b.cast_to_typeof(a);
	return a;
}
template<typename T>
INLINE Vector<T> &operator+=(Vector<T>& a, vbx_word_t b){
	a=a + b;
	return a;
}
template<typename T>
INLINE Vector<T> &operator+=(Vector<T>& a, enum_t b){
	a=a + b;
	return a;
}


////////
//compound assignment
//////
template<typename T,int dim1,
         typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim2,acc_choice acc>
INLINE Vector<T> &operator-=(Vector<T,dim1>& a, const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& b){
	a=a - b.cast_to_typeof(a);
	return a;
}

template<typename T,typename U,int dim>
INLINE Vector<T> &operator-=(Vector<T,dim>& a, const Vector<U,dim>& b){
	a=a - b.cast_to_typeof(a);
	return a;
}
template<typename T>
INLINE Vector<T> &operator-=(Vector<T>& a, vbx_word_t b){
	a=a - b;
	return a;
}
template<typename T>
INLINE Vector<T> &operator-=(Vector<T>& a, enum_t b){
	a=a - b;
	return a;
}


////////
//compound assignment
//////
template<typename T,int dim1,
         typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim2,acc_choice acc>
INLINE Vector<T> &operator*=(Vector<T,dim1>& a, const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& b){
	a=a * b.cast_to_typeof(a);
	return a;
}

template<typename T,typename U,int dim>
INLINE Vector<T> &operator*=(Vector<T,dim>& a, const Vector<U,dim>& b){
	a=a * b.cast_to_typeof(a);
	return a;
}
template<typename T>
INLINE Vector<T> &operator*=(Vector<T>& a, vbx_word_t b){
	a=a * b;
	return a;
}
template<typename T>
INLINE Vector<T> &operator*=(Vector<T>& a, enum_t b){
	a=a * b;
	return a;
}


////////
//compound assignment
//////
template<typename T,int dim1,
         typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim2,acc_choice acc>
INLINE Vector<T> &operator&=(Vector<T,dim1>& a, const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& b){
	a=a & b.cast_to_typeof(a);
	return a;
}

template<typename T,typename U,int dim>
INLINE Vector<T> &operator&=(Vector<T,dim>& a, const Vector<U,dim>& b){
	a=a & b.cast_to_typeof(a);
	return a;
}
template<typename T>
INLINE Vector<T> &operator&=(Vector<T>& a, vbx_word_t b){
	a=a & b;
	return a;
}
template<typename T>
INLINE Vector<T> &operator&=(Vector<T>& a, enum_t b){
	a=a & b;
	return a;
}


////////
//compound assignment
//////
template<typename T,int dim1,
         typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim2,acc_choice acc>
INLINE Vector<T> &operator^=(Vector<T,dim1>& a, const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& b){
	a=a ^ b.cast_to_typeof(a);
	return a;
}

template<typename T,typename U,int dim>
INLINE Vector<T> &operator^=(Vector<T,dim>& a, const Vector<U,dim>& b){
	a=a ^ b.cast_to_typeof(a);
	return a;
}
template<typename T>
INLINE Vector<T> &operator^=(Vector<T>& a, vbx_word_t b){
	a=a ^ b;
	return a;
}
template<typename T>
INLINE Vector<T> &operator^=(Vector<T>& a, enum_t b){
	a=a ^ b;
	return a;
}


////////
//compound assignment
//////
template<typename T,int dim1,
         typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim2,acc_choice acc>
INLINE Vector<T> &operator|=(Vector<T,dim1>& a, const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& b){
	a=a | b.cast_to_typeof(a);
	return a;
}

template<typename T,typename U,int dim>
INLINE Vector<T> &operator|=(Vector<T,dim>& a, const Vector<U,dim>& b){
	a=a | b.cast_to_typeof(a);
	return a;
}
template<typename T>
INLINE Vector<T> &operator|=(Vector<T>& a, vbx_word_t b){
	a=a | b;
	return a;
}
template<typename T>
INLINE Vector<T> &operator|=(Vector<T>& a, enum_t b){
	a=a | b;
	return a;
}


////////
//compound assignment
//////
template<typename T,int dim1,
         typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim2,acc_choice acc>
INLINE Vector<T> &operator<<=(Vector<T,dim1>& a, const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& b){
	a=a << b.cast_to_typeof(a);
	return a;
}

template<typename T,typename U,int dim>
INLINE Vector<T> &operator<<=(Vector<T,dim>& a, const Vector<U,dim>& b){
	a=a << b.cast_to_typeof(a);
	return a;
}
template<typename T>
INLINE Vector<T> &operator<<=(Vector<T>& a, vbx_word_t b){
	a=a << b;
	return a;
}
template<typename T>
INLINE Vector<T> &operator<<=(Vector<T>& a, enum_t b){
	a=a << b;
	return a;
}


////////
//compound assignment
//////
template<typename T,int dim1,
         typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim2,acc_choice acc>
INLINE Vector<T> &operator>>=(Vector<T,dim1>& a, const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim2,acc>& b){
	a=a >> b.cast_to_typeof(a);
	return a;
}

template<typename T,typename U,int dim>
INLINE Vector<T> &operator>>=(Vector<T,dim>& a, const Vector<U,dim>& b){
	a=a >> b.cast_to_typeof(a);
	return a;
}
template<typename T>
INLINE Vector<T> &operator>>=(Vector<T>& a, vbx_word_t b){
	a=a >> b;
	return a;
}
template<typename T>
INLINE Vector<T> &operator>>=(Vector<T>& a, enum_t b){
	a=a >> b;
	return a;
}


template<typename T,int dim>
Vector<T,dim> operator!(const Vector<T,dim>& v)
{
	Vector<T,dim> to_ret(v.data,v.size);
	to_ret.cmv=_internal::get_inv_cmv(v.cmv);
	return to_ret;
}
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype,int dim>
_internal::bin_op<lhs_t,rhs_t,_internal::invert_cmv<instr>::instr,btype,dim>
operator!(const _internal::bin_op<lhs_t,rhs_t,instr,btype,dim>& b)
{
	return _internal::bin_op<lhs_t,rhs_t,_internal::invert_cmv<instr>::instr,btype,dim>(b.lhs,b.rhs) ;
}

template<typename T,int dim>
_internal::bin_op<vbx_word_t,Vector<T>,VSUB,T,dim> operator -(const Vector<T,dim>& a){
	return _internal::bin_op<vbx_word_t,Vector<T,dim>,VSUB,T,dim>(0,a);
}
template<typename lhs_t,typename rhs_t,vinstr_t vinstr,typename btype,int dim>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,vinstr,btype,dim>,VSUB,btype,dim>
operator -(const  _internal::bin_op<lhs_t,rhs_t,vinstr,btype,dim>& a){
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,vinstr,btype,dim>,VSUB,btype,dim>(0,a);
}

template<typename T>
void operator++(VBX::Vector<T>& vec){
	vec+=1;
}
template<typename T>
void operator++(VBX::Vector<T>& vec,int){
	vec+=1;
}
template<typename T>
void operator--(VBX::Vector<T>& vec){
	vec-=1;
}
template<typename T>
void operator--(VBX::Vector<T>& vec,int){
	vec-=1;
}
#endif //__operators_hpp__
