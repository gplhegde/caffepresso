#include "pool_layer.h"
#include "debug_control.h"
#include "vbx.h"
#include "vbx_port.h"
#include <float.h>
/* \brief Pooling layer using MXP. The feature maps are in fixed point.
*
* @param pInMaps 	: Input feature maps for the pooling layer.
* @param pPoolInfo	: Parameters for pooling layer.
* @param mapLyr		: Input map layout. Either isolated or concatenated.
* @param pOutMaps 	: Buffer to store the output maps.
* @return			: Pooling layer status
*/
APP_STATUS_E vector_fix_pool_layer(FP_MAP_PIXEL *pInMaps, POOL_INFO_T *pPoolInfo, MAP_LAYOUT_E mapLyt, FP_MAP_PIXEL *pOutMaps) {

	int vl, row, map, iW, iH, oW, oH, maxVlen;
	FP_MAP_PIXEL *spRowEven, *spRowOdd, *spTemp;

	iW = pPoolInfo->mapW;
	iH = pPoolInfo->mapH;
	// Vector implementation supports only 2x2 window size and stride of 2
	REL_ASSERT((pPoolInfo->stride == 2) && (pPoolInfo->winSize = 2));
	oH = (pPoolInfo->mapH + 2*pPoolInfo->pad - pPoolInfo->winSize + 1 + pPoolInfo->stride - 1)/ pPoolInfo->stride;
	oW = (pPoolInfo->mapW + 2*pPoolInfo->pad - pPoolInfo->winSize + 1 + pPoolInfo->stride - 1)/ pPoolInfo->stride;
	
	maxVlen = (64*1024) / 3;	//TODO: Compute max vector length using available SP and buffer requirement
	vbx_dcache_flush_all();

	switch( pPoolInfo->poolType) {
		case MAX_POOL:
			switch (mapLyt) {
				case MAP_ISOLATED:
					if (iW <= maxVlen) {
						vl = iW;
					} else {
						//TODO: normally map width will be less than maxVlen
						REL_INFO("TODO: Handle map width > maxVectorLength\n");
						REL_ASSERT(1 < 0);
					}

					// Allocate SP buffer
					if ((NULL == (spRowEven = (FP_MAP_PIXEL *)vbx_sp_malloc(vl * sizeof(FP_MAP_PIXEL)))) ||
						(NULL == (spRowOdd = (FP_MAP_PIXEL *)vbx_sp_malloc(vl * sizeof(FP_MAP_PIXEL))))  ||
						(NULL == (spTemp = (FP_MAP_PIXEL *)vbx_sp_malloc(vl * sizeof(FP_MAP_PIXEL))))
					) {
						REL_INFO("Failed to allocate SP buffer\n");
						return SP_MALLOC_FAIL;
					}

					vbx_set_vl(vl);
					for (map = 0; map < pPoolInfo->nMaps; map++) {
						for (row = 0; row < (iH-1); row += 2) {
							// DMA 2 map rows
							vbx_dma_to_vector(spRowEven, pInMaps + map * iW * iH + row * iW, iW * sizeof(FP_MAP_PIXEL));
							vbx_dma_to_vector(spRowOdd, pInMaps + map * iW * iH + (row+1) * iW, iW * sizeof(FP_MAP_PIXEL));
							// Max pool across 2 rows
							vbx(VVH, VSUB, spTemp, spRowEven, spRowOdd);
							vbx(VVH, VCMV_LTZ, spRowEven, spRowOdd, spTemp);
							// max pool across adjacent columns
							vbx(VVH, VSUB, spTemp, spRowEven, spRowEven + 1);	
							vbx(VVH, VCMV_LTZ, spRowEven, spRowEven + 1, spTemp);
							// discard alternate elements
							vbx(VVWH, VMOV, spRowEven, (uint32_t *)spRowEven, 0);
							// DMA the result back to host memory
							vbx_dma_to_host(pOutMaps + map * oW * oH + (row / 2) * oW, spRowEven, oW * sizeof(FP_MAP_PIXEL));
						}
					}	
					break;
				case MAP_CONCAT:
					// Map width must be even for concatenated layout for pooling to be correct.
					REL_ASSERT(iW % 2 == 0);

					if (pPoolInfo->nMaps * iW <= maxVlen) {
						vl = pPoolInfo->nMaps * iW;
					} else {
						REL_INFO("TODO: Handle concat map width > max vector length\n");
						REL_ASSERT(1 < 0);
					}

					// Allocate SP buffer
					if ((NULL == (spRowEven = (FP_MAP_PIXEL *)vbx_sp_malloc(vl * sizeof(FP_MAP_PIXEL)))) ||
						(NULL == (spRowOdd = (FP_MAP_PIXEL *)vbx_sp_malloc(vl * sizeof(FP_MAP_PIXEL))))  ||
						(NULL == (spTemp = (FP_MAP_PIXEL *)vbx_sp_malloc(vl * sizeof(FP_MAP_PIXEL))))
					) {
						REL_INFO("Failed to allocate SP buffer\n");
						return SP_MALLOC_FAIL;
					}

					vbx_set_vl(vl);
					for (row = 0; row < (iH-1); row += 2) {
						// DMA 2 rows of concatenated map to SP
						vbx_dma_to_vector(spRowEven, pInMaps + row * iW * pPoolInfo->nMaps, 
							iW * pPoolInfo->nMaps * sizeof(FP_MAP_PIXEL));
						vbx_dma_to_vector(spRowOdd, pInMaps + (row + 1) * iW * pPoolInfo->nMaps, 
							iW * pPoolInfo->nMaps * sizeof(FP_MAP_PIXEL));
						// Max pool across 2 rows
						vbx(VVH, VSUB, spTemp, spRowEven, spRowOdd);
						vbx(VVH, VCMV_LTZ, spRowEven, spRowOdd, spTemp);
						// max pool across adjacent columns
						vbx(VVH, VSUB, spTemp, spRowEven, spRowEven + 1);	
						vbx(VVH, VCMV_LTZ, spRowEven, spRowEven + 1, spTemp);
						// discard alternate elements
						vbx(VVWH, VMOV, spRowEven, (uint32_t *)spRowEven, 0);
						// DMA the result back to host memory
						vbx_dma_to_host(pOutMaps + (row / 2) * oW * pPoolInfo->nMaps, spRowEven,
							oW * pPoolInfo->nMaps * sizeof(FP_MAP_PIXEL));
					}
					break;
				default:
					REL_INFO("Invalid map arrangement mode\n");
					return FAILED;
			}
			break;
		case AVG_POOL:
			REL_INFO("Not implemented yet\n");
			break;
		default:
			REL_INFO("Invalid pooling type\n");
			break;
	}
	// Wait for final DMA completion
	vbx_sync();
	// Release scratchpad
	vbx_sp_free();	
	return SUCCESS;
}
//===============================================================
// END OF MXP VECTOR IMPLEMENTATION
//===============================================================

