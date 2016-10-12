#ifndef __RESOLVE_HPP__
#define __RESOLVE_HPP__
#include "expression_width.hpp"
namespace VBX{
	template<typename T>
	class Vector;

	namespace _internal{

		template<typename dest_t,bool is_masked>
		struct resolve{
			//Vector leaf
			template <typename T>
			INLINE static typename same_sign_as<T,dest_t>::type* _resolve(const Vector<T>& src,dest_t* &sp,size_t vlen)
			{
				typedef typename same_sign_as<T,dest_t>::type t_t;
				return (t_t*)src.data;
			}
			//Scalar Leaf

			INLINE static vbx_word_t _resolve(const vbx_word_t &src,dest_t* &sp,size_t vlen){
				return src;
			}
			//ENUM Leaf
			INLINE static vbx_enum_t* _resolve(VBX:: enum_t src,dest_t* &sp,size_t vlen){
				return (vbx_enum_t*)0;
			}

			//bin_op
			template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
			INLINE static dest_t* _resolve(const bin_op<lhs_t,rhs_t,instr,btype>& src,dest_t* &sp, size_t vlen)
			{
				//find out proper src_t
				typedef typename get_op_size<lhs_t>::type t1;
				typedef typename get_op_size<rhs_t>::type t2;
				typedef typename types_are_equivalent<t1,t2>::type t3;
				typedef typename source_resolve<dest_t,t3>::type src_t;
				//save sp, because we need to restore in case both sources increment it.
				dest_t* dest=sp;
				//if source is smaller than the dest, make sure the dest doesn't overwrite
				//the source
				sp+=(sizeof(src_t)<sizeof(dest_t))*vlen;
				src_t* src_sp=(src_t*)sp;
				typedef typename same_sign_as<dest_t,src_t>::type d_t;
				vbx_func<is_masked,false>::func(get_arith<instr>::instr,
				                                (d_t*)dest,
				                                resolve<src_t,is_masked>::_resolve(src.lhs,src_sp,vlen),
				                                resolve<src_t,is_masked>::_resolve(src.rhs,src_sp,vlen));
				sp=dest+vlen;
				return dest;

			}


		};


	}//_internal
}//VBX

#endif //__RESOLVE_HPP__
