#include "conv_layer.h"
#include "struct_defs.h"
#include "imglib.h"
#include <string.h>

#define MAX_FEATUE_MAP_WIDTH	256

extern unsigned int core_id;
/* This is per core temporary buffer for storing 1 row output of the convolution.
 * This is used to support the strided convolution which is  not supported in the IMGLIB
 * We use IMGLIB APIs to do full convolution and then discard pixels based on the stride.
 * This buffer is used to store the result of full convolution
 */
#pragma DATA_SECTION(private_temp_buff, ".local_ram")
static FIX_MAP private_temp_buff[MAX_FEATUE_MAP_WIDTH]; // FIXME: this array must be 32bit aligned which is a requirement for IMGLIB.

static inline void strided_move(FIX_MAP *p_input, int len, int stride) {
	int col, i;
	for(i = 0, col = 0; i < len; i++, col += stride) {
		p_input[i] = p_input[col];
	}
}

static inline void dsp_vv_add(FIX_MAP *p_acc, FIX_MAP *p_in, int len) {
	int i;
	for(i = 0; i < len; i++) {
		// FIXME: chances of overflow. Perform saturation
		p_acc[i] += p_in[i];
	}
}

static inline void dsp_vs_add(FIX_MAP *p_acc, FIX_MAP s, int len) {
	int i;
	for(i = 0; i < len; i++) {
		// FIXME: chances of overflow. Perform saturation
		p_acc[i] += s;
	}
}