/* \brief Scalar fixed point version of pooling. 
*
* @param pInMaps	: Input feature maps.
* @param pPoolInfo	: Pooling parameters. Refer to structure in pool_layer.h
* @param mapLyt		: Input map layout.
* @param pOutMaps 	: Output feature map buffer. Maps are stored in same format as input.
* @return 			: Pooling status.
*/
APP_STATUS_E scalar_fix_pool_layer(FP_MAP_PIXEL *pInMaps, POOL_INFO_T *pPoolInfo, MAP_LAYOUT_E mapLyt, FP_MAP_PIXEL *pOutMaps) {

	int map, oW, oH, row, col, iW, iH, i, j;	
	FP_MAP_PIXEL max;
	int sum;

	iW = pPoolInfo->mapW;
	iH = pPoolInfo->mapH;
	oH = (pPoolInfo->mapH + 2*pPoolInfo->pad - pPoolInfo->winSize + 1 + pPoolInfo->stride - 1)/ pPoolInfo->stride;
	oW = (pPoolInfo->mapW + 2*pPoolInfo->pad - pPoolInfo->winSize + 1 + pPoolInfo->stride - 1)/ pPoolInfo->stride;
	// FIXME: below expressions are wrong
	switch(pPoolInfo->poolType) {
		case MAX_POOL:
			DBG_INFO("Scalar fix max pool\n");
			switch(mapLyt) {
				case MAP_ISOLATED:
					for (map = 0; map < pPoolInfo->nMaps; map++) {
						for(row = 0; row < iH - pPoolInfo->winSize + 1; row += pPoolInfo->stride) {
							for(col = 0; col < iW - pPoolInfo->winSize + 1; col += pPoolInfo->stride) {
								max = -32768; // TODO: define -ve min of FP_MAP_PIXEL type and use here.
								for ( i = 0; i < pPoolInfo->winSize; i++) {
									for ( j = 0; j < pPoolInfo->winSize; j++) {
										max = MAX(max, pInMaps[map * iW * iH + (row + i) * iW + col + j]);
									}
								}
								pOutMaps[map*oW*oH + oW * (row / pPoolInfo->stride) + (col / pPoolInfo->stride)] = max;
							}
						}
					}
					break;
				case MAP_CONCAT:
					REL_INFO("MAP CONCAT mode is not supported as of now. Isolate maps and use MAP_ISOLATED mode\n");
					break;
				default:
					REL_INFO("Invalid map layout\n");
					return FAILED;
			}
			break;
		case AVG_POOL:
			switch(mapLyt) {
				case MAP_ISOLATED:		// all rows of a map are in continuous memory
					for (map = 0; map < pPoolInfo->nMaps; map++) {
						for(row = 0; row < iH; row += pPoolInfo->stride) {
							for(col = 0; col < iW; col += pPoolInfo->stride) {
								sum = 0;
								for ( i = 0; i < pPoolInfo->winSize; i++) {
									for ( j = 0; j < pPoolInfo->winSize; j++) {
										sum += pInMaps[map * iW * iH + (row + i) * iW + col + j];
									}
								}
								pOutMaps[map*oW*oH + oW * (row / pPoolInfo->stride) + (col / pPoolInfo->stride)] =
									(FP_MAP_PIXEL)(sum / (pPoolInfo->winSize * pPoolInfo->winSize));
							}
						}
					}
					break;
				case MAP_CONCAT:
					REL_INFO("MAP CONCAT mode is not supported as of now. Isolate maps and use MAP_ISOLATED mode\n");
					break;
				default:
					REL_INFO("Invalid map layout\n");
					return FAILED;
			}
			break;
		default:
			break;
	}

	return SUCCESS;
}

