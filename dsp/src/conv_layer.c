#include "conv_layer.h"
#include "struct_defs.h"
#include "imglib.h"
#include "dsplib.h"
#include <string.h>
#include "debug_control.h"
#include "mem_manager.h"
#include "misc_utils.h"
#include "ext_dsplib.h"
#include "edma_module.h"


extern unsigned int core_id;
/* This is per core temporary buffer for storing 1 row output of the convolution.
 * This is used to support the strided convolution which is  not supported in the IMGLIB
 * We use IMGLIB APIs to do full convolution and then discard pixels based on the stride.
 * This buffer is used to store the result of full convolution
 */
#pragma DATA_ALIGN(private_temp_buff, 8);
#pragma DATA_SECTION(private_temp_buff, ".local_ram")
uint8_t far private_temp_buff[PRIVATE_TEMP_BUFF_SIZE]; // FIXME: this array must be 32bit aligned which is a requirement for IMGLIB.

#pragma DATA_ALIGN(private_conv_buff, 8);
#pragma DATA_SECTION(private_conv_buff, ".local_ram")
uint8_t far private_conv_buff[PRIVATE_TEMP_BUFF_SIZE];

#pragma DATA_ALIGN(line_buff_ping, 8);
#pragma DATA_SECTION(line_buff_ping, ".local_ram")
uint8_t far line_buff_ping[MAX_SUPPORTED_KER_SIZE * MAX_INPUT_MAP_WIDTH];

#pragma DATA_ALIGN(line_buff_pong, 8);
// TODO: Enable linker attr. Linker warning due to neardata out of range. Handle this issue later.
//#pragma DATA_SECTION(line_buff_pong, ".local_ram")
uint8_t far line_buff_pong[MAX_SUPPORTED_KER_SIZE * MAX_INPUT_MAP_WIDTH];

FIX_MAP *p_line_buff[2][MAX_SUPPORTED_KER_SIZE];

// Buffers on L2 memory for loading conv kernel weights when they are stored on DDR
uint8_t far ker_buff_ping[MAX_SUPPORTED_KER_SIZE * MAX_SUPPORTED_KER_SIZE * MAX_SUPPORTED_INPUT_MAPS];
uint8_t far ker_buff_pong[MAX_SUPPORTED_KER_SIZE * MAX_SUPPORTED_KER_SIZE * MAX_SUPPORTED_INPUT_MAPS];

FIX_KER *p_conv_ker_buff[2];

// This is non-optimized fixed point implementation to be used for those kernel sizes for which there is no IMGLIB APIs
STATUS_E dsp_fix_conv_layer_unoptimized(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height including padding if present
	int in_width,		// input feature map width including padding if present
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps,		// no of feature maps assigned to this core.
	int ker_size,		// kernel size. We support only square sized kernels
	int pad,			// padding in all 4 sides. Only equal padding in all dimensions is supported.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {

	int omap, imap, in_row, out_row, in_col, out_col, kr, kc;
	int o_w, o_h;
	int32_t sum;
	STATUS_E status = SUCCESS;


	o_h = (in_height + 2 * pad - ker_size + 1 + stride - 1) / stride;
	o_w = (in_width  + 2 * pad - ker_size + 1 + stride - 1) / stride;
	for(omap = start_map; omap < start_map + no_maps; omap++) {
		in_row = -pad;
		for(out_row = 0; out_row < o_h; out_row++) {
			in_col = -pad;
			for(out_col = 0; out_col < o_w; out_col++) {
				sum = 0.0;
				for(imap = 0; imap < no_inputs; imap++){
					for(kr = 0; kr < ker_size; kr++) {
						for(kc = 0; kc < ker_size; kc++){
							if(is_a_ge_zero_and_a_lt_b(in_row + kr, in_height) & is_a_ge_zero_and_a_lt_b(in_col + kc, in_width)) {
								sum += p_weight[((omap * no_inputs + imap) * ker_size + kr) * ker_size + kc] * p_input[(imap * in_height + in_row + kr) * in_width + in_col + kc];
							}
						}
					}
				}
				sum = sum >> shift;
				sum += p_bias[omap];
				p_output[omap * o_h * o_w + o_w * out_row + out_col] = (FIX_MAP)sum;
				in_col += stride;
			}
			in_row += stride;
		}
	}

	return status;
}