STATUS_E dsp_fix_conv_layer(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps, 		// no of feature maps assigned to this core.
	int ker_size,		// kernel size. We support only square sized kernels
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {

	int o_h, o_w, omap, imap, row, new_width;
	STATUS_E status = FAILED;
	o_h = (in_height - ker_size + 1 + stride - 1) / stride;
	o_w = (in_width - ker_size + 1 + stride - 1) / stride;



	// reset the output maps buffer; only the portion that belongs to this core.
	memset(p_output + start_map * o_h * o_w, 0, o_h * o_w * no_maps * sizeof(FIX_MAP));

	// FIXME: All the APIs from IMGLIB rotates the kernel 180 deg. We need to pre-rotate all the weights and
	// store in prior to nullify this effect.

	switch(ker_size) {
		case 3:
			// The input and output width for  IMG_conv_3x3_i16_c16s API must be multiple of 4.
			new_width = in_width - ker_size + 1;
			new_width = ((new_width & 0x3) == 0)? new_width : (new_width + new_width % 4);
			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					for(row = 0; row < in_height - ker_size + 1; row += stride) {
						IMG_conv_3x3_i16_c16s(p_input + (imap * in_height + row ) * in_width,
							private_temp_buff,
							new_width,
							p_weight + (omap * no_inputs + imap) * ker_size * ker_size,
							shift
							);
						// throw unnecessary pixels to mimic column stride
						if(stride != 1) {
							strided_move(private_temp_buff, o_w, stride);
						}
						// add the output corresponding to one input map.
						// FIXME: This may cause the overflow. Need to saturate.
						// TODO: Use APIs from DSPLIB for this.
						dsp_vv_add(p_output + (omap * o_h + row / stride) * o_w, private_temp_buff, o_w);
					}
				}
				// add bias
				dsp_vs_add(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
			break;
		case 5:
			// The input and output width for  IMG_conv_5x5_i16s_c16s API must be multiple of 2.
			new_width = ((in_width & 0x1) == 0)? in_width : in_width + 1;
			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					for(row = 0; row < in_height - ker_size + 1; row += stride) {
						IMG_conv_5x5_i16s_c16s(p_input + (imap * in_height + row ) * in_width,
							private_temp_buff,
							new_width - ker_size + 1,
							new_width,
							p_weight + (omap * no_inputs + imap) * ker_size * ker_size,
							shift
							);
						// throw unnecessary pixels
						if(stride != 1) {
							strided_move(private_temp_buff, o_w, stride);
						}
						// add the output corresponding to one input map.
						// FIXME: This may cause the overflow. Need to saturate.
						// TODO: Use APIs from DSPLIB for this.
						dsp_vv_add(p_output + (omap * o_h + row / stride) * o_w, private_temp_buff, o_w);
					}
				}
				// add bias
				dsp_vs_add(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
			break;
		case 7:
			// The input and output width for  IMG_conv_5x5_i16s_c16s API must be multiple of 8.

			new_width = in_width - ker_size + 1;
			new_width = ((new_width & 0x7) == 0)? new_width : (new_width + new_width % 8);
			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					for(row = 0; row < in_height - ker_size + 1; row += stride) {
						IMG_conv_7x7_i16s_c16s(p_input + (imap * in_height + row ) * in_width,
							private_temp_buff,
							new_width,
							new_width,
							p_weight + (omap * no_inputs + imap) * ker_size * ker_size,
							shift
							);
						// throw unnecessary pixels
						if(stride != 1) {
							strided_move(private_temp_buff, o_w, stride);
						}
						// add the output corresponding to one input map.
						// FIXME: This may cause the overflow. Need to saturate.
						// TODO: Use APIs from DSPLIB for this.
						dsp_vv_add(p_output + (omap * o_h + row / stride) * o_w, private_temp_buff, o_w);
					}
				}
				// add bias
				dsp_vs_add(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
			break;
		case 11:
			// The input and output width for  IMG_conv_5x5_i16s_c16s API must be multiple of 8.

			new_width = in_width - ker_size + 1;
			new_width = ((new_width & 0x3) == 0)? new_width : (new_width + new_width % 4);
			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					for(row = 0; row < in_height - ker_size + 1; row += stride) {
						IMG_conv_11x11_i16s_c16s(p_input + (imap * in_height + row ) * in_width,
							private_temp_buff,
							new_width,
							new_width,
							p_weight + (omap * no_inputs + imap) * ker_size * ker_size,
							shift
							);
						// throw unnecessary pixels
						if(stride != 1) {
							strided_move(private_temp_buff, o_w, stride);
						}
						// add the output corresponding to one input map.
						// FIXME: This may cause the overflow. Need to saturate.
						// TODO: Use APIs from DSPLIB for this.
						dsp_vv_add(p_output + (omap * o_h + row / stride) * o_w, private_temp_buff, o_w);
					}
				}
				// add bias
				// FIXME: make sure that both bias and output are in the same Q format
				dsp_vs_add(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
			break;
		default:
			DBG_INFO("Kernel size not supported");
	}

	return status;
}

// This is non-optimized floating point implementation just for comparison
STATUS_E dsp_flt_conv_layer(FLT_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FLT_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FLT_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height including padding if present
	int in_width,		// input feature map width including padding if present
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps,		// no of feature maps assigned to this core.
	int ker_size,		// kernel size. We support only square sized kernels
	int stride,			// convolution window stride in both horizontal and vertical direction.
	FLT_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {

	int omap, imap, row, col, kr, kc;
	int o_w, o_h;
	FLT_MAP sum;
	STATUS_E status = SUCCESS;


	o_h = (in_height - ker_size + 1 + stride - 1) / stride;
	o_w = (in_width - ker_size + 1 + stride - 1) / stride;
	for(omap = start_map; omap < start_map + no_maps; omap++) {
		for(row = 0; row < in_height - ker_size + 1; row += stride) {
			for(col = 0; col < in_width - ker_size + 1; col += stride) {
				sum = 0.0;
				for(imap = 0; imap < no_inputs; imap++){
					for(kr = 0; kr < ker_size; kr++) {
						for(kc = 0; kc < ker_size; kc++){
							sum += p_weight[((omap * no_inputs + imap) * ker_size + kr) * ker_size + kc] * p_input[(imap * in_height + row) * in_width + col];
						}
					}
				}
				sum += p_bias[omap];
				p_output[(omap * o_h + row/stride) * o_w + col/stride] = sum;
			}
		}
	}

	return status;
}

STATUS_E dsp_conv_layer(CONV_LYR_CTX_T *p_conv_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_map) {
	STATUS_E status = FAILED;

	if(p_conv_ctx->lyr_arith_mode == FIXED_POINT) {
		status = dsp_fix_conv_layer(p_fix_in_map,
			p_conv_ctx->p_fix_ker,
			p_conv_ctx->p_fix_bias,
			p_conv_ctx->conv_info.map_h + 2 * p_conv_ctx->conv_info.pad,
			p_conv_ctx->conv_info.map_w + 2 * p_conv_ctx->conv_info.pad,
			p_conv_ctx->conv_info.no_inputs,
			p_conv_ctx->conv_info.no_outputs,
			p_conv_ctx->start_map[core_id],
			p_conv_ctx->no_maps[core_id],
			p_conv_ctx->conv_info.ker_size,
			p_conv_ctx->conv_info.stride,
			p_conv_ctx->conv_info.no_ker_frac_bits,
			p_conv_ctx->p_fix_output // TODO: add offset corresponding to this core.
			);
	} else {
		status = dsp_flt_conv_layer(p_flt_in_maps,
			p_conv_ctx->p_flt_ker,
			p_conv_ctx->p_flt_bias,
			p_conv_ctx->conv_info.map_h + 2 * p_conv_ctx->conv_info.pad,
			p_conv_ctx->conv_info.map_w + 2 * p_conv_ctx->conv_info.pad,
			p_conv_ctx->conv_info.no_inputs,
			p_conv_ctx->conv_info.no_outputs,
			p_conv_ctx->start_map[core_id],
			p_conv_ctx->no_maps[core_id],
			p_conv_ctx->conv_info.ker_size,
			p_conv_ctx->conv_info.stride,
			p_conv_ctx->p_flt_output
			);
	}

	return status;
}
