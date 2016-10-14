#include "conv_layer.h"
#include "vbx.h"
#include "debug_control.h"
#include <string.h>
#include "vbx_port.h"
#include "struct_defs.h"

/* \breif Accumulate all concatenated feature map rows and scale by number of maps.
*
* @param pMapRow 	: A concatenated row in which 1 row of all feature maps are stacked side by side.
* @param pAcc 		: Accumulator for temperary storage.
* @param pAccMapRow	: Accumulated and scaled row. The accumulated row is replicated nCurMaps times.
* @param W			: Width of individual feature map.
* @param nCurMaps	: No of feature maps that this convolution layer computes.
* @param nPrevMaps	: No of input feature maps for this layer.
* @return			: void
*/
inline static void acc_all_map_rows(FP_MAP_PIXEL *pMapRow, int32_t * pAcc, FP_MAP_PIXEL *pAccMapRow,
    int W, int nCurMaps, int nPrevMaps){
    int vl, map;
    vbx_get_vl(&vl);
	vbx(SVH, VMUL, pAccMapRow, 0, pAccMapRow);
    vbx_set_vl(W);
	for(map = 0; map < nPrevMaps; map++) {
		// FIXME: chance of overflow as the maps are already in Q15 format. Adding 10 Q15 numbers will lead to overflow
		// Fix would be to use 32 bit spAcc buffer as accumulator and then do format conversion after division by nCurMaps
        vbx(VVH, VADD, pAccMapRow, pAccMapRow, (pMapRow + W * map));
	}
	// TODO: Divide the sum by nPrevMaps to maintain the range of values
	// replicate the sum to all other map fields.
	for(map = 1; map < nCurMaps; map++) {
		vbx(VVH, VMOV, pAccMapRow + map * W, pAccMapRow, 0);
	}
    vbx_set_vl(vl);
}

/* \brief Replicate the kernel coefficient W times to make it a vector suitable for vector-vector multiplication.
*
* @param spExtKer	: Pointer to scratchpad buffer to store the extended kernel.
* @param spTemp		: A temp scratchpad buffer of length at least W
* @param coeff		: kernel coefficient to replicate
* @param W			: No of times to repeat the kernel coefficient for each map.
* @param N			: No of feature maps.
* @return			: void
*/
inline static void extend_kernel(FP_KERNEL *spExtKer, FP_KERNEL *spTemp, FP_KERNEL coeff, int W, int N) {
	int vl, map;
	vbx_get_vl(&vl);
	vbx_set_vl(W);
	vbx(SVH, VMUL, spTemp, 0, spTemp);
	for(map = 0; map < N; map++) {
		vbx(SVH, VCMV_Z, spExtKer + map*W, coeff, spTemp);
	}
	vbx_set_vl(vl);
}

/* \brief Replicate 1 row of feature map given width 
*
* @param pMapRow	: Pointer to scratchpad buffer where a single row of the feature map is stored.
* @param pExtRow	: Pointer to scratchpad buffer to store the extended feature map row.
* @param W			: Feature map width
* @param nMaps		: No of feature maps
* @return			: void
*/
inline static void extend_map_row(FP_MAP_PIXEL *pMapRow, FP_MAP_PIXEL *pExtRow, int W, int nMaps) {
    int map, vl;
    vbx_get_vl(&vl);
    vbx_set_vl(W);
    for (map = 0; map < nMaps; map++) {
        vbx(VVH, VMOV, pExtRow + map * W, pMapRow, 0);
    }
    vbx_set_vl(vl);
}

