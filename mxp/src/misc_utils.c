#include "misc_utils.h"
#include "debug_control.h"
#include <math.h>
#include <stdlib.h>
#include "conv_layer.h"
#include "pool_layer.h"

float mean_normalize(uint8_t *pImg, int H, int W, float *var, float *pNormImg){
	uint32_t sum, pixel;
	float mean, normPixel;
	
	sum = 0;
	for (pixel = 0; pixel < H * W; pixel++){
		sum += pImg[pixel];
	}

	mean = sum / (H * W);
	*var = 0;
	// variance
	for (pixel = 0; pixel < H * W; pixel++){
		*var += pow((pImg[pixel] - mean), 2);
	}
	// mean normalize
	for (pixel = 0; pixel < H * W; pixel++){
		normPixel = ((float)(pImg[pixel]) - mean)/ sqrt(*var + 1e-8);	
		pNormImg[pixel] = normPixel;	
	}
	
	return mean;
}

void float_to_fix_data(FL_MAP_PIXEL *pFltData, int N, int fracBits, FP_MAP_PIXEL *pFixData) {
	int pix;
	for (pix = 0; pix < N; pix++) {
		pFixData[pix] = (FP_MAP_PIXEL) floor(pFltData[pix] * (1 << fracBits));
	}
}


void fix16_to_float_data(FP_MAP_PIXEL *pFixData, int N, int fracBits, float *pFltData) {
	int pix;
	for (pix = 0; pix < N; pix++) {
		pFltData[pix] = (float) pFixData[pix] / (1 << fracBits); 	// FIXME: Rounding is not considered
	}
}

void mean_denormalize(float *pNormImg, int H, int W, float mean, uint8_t maxPixel, char *pDenormImg) {
	float pixel;
	int p;
	for (p = 0; p < H * W; p++) {
		pixel = pNormImg[p] * ((float)(maxPixel + 1) - mean);
		pixel += mean;
		DBG_ASSERT(pixel >= 0 && pixel <= 255);
		pDenormImg[p] = (char) pixel;
	}
}

void print_fix_img(FP_MAP_PIXEL *pImg, int H, int W) {
	int r, c;
	DBG_INFO("-------------------------------------\n");
	for (r = 0; r < H; r++) {
		for (c = 0; c < W; c++) {
			printf("%d\t", pImg[r * W + c]);
		}
		printf("\n");
	}
	DBG_INFO("-------------------------------------\n");
}

void print_fix_map(FP_MAP_PIXEL *pMap, int nMaps, int H, int W, int map, MAP_LAYOUT_E mode) {
	int r, c;
	DBG_INFO("-------------------------------------\n");
	for (r = 0; r < H; r++) {
		for (c = 0; c < W; c++) {
			if (mode == MAP_CONCAT) {
				printf("%d\t", pMap[r * W * nMaps + map * W +  c]);
			} else {
				printf("%d\t", pMap[map*H*W + r * W + c]);
			}
		}
		printf("\n");
	}
	DBG_INFO("-------------------------------------\n");
}
void print_float_img(FL_MAP_PIXEL *pImg, int H, int W) {
    int r, c;
	DBG_INFO("-------------------------------------\n");
    for (r = 0; r < H; r++) {
        for (c = 0; c < W; c++) {
            printf("%E\t", pImg[r * W + c]);
        }
        printf("\n");
    }
	DBG_INFO("-------------------------------------\n");
}

float float_img_diff(FL_MAP_PIXEL *pImg1, FL_MAP_PIXEL *pImg2, int H, int W, FL_MAP_PIXEL *pRes) {

    int r, c;
	float mean;
	mean = 0;
    for (r = 0; r < H; r++) {
        for (c = 0; c < W; c++) {
            pRes[r * W + c] = pImg1[r * W + c] - pImg2[r * W + c];
			mean +=  pow(pRes[r * W + c], 2);
        }
    }
	return sqrt(mean / (H * W));

}