#define PADDING_SUPPORT 0
#define CONV_BUFFERING 0
#define CONV_EDMA 0

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
	int pad,			// padding in all 4 sides. Only equal padding in all dimensions is supported.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {
	Bool w_ocm;
	STATUS_E status = SUCCESS;
	// check whether the weights are on on-chip memory
	w_ocm = is_on_chip_addr((Uint32)p_weight);

	switch(ker_size) {
		case 3:
			if(pad == 0 && w_ocm == 1 && in_width % 2 == 0) {
				status = dsp_fix_conv_3x3_constrained(p_input, p_weight, p_bias, in_height, in_width, no_inputs, no_outputs, start_map, no_maps, stride, shift, p_output);
			} else {
				status = dsp_fix_conv_3x3(p_input, p_weight, p_bias, in_height, in_width, no_inputs, no_outputs, start_map, no_maps, pad, stride, shift, p_output);
			}
			break;
		case 5:
			if(pad == 0 && w_ocm == 1 && in_width % 2 == 0) {
				status = dsp_fix_conv_5x5_constrained(p_input, p_weight, p_bias, in_height, in_width, no_inputs, no_outputs, start_map, no_maps, stride, shift, p_output);
			} else {
				status = dsp_fix_conv_5x5(p_input, p_weight, p_bias, in_height, in_width, no_inputs, no_outputs, start_map, no_maps, pad, stride, shift, p_output);
			}
			break;
		case 7:
			if(pad == 0 && w_ocm == 1 && in_width % 4 == 0) {
				status = dsp_fix_conv_7x7_constrained(p_input, p_weight, p_bias, in_height, in_width, no_inputs, no_outputs, start_map, no_maps, stride, shift, p_output);
			} else {
				status = dsp_fix_conv_7x7(p_input, p_weight, p_bias, in_height, in_width, no_inputs, no_outputs, start_map, no_maps, pad, stride, shift, p_output);
			}
			break;
		case 11:
			if(pad == 0 && w_ocm == 1) {
				status = dsp_fix_conv_11x11_constrained(p_input, p_weight, p_bias, in_height, in_width, no_inputs, no_outputs, start_map, no_maps, stride, shift, p_output);
			} else {
				status = dsp_fix_conv_11x11(p_input, p_weight, p_bias, in_height, in_width, no_inputs, no_outputs, start_map, no_maps, pad, stride, shift, p_output);
			}
			break;
		default:
			status = dsp_fix_conv_layer_unoptimized(p_input, p_weight,
				p_bias, in_height, in_width, no_inputs, no_outputs,
				start_map, no_maps, ker_size, pad, stride, shift, p_output);

			DBG_INFO("Kernel size not supported in IMGLIB. Using unoptimized function for this layer.\n");
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
	int pad,			// padding in all 4 sides. Only equal padding in all dimensions is supported.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	FLT_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {

	int omap, imap, in_row, out_row, in_col, out_col, kr, kc;
	int o_w, o_h;
	FLT_MAP sum;
	STATUS_E status = SUCCESS;


	o_h = (in_height + 2 * pad - ker_size + 1 + stride - 1) / stride;
	o_w = (in_width  + 2 * pad - ker_size + 1 + stride - 1) / stride;
	for(omap = start_map; omap < start_map + no_maps; omap++) {
		in_row = -pad;
		for(out_row = 0; out_row < o_h; out_row++) {
			in_col = -pad;
			for(out_col = 0; out_col < o_w; out_col++) {
				sum = 0.0;
				for(imap = 0; imap < no_inputs; imap++){
					for(kr = 0; kr < ker_size; kr++) {
						for(kc = 0; kc < ker_size; kc++){
							if(is_a_ge_zero_and_a_lt_b(in_row + kr, in_height) & is_a_ge_zero_and_a_lt_b(in_col + kc, in_width)) {
								sum += p_weight[((omap * no_inputs + imap) * ker_size + kr) * ker_size + kc] * p_input[(imap * in_height + in_row + kr) * in_width + in_col + kc];
							}
						}
					}
				}
				sum += p_bias[omap];
				p_output[omap * o_h * o_w + o_w * out_row + out_col] = sum;
				in_col += stride;
			}
			in_row += stride;
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
			p_conv_ctx->conv_info.map_h,
			p_conv_ctx->conv_info.map_w,
			p_conv_ctx->conv_info.no_inputs,
			p_conv_ctx->conv_info.no_outputs,
			p_conv_ctx->start_map[core_id],
			p_conv_ctx->no_maps[core_id],
			p_conv_ctx->conv_info.ker_size,
			p_conv_ctx->conv_info.pad,
			p_conv_ctx->conv_info.stride,
			p_conv_ctx->conv_info.no_ker_frac_bits,
			p_conv_ctx->p_fix_output
			);

	} else {
		status = dsp_flt_conv_layer(p_flt_in_maps,
			p_conv_ctx->p_flt_ker,
			p_conv_ctx->p_flt_bias,
			p_conv_ctx->conv_info.map_h,
			p_conv_ctx->conv_info.map_w,
			p_conv_ctx->conv_info.no_inputs,
			p_conv_ctx->conv_info.no_outputs,
			p_conv_ctx->start_map[core_id],
			p_conv_ctx->no_maps[core_id],
			p_conv_ctx->conv_info.ker_size,
			p_conv_ctx->conv_info.pad,
			p_conv_ctx->conv_info.stride,
			p_conv_ctx->p_flt_output
			);
	}

	return status;
}