/* \brief Generate convolution output of a layer using feature maps of previous layer. The input feature
* maps are assumed to be in concatenated mode.
*
* @param pInMap 	: Pointer(DRAM) to memory containing input concatenated feature maps.
* @param ppExtKer	: Pointer to buffer containing pointers of extended kernel. Each buffer contains
*					  extension of 1 coefficient from each kernel
* @param pConvInfo	: Parameters required for convolution. See the structure for more info.
* @param pConcatMap	: Buffer to store the concatenated output maps.
* @return 			: status of convolution operation.
*/
// TODO: Need to add bias after convolution
APP_STATUS_E vector_block_conv_layer(FP_MAP_PIXEL *pImap, FP_KERNEL **ppExtKer, CONV_INFO_T *pConvInfo, FP_MAP_PIXEL *pConcatMap) {

	FP_MAP_PIXEL *spMapRow, **sppAccMapRows, *spFiltRow, *spTemp;
	int32_t *spAcc, *spProd;
	FP_KERNEL *spExtKerPing, *spExtKerPong, *spTempKer;
	int row, kr, kc;

    // Allocate memory to hold buffer pointers
    sppAccMapRows = (FP_MAP_PIXEL **)malloc(pConvInfo->K * sizeof(FP_MAP_PIXEL *));

	// SP buffer allocation
	for (row = 0; row < pConvInfo->K; row++) {
		if ((NULL == (sppAccMapRows[row] = (FP_MAP_PIXEL *)vbx_sp_malloc(pConvInfo->nOutMaps * pConvInfo->mapW * sizeof(FP_MAP_PIXEL))))) { 
			REL_INFO("SP buffer allocation failed\n");
			return SP_MALLOC_FAIL;
		}
	}
	if ((NULL == (spMapRow = (FP_MAP_PIXEL *)vbx_sp_malloc(pConvInfo->nInMaps * pConvInfo->mapW * sizeof(FP_MAP_PIXEL)))) ||
		(NULL == (spAcc = (int32_t *)vbx_sp_malloc(pConvInfo->nOutMaps * pConvInfo->mapW * sizeof(int32_t)))) ||
		(NULL == (spProd = (int32_t *)vbx_sp_malloc(pConvInfo->nOutMaps * pConvInfo->mapW * sizeof(int32_t)))) ||
		(NULL == (spFiltRow = (FP_MAP_PIXEL *)vbx_sp_malloc(pConvInfo->nOutMaps * pConvInfo->mapW * sizeof(FP_MAP_PIXEL)))) ||
		(NULL == (spExtKerPong = (FP_KERNEL *)vbx_sp_malloc(pConvInfo->nOutMaps * pConvInfo->mapW  * sizeof(FP_KERNEL)))) ||
		(NULL == (spExtKerPing = (FP_KERNEL *)vbx_sp_malloc(pConvInfo->nOutMaps * pConvInfo->mapW  * sizeof(FP_KERNEL))))){
		REL_INFO("SP buffer allocation failed\n");
		return SP_MALLOC_FAIL;
	}


	vbx_dcache_flush_all();
	vbx_set_vl(pConvInfo->nOutMaps * pConvInfo->mapW);

	// DMA K - 1 rows of  previous layer maps
	for (row = 0; row < pConvInfo->K - 1; row++) {
		vbx_dma_to_vector(spMapRow,
			pImap + row * pConvInfo->nInMaps * pConvInfo->mapW,
			pConvInfo->nInMaps * pConvInfo->mapW * sizeof(FP_MAP_PIXEL));
		// Extend or average map rows depending on input/hidden layer
		extend_map_row(spMapRow, sppAccMapRows[row], pConvInfo->mapW, pConvInfo->nOutMaps);
	}
 
	
	for (row = 0; row < pConvInfo->mapH - pConvInfo->K + 1; row++) {
		// DMA new  map row
		vbx_dma_to_vector(spMapRow, pImap + (row + pConvInfo->K - 1) * pConvInfo->nInMaps * pConvInfo->mapW,
			pConvInfo->nInMaps * pConvInfo->mapW * sizeof(FP_MAP_PIXEL));

		// Extend map rows
		extend_map_row(spMapRow, sppAccMapRows[pConvInfo->K - 1], pConvInfo->mapW, pConvInfo->nOutMaps);

		// Load extended kernel segment corresponding to K(0,0)
		vbx_dma_to_vector(spExtKerPing, ppExtKer[0], pConvInfo->mapW * pConvInfo->nOutMaps * sizeof(FP_KERNEL));

		// Reset accumulator
		vbx(SVW, VMUL, spAcc, 0, spAcc);
		for (kr = 0; kr < pConvInfo->K; kr++) {
			// Compute partial product and accumulate
			for (kc = 0; kc < pConvInfo->K; kc++) {
				// Kernel segment double buffering
				if ((kr * pConvInfo->K + kc) < (pConvInfo->K * pConvInfo->K - 1)) {
					vbx_dma_to_vector(spExtKerPong,
						ppExtKer[kr * pConvInfo->K + kc + 1],
						pConvInfo->mapW * pConvInfo->nOutMaps * sizeof(FP_KERNEL));
				}
	
				vbx(VVHW, VMUL, spProd, spExtKerPing, sppAccMapRows[kr]);
				// TODO: remove 1 sign bit to improve precesion by 1 bit
				vbx(VVW, VADD, spAcc, spAcc, spProd + kc); // FIXME: buffer address overflow due to offset
				// Wrap kernel double buffering pointers
				spTempKer 		= spExtKerPing;
				spExtKerPing 	= spExtKerPong;
				spExtKerPong 	= spTempKer;
			}
		}

		// convert the filtered row to Q15 format. Assuming kernels and input image are normalized, the filtered
		// output will be between -1 to +0.99999999 and hence can be represented in Q15 format.
		vbx(SVWH, VSHR, spFiltRow, pConvInfo->nKerFractionBits, spAcc);

		// DMA the result to host memory
		vbx_dma_to_host(pConcatMap + row * pConvInfo->mapW * pConvInfo->nOutMaps,
			spFiltRow, pConvInfo->mapW * pConvInfo->nOutMaps * sizeof(FP_MAP_PIXEL));

		// wrap pointers
        spTemp = sppAccMapRows[0];
        for (kr = 0; kr < pConvInfo->K - 1; kr++) {
            sppAccMapRows[kr] = sppAccMapRows[kr + 1];
        }
        sppAccMapRows[pConvInfo->K - 1] = spTemp;
	}
	
	vbx_sync();
	vbx_sp_free();
	return SUCCESS;
}

