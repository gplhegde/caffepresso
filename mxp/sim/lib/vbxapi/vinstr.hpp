#ifndef __VINSTR_HPP__
#define __VINSTR_HPP__
template<typename dest_t,typename srca_t,typename srcb_t>
static inline void vbxx(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	srcb_t* srcb_arg = srcb.data;
	vbxx(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srcb_t>
static inline void vbxx(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	srcb_t* srcb_arg = srcb.data;
	vbxx(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t>
static inline void vbxx(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbxx(instr,dest_arg,srca_arg);
}
template<typename dest_t>
static inline void vbxx(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbxx(instr,dest_arg,srca_arg);
}

template<typename dest_t,typename srca_t,typename srcb_t>
static inline void vbxx_masked(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	srcb_t* srcb_arg = srcb.data;
	vbxx_masked(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srcb_t>
static inline void vbxx_masked(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	srcb_t* srcb_arg = srcb.data;
	vbxx_masked(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_masked(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_masked(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t>
static inline void vbxx_masked(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_masked(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_masked(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbxx_masked(instr,dest_arg,srca_arg);
}
template<typename dest_t>
static inline void vbxx_masked(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbxx_masked(instr,dest_arg,srca_arg);
}

template<typename dest_t,typename srca_t,typename srcb_t>
static inline void vbxx_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	srcb_t* srcb_arg = srcb.data;
	vbxx_acc(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srcb_t>
static inline void vbxx_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	srcb_t* srcb_arg = srcb.data;
	vbxx_acc(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_acc(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t>
static inline void vbxx_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_acc(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbxx_acc(instr,dest_arg,srca_arg);
}
template<typename dest_t>
static inline void vbxx_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbxx_acc(instr,dest_arg,srca_arg);
}

template<typename dest_t,typename srca_t,typename srcb_t>
static inline void vbxx_masked_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	srcb_t* srcb_arg = srcb.data;
	vbxx_masked_acc(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srcb_t>
static inline void vbxx_masked_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	srcb_t* srcb_arg = srcb.data;
	vbxx_masked_acc(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_masked_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_masked_acc(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t>
static inline void vbxx_masked_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_masked_acc(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_masked_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbxx_masked_acc(instr,dest_arg,srca_arg);
}
template<typename dest_t>
static inline void vbxx_masked_acc(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbxx_masked_acc(instr,dest_arg,srca_arg);
}

template<typename dest_t,typename srca_t,typename srcb_t>
static inline void vbxx_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	srcb_t* srcb_arg = srcb.data;
	vbxx_2D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srcb_t>
static inline void vbxx_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	srcb_t* srcb_arg = srcb.data;
	vbxx_2D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_2D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t>
static inline void vbxx_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_2D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbxx_2D(instr,dest_arg,srca_arg);
}
template<typename dest_t>
static inline void vbxx_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbxx_2D(instr,dest_arg,srca_arg);
}

template<typename dest_t,typename srca_t,typename srcb_t>
static inline void vbxx_acc_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	srcb_t* srcb_arg = srcb.data;
	vbxx_acc_2D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srcb_t>
static inline void vbxx_acc_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	srcb_t* srcb_arg = srcb.data;
	vbxx_acc_2D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_acc_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_acc_2D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t>
static inline void vbxx_acc_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_acc_2D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_acc_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbxx_acc_2D(instr,dest_arg,srca_arg);
}
template<typename dest_t>
static inline void vbxx_acc_2D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbxx_acc_2D(instr,dest_arg,srca_arg);
}

template<typename dest_t,typename srca_t,typename srcb_t>
static inline void vbxx_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	srcb_t* srcb_arg = srcb.data;
	vbxx_3D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srcb_t>
static inline void vbxx_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	srcb_t* srcb_arg = srcb.data;
	vbxx_3D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_3D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t>
static inline void vbxx_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_3D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbxx_3D(instr,dest_arg,srca_arg);
}
template<typename dest_t>
static inline void vbxx_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbxx_3D(instr,dest_arg,srca_arg);
}

template<typename dest_t,typename srca_t,typename srcb_t>
static inline void vbxx_acc_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	srcb_t* srcb_arg = srcb.data;
	vbxx_acc_3D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srcb_t>
static inline void vbxx_acc_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::Vector<srcb_t>& srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	srcb_t* srcb_arg = srcb.data;
	vbxx_acc_3D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_acc_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_acc_3D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t>
static inline void vbxx_acc_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca,const VBX::enum_t&  srcb )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbx_enum_t* srcb_arg = (vbx_enum_t*)0;
	vbxx_acc_3D(instr,dest_arg,srca_arg,srcb_arg);
}
template<typename dest_t,typename srca_t>
static inline void vbxx_acc_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,const VBX::Vector<srca_t>& srca )
{
	dest_t* dest_arg = dest.data;
	srca_t* srca_arg = srca.data;
	vbxx_acc_3D(instr,dest_arg,srca_arg);
}
template<typename dest_t>
static inline void vbxx_acc_3D(vinstr_t instr,const VBX::Vector<dest_t>& dest,vbx_word_t srca )
{
	dest_t* dest_arg = dest.data;
	vbx_word_t srca_arg = srca;
	vbxx_acc_3D(instr,dest_arg,srca_arg);
}

#endif //__VINSTR_HPP__
