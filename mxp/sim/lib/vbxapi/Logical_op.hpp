#ifndef LOGICAL_OP_HPP
#define LOGICAL_OP_HPP
#include <assert.h>
#include "convert_vinstr.hpp"
namespace VBX{
	namespace _internal{

		template<log_op_t op>struct invert_logical;
		template<>struct invert_logical<LOGICAL_AND>{const static log_op_t op=LOGICAL_OR;};
		template<>struct invert_logical<LOGICAL_OR >{const static log_op_t op=LOGICAL_AND;};


		template<typename lhs_t,typename rhs_t,log_op_t lop,bool negate>
		struct Logical_vop
		{
			const lhs_t& lhs;
			const rhs_t& rhs;

			Logical_vop(const lhs_t& lhs,const rhs_t& rhs)
				:lhs(lhs),rhs(rhs)
			{
			}

			Logical_vop<lhs_t,rhs_t,lop,!negate>
			operator!(){
				return Logical_vop<lhs_t,rhs_t,lop,!negate>(lhs,rhs);
			}

		};

		template<typename T>
		struct is_allowed_logical_operand;
		template<typename T>
		struct is_allowed_logical_operand<Vector<T> >{};
		template<typename lhs_t,typename rhs_t,typename btype>
		struct is_allowed_logical_operand<VBX::_internal::bin_op<lhs_t,rhs_t,VCMV_NZ ,btype> >{};
		template<typename lhs_t,typename rhs_t,typename btype>
		struct is_allowed_logical_operand<VBX::_internal::bin_op<lhs_t,rhs_t,VCMV_Z  ,btype> >{};
		template<typename lhs_t,typename rhs_t,typename btype>
		struct is_allowed_logical_operand<VBX::_internal::bin_op<lhs_t,rhs_t,VCMV_LTZ,btype> >{};
		template<typename lhs_t,typename rhs_t,typename btype>
		struct is_allowed_logical_operand<VBX::_internal::bin_op<lhs_t,rhs_t,VCMV_GTZ,btype> >{};
		template<typename lhs_t,typename rhs_t,typename btype>
		struct is_allowed_logical_operand<VBX::_internal::bin_op<lhs_t,rhs_t,VCMV_GEZ,btype> >{};
		template<typename lhs_t,typename rhs_t,typename btype>
		struct is_allowed_logical_operand<VBX::_internal::bin_op<lhs_t,rhs_t,VCMV_LEZ,btype> >{};
		template<typename lhs_t,typename rhs_t,log_op_t lop,bool negate>
		struct is_allowed_logical_operand<Logical_vop<lhs_t,rhs_t,lop,negate> >{};

		template<typename lhs_t,typename rhs_t>
		Logical_vop<lhs_t,rhs_t,LOGICAL_AND> operator&&(const lhs_t& lhs,const rhs_t& rhs){
			is_allowed_logical_operand<lhs_t>();
			is_allowed_logical_operand<rhs_t>();
			return Logical_vop<lhs_t,rhs_t,LOGICAL_AND>(lhs,rhs);
		}

		template<typename lhs_t,typename rhs_t>
		Logical_vop<lhs_t,rhs_t,LOGICAL_OR> operator||(const lhs_t& lhs,const rhs_t& rhs){
			is_allowed_logical_operand<lhs_t>();
			is_allowed_logical_operand<rhs_t>();
			return Logical_vop<lhs_t,rhs_t,LOGICAL_OR>(lhs,rhs);
		}

		//is_all_and allows us to do narrowing maskes rather than conditional moves
		//on mask_narrow( a && b)
		template<typename T>
		struct is_all_and<Vector<T> >{
			static const bool result=true;
		};
		template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
		struct is_all_and<bin_op<lhs_t,rhs_t,instr,btype> >{
			static const bool result=true;
		};
		template<typename lhs_t,typename rhs_t,log_op_t lop,bool negate>
		struct is_all_and<Logical_vop<lhs_t,rhs_t,lop,negate> >//TODO capture negated OR
		{
			static const bool result = (lop == LOGICAL_AND && !negate &&
			                            is_all_and<lhs_t>::result &&
			                            is_all_and<rhs_t>::result );
		};



		template<typename dest_t,bool is_masked,log_op_t parent_op=NONE>
		struct resolve_logical
		{
			//resolve negated logical op
			template<typename lhs_t,typename rhs_t,log_op_t lop>
			static dest_t* resolve(const Logical_vop<lhs_t,rhs_t,lop,true>& lvo,dest_t* &sp,int vlen){
				//demorgans law !(A | B) <-> (!A & !B)
				//              !(A & B) <-> (!A | !B)
				typedef typeof(!lvo.lhs) n_lhs_t;
				typedef typeof(!(lvo.rhs)) n_rhs_t;

				return resolve_logical<dest_t,is_masked,parent_op>
					::resolve(
					          Logical_vop<n_lhs_t,n_rhs_t,invert_logical<lop>::op> (!lvo.lhs,!lvo.rhs),
					          sp,vlen );
			}
			template<typename lhs_t,typename rhs_t,log_op_t lop>
			static dest_t* resolve(const Logical_vop<lhs_t,rhs_t,lop,false>& lvo,dest_t* &sp,int vlen)
			{
				typedef typename word_sized<dest_t>::type word_t;
				if(parent_op == NONE){
					vbxx(VMOV,
					     sp,
					     (word_t)(lop==LOGICAL_OR? 0:1));
					resolve_logical<dest_t,is_masked,lop>::resolve(lvo,sp,vlen);
					dest_t* dest=sp;
					sp+=vlen;
					return dest;
				}else if (parent_op == lop) {
					resolve_logical<dest_t,is_masked,lop>::resolve(lvo.lhs,sp,vlen);
					resolve_logical<dest_t,is_masked,lop>::resolve(lvo.rhs,sp,vlen);
					return NULL;
				} else {
					//parent != lop, resolve this into a 1 or zero. then merge with sp
					dest_t* new_sp=sp+vlen;
					new_sp=resolve_logical<dest_t,is_masked,NONE>::resolve(lvo,new_sp,vlen);
					//AND: set sp to zero if new_sp is ZERO
					//OR : set sp to one  if new_sp is Not ZERO
					vbxx(lop==LOGICAL_OR?VCMV_Z:VCMV_NZ,
					     sp,
					     (word_t)(lop==LOGICAL_OR?0:1),
					     new_sp);
					return NULL;
				}
			}

			template<typename lhs_t,typename rhs_t,vinstr_t vinstr,typename btype>
			static dest_t* resolve(const bin_op<lhs_t,rhs_t,vinstr,btype>& src,dest_t* &sp,int vlen){
				typedef typename source_resolve<dest_t,btype>::type src_t;
				src_t* src_sp=sp+vlen;
				vbx_word_t val;
				vinstr_t instr;
				if(parent_op==LOGICAL_OR){
					val=1;
					instr=get_cmv(src);
				}
				else if(parent_op==LOGICAL_AND){
					val=0;
					instr=get_inv_cmv(get_cmv(src));
				}
				else{
					assert("SHOULD NOT BE HERE"&&0);
				}
				src_t* a=_internal::resolve<src_t,is_masked>::_resolve(src,src_sp,vlen);
				vbxx(instr,
				     sp,
				     val,
				     a);
				return sp;
			}
		};

	}//namespace internal
}//namespace VBX
#endif //LOGICAL_OP_HPP
