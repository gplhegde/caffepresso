#include "misc_utils.h"
#include "debug_control.h"
#include <math.h>
#include <stdlib.h>
#include "conv_layer.h"
#include "pool_layer.h"
#include "cnn_layers.h"

extern unsigned int core_id;

float mean_normalize(uint8_t *p_img, int H, int W, float *var, float *p_norm_img){
	uint32_t sum, pixel;
	float mean, norm_pixel;
	
	sum = 0;
	for (pixel = 0; pixel < H * W; pixel++){
		sum += p_img[pixel];
	}

	mean = sum / (H * W);
	*var = 0;
	// variance
	for (pixel = 0; pixel < H * W; pixel++){
		*var += pow((p_img[pixel] - mean), 2);
	}
	// mean normalize
	for (pixel = 0; pixel < H * W; pixel++){
		norm_pixel = ((float)(p_img[pixel]) - mean)/ sqrt(*var + 1e-8);
		p_norm_img[pixel] = norm_pixel;
	}
	
	return mean;
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

void mean_denormalize(float *p_norm_img, int H, int W, float mean, uint8_t max_pixel, char *p_denorm_img) {
	float pixel;
	int p;
	for (p = 0; p < H * W; p++) {
		pixel = p_norm_img[p] * ((float)(max_pixel + 1) - mean);
		pixel += mean;
		DBG_ASSERT(pixel >= 0 && pixel <= 255);
		p_denorm_img[p] = (char) pixel;
	}
}

void print_fix_img(FIX_MAP *p_img, int H, int W) {
	int r, c;
	DBG_INFO("-------------------------------------\n");
	for (r = 0; r < H; r++) {
		for (c = 0; c < W; c++) {
			printf("%d\t", p_img[r * W + c]);
		}
		printf("\n");
	}
	DBG_INFO("-------------------------------------\n");
}

void print_fix_map(FIX_MAP *p_map, int n_maps, int H, int W, int map) {
	int r, c;
	DBG_INFO("-------------------------------------\n");
	for (r = 0; r < H; r++) {
		for (c = 0; c < W; c++) {
			printf("%d\t", p_map[map*H*W + r * W + c]);
		}
		printf("\n");
	}
	DBG_INFO("-------------------------------------\n");
}
void print_float_img(FLT_MAP *p_img, int H, int W) {
    int r, c;
	DBG_INFO("-------------------------------------\n");
    for (r = 0; r < H; r++) {
        for (c = 0; c < W; c++) {
            printf("%f\t", p_img[r * W + c]);
        }
        printf("\n");
    }
	DBG_INFO("-------------------------------------\n");
}

float float_img_diff(FLT_MAP *p_img1, FLT_MAP *p_img2, int H, int W, FLT_MAP *p_res) {

    int r, c;
	float mean;
	mean = 0;
    for (r = 0; r < H; r++) {
        for (c = 0; c < W; c++) {
            p_res[r * W + c] = p_img1[r * W + c] - p_img2[r * W + c];
			mean +=  pow(p_res[r * W + c], 2);
        }
    }
	return sqrt(mean / (H * W));

}

void feat_maps_mse(void *p_lyr_ctx, CNN_LAYER_TYPE_E lyr_type, int n_frac_bits) {

	int map_w, map_h, map;
	float *p_fix_float_map, mse;
	switch(lyr_type) {
		case CONV:
		{
			CONV_LYR_CTX_T *p_conv_ctx = (CONV_LYR_CTX_T *)p_lyr_ctx;
			map_h = p_conv_ctx->conv_info.map_h - p_conv_ctx->conv_info.ker_size + 1;
			map_w = p_conv_ctx->conv_info.map_w - p_conv_ctx->conv_info.ker_size + 1;
			p_fix_float_map = (float *) malloc(map_h * map_w * sizeof(float));

			for (map = 0; map < p_conv_ctx->conv_info.no_outputs; map++) {
				fix16_to_float_data(p_conv_ctx->p_fix_output + map * map_h * map_w,
					map_h * map_w,  n_frac_bits, p_fix_float_map);
				mse = float_img_diff(p_conv_ctx->p_flt_output + map * map_h * map_w,
						p_fix_float_map, map_h, map_w, p_fix_float_map);
				printf("Map = %d\tMSE = %f\n", map, mse);			
			}	
			break;
		}
		case POOL:
		{
			POOL_LYR_CTX_T *p_pool_ctx = (POOL_LYR_CTX_T *)p_lyr_ctx;
			map_h = p_pool_ctx->pool_info.map_h / p_pool_ctx->pool_info.stride;
			map_w = p_pool_ctx->pool_info.map_w / p_pool_ctx->pool_info.stride;
			p_fix_float_map = (float *) malloc(map_h * map_w * sizeof(float));
			for (map = 0; map < p_pool_ctx->pool_info.no_outputs; map++) {
				fix16_to_float_data(p_pool_ctx->p_fix_output + map * map_h * map_w,
					map_h * map_w,  n_frac_bits, p_fix_float_map);
				mse = float_img_diff(p_pool_ctx->p_flt_output + map * map_h * map_w,
						p_fix_float_map, map_h, map_w, p_fix_float_map);
				printf("Map = %d\tMSE = %f\n", map, mse);			
			}	
			break;
		}
	}
	free(p_fix_float_map);

}

void print_org_img(uint8_t *p_img, int H, int W) {
    int r, c;
	DBG_INFO("-------------------------------------\n");
    for (r = 0; r < H; r++) {
        for (c = 0; c < W; c++) {
            printf("%d\t", p_img[r * W + c]);
        }
        printf("\n");
    }
	DBG_INFO("-------------------------------------\n");
}

void compare_fix_img(FIX_MAP *p_img1, FIX_MAP *p_img2, int H, int W) {
	int r, c, mismatch_cnt;

	mismatch_cnt = 0;
	for (r = 0; r < H ; r++) {
		for (c = 0; c < W; c++) {
			if (p_img1[r * W + c] != p_img2 [r * W + c]) {
				printf("Pixel mismatch at row = %d, col = %d\n", r, c);
				mismatch_cnt++;
			}
		}
	}
	if (mismatch_cnt == 0) {
		DBG_INFO("Two maps are matching\n");
	}
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

void rotate_180(FIX_KER *mat, int M, int N) {
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

void check_cmp_status(CMP_STATUS_T *p_status) {
	if(p_status->flag == TEST_PASS) {
		printf("PASSED\n");
	} else {
		printf("Data mismatch occured\n");
		printf("Map: %d\tRow: %d\tCol: %d\n", p_status->mis_map, p_status->mis_row, p_status->mis_col);
		printf("TEST FAILED\n");
		exit(-1);
	}

}

void print_layer_node_ctx(CNN_LYR_NODE_T *p_lyr_nodes, int no_layers) {
	int lyr;
	IP_LYR_CTX_T *p_ip_ctx;
	CONV_LYR_CTX_T *p_conv_ctx;
	POOL_LYR_CTX_T *p_pool_ctx;
	ACT_LYR_CTX_T *p_act_ctx;
	SMAX_LYR_CTX_T *p_smax_ctx; 
	CNN_LYR_NODE_T *p_node = p_lyr_nodes;

	printf("\n==================================================\n");
	printf("Per core workload for all layers : CORE %d\n", core_id);
	for (lyr = 0; lyr < no_layers; lyr++) {
		switch(p_node->lyr_type) {
			case CONV:
				p_conv_ctx = (CONV_LYR_CTX_T *)p_node->p_lyr_ctx;
				printf("\n----- Layer %d\tType : CONV -----\n", lyr);
				printf("Shape : (%d, %d, %d, %d)", p_conv_ctx->conv_info.no_outputs, p_conv_ctx->conv_info.no_inputs,
					p_conv_ctx->conv_info.map_h, p_conv_ctx->conv_info.map_w);
				printf("Start map : %d\tNo maps : %d\n", p_conv_ctx->start_map[core_id], p_conv_ctx->no_maps[core_id]);

				break;
			case POOL:
				p_pool_ctx = (POOL_LYR_CTX_T *)p_node->p_lyr_ctx;
				printf("\n----- Layer %d\tType : POOL -----\n", lyr);
				printf("Shape : (%d, %d, %d, %d)", p_pool_ctx->pool_info.no_outputs, p_pool_ctx->pool_info.no_inputs,
						p_pool_ctx->pool_info.map_h, p_pool_ctx->pool_info.map_w);
				printf("Start map : %d\tNo maps : %d\n", p_pool_ctx->start_map[core_id], p_pool_ctx->no_maps[core_id]);
				
				break;
			case ACT:
				p_act_ctx = (ACT_LYR_CTX_T *)p_node->p_lyr_ctx;
				printf("\n----- Layer %d\tType : ACT -----\n", lyr);
				printf("Shape : (%d, %d, %d, %d)", p_act_ctx->act_info.no_outputs, p_act_ctx->act_info.no_inputs,
						p_act_ctx->act_info.map_h, p_act_ctx->act_info.map_w);
				printf("Start map : %d\tNo maps : %d\n", p_act_ctx->start_map[core_id], p_act_ctx->no_maps[core_id]);
				break;
			case INNER_PROD:
				p_ip_ctx = (IP_LYR_CTX_T *)p_node->p_lyr_ctx;
				printf("\n----- Layer %d\tType : INNER_PROD -----\n", lyr);
				printf("Shape : (%d, %d, %d, %d)", p_ip_ctx->ip_info.no_outputs, p_ip_ctx->ip_info.no_inputs,
						p_ip_ctx->ip_info.map_h, p_ip_ctx->ip_info.map_w);
				printf("Start map : %d\tNo maps : %d\n", p_ip_ctx->start_map[core_id], p_ip_ctx->no_maps[core_id]);
				break;
			case SOFTMAX:
				p_smax_ctx = (SMAX_LYR_CTX_T *)p_node->p_lyr_ctx;
				printf("\n----- Layer %d\tType : SOFTMAX -----\n", lyr);
				break;
			default:
				break;
		}
		p_node++;
	}
	printf("\n==================================================\n");
}