//==================================================================
// END OF MXP VECTORIZED IMPLEMENTATION OF DEEP CONV LAYER
//==================================================================

/* \brief Fixed point 2D convolution using scalar operations. Boundary pixels are not considered.
*
* @param pInMap 	: Input maps in map isolated mode.
* @param pKer		: Kernel buffer
* @param pConvInfo	: Structure containing parameters for the convolution.
* @param pOutMap	: Place to store the convolution output.
* @return			: status 
*/
APP_STATUS_E scalar_fix_conv2D(FP_MAP_PIXEL *pInMap, FP_KERNEL *pKer, CONV_INFO_T *pConvInfo, FP_MAP_PIXEL *pOutMap) {
	int row, col, kr, kc, iH, iW, oW;
	int32_t sop;

	iH = pConvInfo->mapH + 2 * pConvInfo->pad;
	iW = pConvInfo->mapW + 2 * pConvInfo->pad;
	oW = (pConvInfo->mapW + 2 * pConvInfo->pad - pConvInfo->K + 1 + pConvInfo->stride - 1) / pConvInfo->stride;

	for (row = 0; row < iH - pConvInfo->K + 1; row+=pConvInfo->stride)
	{
		for (col = 0; col < iW - pConvInfo->K + 1; col+=pConvInfo->stride)
		{
			// Reset accumulator
			sop = 0;
			for ( kr = 0; kr < pConvInfo->K; kr ++)
			{
				for (kc = 0; kc < pConvInfo->K; kc++)
				{
					// MAC
					sop += pKer[pConvInfo->K * kr + kc] * 
						pInMap[iW * row + col + iW * kr + kc];
				}
			}
			sop = sop >> pConvInfo->nKerFractionBits;
			pOutMap[row/pConvInfo->stride * oW + col/pConvInfo->stride] = (FP_MAP_PIXEL)sop;
		}
	}

	return SUCCESS;
}

