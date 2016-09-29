#ifndef DSP_ADD16_H_
#define DSP_ADD16_H_ 1

/**
 *     This program performs a addition on a vector.
 *     The result is stored in a vector.
 *  
 * 			@param x = Input data array of shorts
 * 			@param y = Input data array of shorts
 * 			@param r = Output data array of shorts
 * 			@param nx = Number of elements in the arrays
 * 
 *
 * @par Assumptions: 
 *     Arrays x, y, r do not overlap. <BR>        
 *     Arrays x, y, r are aligned on double word boundaries. <BR>        
 *
 *  
 */


void DSP_add16_shift (
    short * restrict x,   /* Input array of length nx  */
    short * restrict y,   /* Input array of length nx  */
    short * restrict r,   /* Output array of length nx */
    int     nx,            /* Number of elements.       */
	short shift
);

#endif /* DSP_ADD16_H_ */


