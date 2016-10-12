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

/**@file*/

#include "vbx.h"
#include "vbw_exit_codes.h"

/// If the number of elements is below this threshold, a scalar algorithm
/// will be used (only in ext version).
#ifndef SCALAR_THRESHOLD
#define SCALAR_THRESHOLD 16
#endif

// This funtion finds a tile no larger than max_elements. Neither dimension will be
// larger than max_elements / min_dim, but at least one dimension will be no smaller
// than min_dim (if possible). Greatest preference is given to finding tiles that span
// over the full height or the full width of the matrix, then to square tiles with dimension
// 2^n + m * min_dim, where n and m are made as large as max_elements will allow.
// The tile that is found may be larger than the matrix in one dimension, so that
// should be controlled for by the caller.
static inline void findTileSize( int *ph, int *pw, const int height,
		const int width, const int max_elements, const int min_dim )
{
	int max_dim = max_elements / min_dim;

	if (height <= max_dim) {
		*ph = height;
		*pw = max_elements / height;
	} else if (width <= max_dim) {
		*ph = max_elements / width;
		*pw = width;
	} else {
		int i;
		for( i=1; i*i < max_elements; i<<=1 );
		while( i*i > max_elements ) i -= min_dim;
		*ph = *pw = i;
	}

	// printf("Tile height %d, width %d\n", *ph, *pw );
}

/** VBX Matrix Transpose *in the scratchpad*.
 *  Transposes entirely within scratchpad (height,width decribe v_src layout)
 *  @pre  v_src must not overlap v_dst.
 *  @pre  v_src contains the data to transpose.
 *  @pre  v_dst points to preallocated scratchpad space
 *  @post v_dst contains the transposed data.
 *
 *  @param[out] v_dst Scratchpad pointer to output matrix
 *  @param[in] v_src Scratchpad pointer to input matrix
 *  @param[in] INROWS Number of rows in v_src.
 *  @param[in] INCOLS Number of cols in v_src.
 *  @returns negative on error condition. See vbw_exit_codes.h
 */
template<typename vbx_sp_t>
int vbw_mtx_xp(vbx_sp_t *v_dst, vbx_sp_t *v_src, const int INROWS, const int INCOLS )
{
	vbx_set_vl( 1 );
	vbx_set_2D(  INCOLS, INROWS*sizeof(vbx_sp_t),       sizeof(vbx_sp_t), 0 );
	vbx_set_3D( INROWS,        sizeof(vbx_sp_t), INCOLS*sizeof(vbx_sp_t), 0 );
	vbxx_3D( VMOV, v_dst, v_src);
	return VBW_SUCCESS;
}


/** VBX Matrix Transpose *in memory*.
 *  @pre  in must not overlap out.
 *  @pre  in contains the data to transpose.
 *  @post out contains the transposed data.
 *
 *  @param[out] out Pointer to output matrix
 *  @param[in] in Pointer to input matrix
 *  @param[in] INROWS Number of rows in input
 *  @param[in] INCOLS Number of columns in input
 *  @returns negative on error condition. See vbw_exit_codes.h
 */