/* \brief Abstrat scalar fixed point convolution API. This takes a set of input feature maps and 
* computes a set of feature maps. The feature input maps must be in isolated mode.
*
* @param pInMaps 	: Pointer to buffer containing input feature maps stored in isolated mode.
* @param pKer		: Pointer to kernel buffer.
* @param pConvInfo	: Parameters for the convolution layer.
* @param pOutMaps	: Place to store the output feature maps of the convolution.
* @return			: status of computation
*/
APP_STATUS_E scalar_fix_conv_layer(FP_MAP_PIXEL *pInMaps, FP_KERNEL *pKer, FP_KERNEL *pBias,
	CONV_INFO_T *pConvInfo, FP_MAP_PIXEL *pOutMaps) {

	int omap, pixel, imap, oW, oH, iW, iH;
	int32_t *pAccMap;
	FP_MAP_PIXEL *pTempMap;

	iH = pConvInfo->mapH + 2 * pConvInfo->pad;
	iW = pConvInfo->mapW + 2 * pConvInfo->pad;
	oH = (pConvInfo->mapH + 2 * pConvInfo->pad - pConvInfo->K + 1 + pConvInfo->stride - 1) / pConvInfo->stride;
	oW = (pConvInfo->mapW + 2 * pConvInfo->pad - pConvInfo->K + 1 + pConvInfo->stride - 1) / pConvInfo->stride;

	if( (NULL == (pAccMap = (int32_t *)malloc(oH * oW * sizeof(int32_t)))) ||
		(NULL == (pTempMap 	= (FP_MAP_PIXEL *)malloc(oH * oW * sizeof(FP_MAP_PIXEL))))) {
		REL_INFO("Malloc failed\n");
		return MALLOC_FAIL;
	}

	for ( omap = 0; omap < pConvInfo->nOutMaps; omap++) {
		memset(pAccMap, 0, oH * oW * sizeof(int32_t));
		for (imap = 0; imap < pConvInfo->nInMaps; imap++) {
			scalar_fix_conv2D(pInMaps + imap * iH * iW,
				pKer + (omap * pConvInfo->nInMaps + imap) * pConvInfo->K * pConvInfo->K,
				pConvInfo,
				pTempMap);
			// Add the convolved map to accumulator
			for ( pixel = 0; pixel < oH * oW; pixel++) {
				pAccMap[pixel] += pTempMap[pixel];
			}
		}
		// Scale and store 
		for(pixel = 0; pixel < oH * oW; pixel++) {
			//TODO: make sure that the bias is in correct Q format before adding.
			pOutMaps[omap * oW * oH + pixel] = (FP_MAP_PIXEL)(pAccMap[pixel] + pBias[omap]);
		}
	}
	// Free up heap
	free(pAccMap);	
	free(pTempMap);

	return SUCCESS;
}

/* \brief Floating point 2D convolution using scalar operations. Boundary pixels are not considered.
*
* @param pInMap 	: Input maps in map isolated mode.
* @param pKer		: Kernel buffer
* @param pConvInfo	: Structure containing parameters for the convolution.
* @param pOutMap	: Place to store the convolution output.
* @return			: status 
*/
APP_STATUS_E scalar_float_conv2D(FL_MAP_PIXEL *pInMap, FL_KERNEL *pKer, CONV_INFO_T *pConvInfo, FL_MAP_PIXEL *pOutMap) {
	int row, col, kr, kc, iH, iW, oW;
	FL_MAP_PIXEL sop;

	iH = pConvInfo->mapH + 2 * pConvInfo->pad;
	iW = pConvInfo->mapW + 2 * pConvInfo->pad;
	oW = (pConvInfo->mapW + 2 * pConvInfo->pad - pConvInfo->K + 1 + pConvInfo->stride - 1) / pConvInfo->stride;

	for (row = 0; row < iH - pConvInfo->K + 1; row+=pConvInfo->stride)
	{
		for (col = 0; col < iW - pConvInfo->K + 1; col+=pConvInfo->stride)
		{
			// Reset accumulator
			sop = 0.0;
			for ( kr = 0; kr < pConvInfo->K; kr ++)
			{
				for (kc = 0; kc < pConvInfo->K; kc++)
				{
					// MAC
					sop += pKer[pConvInfo->K * kr + kc] * pInMap[iW * row + col + iW * kr + kc];
				}
			}
			pOutMap[row/pConvInfo->stride * oW + col/pConvInfo->stride] = sop;
		}
	}

	return SUCCESS;
}

