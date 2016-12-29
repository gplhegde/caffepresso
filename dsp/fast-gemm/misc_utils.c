#include "misc_utils.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>


extern unsigned int core_id;

// Taken from Caffe implementation
// https://github.com/BVLC/caffe/blob/master/src/caffe/util/im2col.cpp
inline uint32_t is_a_ge_zero_and_a_lt_b(int a, int b) {
  return (unsigned int)a < (unsigned int)b;
}



uint32_t global_address (uint32_t addr) {

	addr = addr + (0x10000000 + core_id * 0x1000000);
	return addr;
}

void get_fix_eye_matrix(FIX_MAP *p_mat, int R, int C) {
	int r, c;
	REL_ASSERT(R == C);
	memset(p_mat, 0, R * C * sizeof(FIX_MAP));
	for(r = 0; r < R; r++) {
		for(c = 0; c < C; c++) {
			if(r == c) {
				p_mat[r * C + c] = 1;
			}
		}
	}
}
void get_flt_eye_matrix(FLT_MAP *p_mat, int R, int C) {
	int r, c;
	REL_ASSERT(R == C);
	memset(p_mat, 0, R * C * sizeof(FLT_MAP));
	for(r = 0; r < R; r++) {
		for(c = 0; c < C; c++) {
			if(r == c) {
				p_mat[r * C + c] = 1.0;
			}
		}
	}
}

void float_to_fix_data(FLT_MAP *p_flt_data, int N, int frac_bits, FIX_MAP *p_fix_data) {
	int pix;
	for (pix = 0; pix < N; pix++) {
		p_fix_data[pix] = (FIX_MAP) floor(p_flt_data[pix] * (1 << frac_bits));
	}
}


void fix16_to_float_data(FIX_MAP *p_fix_data, int N, int frac_bits, float *p_flt_data) {
	int pix;
	for (pix = 0; pix < N; pix++) {
		p_flt_data[pix] = (float) p_fix_data[pix] / (1 << frac_bits); 	// FIXME: Rounding is not considered
	}
}



void print_fix_img(FIX_MAP *p_img, int H, int W) {
	int r, c;
	printf("-------------------------------------\n");
	for (r = 0; r < H; r++) {
		for (c = 0; c < W; c++) {
			printf("%d\t", p_img[r * W + c]);
		}
		printf("\n");
	}
	printf("-------------------------------------\n");
}


void print_float_img(FLT_MAP *p_img, int H, int W) {
    int r, c;
    printf("-------------------------------------\n");
    for (r = 0; r < H; r++) {
        for (c = 0; c < W; c++) {
            printf("%.4f\t", p_img[r * W + c]);
        }
        printf("\n");
    }
    printf("-------------------------------------\n");
}





void print_fix_img_diff(FIX_MAP *p_img1, FIX_MAP *p_img2, int H, int W) {
        int r, c;
        for (r = 0; r < H ; r++) {
                for (c = 0; c < W; c++) {
                       printf("%d,   ", p_img1[r * W + c] - p_img2 [r * W + c]);
                }
		printf("\n");
        }
}

void generate_random_img(uint8_t *p_img, int H, int W) {
	int pix;
	for (pix = 0; pix < H * W; pix++) {
		p_img[pix] = rand() % 256;
	}
}

void generate_random_data(float *p_data, int N, int seed) {
	int k;
	srand(seed);
	for (k = 0; k < N; k++) {
		p_data[k] = 2*((float)rand() / RAND_MAX) - 1.0;
	}
}

void rotate_180(FIX_MAP *mat, int M, int N) {
	int row, col, temp;
	for(row = 0; row < M/2; row++){
		for(col = 0; col < N; col++) {
			temp = mat[row * N + col];
			mat[row * N + col] = mat[(M - row - 1) * N + (N-col-1)];
			mat[(M - row - 1) * N + (N-col-1)] = temp;
		}
	}
	if(M % 2) {
		row = M / 2;
		for(col = 0; col < N / 2; col++) {
			temp = mat[row * N + col];
			mat[row * N + col] = mat[(M - row - 1) * N + (N-col-1)];
			mat[(M - row - 1) * N + (N-col-1)] = temp;
		}
	}
}


