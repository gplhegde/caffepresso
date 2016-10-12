#ifndef __CONVERT_VINSTR_HPP__
#define __CONVERT_VINSTR_HPP__
#include <assert.h>
template<vinstr_t _instr>struct get_arith{ static const vinstr_t instr=_instr;};
template<> struct get_arith<VCMV_LTZ>{static const vinstr_t instr=VSUB;};
template<> struct get_arith<VCMV_GTZ>{static const vinstr_t instr=VSUB;};
template<> struct get_arith<VCMV_LEZ>{static const vinstr_t instr=VSUB;};
template<> struct get_arith<VCMV_GEZ>{static const vinstr_t instr=VSUB;};
template<> struct get_arith<VCMV_Z  >{static const vinstr_t instr=VSUB;};
template<> struct get_arith<VCMV_NZ >{static const vinstr_t instr=VSUB;};

template<vinstr_t _instr>struct get_cmv_t{ static const vinstr_t instr=VCMV_NZ;};
template<> struct get_cmv_t<VCMV_LTZ>{static const vinstr_t instr=VCMV_LTZ;};
template<> struct get_cmv_t<VCMV_GTZ>{static const vinstr_t instr=VCMV_GTZ;};
template<> struct get_cmv_t<VCMV_LEZ>{static const vinstr_t instr=VCMV_LEZ;};
template<> struct get_cmv_t<VCMV_GEZ>{static const vinstr_t instr=VCMV_GEZ;};
template<> struct get_cmv_t<VCMV_Z  >{static const vinstr_t instr=VCMV_Z  ;};
template<> struct get_cmv_t<VCMV_NZ >{static const vinstr_t instr=VCMV_NZ ;};

template<vinstr_t _instr> struct invert_cmv{static const vinstr_t instr=VCMV_Z;};
template<> struct invert_cmv<VCMV_LTZ>{static const vinstr_t instr=VCMV_GEZ;};
template<> struct invert_cmv<VCMV_GTZ>{static const vinstr_t instr=VCMV_LEZ;};
template<> struct invert_cmv<VCMV_LEZ>{static const vinstr_t instr=VCMV_GTZ;};
template<> struct invert_cmv<VCMV_GEZ>{static const vinstr_t instr=VCMV_LTZ;};
template<> struct invert_cmv<VCMV_Z>{static const vinstr_t instr=VCMV_NZ;};
template<> struct invert_cmv<VCMV_NZ>{static const vinstr_t instr=VCMV_Z;};

template<typename T>
inline vinstr_t get_cmv(const VBX::Vector<T>& v ){
	return v.cmv;
}
template<typename lhs_t,typename rhs_t,vinstr_t instr,typename btype>
inline vinstr_t get_cmv(const bin_op<lhs_t,rhs_t,instr,btype>& ){
	return get_cmv_t<instr>::instr;
}
template<typename lhs_t,typename rhs_t,_internal::log_op_t lop,bool negate>
inline vinstr_t get_cmv(const Logical_vop<lhs_t,rhs_t,lop,negate>& lvo ){
	return VCMV_NZ;
}

inline vinstr_t get_inv_cmv(vinstr_t instr ){
	switch(instr){
	case VCMV_LTZ:
		return VCMV_GEZ;
	case VCMV_GTZ:
		return VCMV_LEZ;
	case VCMV_LEZ:
		return VCMV_GTZ;
	case VCMV_GEZ:
		return VCMV_LTZ;
	case VCMV_Z :
		return VCMV_NZ;
	case VCMV_NZ :
		return VCMV_Z;
	default:
		assert(0);
		return instr;
	}
}

//resolve
//vec a log b
// ->
#endif //__CONVERT_VINSTR_HPP__