template<typename vbx_mm_t>
int vbw_mtx_xp_ext(vbx_mm_t *out, vbx_mm_t *in, const int INROWS, const int INCOLS )
{
	typedef vbx_mm_t vbx_sp_t;

	int elements = INROWS * INCOLS;

	if(elements < SCALAR_THRESHOLD) {
		vbx_sync();  //in case we input is waiting on a DMA transfer
		int i,j;
		for(i = 0; i < INROWS; i++) {
			for(j = 0; j < INCOLS; j++) {
				out[j*INROWS+i] = in[i*INCOLS+j];
			}
		}
		return VBW_SUCCESS;
	}

	vbx_sp_push();

	vbx_sp_t *v_in;
	vbx_sp_t *v_out;

	int tile_height     = 0;
	int tile_width      = 0;
	int prev_tile_width = 0;
	int tile_y          = 0;
	int tile_x          = 0;

	vbx_mxp_t *this_mxp = VBX_GET_THIS_MXP();
	int SP_WIDTH_B = this_mxp->scratchpad_alignment_bytes;
	int SP_SIZE = vbx_sp_getfree();
	int max_sp_elements   = vbx_sp_getfree() / sizeof(vbx_sp_t);
	int max_tile_elements = VBX_PAD_DN( SP_SIZE/2, SP_WIDTH_B ) / sizeof(vbx_sp_t);


	if( INROWS == 1 || INCOLS == 1 ) {           // 1D transpose becomes a simple copy operation
		if( elements <= max_sp_elements ) {      // We can use the whole scratchpad for this
			v_in = (vbx_sp_t*)vbx_sp_malloc( elements * sizeof(vbx_sp_t) );
			vbx_dma_to_vector( v_in, in, elements*sizeof(vbx_mm_t) );
			v_out = v_in;
			vbx_dma_to_host( out, v_out, elements*sizeof(vbx_mm_t) );
		} else {                                 // To test this, you'll need a very large 1D matrix (or a small SP)
			tile_width = max_sp_elements;
			v_in = (vbx_sp_t*)vbx_sp_malloc( tile_width * sizeof(vbx_sp_t) );
			for (tile_x = 0; tile_x < elements; tile_x += tile_width) {
				if( tile_x + tile_width > elements) tile_width = elements - tile_x;
				vbx_dma_to_vector( v_in, in + tile_x, tile_width*sizeof(vbx_mm_t) );
				v_out = v_in;
				vbx_dma_to_host( out+tile_x, v_out, tile_width*sizeof(vbx_mm_t) );
			}
		}
	} else if( elements < max_tile_elements ) {  // Matrix is small enough to handle entirely in SP
		v_in  = (vbx_sp_t*)vbx_sp_malloc( elements * sizeof(vbx_sp_t) );
		v_out = (vbx_sp_t*)vbx_sp_malloc( elements * sizeof(vbx_sp_t) );

		vbx_dma_to_vector( v_in, in, elements*sizeof(vbx_mm_t) );

		vbw_mtx_xp(v_out,v_in,INROWS,INCOLS);

		vbx_dma_to_host( out, v_out, elements*sizeof(vbx_mm_t) );
	} else {                                     // At this point we know at least one full tile will be needed
		#define QUICK_A_LANES_THRESHOLD 8        // Use merge transpose if there are at least this many lanes
		#define QUICK_A_TILE_WIDTH 128
		#define QUICK_A_TILE_ELEMENTS (QUICK_A_TILE_WIDTH*QUICK_A_TILE_WIDTH)
		#define QUICK_A_VF_ELEMENTS (QUICK_A_TILE_ELEMENTS/2)
		#define QUICK_A_REQ_ELEMENTS (2*VBX_PAD_UP(QUICK_A_TILE_ELEMENTS,SP_WIDTH_B/sizeof(vbx_sp_t)) + VBX_PAD_UP(QUICK_A_VF_ELEMENTS,sizeof(vbx_sp_t)))

		#define QUICK_B_LANES_THRESHOLD 16        // Use smaller merge transpose tile only if there are a lot of lanes
		#define QUICK_B_TILE_WIDTH 64             //     and only if larger tile A size cannot be used.
		#define QUICK_B_TILE_ELEMENTS (QUICK_B_TILE_WIDTH*QUICK_B_TILE_WIDTH)
		#define QUICK_B_VF_ELEMENTS (QUICK_B_TILE_ELEMENTS/2)
		#define QUICK_B_REQ_ELEMENTS (2*VBX_PAD_UP(QUICK_B_TILE_ELEMENTS,SP_WIDTH_B/sizeof(vbx_sp_t)) + VBX_PAD_UP(QUICK_B_VF_ELEMENTS,sizeof(vbx_sp_t)))

		int NUM_LANES = this_mxp->vector_lanes;
		int DMA_BYTES = this_mxp->dma_alignment_bytes;
		int min_tile_dim = DMA_BYTES / sizeof(vbx_sp_t);

		vbx_sp_t *v_out_sel;
		vbx_sp_t *vf = 0;

		if( NUM_LANES >= QUICK_A_LANES_THRESHOLD       // Check for appropriate conditions to use merge transpose tiles
					&& INCOLS >= QUICK_A_TILE_WIDTH
					&& INROWS >= QUICK_A_TILE_WIDTH
			&& (unsigned)max_sp_elements >= QUICK_A_REQ_ELEMENTS ) {
			tile_width = tile_height = QUICK_A_TILE_WIDTH;
			vf = (vbx_sp_t *)vbx_sp_malloc( QUICK_A_VF_ELEMENTS * sizeof(vbx_sp_t));
		} else if( NUM_LANES >= QUICK_B_LANES_THRESHOLD
					&& INCOLS >= QUICK_B_TILE_WIDTH
					&& INROWS >= QUICK_B_TILE_WIDTH
			&& (unsigned)max_sp_elements >= QUICK_B_REQ_ELEMENTS ) {
			tile_width = tile_height = QUICK_B_TILE_WIDTH;
			vf = (vbx_sp_t *)vbx_sp_malloc( QUICK_B_VF_ELEMENTS * sizeof(vbx_sp_t));
		} else {
			findTileSize( &tile_height, &tile_width, INROWS, INCOLS, max_tile_elements, min_tile_dim );
		}

		prev_tile_width = tile_width;

		v_in  = (vbx_sp_t*)vbx_sp_malloc( tile_height*tile_width * sizeof(vbx_sp_t) );
		v_out = (vbx_sp_t*)vbx_sp_malloc( tile_height*tile_width * sizeof(vbx_sp_t) );


		if( v_out==NULL ) {
			vbx_sp_pop();
			return VBW_ERROR_SP_ALLOC_FAILED;
		}

		vbx_sp_t *v[2] = { v_in, v_out };

		tile_y = 0;                              // Reset y position for new col
		while( tile_y < INROWS ) {
		vbx_set_2D( tile_width, tile_height*sizeof(vbx_sp_t), sizeof(vbx_sp_t), sizeof(vbx_sp_t) );
		vbx_set_3D( tile_height, sizeof(vbx_sp_t), tile_width*sizeof(vbx_sp_t), tile_width*sizeof(vbx_sp_t) );
			tile_x = 0;                          // Reset x position for new row
			while( tile_x < INCOLS ) {

				vbx_dma_to_vector_2D(
						v_in,
						in+(tile_y*INCOLS)+tile_x,
						tile_width*sizeof(vbx_mm_t),
						tile_height,
						tile_width*sizeof(vbx_sp_t),
						INCOLS*sizeof(vbx_mm_t) );

				v_out_sel = v_out;                         // select v_out as default vector to DMA to MM

				/* *** merge transpose (matrix must be square and a power of 2 wide) *** */
				if( vf && tile_width == tile_height
							&& (tile_width==QUICK_A_TILE_WIDTH || tile_width==QUICK_B_TILE_WIDTH) ) {
					int src = 0;
					int n;
					for( n=1; n<tile_width; n *= 2 ) {     // can't do 1st iteration until entire tile is DMA'd in
						const int nn = 2*n;

						// copy the destination matrix
						vbx_set_vl( tile_width*tile_width );    // use v_in & v_out as working matrices (clobber v_in)
						vbxx(  VMOV, v[!src], v[src]);

						// do the work
						vbx_set_vl( n*tile_width );
						vbxx( VAND, vf, n, (vbx_enum_t*)0 );           // mask for merging: 0101010... then 00110011...
						vbx_set_2D( tile_width/nn, nn*tile_width*sizeof(vbx_sp_t), nn*tile_width*sizeof(vbx_sp_t), 0 );
						vbxx_2D( VCMV_Z, v[!src]+n*tile_width, v[src]+n           , vf );
						vbxx_2D( VCMV_Z, v[!src]+n,            v[src]+n*tile_width, vf );

						src = !src;
					}

					v_out_sel = v[src];     // depending on the size of the mtx, the final result may be in v_in or v_out
				} else {
					vbx_set_vl( 1 );        // 2D and 3D will be set by the x and y edge conditions, even using merge
					vbxx_3D(VMOV, v_out, v_in );
				}

				vbx_dma_to_host_2D(
						out+(tile_x*INROWS)+tile_y,
						v_out_sel,
						tile_height*sizeof(vbx_mm_t),
						tile_width,
						INROWS*sizeof(vbx_mm_t),
						tile_height*sizeof(vbx_sp_t) );

				tile_x += tile_width;                 // Set up width for next tile
				if( tile_x + tile_width > INCOLS ) {  // Temporarily reduce tile width when reaching right edge of matrix
					tile_width = INCOLS - tile_x;
					vbx_set_2D( tile_width, tile_height*sizeof(vbx_sp_t), sizeof(vbx_sp_t), sizeof(vbx_sp_t) );
					vbx_set_3D( tile_height, sizeof(vbx_sp_t), tile_width*sizeof(vbx_sp_t), tile_width*sizeof(vbx_sp_t) );
				}
			}
			tile_y += tile_height;                    // Set up width and height for next row of tiles
			tile_width = prev_tile_width;             // Restore original tile width for next row of tiles

			/* *** Permanently reduce tile height when reaching bottom of matrix *** */
			tile_height = ( tile_y + tile_height > INROWS ) ? INROWS - tile_y : tile_height;		}
	}
	vbx_sp_pop();
	vbx_sync();
	return VBW_SUCCESS;
}
