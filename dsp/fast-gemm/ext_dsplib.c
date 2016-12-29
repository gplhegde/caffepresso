/*
 * ext_dsplib.c
 *
 *  Created on: 23 Dec 2016
 *      Author: Gopalakrishna  Hegde, NTU Singapore.
 */

#include <ti/dsplib/dsplib.h>

#pragma CODE_SECTION(DSP_vs_add16_unroll_4, ".text:optimized");
void DSP_vs_add16_unroll_4 (
    short * restrict x,   /* Input array of length nx  */
    short y,   			  /* Scalar value to be added */
    short * restrict r,   /* Output array of length nx. even though the output array can be same as input in this case(because it is pointwise op) using restrict to fool the compiler*/
    int              nx   /* Number of elements.       */
)
{
    int i;

    _nassert(((int)x & 4) ==0);
    _nassert(((int)r & 4) ==0);
    #pragma MUST_ITERATE(4,,4);
    #pragma UNROLL(4);

    for(i=0; i<nx; i++) {
        r[i] = x[i] + y;
    }
}

#pragma CODE_SECTION(DSPF_vs_add_unroll_4, ".text:optimized");
void DSPF_vs_add_unroll_4 (
    float * restrict x,   /* Input array of length nx  */
    float y,   			  /* Scalar to be added */
    float * restrict r,   /* Output array of length nx */
    int              nx   /* Number of elements.       */
)
{
    int i;

    _nassert(((int)x & 7) ==0);
    _nassert(((int)y & 7) ==0);
    _nassert(((int)r & 7) ==0);
    #pragma MUST_ITERATE(4,,4);
    #pragma UNROLL(4);

    for(i=0; i<nx; i++) {
        r[i] = x[i] + y;
    }
}

#pragma CODE_SECTION(DSP_vs_add_unroll_8, ".text:optimized");
void DSP_vs_add_unroll_8 (
    short * restrict x,   /* Input array of length nx  */
    short y,   			  /* Scalar to be added */
    int              nx   /* Number of elements.       */
)
{
    int i, n, rem;

    // Perform scalar computations until x is 8byte aligned.
    while((int)x % 8 != 0) {
    	*x = *x + y;
    	x++;
    	nx--;
    }

    n = (nx / 8) * 8;
    rem = nx % 8;

    #pragma MUST_ITERATE(8,,8);
    #pragma UNROLL(8);
    for(i = 0; i < n; i++) {
        x[i] += y;
    }

    for(i = n; i < n + rem; i++) {
    	x[i] += y;
    }
}

// This does x * s + t where x is a vector of length nx and s,t are scalars
#pragma CODE_SECTION(DSP_vector_scale_translate, ".text.optimized");
void DSP_vector_scale_translate(
		short * restrict x,
		short * restrict y,
		short s,
		short t,
		int nx,
		short shift
	) {
    int i, n, rem;

    // Perform scalar computations until x is 8byte aligned.
    while((int)x % 8 != 0) {
    	*y = ((*x * s) >> shift ) + t;
    	x++; y++;
    	nx--;
    }

    n = (nx / 8) * 8;
    rem = nx % 8;
    #pragma MUST_ITERATE(8,,8);
    #pragma UNROLL(8);
    for(i = 0; i < n; i++) {
        y[i] = ((x[i] * s) >> shift) + t;
    }

    for(i = n; i < n + rem; i++) {
    	y[i] = ((x[i] * s) >> shift) + t;
    }
}

