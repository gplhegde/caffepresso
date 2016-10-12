#ifndef RANGE_HPP
#define RANGE_HPP

namespace VBX{
		struct range_t{
			const int& from;
			const int& to;
			range_t(const int& from,const int&to):from(from),to(to){}
		};
		struct range_op_t{};
		struct range_op_lhs{
			const int& from;
			range_op_lhs(const int& from):from(from){}
		};
	static range_op_t RANGE __attribute__((unused));

	static inline range_op_lhs operator,(const int & from, const range_op_t&)
	{
		return range_op_lhs(from);
	}
	static inline range_t operator,(const range_op_lhs& lhs,const int& to)
	{
		return range_t(lhs.from,to);
	}
}
#define upto ,VBX::RANGE,
#endif //RANGE_HPP