/* \brief Scalar floating point version of pooling. 
*
* @param pInMaps	: Input feature maps in floating point format.
* @param pPoolInfo	: Pooling parameters. Refer to structure in pool_layer.h
* @param mapLyt		: Input map layout.
* @param pOutMaps 	: Output feature map buffer. Maps are stored in same format as input.
* @return 			: Pooling status.
*/
APP_STATUS_E scalar_float_pool_layer(FL_MAP_PIXEL *pInMaps, POOL_INFO_T *pPoolInfo, MAP_LAYOUT_E mapLyt, FL_MAP_PIXEL *pOutMaps) {

	int map, oW, oH, row, col, iW, iH, i, j;
	FL_MAP_PIXEL max;
	FL_MAP_PIXEL sum;
	
	iW = pPoolInfo->mapW;
	iH = pPoolInfo->mapH;
	oH = (pPoolInfo->mapH + 2*pPoolInfo->pad - pPoolInfo->winSize + 1 + pPoolInfo->stride - 1)/ pPoolInfo->stride;
	oW = (pPoolInfo->mapW + 2*pPoolInfo->pad - pPoolInfo->winSize + 1 + pPoolInfo->stride - 1)/ pPoolInfo->stride;
	switch(pPoolInfo->poolType) {
		case MAX_POOL:
			switch(mapLyt) {
				case MAP_ISOLATED:
					for (map = 0; map < pPoolInfo->nMaps; map++) {
						for(row = 0; row < iH - pPoolInfo->winSize + 1; row += pPoolInfo->stride) {
							for(col = 0; col < iW - pPoolInfo->winSize + 1; col += pPoolInfo->stride) {
								max = -FLT_MAX;
								for ( i = 0; i < pPoolInfo->winSize; i++) {
									for ( j = 0; j < pPoolInfo->winSize; j++) {
										max = MAX(max, pInMaps[map * iW * iH + (row + i) * iW + col + j]);
									}
								}
								pOutMaps[map*oW*oH + oW * (row / pPoolInfo->stride) + (col / pPoolInfo->stride)] = max;
							}
						}
					}
					break;
				case MAP_CONCAT:
					REL_INFO("MAP CONCAT mode is not supported as of now. Isolate maps and use MAP_ISOLATED mode\n");
					break;
				default:
					REL_INFO("Invalid map layout\n");
					return FAILED;
			}
			break;
		case AVG_POOL:
			switch(mapLyt) {
				case MAP_ISOLATED:		// all rows of a map are in continuous memory
					for (map = 0; map < pPoolInfo->nMaps; map++) {
						for(row = 0; row < iH - pPoolInfo->winSize + 1; row += pPoolInfo->stride) {
							for(col = 0; col < iW - pPoolInfo->winSize + 1; col += pPoolInfo->stride) {
								sum = 0.0;
								for ( i = 0; i < pPoolInfo->winSize; i++) {
									for ( j = 0; j < pPoolInfo->winSize; j++) {
										sum += pInMaps[map * iW * iH + (row + i) * iW + col + j];
									}
								}
								pOutMaps[map*oW*oH + oW * (row / pPoolInfo->stride) + (col / pPoolInfo->stride)] =
									(FL_MAP_PIXEL)(sum / (pPoolInfo->winSize * pPoolInfo->winSize));
							}
						}
					}
					break;
				case MAP_CONCAT:
					REL_INFO("MAP CONCAT mode is not supported as of now. Isolate maps and use MAP_ISOLATED mode\n");
					break;
				default:
					REL_INFO("Invalid map layout\n");
					return FAILED;
			}
			break;
		default:
			break;
	}

	return SUCCESS;
}

