#ifndef __ASSIGN_HPP__
#define __ASSIGN_HPP__
#include "expression_width.hpp"
#include <assert.h>
namespace VBX{


	namespace _internal{
struct vector_length{
#ifndef CHECK_VECTOR_LENGTH
			template<typename T>
			static void check(size_t vector_len,const T&){
				return;
			}
#else
			template<typename T>
			static void check(size_t vector_len,const Vector<T>& v){
				if(v.size != vector_len)
					assert("Vector lengths don't match" && 0);
			}
			template<typename lhs_t,typename rhs_t,vinstr_t instr>
			static void check(size_t vector_len,const bin_op<lhs_t,rhs_t,instr>& b){
				check(vector_len,b.lhs);
				check(vector_len,b.rhs);
			}
			template<typename T>
			static void check(size_t vector_len,const inv_cmv<T>& inv){
				check(vector_len,inv.wrapped);
			}
			static void check(size_t vector_len,const enum_t& ){}
			static void check(size_t vector_len,vbx_word_t ){}
			static void check(size_t vector_len,vbx_uword_t ){}
#endif //CHECK_VECTOR_LENGTH
		};

		template<typename dest_t,bool is_masked,bool acc>
		struct assignment{
			//Bin_Op
			template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
			INLINE static void assign(dest_t* dest,const bin_op <lhs_t,rhs_t,instr,btype >& src,size_t vlen)
			{
				typedef typename source_resolve<dest_t,btype>::type src_t;
				vbx_mxp_t *this_mxp = VBX_GET_THIS_MXP();
				src_t* sp=(src_t*)this_mxp->sp;
				typedef typename same_sign_as<dest_t,src_t>::type d_t;
				vbx_func<is_masked,acc>::func(get_arith<instr>::instr,
				                              (d_t*)dest,
				                              resolve<src_t,is_masked>::_resolve(src.lhs,sp,vlen),
				                              resolve<src_t,is_masked>::_resolve(src.rhs,sp,vlen));
			}
			//Vector
			template<typename T>
			INLINE static void assign(dest_t* dest, const Vector<T>& src,size_t vlen){
				typedef typename same_sign_as<dest_t,T>::type d_t;
				vbx_func<is_masked,acc>::func(VMOV,(d_t*)dest,src.data);
			}

			template<typename if_t,typename then_t,typename btype>
			INLINE static void cond_move(dest_t* data, const  if_t& _if_,const then_t& _then_,size_t vlen,btype* a)
			{
				(void)a;
				typedef typename source_resolve<dest_t,btype>::type src_t;

				vbx_mxp_t *this_mxp = VBX_GET_THIS_MXP();
				src_t* sp=(src_t*)this_mxp->sp;
				vinstr_t cmv=get_cmv(_if_);
				typedef typename same_sign_as<dest_t,src_t>::type d_t;
				vbx_func<is_masked,false>::func(cmv,
				                                (d_t*)data,
				                                resolve<src_t,is_masked>::_resolve(_then_,sp,vlen),
				                                resolve<src_t,is_masked>::_resolve(_if_,sp,vlen));
			}
			template<typename lhs_t,typename rhs_t,log_op_t lop,bool negate,typename then_t,typename btype>
			INLINE static void cond_move(dest_t* data, const Logical_vop<lhs_t,rhs_t,lop,negate> _if_
			                             ,const then_t& _then_,size_t vlen,btype* a)
			{
				(void)a;
				typedef typename source_resolve<dest_t,btype>::type src_t;

				vbx_mxp_t *this_mxp = VBX_GET_THIS_MXP();
				src_t* sp=(src_t*)this_mxp->sp;
				vinstr_t cmv=get_cmv(_if_);
				typedef typename same_sign_as<dest_t,src_t>::type d_t;
				vbx_func<is_masked,false>::func(cmv,
				                                (d_t*)data,
				                                resolve<src_t,is_masked>::_resolve(_then_,sp,vlen),
				                                resolve_logical<src_t,is_masked,NONE>::resolve(_if_,sp,vlen));
			}

		};
	}//namespace _internal
}//namespace VBX
#endif //__ASSIGN_HPP__
