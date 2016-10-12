#ifndef VBX_OR_MASK_HPP
#define VBX_OR_MASK_HPP
#include "vbx.h"
struct vector_mask_obj;
namespace VBX{
	namespace _internal{
		template<typename T,typename U,typename V>
		inline void vbxx_or_mask(vinstr_t vinstr,T dest,U srcA,V srcB);
		template<typename T,typename U>
		inline void vbxx_or_mask(vinstr_t vinstr,T dest,U src);
		template<typename T,typename U,typename V>
		inline void vbxx_or_mask_acc(vinstr_t vinstr,T dest,U srcA,V srcB);
		template<typename T,typename U>
		inline void vbxx_or_mask_acc(vinstr_t vinstr,T dest,U src);
	}
}
#endif //VBX_OR_MASK_HPP
