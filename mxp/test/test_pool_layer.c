#include "unit_test.h"
#include "pool_layer.h"
#include <float.h>
#include "misc_utils.h"
#include <math.h>

FL_MAP_PIXEL *pRefFltOutput;

void compute_pool_ref(POOL_LYR_CTX_T *pCtx, FL_MAP_PIXEL *pFloatInput) {
	int map, row, col, oH, oW, iH, iW;
	iH = pCtx->poolInfo.mapH + 2*pCtx->poolInfo.pad;
	iW = pCtx->poolInfo.mapW + 2*pCtx->poolInfo.pad;

	oH = (pCtx->poolInfo.mapH + 2*pCtx->poolInfo.pad - pCtx->poolInfo.winSize + 1 + pCtx->poolInfo.stride - 1)/ pCtx->poolInfo.stride;
	oW = (pCtx->poolInfo.mapW + 2*pCtx->poolInfo.pad - pCtx->poolInfo.winSize + 1 + pCtx->poolInfo.stride - 1)/ pCtx->poolInfo.stride;
	for(map = 0; map < pCtx->poolInfo.nMaps; map++) {
		for(row = 0; row < oH; row++) {
			for(col = 0; col < oW; col++) {
				int hstart = row * pCtx->poolInfo.stride;
				int wstart = col * pCtx->poolInfo.stride;
				int hend = hstart + pCtx->poolInfo.winSize;
				int wend = wstart + pCtx->poolInfo.winSize;
				FL_MAP_PIXEL maxFltVal = -FLT_MAX;
				for(int r = hstart; r < hend; r++) {
					for(int c = wstart; c < wend; c++) {
						maxFltVal = MAX(maxFltVal , pFloatInput[(map*iH + r)*iW + c]);
					}
				}
				pRefFltOutput[(map*oH + row)*oW + col] = maxFltVal;
			}
		}
	}
}

CMP_STATUS_T compare_pool_out(POOL_LYR_CTX_T *pCtx, FL_MAP_PIXEL *pOutput) {
	int oW, oH;
	CMP_STATUS_T status;
	status.misMap = -1;
	status.misRow = -1;
	status.misCol = -1;
	status.flag = TEST_PASS;

	oH = (pCtx->poolInfo.mapH + 2*pCtx->poolInfo.pad - pCtx->poolInfo.winSize + 1 + pCtx->poolInfo.stride - 1)/ pCtx->poolInfo.stride;
	oW = (pCtx->poolInfo.mapW + 2*pCtx->poolInfo.pad - pCtx->poolInfo.winSize + 1 + pCtx->poolInfo.stride - 1)/ pCtx->poolInfo.stride;
	for(int	map = 0; map < pCtx->poolInfo.nMaps; map++) {
		for(int row = 0; row < oH; row++) {
			for(int col = 0; col < oW; col++) {
				if(fabs(pOutput[(map*oH + row)*oW + col] - pRefFltOutput[(map*oH + row)*oW + col]) > ERR_THRESHOLD) {
					status.misMap = map;
					status.misRow = row;
					status.misCol = col;
					status.flag = TEST_FAIL;
					return status;
				}
			}
		}
	}
	return status;
}

TEST_STATUS_E test_pool_layer() {
	int noInputs, inputHeight, inputWidth, winSize, stride, pad;
	int outWidth, outHeight;
	POOL_TYPE_E poolType;
	FL_MAP_PIXEL *pFltInput;
	FP_MAP_PIXEL *pFixInput;
	
	CMP_STATUS_T status;

	POOL_LYR_CTX_T poolCtx;

	printf("Testing POOL Layer\n");
	noInputs = 3;
	inputHeight = 16;
	inputWidth = 16;
	winSize = 2;
	stride = 2;
	pad = 0;
	poolType = MAX_POOL;

	outHeight = (inputHeight + 2*pad - winSize + 1 + stride - 1)/ stride;
	outWidth = (inputWidth + 2*pad - winSize + 1 + stride - 1)/ stride;
	// populate pool layer context
	poolCtx.poolInfo = (POOL_INFO_T){inputHeight, inputWidth, noInputs, winSize, stride, pad, poolType};
	poolCtx.mapLyt = MAP_ISOLATED;
	poolCtx.lyrArithMode = FLOAT_POINT; 
	poolCtx.optType = SCALAR;

	// input and output buffer allocation	
	poolCtx.pFloatOutput = malloc(outHeight * outWidth * noInputs * sizeof(FL_MAP_PIXEL));
	poolCtx.pFixOutput = malloc(outHeight * outWidth * noInputs * sizeof(FP_MAP_PIXEL));
	pRefFltOutput = malloc(outHeight * outWidth * noInputs * sizeof(FL_MAP_PIXEL));
	pFltInput = malloc((inputHeight + 2*pad)*(inputWidth + 2*pad) * noInputs * sizeof(FL_MAP_PIXEL));
	pFixInput = malloc((inputHeight + 2*pad)*(inputWidth + 2*pad) * noInputs * sizeof(FP_MAP_PIXEL));

	// random input
	generate_random_data(pFltInput, (inputHeight + 2*pad)*(inputWidth + 2*pad) * noInputs, 123);
	float_to_fix_data(pFltInput, (inputHeight + 2*pad)*(inputWidth + 2*pad) * noInputs, 15, pFixInput);

	//print_float_img(pFltInput, inputHeight, inputWidth);

	cnn_pool_layer(&poolCtx, pFltInput, pFixInput, MAP_ISOLATED);
	poolCtx.lyrArithMode = FIXED_POINT; 
	//poolCtx.optType = VECTOR_MXP;
	cnn_pool_layer(&poolCtx, pFltInput, pFixInput, MAP_ISOLATED);

	//print_float_img(poolCtx.pFloatOutput, outHeight, outWidth);

	compute_pool_ref(&poolCtx, pFltInput);
	//print_float_img(pRefFltOutput, outHeight, outWidth);

	status = compare_pool_out(&poolCtx, poolCtx.pFloatOutput);
	check_cmp_status(&status);

	fix16_to_float_data(poolCtx.pFixOutput, outHeight * outWidth * noInputs, 15, poolCtx.pFloatOutput);
	//print_float_img(poolCtx.pFloatOutput, outHeight, outWidth);
	status = compare_pool_out(&poolCtx, poolCtx.pFloatOutput);
	check_cmp_status(&status);

	free(poolCtx.pFloatOutput);
	free(poolCtx.pFixOutput);
	free(pRefFltOutput);
	free(pFixInput);
	free(pFltInput);
	return status.flag;
}
