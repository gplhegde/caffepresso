#ifndef VECTOR_MASK_OBJ_HPP
#define VECTOR_MASK_OBJ_HPP

struct vector_mask_obj{
	static int nested;
	bool done_flag;
	VBX::Vector<vbx_byte_t>* saved;

	template<typename T>
	void constructor(const VBX::Vector<T>& msk);

	template<typename T,typename U,vinstr_t vinstr,typename btype>
	vector_mask_obj(const VBX::_internal::bin_op<T,U,vinstr,btype>& msk,int len);

	template<typename T>
	vector_mask_obj(const VBX::Vector<T>& msk,int len);

	template<typename lhs_t,typename rhs_t,VBX::_internal::log_op_t lop,bool negate>
	void log_op_constructor(const VBX::_internal::Logical_vop<lhs_t,rhs_t,lop,negate>& msk,int len);

	template<typename lhs_t,typename rhs_t,VBX::_internal::log_op_t lop,bool negate>
	vector_mask_obj(const VBX::_internal::Logical_vop<lhs_t,rhs_t,lop,negate>& msk,int len);
	//destructor, run at the end of the masked block
	~vector_mask_obj();

};

#endif //VECTOR_MASK_OBJ_HPP
