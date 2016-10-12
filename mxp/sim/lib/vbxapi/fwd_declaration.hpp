#ifndef FWD_DECLARATION_HPP
#define FWD_DECLARATION_HPP
namespace VBX{

	template<typename T>
	class Vector;
	class enum_t;
	namespace _internal{
		template<typename T,typename U,vinstr_t instr,typename btype>
		struct bin_op;

		template<typename T>
		struct accum_op;

		template<typename dest_t,bool is_masked,bool acc=false>
		struct assignment;

		template<typename dest_t,bool is_masked>
		struct resolve;

		template<typename dest_t,bool is_masked>
		struct conditional_move;

		template<bool is_mask,bool is_acc >
		struct vbx_func;

		enum log_op_t{
			LOGICAL_OR,LOGICAL_AND,NONE
		};
		template<typename lhs_t,typename rhs_t,log_op_t lop,bool negate=false>
		struct Logical_vop;

		template<typename T>
		struct is_all_and;

	}
}
template <typename D,typename B>
struct source_resolve;

#endif //FWD_DECLARATION_HPP
