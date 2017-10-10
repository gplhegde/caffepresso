#ifndef _MISC_UTILS_H_
#define _MISC_UTILS_H_

#include "common.h"
#include "struct_defs.h"
#include "conv_layer.h"
#include "caffe_frontend.h"

float mean_normalize(IMAGE_T *pImg, int H, int W, float *var, float *pNormImg);

void float_to_fix_data(FL_MAP_PIXEL *pFltData, int N, int fracBits, FP_MAP_PIXEL *pFixData);

void print_org_img(IMAGE_T *pImg, int H, int W);

void print_fix_img(FP_MAP_PIXEL *pImg, int H, int W);

void print_fix_map(FP_MAP_PIXEL *pMap, int nMaps, int H, int W, int map, MAP_LAYOUT_E mode);

void print_float_img(FL_MAP_PIXEL *pImg, int H, int W);

void compare_fix_img(FP_MAP_PIXEL *pImg1, FP_MAP_PIXEL *pImg2, int H, int W);

void print_fix_img_diff(FP_MAP_PIXEL *pImg1, FP_MAP_PIXEL *pImg2, int H, int W);

float float_img_diff(FL_MAP_PIXEL *pImg1, FL_MAP_PIXEL *pImg2, int H, int W, FL_MAP_PIXEL *pRes);

void feat_maps_mse(void *pLyrCtx, CNN_LAYER_TYPE_E lyrType, int nFracBits);

void fix16_to_float_data(FP_MAP_PIXEL *pFixData, int N, int fracBits, float *pFltData);

void mean_denormalize(float *pNormImg, int H, int W, float mean, IMAGE_T maxPixel, char *pDenormImg);

void generate_random_img(IMAGE_T *pImg, int H, int W);

void generate_random_data(float *pData, int N, int seed);

void check_cmp_status(CMP_STATUS_T *pStatus);
#endif // _TEST_UTILITY_H_
