#ifndef _MISC_UTILS_H_
#define _MISC_UTILS_H_
#include <stdint.h>
#include "config.h"

inline uint32_t is_a_ge_zero_and_a_lt_b(int a, int b);
uint32_t global_address (uint32_t addr);
void get_fix_eye_matrix(FIX_MAP *p_mat, int R, int C);

void get_flt_eye_matrix(FLT_MAP *p_mat, int R, int C);

void float_to_fix_data(FLT_MAP *p_flt_data, int N, int frac_bits, FIX_MAP *p_fix_data);

void print_fix_img(FIX_MAP *p_img, int H, int W);

void print_float_img(FLT_MAP *p_img, int H, int W);

void fix16_to_float_data(FIX_MAP *p_fix_data, int N, int frac_bits, float *p_flt_data);

void generate_random_img(uint8_t *p_img, int H, int W);

void generate_random_data(float *p_data, int N, int seed);

void rotate_180(FIX_MAP *mat, int M, int N);



#endif // _TEST_UTILITY_H_
