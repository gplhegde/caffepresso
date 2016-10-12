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
 * @defgroup vbx_common vbx common
 * @brief vbx common
 *
 * @ingroup vbxapi
 */
/**@{*/

#ifndef __VBX_COMMON_H
#define __VBX_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef min

/** Macro returning min of two inputs
 *
 * @param[in] x
 * @param[in] y
 * @retval returns minimum of x and y
 */
#define min(x,y) \
	({\
		typeof(x) __x__ = (x);\
		typeof(y) __y__ = (y);\
		(__x__<__y__)? __x__ : __y__;\
	})
#endif // min()

#ifndef max

/** Macro returning max of two inputs
 *
 * @param[in] x
 * @param[in] y
 * @retval returns maximum of x and y
 */
#define max(x,y) \
	({\
		typeof(x) __x__ = (x);\
		typeof(y) __y__ = (y);\
		(__x__>__y__)? __x__ : __y__;\
	})
#endif // max()


#define LOG2(n) ((n)>=(1<<31) ? 31 :												\
					  (n)>=(1<<30) ? 30 :												\
					  (n)>=(1<<29) ? 29 :												\
					  (n)>=(1<<28) ? 28 :												\
					  (n)>=(1<<27) ? 27 :												\
					  (n)>=(1<<26) ? 26 :												\
					  (n)>=(1<<25) ? 25 :												\
					  (n)>=(1<<24) ? 24 :												\
					  (n)>=(1<<23) ? 23 :												\
					  (n)>=(1<<22) ? 22 :												\
					  (n)>=(1<<21) ? 21 :												\
					  (n)>=(1<<20) ? 20 :												\
					  (n)>=(1<<19) ? 19 :												\
					  (n)>=(1<<18) ? 18 :												\
					  (n)>=(1<<17) ? 17 :												\
					  (n)>=(1<<16) ? 16 :												\
					  (n)>=(1<<15) ? 15 :												\
					  (n)>=(1<<14) ? 14 :												\
					  (n)>=(1<<13) ? 13 :												\
					  (n)>=(1<<12) ? 12 :												\
					  (n)>=(1<<11) ? 11 :												\
					  (n)>=(1<<10) ? 10 :												\
					  (n)>=(1<<9) ? 9 :													\
					  (n)>=(1<<8) ? 8 :													\
					  (n)>=(1<<7) ? 7 :													\
					  (n)>=(1<<6) ? 6 :													\
					  (n)>=(1<<5) ? 5 :													\
					  (n)>=(1<<4) ? 4 :													\
					  (n)>=(1<<3) ? 3 :													\
					  (n)>=(1<<2) ? 2 :													\
					  (n)>=(1<<1) ? 1 : 0)

#ifdef __cplusplus
}
#endif

#endif // __VBX_COMMON_H
/**@}*/
