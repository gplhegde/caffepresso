/* VECTORBLOX MXP SOFTWARE DEVELOPMENT KIT
 *
 * Copyright (C) 2012-2015 VectorBlox Computing Inc., Vancouver, British Columbia, Canada.
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
 * @defgroup VBX_API VBX API
 * @brief VBX API
 *
 * @ingroup VBXapi
 */
/**@{*/

#ifndef __VBX_API_H
#define __VBX_API_H
#include "vbx_macros.h"
#include "vbx_extern.h"
#ifdef __cplusplus
extern "C" {
#endif
// -----------------------------------------------------------
// DEVELOPER API SECTION
// -----------------------------------------------------------

void        _vbx_init( vbx_mxp_t *this_mxp );

// Scratchpad APIs

vbx_void_t *vbx_sp_malloc_nodebug(                      size_t num_bytes );
vbx_void_t *vbx_sp_malloc_debug( int LINE, const char *FNAME, size_t num_bytes );

void        vbx_sp_free_debug( int LINE, const char *FNAME );
void        vbx_sp_free_nodebug();

vbx_void_t *vbx_sp_get();

void        vbx_sp_set_nodebug(                      vbx_void_t *new_sp );
void        vbx_sp_set_debug( int LINE, const char *FNAME, vbx_void_t *new_sp );

int         vbx_sp_getused();
int         vbx_sp_getfree();

/** Push current scratchpad address to stack
 *
 * Use with @ref vbx_sp_pop for partial freeing of scratchpad memory
 */

__attribute__((always_inline)) static inline void vbx_sp_push()
{
	// do it, but do not print pretty error messages
	vbx_mxp_t *this_mxp = VBX_GET_THIS_MXP();
	if( this_mxp && this_mxp->spstack){
		if(this_mxp->spstack_top == this_mxp->spstack_max ) {
#if (!VBX_STATIC_ALLOCATE_SP_STACK)
			//double the stack space
			this_mxp->spstack_max*=2;
			size_t spstack_size=this_mxp->spstack_max*sizeof(void*);

			printf("realloc sp_stack %d\n",this_mxp->spstack_max);
			this_mxp->spstack=(void**)realloc((void*)this_mxp->spstack,spstack_size);

			if ( !this_mxp->spstack ) {
				VBX_PRINTF("ERROR: Failed to malloc %d bytes for spstack.\n", (int)spstack_size);
				VBX_FATAL(__LINE__, __FILE__, -1);
			}
#else
			VBX_PRINTF("ERROR: Scratchpad allocator stack out of space.\n");
			VBX_FATAL(__LINE__, __FILE__, -1);
#endif
		}
		this_mxp->spstack[ this_mxp->spstack_top++ ] = this_mxp->sp;
	}else{
		VBX_PRINTF( "ERROR: failed to call _vbx_init().\n" );
		VBX_FATAL(__LINE__, __FILE__, -1);
	}
}
/** Pop current scratchpad address to stack
 *
 * Use with @ref vbx_sp_push for partial freeing of scratchpad memory
 */
__attribute__((always_inline)) static inline void vbx_sp_pop()
{
	// do it, but do not print pretty error messages
	vbx_mxp_t *this_mxp = VBX_GET_THIS_MXP();
	if( this_mxp  &&  this_mxp->spstack  &&  0 < this_mxp->spstack_top ) {
		this_mxp->sp = this_mxp->spstack[ --this_mxp->spstack_top ];
	}
}


void        vbx_sp_pop_debug( int LINE, const char *FNAME );


// Memory APIs

void       *vbx_shared_alloca_nodebug( size_t num_bytes, void *p );
void       *vbx_shared_alloca_debug( int LINE,const  char *FNAME, size_t num_bytes, void *p );
void       *vbx_shared_malloc( size_t num_bytes );
void        vbx_shared_free( void *shared_ptr );

// MXP device APIs
vbx_mxp_t  *vbx_open( const char* name );

#ifdef __cplusplus
}
#endif

#endif // __VBX_API_H
/**@}*/