/* \brief Abstrat scalar floating point convolution API. This takes a set of input feature maps and 
* computes a set of feature maps. The feature input maps must be in isolated mode.
*
* @param pInMaps 	: Pointer to buffer containing input feature maps stored in isolated mode.
* @param pKer		: Pointer to kernel buffer.
* @param pConvInfo	: Parameters for the convolution layer.
* @param pOutMaps	: Place to store the output feature maps of the convolution.
* @return			: status of computation
*/
APP_STATUS_E scalar_float_conv_layer(FL_MAP_PIXEL *pInMaps, FL_KERNEL *pKer, FL_KERNEL *pBias,
	CONV_INFO_T *pConvInfo, FL_MAP_PIXEL *pOutMaps) {

	int imap, omap, pixel, oW, oH, iH, iW;
	FL_MAP_PIXEL *pAccMap, *pTempMap;

	iH = pConvInfo->mapH + 2 * pConvInfo->pad;
	iW = pConvInfo->mapW + 2 * pConvInfo->pad;
	oH = (pConvInfo->mapH + 2 * pConvInfo->pad - pConvInfo->K + 1 + pConvInfo->stride - 1) / pConvInfo->stride;
	oW = (pConvInfo->mapW + 2 * pConvInfo->pad - pConvInfo->K + 1 + pConvInfo->stride - 1) / pConvInfo->stride;

	if ((NULL == (pAccMap = (FL_MAP_PIXEL *)malloc(oH * oW * sizeof(FL_MAP_PIXEL)))) ||
		(NULL == (pTempMap = (FL_MAP_PIXEL *)malloc(oW * oH * sizeof(FL_MAP_PIXEL))))) {
		REL_INFO("Malloc failed\n");
		return MALLOC_FAIL;
	}

	for ( omap = 0; omap < pConvInfo->nOutMaps; omap++) {
		memset(pAccMap, 0, oH * oW * sizeof(FL_MAP_PIXEL));
		for (imap = 0; imap < pConvInfo->nInMaps; imap++) {
			scalar_float_conv2D(pInMaps + imap * iW * iH,
				pKer + (omap * pConvInfo->nInMaps + imap) * pConvInfo->K * pConvInfo->K,
				pConvInfo,
				pTempMap);
			// Add the convolved map to accumulator
			for ( pixel = 0; pixel < oH * oW; pixel++) {
				pAccMap[pixel] += pTempMap[pixel];
			}
		}
		for(pixel = 0; pixel < oH * oW; pixel++) {			
			pOutMaps[omap * oW * oH + pixel] = pAccMap[pixel] + pBias[omap];
		}
	}
	// Free up heap
	free(pAccMap);	
	free(pTempMap);

	return SUCCESS;
}

/* \brief Extracts a block of each feature map from the given concatenated feature maps.
*
* @param pInMaps	: Concatenated feature maps from which a block to be extracted.
* @param rowOffset	: Row offset into each map from which a block to be extracted.
* @param colOffset	: Column offset into each map from which a block to be extracted.
* @param blkH		: Height of the block to be extracted.
* @param blkW		: Width feature block buffer per feature block
* @param W			: Width of the feature block to be copied into feature block buffer.
* @param pMapInfo	: Paramters regarding feature maps.
* @param pFeatureBlk: Buffer to store the concatenated feature blocks.
* @return			: void
*/
// TODO: Splitting across maps dimension is not considered here. Need to introduce the mapOffset here 
// for extracting block for subset of maps. As of now, it extracts blocks from all maps.
static void get_concat_feature_blocks(FP_MAP_PIXEL *pInMaps, int rowOffset,
	int colOffset, int blkH, int blkW, int W, CONV_INFO_T *pMapInfo, FP_MAP_PIXEL *pFeatureBlk) {

	int row, map;
	for ( row = 0; row < blkH; row++) {
		for ( map = 0; map < pMapInfo->nInMaps; map++) {
			memcpy(pFeatureBlk + row * blkW * pMapInfo->nInMaps + map * blkW,
			pInMaps + (row + rowOffset) * pMapInfo->nInMaps * pMapInfo->mapW + map * pMapInfo->mapW + colOffset,
			W * sizeof(FP_MAP_PIXEL));
		}
	}
}

