#include "common.h"
#include "conv_layer.h"
#include "debug_control.h"
#include <string.h>
#include "struct_defs.h"


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
			// TODO: check for overflow and do saturation
			pOutMap[row/pConvInfo->stride * oW + col/pConvInfo->stride] = (FP_MAP_PIXEL)sop;
		}
	}

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

/* \brief A generic convolution layer API supporting float/fixed point and scalar/vector inplementation.
*
* @param pConvCtx		: Convolution layer context.
* @param pFloatInMaps	: Input feature maps in floating point.
* @param pInFixMaps		: Input feature maps in fixed point format.
* @param inMapLyt  		: Input feature map storage layout. isolated/concatenated.
* @return				: status of conv layer computation
*/
APP_STATUS_E cnn_conv_layer(CONV_LYR_CTX_T *pConvCtx, FL_MAP_PIXEL *pFloatInMaps, MAP_LAYOUT_E inMapLyt) {

	APP_STATUS_E retStatus = FAILED;
	switch(pConvCtx->lyrArithMode) {
		case FLOAT_POINT:
			switch(pConvCtx->optType) {
				case SCALAR:
					// DBG_INFO("Scalar Floating point conv layer\n");
					retStatus = scalar_float_conv_layer(pFloatInMaps,
						pConvCtx->pFloatKer, pConvCtx->pFloatBias,
						&pConvCtx->convInfo,
						pConvCtx->pFloatOutput);
					break;
				default:
					REL_INFO("Invalid optimization type\n");
					break;
			}
			break;
		default:
			REL_INFO("Only FP arithmetic supported.\n");
			break;
	}
	return retStatus;
}
