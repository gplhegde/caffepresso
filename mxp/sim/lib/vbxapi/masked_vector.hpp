#ifndef MASKED_VECTOR_HPP
#define MASKED_VECTOR_HPP

#include "Vector.hpp"
template<typename T,typename U,vinstr_t vinstr,typename btype>
inline void Vector_mask_narrow(const VBX::_internal::bin_op<T,U,vinstr,btype>& msk,int len)
{

	VBX::Vector<btype> cmp(len);
	cmp = msk;
	vbx_setup_mask_masked(cmp.cmv,cmp.data);
}
template<typename T>
inline void Vector_mask_narrow(const VBX::Vector<T>& msk,int len)
{
	vbx_setup_mask_masked(msk.cmv,msk.data);
}

template<typename lhs_t,typename rhs_t,VBX::_internal::log_op_t lop,bool negate>
inline void Vector_mask_narrow(const VBX::_internal::Logical_vop<lhs_t,rhs_t,lop,negate>& msk,int len)
{
	//since all of logical operators are &&, we can progressively narrow the mask
	if(VBX::_internal::is_all_and<VBX::_internal::Logical_vop<lhs_t,rhs_t,lop,negate> >::result){
		Vector_mask_narrow(msk.lhs,len);
		Vector_mask_narrow(msk.rhs,len);
	}else{
		VBX::Vector<char> cmp(len);
		cmp=msk;
		vbx_setup_mask_masked(cmp.cmv,cmp.data);
	}
}
#define Vector_mask( comp,len ) for( vector_mask_obj v((comp),(len)); !v.done_flag;v.done_flag=true)
#define Vector_mask_loop( comp,len,while_cond ) for( vector_mask_obj v((comp),(len)); (Vector_mask_narrow(comp,len),while_cond);)

//usage :
// Vector_mask( a<b,len){
//  ...
//  masked calculations
//  ...
// }

template<typename T>
inline void vector_mask_obj::constructor(const VBX::Vector<T>& msk){
	vbx_setup_mask(msk.cmv,msk.data);
	vector_mask_obj::nested++;
	done_flag=false;
}
template<typename T,typename U,vinstr_t vinstr,typename btype>
inline vector_mask_obj::vector_mask_obj(const VBX::_internal::bin_op<T,U,vinstr,btype>& msk,int len)
{
	VBX::Vector<btype> cmp(len);
	cmp = msk;
	constructor(cmp);
}
template<typename T>
inline vector_mask_obj::vector_mask_obj(const VBX::Vector<T>& msk,int len)
{
	vbx_set_vl(len);
	constructor(msk);
}

template<typename lhs_t,typename rhs_t,VBX::_internal::log_op_t lop,bool negate>
inline void vector_mask_obj::log_op_constructor(const VBX::_internal::Logical_vop<lhs_t,rhs_t,lop,negate>& msk,int len)
{
	if(VBX::_internal::is_all_and<VBX::_internal::Logical_vop<lhs_t,rhs_t,lop,negate> >::result){
		log_op_constructor(msk.lhs,len);
		Vector_mask_narrow(msk.rhs,len);
	}else{
		VBX::Vector<char> cmp(len);
		cmp=msk;
		constructor(cmp);
	}
}

template<typename lhs_t,typename rhs_t,VBX::_internal::log_op_t lop,bool negate>
inline vector_mask_obj::vector_mask_obj(const VBX::_internal::Logical_vop<lhs_t,rhs_t,
                                                              lop,negate>& msk,int len)
{
	log_op_constructor(msk,len);
}
//destructor, run at the end of the masked block
inline vector_mask_obj::~vector_mask_obj()
{
	nested--;
}



#endif //MASKED_VECTOR_HPP