/* \brief Extracts a block from given single feature map.
*
* @param pInMap 	: Feature map from which a block to be extracted.
* @param rowOffset	: Row offset into each map from which a block to be extracted.
* @param colOffset	: Column offset into each map from which a block to be extracted.
* @param blkH		: Height of the block to be extracted.
* @param blkW		: Width feature block buffer per feature block
* @param W			: Width of the feature block to be copied into feature block buffer.
* @param mapW		: Width of the full feature map
* @param pFeatureBlk: Buffer to store the concatenated feature blocks.
* @return			: void
*/
static void get_feature_block(FP_MAP_PIXEL *pInMap, int rowOffset,
	int colOffset, int blkH, int blkW, int W, int mapW, FP_MAP_PIXEL *pFeatureBlk) {

	int row;
	for ( row = 0; row < blkH; row++) {
			memcpy(pFeatureBlk + row * blkW,
			pInMap + (row + rowOffset) * mapW + colOffset,
			W * sizeof(FP_MAP_PIXEL));
	}
}

/* \brief This API will put the concatenated feature  blocks into corresponding position in the final complete
* feature map. The output maps can be concatenated or isolated, but all calls to arrange blocks of same layer 
* must follow same map arrangement mode failing which the maps will be jumpled up.
* Input feature blocks are assumed to be in concatenated mode if there are more than one map.
*
* @param pFeatureBlk	: Feature block containing block of output maps in concatenated mode.
* @param blkheight		: Height of feature block
* @param curBlkWidth	: Existing width of feature block. This includes some junk pixels resulted from border convolution
* @param targetBlkWidth	: Required block width. This must be less than or equal to curBlkWidth.
* @param nMaps			: No of stacked input feature blocks.
* @param mapW			: Full map width of individual featue map
* @param mapH			: Full map height
* @param rowOffset		: Row offset to place the feature blocks in the output map buffer.
* @param colOffset		: Column offset to place the feature blocks in the output map buffer.
* @param outMapLyt		: Output map layout. Either isolated OR cancatenated but must be same for the a layer.
* @param pOutMaps		: Output map buffer. Maps will be stored in specified layout.
* @return				: void
*/
static void stitch_feature_blocks(FP_MAP_PIXEL *pFeatureBlk,
		int blkHeight, int curBlkWidth, int targetBlkWidth, int nMaps, int mapW, int mapH,
		int rowOffset, int colOffset, MAP_LAYOUT_E outMapLyt, FP_MAP_PIXEL *pOutMaps) {
	int row, map, col;

	switch(outMapLyt) {
		case MAP_CONCAT:
			for (row = 0; row < blkHeight; row++) {
				for (map = 0; map < nMaps; map++) {
					/*memcpy(pMapCont->pMap + (row + rowOffset) * pMapCont->width * pMapCont->nMaps + map * pMapCont->width + colOffset, 
						pFeatureBlk + row * pMapCont->nMaps * curBlkWidth + map * curBlkWidth,
						targetBlkWidth * sizeof(FP_MAP_PIXEL));*/
					for (col = 0; col < targetBlkWidth; col++) {
						pOutMaps[(row + rowOffset) * mapW * nMaps + map * mapW + colOffset + col] = 
						pFeatureBlk[row * nMaps * curBlkWidth + map * curBlkWidth + col];
					}
				}
			}
			break;
		case MAP_ISOLATED:
			for ( row = 0; row < blkHeight; row++) {
				for ( map = 0; map < nMaps; map++) {
					memcpy(pOutMaps + map * mapW * mapH + (row + rowOffset) * mapW + colOffset,
						pFeatureBlk + row * curBlkWidth * nMaps + map * curBlkWidth,
						targetBlkWidth * sizeof(FP_MAP_PIXEL));
				}
			}
			break;
		default:
			REL_INFO("Invalid map arrangement mode\n");
	}

}

