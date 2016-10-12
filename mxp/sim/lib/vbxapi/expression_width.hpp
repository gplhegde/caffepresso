#ifndef EXPRESSION_WIDTH_HPP
#define EXPRESSION_WIDTH_HPP
template<typename T>
struct error_with_type;
template<typename T>
struct expression_width;
template<>
struct expression_width<long>{typedef vbx_byte_t type;}; //should not make width larger than necessary
template<>
struct expression_width<unsigned long>{typedef vbx_ubyte_t type;}; //should not make width larger than necessary
template<>
struct expression_width<int>{typedef vbx_byte_t type;}; //should not make width larger than necessary
template<>
struct expression_width<unsigned int>{typedef vbx_ubyte_t type;}; //should not make width larger than necessary
template<>
struct expression_width<short>{typedef vbx_byte_t type;}; //should not make width larger than necessary
template<>
struct expression_width<unsigned short>{typedef vbx_ubyte_t type;}; //should not make width larger than necessary
template<>
struct expression_width<signed char>{typedef vbx_byte_t type;}; //should not make width larger than necessary
template<>
struct expression_width<unsigned char>{typedef vbx_ubyte_t type;}; //should not make width larger than necessary
template<>
struct expression_width<VBX::enum_t>{typedef vbx_ubyte_t type;}; //should not make width larger than necessary
template<typename lhs_t,typename rhs_t,VBX::_internal::log_op_t lop,bool negate>
struct expression_width<VBX::_internal::Logical_vop<lhs_t,rhs_t,lop,negate> >{typedef vbx_ubyte_t type;}; //should not make width larger than necessary

template<typename T>
struct expression_width<VBX::Vector<T> >{typedef T type;};
template<typename lhs_t, typename rhs_t,vinstr_t instr,typename btype>
struct expression_width<VBX::_internal::bin_op<lhs_t,rhs_t,instr,btype> >{
private:
	typedef typename expression_width<lhs_t>::type t1;
	typedef typename expression_width<rhs_t>::type t2;
public:
	typedef typename biggest<t1,t2>::type type;
};

template<typename T>
struct expression_width<const T>{
	typedef typename expression_width<T>::type type;
};

//source resolve

template <typename D_t,typename B_t>
struct source_resolve{typedef B_t type;};
template <typename D_t>
struct source_resolve<D_t,vbx_enum_t>{typedef D_t type;};

template<typename T>
struct get_op_size{typedef vbx_enum_t type;};
template<typename lhs_t, typename rhs_t,vinstr_t instr,typename btype>
struct get_op_size<VBX::_internal::bin_op<lhs_t,rhs_t,instr,btype> >{typedef btype type;};
template<typename T>
struct get_op_size<VBX::Vector<T> >{typedef T type;};
template<typename lhs_t,typename rhs_t,VBX::_internal::log_op_t lop,bool negate>
struct get_op_size<VBX::_internal::Logical_vop<lhs_t,rhs_t,lop,negate> >{typedef vbx_enum_t type;};

#endif //EXPRESSION_WIDTH_HPP
