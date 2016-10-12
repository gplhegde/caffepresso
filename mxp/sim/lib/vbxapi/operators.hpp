#ifndef __operators_hpp__
#define __operators_hpp__

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VADD,type>
INLINE operator+ (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VADD,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VADD,T>
INLINE operator+ (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VADD,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VADD,T>
INLINE operator+ (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VADD,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VADD,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator+ (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VADD,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VADD,btype>
INLINE operator+ ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VADD,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VADD,T>
INLINE operator+ (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VADD,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VADD,T>
INLINE operator+ (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator+(rhs, lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VADD,T>
 operator+ (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VADD,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VADD,btype>
INLINE operator+ (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return operator+(rhs, lhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VADD,T>
INLINE operator+ (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VADD,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VADD,vbx_enum_t>
INLINE operator+ (const enum_t &lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VADD,vbx_enum_t>(rhs,lhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VADD,T>
INLINE operator+ (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VADD,T>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VADD,type>
INLINE operator+ (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VADD,type>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VADD,vbx_enum_t>
INLINE operator+ (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VADD,vbx_enum_t>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VSUB,type>
INLINE operator- (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VSUB,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSUB,T>
INLINE operator- (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSUB,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VSUB,T>
INLINE operator- (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VSUB,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VSUB,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator- (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VSUB,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VSUB,T>
INLINE operator- (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VSUB,T>(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VSUB,btype>
INLINE operator- ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VSUB,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VSUB,T>
INLINE operator- (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VSUB,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VADD,T>
INLINE operator- (const Vector<T>& lhs, vbx_word_t rhs)
{
	return (-rhs) + lhs;
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VSUB,vbx_enum_t>
INLINE operator- (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VSUB,vbx_enum_t>(lhs,rhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VSUB,T>
 operator- (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VSUB,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VADD,btype>
INLINE operator- (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return (-rhs) + lhs;
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VSUB,type>
INLINE operator- (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VSUB,type>(lhs,rhs);
}

//EV
//TODO: implement a Vector subtracted from an ENUM

//ES
_internal::bin_op<vbx_word_t,enum_t,VADD,vbx_enum_t>
INLINE operator- (const enum_t &lhs,const vbx_word_t& rhs)
{
	return (-rhs) + lhs;
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VMUL,type>
INLINE operator* (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VMUL,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VMUL,T>
INLINE operator* (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VMUL,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VMUL,T>
INLINE operator* (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VMUL,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VMUL,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator* (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VMUL,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VMUL,btype>
INLINE operator* ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VMUL,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VMUL,T>
INLINE operator* (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VMUL,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VMUL,T>
INLINE operator* (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator*(rhs, lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VMUL,T>
 operator* (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VMUL,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VMUL,btype>
INLINE operator* (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return operator*(rhs, lhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VMUL,T>
INLINE operator* (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VMUL,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VMUL,vbx_enum_t>
INLINE operator* (const enum_t &lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VMUL,vbx_enum_t>(rhs,lhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VMUL,T>
INLINE operator* (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VMUL,T>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VMUL,type>
INLINE operator* (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VMUL,type>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VMUL,vbx_enum_t>
INLINE operator* (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VMUL,vbx_enum_t>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VAND,type>
INLINE operator& (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VAND,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VAND,T>
INLINE operator& (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VAND,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VAND,T>
INLINE operator& (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VAND,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VAND,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator& (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VAND,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VAND,btype>
INLINE operator& ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VAND,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VAND,T>
INLINE operator& (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VAND,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VAND,T>
INLINE operator& (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator&(rhs, lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VAND,T>
 operator& (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VAND,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VAND,btype>
INLINE operator& (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return operator&(rhs, lhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VAND,T>
INLINE operator& (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VAND,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VAND,vbx_enum_t>
INLINE operator& (const enum_t &lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VAND,vbx_enum_t>(rhs,lhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VAND,T>
INLINE operator& (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VAND,T>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VAND,type>
INLINE operator& (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VAND,type>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VAND,vbx_enum_t>
INLINE operator& (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VAND,vbx_enum_t>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VXOR,type>
INLINE operator^ (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VXOR,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VXOR,T>
INLINE operator^ (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VXOR,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VXOR,T>
INLINE operator^ (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VXOR,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VXOR,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator^ (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VXOR,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VXOR,btype>
INLINE operator^ ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VXOR,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VXOR,T>
INLINE operator^ (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VXOR,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VXOR,T>
INLINE operator^ (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator^(rhs, lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VXOR,T>
 operator^ (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VXOR,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VXOR,btype>
INLINE operator^ (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return operator^(rhs, lhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VXOR,T>
INLINE operator^ (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VXOR,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VXOR,vbx_enum_t>
INLINE operator^ (const enum_t &lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VXOR,vbx_enum_t>(rhs,lhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VXOR,T>
INLINE operator^ (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VXOR,T>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VXOR,type>
INLINE operator^ (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VXOR,type>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VXOR,vbx_enum_t>
INLINE operator^ (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VXOR,vbx_enum_t>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VOR,type>
INLINE operator| (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VOR,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VOR,T>
INLINE operator| (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VOR,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VOR,T>
INLINE operator| (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VOR,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VOR,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator| (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VOR,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VOR,btype>
INLINE operator| ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VOR,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VOR,T>
INLINE operator| (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VOR,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VOR,T>
INLINE operator| (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return operator|(rhs, lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VOR,T>
 operator| (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VOR,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VOR,btype>
INLINE operator| (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return operator|(rhs, lhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VOR,T>
INLINE operator| (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VOR,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VOR,vbx_enum_t>
INLINE operator| (const enum_t &lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VOR,vbx_enum_t>(rhs,lhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VOR,T>
INLINE operator| (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VOR,T>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VOR,type>
INLINE operator| (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VOR,type>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VOR,vbx_enum_t>
INLINE operator| (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VOR,vbx_enum_t>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VABSDIFF,type>
INLINE absdiff (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VABSDIFF,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VABSDIFF,T>
INLINE absdiff (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VABSDIFF,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VABSDIFF,T>
INLINE absdiff (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VABSDIFF,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VABSDIFF,
	          typename types_are_equivalent<type1,type2>::type >
INLINE absdiff (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VABSDIFF,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VABSDIFF,btype>
INLINE absdiff ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VABSDIFF,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VABSDIFF,T>
INLINE absdiff (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VABSDIFF,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VABSDIFF,T>
INLINE absdiff (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return absdiff(rhs, lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VABSDIFF,T>
 absdiff (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VABSDIFF,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VABSDIFF,btype>
INLINE absdiff (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return absdiff(rhs, lhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VABSDIFF,T>
INLINE absdiff (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VABSDIFF,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VABSDIFF,vbx_enum_t>
INLINE absdiff (const enum_t &lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VABSDIFF,vbx_enum_t>(rhs,lhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VABSDIFF,T>
INLINE absdiff (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VABSDIFF,T>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VABSDIFF,type>
INLINE absdiff (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VABSDIFF,type>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VABSDIFF,vbx_enum_t>
INLINE absdiff (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VABSDIFF,vbx_enum_t>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VMULFXP,type>
INLINE mulfxp (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VMULFXP,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VMULFXP,T>
INLINE mulfxp (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VMULFXP,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VMULFXP,T>
INLINE mulfxp (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VMULFXP,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VMULFXP,
	          typename types_are_equivalent<type1,type2>::type >
INLINE mulfxp (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VMULFXP,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VMULFXP,btype>
INLINE mulfxp ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VMULFXP,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VMULFXP,T>
INLINE mulfxp (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VMULFXP,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VMULFXP,T>
INLINE mulfxp (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return mulfxp(rhs, lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VMULFXP,T>
 mulfxp (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VMULFXP,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VMULFXP,btype>
INLINE mulfxp (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return mulfxp(rhs, lhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VMULFXP,T>
INLINE mulfxp (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VMULFXP,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VMULFXP,vbx_enum_t>
INLINE mulfxp (const enum_t &lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VMULFXP,vbx_enum_t>(rhs,lhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VMULFXP,T>
INLINE mulfxp (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VMULFXP,T>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VMULFXP,type>
INLINE mulfxp (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VMULFXP,type>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VMULFXP,vbx_enum_t>
INLINE mulfxp (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VMULFXP,vbx_enum_t>(lhs,rhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VMULHI,type>
INLINE mulhi (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VMULHI,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VMULHI,T>
INLINE mulhi (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VMULHI,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VMULHI,T>
INLINE mulhi (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VMULHI,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VMULHI,
	          typename types_are_equivalent<type1,type2>::type >
INLINE mulhi (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VMULHI,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VMULHI,btype>
INLINE mulhi ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VMULHI,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VMULHI,T>
INLINE mulhi (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VMULHI,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VMULHI,T>
INLINE mulhi (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return mulhi(rhs, lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VMULHI,T>
 mulhi (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VMULHI,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VMULHI,btype>
INLINE mulhi (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return mulhi(rhs, lhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VMULHI,T>
INLINE mulhi (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VMULHI,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VMULHI,vbx_enum_t>
INLINE mulhi (const enum_t &lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VMULHI,vbx_enum_t>(rhs,lhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VMULHI,T>
INLINE mulhi (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VMULHI,T>(lhs,rhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VMULHI,type>
INLINE mulhi (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VMULHI,type>(lhs,rhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VMULHI,vbx_enum_t>
INLINE mulhi (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VMULHI,vbx_enum_t>(lhs,rhs);
}

//BE
//TODO: add binary_op shifted by Enum

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VSHL,T>
INLINE operator<< (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VSHL,T>(rhs,lhs);
}

//VE
//TODO implement Vector shifted by enum

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2 >
_internal::bin_op<_internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type1>,
       _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type2>,VSHL,typename types_are_equivalent<type1,type2>::type >
INLINE operator<< (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type2>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type1>& rhs)
{
	return _internal::bin_op<_internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,VSHL,
	                         typename types_are_equivalent<type1,type2>::type >(rhs,lhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSHL,T>
INLINE operator<< (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSHL,T>(rhs,lhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VSHL,T>
INLINE operator<< (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VSHL,T>(rhs,lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VSHL,T>
 operator<< (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VSHL,T>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSHL,btype>
INLINE operator<< (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSHL,btype>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VSHL,vbx_enum_t>
INLINE operator<< (const enum_t& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VSHL,vbx_enum_t>(rhs,lhs);
}
//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VSHL,T>
INLINE operator<< (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VSHL,T>(rhs,lhs);
}

//SV
//TODO: add scalar shifted by vector

//SB
//TODO: add scalar shifted by _internal::bin_op

//BE
//TODO: add binary_op shifted by Enum

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VSHR,T>
INLINE operator>> (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VSHR,T>(rhs,lhs);
}

//VE
//TODO implement Vector shifted by enum

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2 >
_internal::bin_op<_internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type1>,
       _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type2>,VSHR,typename types_are_equivalent<type1,type2>::type >
INLINE operator>> (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type2>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type1>& rhs)
{
	return _internal::bin_op<_internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,VSHR,
	                         typename types_are_equivalent<type1,type2>::type >(rhs,lhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSHR,T>
INLINE operator>> (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSHR,T>(rhs,lhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VSHR,T>
INLINE operator>> (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VSHR,T>(rhs,lhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VSHR,T>
 operator>> (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VSHR,T>(rhs,lhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSHR,btype>
INLINE operator>> (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VSHR,btype>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VSHR,vbx_enum_t>
INLINE operator>> (const enum_t& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VSHR,vbx_enum_t>(rhs,lhs);
}
//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VSHR,T>
INLINE operator>> (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VSHR,T>(rhs,lhs);
}

//SV
//TODO: add scalar shifted by vector

//SB
//TODO: add scalar shifted by _internal::bin_op

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_LTZ,type>
INLINE operator< (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_LTZ,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_LTZ,T>
INLINE operator< (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_LTZ,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_LTZ,T>
INLINE operator< (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_LTZ,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VCMV_LTZ,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator< (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VCMV_LTZ,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_LTZ,T>
INLINE operator< (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_LTZ,T>(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_LTZ,btype>
INLINE operator< ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_LTZ,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_LTZ,T>
INLINE operator< (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_LTZ,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_GTZ,T>
INLINE operator< (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_GTZ,T>(rhs,lhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_LTZ,vbx_enum_t>
INLINE operator< (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_LTZ,vbx_enum_t>(lhs,rhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VCMV_LTZ,T>
 operator< (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VCMV_LTZ,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_GTZ,btype>
INLINE operator< (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_GTZ,btype>(rhs,lhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_LTZ,type>
INLINE operator< (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_LTZ,type>(lhs,rhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_GTZ,T>
INLINE operator< (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_GTZ,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_GTZ,vbx_enum_t>
INLINE operator< (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_GTZ,vbx_enum_t>(rhs,lhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_GTZ,type>
INLINE operator> (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_GTZ,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_GTZ,T>
INLINE operator> (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_GTZ,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_GTZ,T>
INLINE operator> (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_GTZ,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VCMV_GTZ,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator> (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VCMV_GTZ,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_GTZ,T>
INLINE operator> (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_GTZ,T>(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_GTZ,btype>
INLINE operator> ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_GTZ,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_GTZ,T>
INLINE operator> (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_GTZ,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_LTZ,T>
INLINE operator> (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_LTZ,T>(rhs,lhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_GTZ,vbx_enum_t>
INLINE operator> (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_GTZ,vbx_enum_t>(lhs,rhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VCMV_GTZ,T>
 operator> (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VCMV_GTZ,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_LTZ,btype>
INLINE operator> (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_LTZ,btype>(rhs,lhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_GTZ,type>
INLINE operator> (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_GTZ,type>(lhs,rhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_LTZ,T>
INLINE operator> (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_LTZ,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_LTZ,vbx_enum_t>
INLINE operator> (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_LTZ,vbx_enum_t>(rhs,lhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_LEZ,type>
INLINE operator<= (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_LEZ,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_LEZ,T>
INLINE operator<= (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_LEZ,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_LEZ,T>
INLINE operator<= (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_LEZ,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VCMV_LEZ,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator<= (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VCMV_LEZ,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_LEZ,T>
INLINE operator<= (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_LEZ,T>(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_LEZ,btype>
INLINE operator<= ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_LEZ,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_LEZ,T>
INLINE operator<= (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_LEZ,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_GEZ,T>
INLINE operator<= (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_GEZ,T>(rhs,lhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_LEZ,vbx_enum_t>
INLINE operator<= (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_LEZ,vbx_enum_t>(lhs,rhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VCMV_LEZ,T>
 operator<= (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VCMV_LEZ,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_GEZ,btype>
INLINE operator<= (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_GEZ,btype>(rhs,lhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_LEZ,type>
INLINE operator<= (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_LEZ,type>(lhs,rhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_GEZ,T>
INLINE operator<= (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_GEZ,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_GEZ,vbx_enum_t>
INLINE operator<= (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_GEZ,vbx_enum_t>(rhs,lhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_GEZ,type>
INLINE operator>= (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_GEZ,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_GEZ,T>
INLINE operator>= (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_GEZ,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_GEZ,T>
INLINE operator>= (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_GEZ,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VCMV_GEZ,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator>= (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VCMV_GEZ,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_GEZ,T>
INLINE operator>= (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_GEZ,T>(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_GEZ,btype>
INLINE operator>= ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_GEZ,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_GEZ,T>
INLINE operator>= (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_GEZ,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_LEZ,T>
INLINE operator>= (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_LEZ,T>(rhs,lhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_GEZ,vbx_enum_t>
INLINE operator>= (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_GEZ,vbx_enum_t>(lhs,rhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VCMV_GEZ,T>
 operator>= (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VCMV_GEZ,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_LEZ,btype>
INLINE operator>= (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_LEZ,btype>(rhs,lhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_GEZ,type>
INLINE operator>= (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_GEZ,type>(lhs,rhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_LEZ,T>
INLINE operator>= (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_LEZ,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_LEZ,vbx_enum_t>
INLINE operator>= (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_LEZ,vbx_enum_t>(rhs,lhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_Z,type>
INLINE operator== (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_Z,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_Z,T>
INLINE operator== (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_Z,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_Z,T>
INLINE operator== (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_Z,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VCMV_Z,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator== (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VCMV_Z,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_Z,T>
INLINE operator== (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_Z,T>(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_Z,btype>
INLINE operator== ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_Z,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_Z,T>
INLINE operator== (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_Z,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_Z,T>
INLINE operator== (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_Z,T>(rhs,lhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_Z,vbx_enum_t>
INLINE operator== (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_Z,vbx_enum_t>(lhs,rhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VCMV_Z,T>
 operator== (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VCMV_Z,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_Z,btype>
INLINE operator== (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_Z,btype>(rhs,lhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_Z,type>
INLINE operator== (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_Z,type>(lhs,rhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_Z,T>
INLINE operator== (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_Z,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_Z,vbx_enum_t>
INLINE operator== (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_Z,vbx_enum_t>(rhs,lhs);
}

//BE
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_NZ,type>
INLINE operator!= (const _internal::bin_op<lhs_t,rhs_t,instr,type>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,type>,enum_t,VCMV_NZ,type>(lhs,rhs);
}

//VB
template<typename T,typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_NZ,T>
INLINE operator!= (const Vector<T>& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<Vector<T>,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_NZ,T>(lhs,rhs);
}

//VE
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_NZ,T>
INLINE operator!= (const Vector<T>& lhs,const enum_t& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_NZ,T>(lhs,rhs);
}

//BB
template<typename lhs_lhs_t,typename lhs_rhs_t,vinstr_t lhs_instr,typename type1,
         typename rhs_lhs_t,typename rhs_rhs_t,vinstr_t rhs_instr,typename type2>
_internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	          _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,VCMV_NZ,
	          typename types_are_equivalent<type1,type2>::type >
INLINE operator!= (const _internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>& lhs,
                    const _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_lhs_t,lhs_rhs_t,lhs_instr,type1>,
	                         _internal::bin_op<rhs_lhs_t,rhs_rhs_t,rhs_instr,type2>,
	                         VCMV_NZ,typename types_are_equivalent<type1,type2>::type >(lhs,rhs);
}

//SV
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_NZ,T>
INLINE operator!= (const vbx_word_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_NZ,T>(lhs,rhs);
}

//EB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_NZ,btype>
INLINE operator!= ( const enum_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,btype>& rhs)
{
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,enum_t,VCMV_NZ,btype>(lhs,rhs);
}

//BV
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename T,typename btype>
_internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_NZ,T>
INLINE operator!= (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const Vector<T>& rhs)
{
	types_are_equivalent<T,btype>();
	return _internal::bin_op<_internal::bin_op<lhs_t,rhs_t,instr,btype>,Vector<T>,VCMV_NZ,T>(lhs,rhs);
}

//VS
template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VCMV_NZ,T>
INLINE operator!= (const Vector<T>& lhs,const typename word_sized<T>::type& rhs)
{
	return _internal::bin_op<vbx_word_t,Vector<T>,VCMV_NZ,T>(rhs,lhs);
}

//SE
_internal::bin_op<vbx_word_t,enum_t,VCMV_NZ,vbx_enum_t>
INLINE operator!= (const vbx_word_t& lhs,const enum_t &rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_NZ,vbx_enum_t>(lhs,rhs);
}

//VV
template <typename T,typename U>
INLINE _internal::bin_op<Vector<T>,Vector<U>,VCMV_NZ,T>
 operator!= (const Vector<T>& lhs,const Vector<U>& rhs)
{
	types_are_equivalent<T,U>();
	return _internal::bin_op<Vector<T>,Vector<T>,VCMV_NZ,T>(lhs,rhs);
}

//BS
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_NZ,btype>
INLINE operator!= (const _internal::bin_op<lhs_t,rhs_t,instr,btype>& lhs,const vbx_word_t& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,btype>,VCMV_NZ,btype>(rhs,lhs);
}

//SB
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename type>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_NZ,type>
INLINE operator!= (const vbx_word_t& lhs,const _internal::bin_op<lhs_t,rhs_t,instr,type>& rhs)
{
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,instr,type>,VCMV_NZ,type>(lhs,rhs);
}

//EV
template<typename T>
_internal::bin_op<Vector<T>,enum_t,VCMV_NZ,T>
INLINE operator!= (const enum_t& lhs,const Vector<T>& rhs)
{
	return _internal::bin_op<Vector<T>,enum_t,VCMV_NZ,T>(rhs,lhs);
}

//ES
_internal::bin_op<vbx_word_t,enum_t,VCMV_NZ,vbx_enum_t>
INLINE operator!= (const enum_t& lhs,vbx_word_t rhs)
{
	return _internal::bin_op<vbx_word_t,enum_t,VCMV_NZ,vbx_enum_t>(rhs,lhs);
}

template<typename T,typename U>
INLINE Vector<T> &operator+=(Vector<T>& a, const U& b){
	a=a + b;
	return a;
}

template<typename T,typename U>
INLINE Vector<T> &operator-=(Vector<T>& a, const U& b){
	a=a - b;
	return a;
}

template<typename T,typename U>
INLINE Vector<T> &operator*=(Vector<T>& a, const U& b){
	a=a * b;
	return a;
}

template<typename T,typename U>
INLINE Vector<T> &operator&=(Vector<T>& a, const U& b){
	a=a & b;
	return a;
}

template<typename T,typename U>
INLINE Vector<T> &operator^=(Vector<T>& a, const U& b){
	a=a ^ b;
	return a;
}

template<typename T,typename U>
INLINE Vector<T> &operator|=(Vector<T>& a, const U& b){
	a=a | b;
	return a;
}

template<typename T,typename U>
INLINE Vector<T> &operator<<=(Vector<T>& a, const U& b){
	a=a << b;
	return a;
}

template<typename T,typename U>
INLINE Vector<T> &operator>>=(Vector<T>& a, const U& b){
	a=a >> b;
	return a;
}

template<typename T>
Vector<T> operator!(const Vector<T>& v)
{
	Vector<T> to_ret(v.data,v.size);
	to_ret.cmv=_internal::get_inv_cmv(v.cmv);
	return to_ret;
}
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
_internal::bin_op<lhs_t,rhs_t,_internal::invert_cmv<instr>::instr,btype>
operator!(const _internal::bin_op<lhs_t,rhs_t,instr,btype>& b)
{
	return _internal::bin_op<lhs_t,rhs_t,_internal::invert_cmv<instr>::instr,btype>(b.lhs,b.rhs) ;
}

template<typename T>
_internal::bin_op<vbx_word_t,Vector<T>,VSUB,T> operator -(const Vector<T>& a){
	return _internal::bin_op<vbx_word_t,Vector<T>,VSUB,T>(0,a);
}
template<typename lhs_t,typename rhs_t,vinstr_t vinstr,typename btype>
_internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,vinstr,btype>,VSUB,btype>
operator -(const  _internal::bin_op<lhs_t,rhs_t,vinstr,btype>& a){
	return _internal::bin_op<vbx_word_t,_internal::bin_op<lhs_t,rhs_t,vinstr,btype>,VSUB,btype>(0,a);
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