/* \brief Abstract convolution layer which uses MXP vector processor. This uses patch based approach to
* compute larger feature maps. The input maps must be in concatenated mode if number of input maps > 1
*
* @param pConvCtx	: Convolution layer context containing all necessary parameters.
* @param pInMaps	: Input maps buffer. As of now maps must be stored concatenated mode
* @param inMapLyt 	: Input map layout. As of now, this must be MAP_CONCAT.
* @return			: status of this convolution layer.
*/
APP_STATUS_E mxp_conv_layer(CONV_LYR_CTX_T *pConvCtx, FP_MAP_PIXEL *pInMaps, MAP_LAYOUT_E inMapLyt) {

	int col, W, imap, omap, oW, oH, pixel, iH, iW;
	FP_MAP_PIXEL *pInBlock, *pOutBlock;
	CONV_INFO_T blkConvInfo;
	int32_t *pMapAcc;

	// Block height and width are smaller than map heiht and width. Other parameters are same as that of conv layer info.
	memcpy(&blkConvInfo, &pConvCtx->convInfo, sizeof(CONV_INFO_T));
	blkConvInfo.mapH = pConvCtx->blkInfo.blkH;
	blkConvInfo.mapW = pConvCtx->blkInfo.blkW;
	blkConvInfo.nOutMaps = pConvCtx->blkInfo.optMaps;
	// This is to generate all conv outputs from 1 single input map at a time.
	blkConvInfo.nInMaps = 1;

	iH = pConvCtx->convInfo.mapH + 2 * pConvCtx->convInfo.pad;
	iW = pConvCtx->convInfo.mapW + 2 * pConvCtx->convInfo.pad;
	oH = (pConvCtx->convInfo.mapH + 2 * pConvCtx->convInfo.pad - pConvCtx->convInfo.K + 1 +
		pConvCtx->convInfo.stride - 1) / pConvCtx->convInfo.stride;
	oW = (pConvCtx->convInfo.mapW + 2 * pConvCtx->convInfo.pad - pConvCtx->convInfo.K + 1 +
		pConvCtx->convInfo.stride - 1) / pConvCtx->convInfo.stride;
	
	// TODO: Allocating memory for patches from all maps. Need to allocate memory for only optMaps in future.
	if ((NULL == (pInBlock = (FP_MAP_PIXEL *)malloc(pConvCtx->blkInfo.blkW * pConvCtx->blkInfo.blkH * 
		blkConvInfo.nInMaps * sizeof(FP_MAP_PIXEL)))) ||
		(NULL == (pOutBlock = (FP_MAP_PIXEL *)malloc(pConvCtx->blkInfo.blkW * pConvCtx->blkInfo.blkH *
		blkConvInfo.nOutMaps * sizeof(FP_MAP_PIXEL)))) ||
		(NULL == (pMapAcc = (int32_t *)malloc(oW * oH * pConvCtx->convInfo.nOutMaps * sizeof(int32_t))))) {
		REL_INFO("Memory allocation failed\n");
		return MALLOC_FAIL;
	}

	if (inMapLyt != MAP_ISOLATED) {
		// Need to re-arrage all maps. We are in trouble
		DBG_ASSERT(inMapLyt == MAP_ISOLATED);
	}

	memset(pMapAcc, 0, oW * oH * pConvCtx->convInfo.nOutMaps * sizeof(int32_t));
	for(imap = 0; imap < pConvCtx->convInfo.nInMaps; imap++) {
		W = pConvCtx->blkInfo.blkW;
		col = 0;
		// FIXME: Need another loop here over chunks of optMaps if optMaps != nOutMaps
		while(col < (oW - pConvCtx->convInfo.K + 1)) {
			// Last block will have width less than or equal to the original block width.
			if ( col + W >= iW) {
				W = iW - col;
			}
			// Extract a block of feature from all maps and concatenate them size by side
			get_feature_block(pInMaps + imap * iW * iH,
				0, col, pConvCtx->blkInfo.blkH,	pConvCtx->blkInfo.blkW, W, iW, pInBlock);

			// Convolve a block of input map with nOutMaps number of kernels. The outputs are
			// concatenated horizontally.
			vector_block_conv_layer(pInBlock,
				pConvCtx->ppExtKer + imap * pConvCtx->convInfo.K * pConvCtx->convInfo.K,
				&blkConvInfo,
				pOutBlock);

			// Stitch the feature blocks
			stitch_feature_blocks(pOutBlock,
				pConvCtx->blkInfo.blkH - pConvCtx->convInfo.K + 1, pConvCtx->blkInfo.blkW,
				W - pConvCtx->convInfo.K + 1, pConvCtx->convInfo.nOutMaps,
				oW, oH,
				0, col, MAP_ISOLATED, pConvCtx->pFixOutput);
			col += (pConvCtx->blkInfo.blkW - pConvCtx->convInfo.K + 1);
		}
		// Now the pFixOutMap contains contribution of 1 input map to all output maps.
		// The filtered outputs/contributions are in ISOLATED mode.
		// We accumulate contribution from each input map towards all output maps.

		// TODO: subsample if stride != 1
		for ( pixel = 0; pixel < oH * oW * pConvCtx->convInfo.nOutMaps; pixel++) {
			pMapAcc[pixel] += pConvCtx->pFixOutput[pixel];
		}
	}
	// add bias
	for(omap = 0; omap < pConvCtx->convInfo.nOutMaps; omap++) {
		for (pixel = 0; pixel < oH * oW; pixel++) {
			pConvCtx->pFixOutput[omap * oW * oH + pixel] = (FP_MAP_PIXEL)(pMapAcc[omap * oW * oH + pixel] + pConvCtx->pFixBias[omap]);
		}
	}
	pConvCtx->mapLyt = MAP_ISOLATED;

	free(pInBlock);
	free(pOutBlock);
	free(pMapAcc);
	return SUCCESS;
}

