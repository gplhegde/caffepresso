#ifndef VBX_FUNC_HPP
#define VBX_FUNC_HPP

namespace VBX{
	namespace _internal{
		template<bool is_mask,bool is_acc >
		struct vbx_func;
		template<>
		struct vbx_func<true,true>{
			template <typename T,typename U,typename V>
			static void func(vinstr_t vinstr,T dest,U srcA,V srcB){
				vbxx_masked_acc(vinstr,dest,srcA,srcB);
			}
			template <typename T,typename U>
			static void func(vinstr_t vinstr,T dest,U srcA){
				vbxx_masked_acc(vinstr,dest,srcA);
			}
		};
		template<>
		struct vbx_func<true,false>{
			template <typename T,typename U,typename V>
			static void func(vinstr_t vinstr,T dest,U srcA,V srcB){
				vbxx_masked(vinstr,dest,srcA,srcB);
			}
			template <typename T,typename U>
			static void func(vinstr_t vinstr,T dest,U srcA){
				vbxx_masked(vinstr,dest,srcA);
			}
		};
		template<>
		struct vbx_func<false,true>{
			template <typename T,typename U,typename V>
			static void func(vinstr_t vinstr,T dest,U srcA,V srcB){
				vbxx_acc(vinstr,dest,srcA,srcB);
			}
			template <typename T,typename U>
			static void func(vinstr_t vinstr,T dest,U srcA){
				vbxx_acc(vinstr,dest,srcA);
			}
		};
		template<>
		struct vbx_func<false,false>{
			template <typename T,typename U,typename V>
			static void func(vinstr_t vinstr,T dest,U srcA,V srcB){
				vbxx(vinstr,dest,srcA,srcB);
			}
			template <typename T,typename U>
			static void func(vinstr_t vinstr,T dest,U srcA){
				vbxx(vinstr,dest,srcA);
			}

		};


	}//namespace _internal
}//namespace VBX



#endif //VBX_FUNC_HPP