/* \brief Abstract pooling API. This supports float/fixed point and scalar/vector implementations.
*
* @param pPoolCtx		: Pooling layer context containing all necessary information of a pooling layer.
* @param pFloatInMaps	: Floating point input maps.
* @param pFixInMaps		: Fixed point input maps.
* @param inMapLyt		: Input map storage layout
* @return				: Pooling layer status.
*/
APP_STATUS_E cnn_pool_layer(POOL_LYR_CTX_T *pPoolCtx, FL_MAP_PIXEL *pFloatInMaps, FP_MAP_PIXEL *pFixInMaps, MAP_LAYOUT_E inMapLyt) {

	APP_STATUS_E retStatus = FAILED;
    switch(pPoolCtx->lyrArithMode) {
        case FLOAT_POINT:
            switch(pPoolCtx->optType) {
                case SCALAR:
					DBG_INFO("Scalar float pooling\n");
                    retStatus = scalar_float_pool_layer(pFloatInMaps,
                        &pPoolCtx->poolInfo, inMapLyt,
                        pPoolCtx->pFloatOutput);
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
            switch(pPoolCtx->optType) {
                case SCALAR:
					DBG_INFO("Scalar fixed point pooling\n");
                    retStatus = scalar_fix_pool_layer(pFixInMaps,
                        &pPoolCtx->poolInfo, inMapLyt,
                        pPoolCtx->pFixOutput);
                    break;
                case VECTOR_NEON:
                    REL_INFO("No supported as of now\n");
                    break;
                case VECTOR_MXP:
					DBG_INFO("MXP vector fixed point pooling\n");
					REL_ASSERT(pPoolCtx->poolInfo.winSize == 2 && pPoolCtx->poolInfo.stride == 2);
                    retStatus = vector_fix_pool_layer(pFixInMaps, &pPoolCtx->poolInfo, inMapLyt, pPoolCtx->pFixOutput);
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
