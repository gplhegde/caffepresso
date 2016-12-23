/*
 * ext_dsplib.c
 *
 *  Created on: 23 Dec 2016
 *      Author: hgashok
 */


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
    n = (nx / 8) * 8;
    rem = nx % 8;
    _nassert(((int)x & 7) ==0);

    #pragma MUST_ITERATE(8,,8);
    #pragma UNROLL(8);
    for(i=0; i<n; i++) {
        x[i] += y;
    }

    for(i = n; i < n + rem; i++) {
    	x[i] += y;
    }
}
