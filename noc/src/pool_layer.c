#include "pool_layer.h"
#include "debug_control.h"
#include <float.h>

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

/* Pooling API -- floating-point only
*
* @param pPoolCtx		: Pooling layer context containing all necessary information of a pooling layer.
* @param pFloatInMaps	: Floating point input maps.
* @param inMapLyt		: Input map storage layout
* @return				: Pooling layer status.
*/
APP_STATUS_E cnn_pool_layer(POOL_LYR_CTX_T *pPoolCtx, FL_MAP_PIXEL *pFloatInMaps, MAP_LAYOUT_E inMapLyt) {

	APP_STATUS_E retStatus = FAILED;
    switch(pPoolCtx->lyrArithMode) {
        case FLOAT_POINT:
            switch(pPoolCtx->optType) {
                case SCALAR:
					// DBG_INFO("Scalar float pooling\n");
                    retStatus = scalar_float_pool_layer(pFloatInMaps,
                        &pPoolCtx->poolInfo, inMapLyt,
                        pPoolCtx->pFloatOutput);
                    break;
                default:
                    REL_INFO("Invalid optimization type\n");
                    break;
            }
            break;
		default:
			REL_INFO("Only Floating Point Arithmetic Mode is Supported\n");
            break;
    }
    return retStatus;
}
