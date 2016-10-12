/* VECTORBLOX MXP SOFTWARE DEVELOPMENT KIT
 *
 * Copyright (C) 2012-2016 VectorBlox Computing Inc., Vancouver, British Columbia, Canada.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of VectorBlox Computing Inc. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This agreement shall be governed in all respects by the laws of the Province
 * of British Columbia and by the laws of Canada.
 *
 * This file is part of the VectorBlox MXP Software Development Kit.
 *
 */

/**
 * @file
 * @defgroup VBX_macros VBX Macros
 * @brief VBX macros
 *
 * @ingroup VBXapi
 */
/**@{*/

#ifndef __VBX_MACROS_H
#define __VBX_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h> // for proper exit() prototype
#include <stdio.h>  // for printf() and putchar()

#include "vbx_extern.h"
#include "vbx_common.h"


// ---------------------------------

/** VBX start for macros ( do{...}while(0) --> VBX_S{...}VBX_E ) */
#define VBX_S do
/** VBX end for macros ( do{...}while(0) --> VBX_S{...}VBX_E ) */
#define VBX_E while(0)

// ---------------------------------

#define VBX_REG_MXPCPU       16

#if VBX_USE_GLOBAL_MXP_PTR
#define VBX_GET_THIS_MXP() (vbx_mxp_ptr)
#else
#define VBX_GET_THIS_MXP() \
	({ int __t__; vbx_get_reg( VBX_REG_MXPCPU, &__t__ ); (vbx_mxp_t*)__t__; })
#endif

//#define VBX_IS_VPTR(PTR)
//	({ vbx_void_t *__vptr__ = (vbx_void_t *)(PTR);
///           ( (VBX_SCRATCHPAD_ADDR<=__vptr__) && (__vptr__<VBX_SCRATCHPAD_END) );
//         })
//
//The alternative macros below only work if you can guarantee scratchpad
// start address is aligned to the scratchpad size.
//#define VPTR_MASK    (~(VECTOR_MEMORY_SIZE*1024-1))
//#define IS_VPTR(PTR) ( (((vbx_void_t *)PTR)&(VPTR_MASK))==(VBX_SCRATCHPAD_ADDR) )

#define VBX_PAD_UP(BYTES,ALIGNMENT) \
	({ \
		size_t __mask__ = ((size_t)(ALIGNMENT))-1; \
		( ((size_t)(BYTES)) + __mask__) & ~__mask__; \
	})

#define VBX_PAD_DN(BYTES,ALIGNMENT) \
	({ \
		size_t __mask__ = ((size_t)(ALIGNMENT))-1; \
		((size_t)(BYTES))  &  ~__mask__; \
	})

#define VBX_IS_MISALIGNED(LENGTH,ALIGNMENT)	((((size_t)(LENGTH))&((size_t)(ALIGNMENT)-1))?1:0)
#define VBX_IS_ALIGNED(LENGTH,ALIGNMENT)	(!VBX_IS_MISALIGNED((LENGTH),(ALIGNMENT)))

// ---------------------------------

#define VBX_PADDING() (VBX_CPU_DCACHE_LINE_SIZE)

// ---------------------------------

#define VBX_DEBUG_FUNC1(fname,...) \
	VBX_SKIP_ALL_CHECKS ? \
		fname##_nodebug(__VA_ARGS__) : \
		fname##_debug(__LINE__,__FILE__,__VA_ARGS__)

#define VBX_DEBUG_FUNC0(fname)\
	VBX_SKIP_ALL_CHECKS ? \
		fname##_nodebug() : \
		fname##_debug(__LINE__,__FILE__)

/** Malloc in scratchpad.
 *
 * @param[in] amount -- number of bytes to allocate
 */
#define vbx_sp_malloc(amount)      ( VBX_DEBUG_FUNC1(vbx_sp_malloc,amount) )

/** Set new position of scratchpad.
 *
 * @param[in] new_sp -- new address pointing to top of scratchpad
 */
#define vbx_sp_set(new_sp)         VBX_S{ VBX_DEBUG_FUNC1( vbx_sp_set, new_sp ); }VBX_E

/** Free entire scratchpad.
 *
 * Use @ref vbx_sp_push and @ref vbx_sp_pop for partial allocating/free
 */
#define vbx_sp_free()              VBX_S{ VBX_DEBUG_FUNC0( vbx_sp_free );        }VBX_E

/** Allocate in stack frame of caller
 *
 * @param[in] amount -- number of bytes to allocate
 */
#define vbx_shared_alloca(amount)  ( VBX_DEBUG_FUNC1( vbx_shared_alloca, amount, alloca(amount+2*VBX_PADDING()) ) )


// ---------------------------------

#define VBX_PRINTF(...) \
	VBX_S{ \
		if( VBX_DEBUG_LEVEL ) { \
			printf( __VA_ARGS__ ); \
		} \
	}VBX_E

	void VBX_FATAL(int , const char* , int);

#define VBX_EXIT(ERR)  \
	VBX_FATAL(__LINE__,__FILE__,ERR)

#define debug(var) printf("%s:%d  %s = %d \r\n",__FILE__,__LINE__,#var,(signed)(size_t)(var))
#define debugx(var) printf("%s:%d  %s = %08X \r\n",__FILE__,__LINE__,#var,(unsigned)(size_t)(var))
#define debugfxp(var,bits) printf("%s:%d  %s = %f \r\n",__FILE__,__LINE__,#var,(double)(var)/(1<<bits))
#define debugfxpw(var) debugfxp(var,VBX_GET_THIS_MXP()->fxp_word_frac_bits)
#ifdef __cplusplus
}
#endif

#endif //__VBX_MACROS_H
/**@}*/