/* \brief A generic convolution layer API supporting float/fixed point and scalar/vector inplementation.
*
* @param pConvCtx		: Convolution layer context.
* @param pFloatInMaps	: Input feature maps in floating point.
* @param pInFixMaps		: Input feature maps in fixed point format.
* @param inMapLyt  		: Input feature map storage layout. isolated/concatenated.
* @return				: status of conv layer computation
*/
APP_STATUS_E cnn_conv_layer(CONV_LYR_CTX_T *pConvCtx, FL_MAP_PIXEL *pFloatInMaps, FP_MAP_PIXEL *pFixInMaps, MAP_LAYOUT_E inMapLyt) {

	APP_STATUS_E retStatus = FAILED;
	switch(pConvCtx->lyrArithMode) {
		case FLOAT_POINT:
			switch(pConvCtx->optType) {
				case SCALAR:
					DBG_INFO("Scalar Floating point conv layer\n");
					retStatus = scalar_float_conv_layer(pFloatInMaps,
						pConvCtx->pFloatKer, pConvCtx->pFloatBias,
						&pConvCtx->convInfo,
						pConvCtx->pFloatOutput);
					break;
				case VECTOR_NEON:
					REL_INFO("No supported as of now\n");
					break;
				case VECTOR_MXP:
					REL_INFO("MXP does not support floating point\n");
					break;
				default:
					REL_INFO("Invalid optimization type\n");
					break;
			}
			break;
		case FIXED_POINT:
			switch(pConvCtx->optType) {
				case SCALAR:
					DBG_INFO("Scalar Fixed point conv layer\n");
					retStatus = scalar_fix_conv_layer(pFixInMaps,
						pConvCtx->pFixKer, pConvCtx->pFixBias,
						&pConvCtx->convInfo,
						pConvCtx->pFixOutput);
					break;
				case VECTOR_NEON:
					REL_INFO("No supported as of now\n");
					break;
				case VECTOR_MXP:
					DBG_INFO("MXP Vector Fixed point conv layer\n");
					retStatus = mxp_conv_layer(pConvCtx, pFixInMaps, inMapLyt);	
					break;
				default:
					REL_INFO("Invalid optimization type\n");
			}
			break;
		default:
			break;
	}
	return retStatus;
}
