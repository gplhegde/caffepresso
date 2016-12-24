#ifndef _MISC_UTILS_H_
#define _MISC_UTILS_H_


#include "struct_defs.h"
#include "conv_layer.h"
#include "caffe_frontend.h"

inline uint32_t is_a_ge_zero_and_a_lt_b(int a, int b);

float mean_normalize(uint8_t *p_img, int H, int W, float *var, float *p_norm_img);

void char_to_float_image(uint8_t *p_char_img, int C, int H, int W, FLT_MAP *p_flt_img);

uint32_t find_max_index(FLT_MAP *p_array, int N);

void get_fix_eye_matrix(FIX_MAP *p_mat, int R, int C);

void get_flt_eye_matrix(FLT_MAP *p_mat, int R, int C);

void float_to_fix_data(FLT_MAP *p_flt_data, int N, int frac_bits, FIX_MAP *p_fix_data);

void print_org_img(uint8_t *p_img, int H, int W);

void print_fix_img(FIX_MAP *p_img, int H, int W);

void print_fix_map(FIX_MAP *p_map, int no_maps, int H, int W, int map);

void print_float_img(FLT_MAP *p_img, int H, int W);

void compare_fix_img(FIX_MAP *p_img1, FIX_MAP *p_img2, int H, int W);

void print_fix_img_diff(FIX_MAP *p_img1, FIX_MAP *p_img2, int H, int W);

float float_img_diff(FLT_MAP *p_img1, FLT_MAP *p_img2, int H, int W, FLT_MAP *p_res);

void feat_maps_mse(void *p_lyr_ctx, CNN_LAYER_TYPE_E lyr_type, int no_frac_bits);

void fix16_to_float_data(FIX_MAP *p_fix_data, int N, int frac_bits, float *p_flt_data);

void mean_denormalize(float *p_norm_img, int H, int W, float mean, uint8_t max_pixel, char *p_denorm_img);

void generate_random_img(uint8_t *p_img, int H, int W);

void generate_random_data(float *p_data, int N, int seed);

void rotate_180(FIX_KER *mat, int M, int N);

void check_cmp_status(CMP_STATUS_T *p_status);
void print_layer_node_ctx(CNN_LYR_NODE_T *p_lyr_nodes, int no_layers);
#endif // _TEST_UTILITY_H_