void feat_maps_mse(void *pLyrCtx, CNN_LAYER_TYPE_E lyrType, int nFracBits) {

	int mapW, mapH, map;
	float *pFixFloatMap, mse;
	switch(lyrType) {
		case CONV:
		{
			CONV_LYR_CTX_T *pConvCtx = (CONV_LYR_CTX_T *)pLyrCtx;
			mapH = pConvCtx->convInfo.mapH - pConvCtx->convInfo.K + 1;
			mapW = pConvCtx->convInfo.mapW - pConvCtx->convInfo.K + 1;
			pFixFloatMap = (float *) malloc(mapH * mapW * sizeof(float));

			for (map = 0; map < pConvCtx->convInfo.nOutMaps; map++) {
				fix16_to_float_data(pConvCtx->pFixOutput + map * mapH * mapW,
					mapH * mapW,  nFracBits, pFixFloatMap);
				mse = float_img_diff(pConvCtx->pFloatOutput + map * mapH * mapW,
					pFixFloatMap, mapH, mapW, pFixFloatMap);
				printf("Map = %d\tMSE = %f\n", map, mse);			
			}	
			break;
		}
		case POOL:
		{
			POOL_LYR_CTX_T *pPoolCtx = (POOL_LYR_CTX_T *)pLyrCtx;
			mapH = pPoolCtx->poolInfo.mapH / pPoolCtx->poolInfo.stride;
			mapW = pPoolCtx->poolInfo.mapW / pPoolCtx->poolInfo.stride;
			pFixFloatMap = (float *) malloc(mapH * mapW * sizeof(float));
			for (map = 0; map < pPoolCtx->poolInfo.nMaps; map++) {
				fix16_to_float_data(pPoolCtx->pFixOutput + map * mapH * mapW,
					mapH * mapW,  nFracBits, pFixFloatMap);
				mse = float_img_diff(pPoolCtx->pFloatOutput + map * mapH * mapW,
					pFixFloatMap, mapH, mapW, pFixFloatMap);
				printf("Map = %d\tMSE = %f\n", map, mse);			
			}	
			break;
		}
	}
	free(pFixFloatMap);

}

void print_org_img(uint8_t *pImg, int H, int W) {
    int r, c;
	DBG_INFO("-------------------------------------\n");
    for (r = 0; r < H; r++) {
        for (c = 0; c < W; c++) {
            printf("%d\t", pImg[r * W + c]);
        }
        printf("\n");
    }
	DBG_INFO("-------------------------------------\n");
}

void compare_fix_img(FP_MAP_PIXEL *pImg1, FP_MAP_PIXEL *pImg2, int H, int W) {
	int r, c, mismatchCount;

	mismatchCount = 0;
	for (r = 0; r < H ; r++) {
		for (c = 0; c < W; c++) {
			if (pImg1[r * W + c] != pImg2 [r * W + c]) {
				printf("Pixel mismatch at row = %d, col = %d\n", r, c);
				mismatchCount++;
			}
		}
	}
	if (mismatchCount == 0) {
		DBG_INFO("Two maps are matching\n");
	}
}

void print_fix_img_diff(FP_MAP_PIXEL *pImg1, FP_MAP_PIXEL *pImg2, int H, int W) {
        int r, c;
        for (r = 0; r < H ; r++) {
                for (c = 0; c < W; c++) {
                       printf("%d,   ", pImg1[r * W + c] - pImg2 [r * W + c]);
                }
		printf("\n");
        }
}

void generate_random_img(uint8_t *pImg, int H, int W) {
	int pix;
	for (pix = 0; pix < H * W; pix++) {
		pImg[pix] = rand() % 256;	
	}
}

void generate_random_data(float *pData, int N, int seed) {
	int k;
	srand(seed);
	for (k = 0; k < N; k++) {
		pData[k] = 2*((float)rand() / RAND_MAX) - 1.0;
	}
}

void check_cmp_status(CMP_STATUS_T *pStatus) {
	if(pStatus->flag == TEST_PASS) {
		printf("PASSED\n");
	} else {
		printf("Data mismatch occured\n");
		printf("Map: %d\tRow: %d\tCol: %d\n", pStatus->misMap, pStatus->misRow, pStatus->misCol);
		printf("TEST FAILED\n");
		exit(-1);
	}

}


