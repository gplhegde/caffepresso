#include "conv_layer.h"
#include "struct_defs.h"
#include "imglib.h"
#include "dsplib.h"
#include <string.h>
#include "debug_control.h"
#include "mem_manager.h"
#include "misc_utils.h"
#include "ext_dsplib.h"

#define MAX_SUPPORTED_KER_SIZE 		(11)
#define MAX_INPUT_MAP_WIDTH			(256)
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
uint8_t private_conv_buff[PRIVATE_TEMP_BUFF_SIZE];

#pragma DATA_ALIGN(line_buff_ping, 8);
#pragma DATA_SECTION(line_buff_ping, ".local_ram")
uint8_t line_buff_ping[MAX_SUPPORTED_KER_SIZE * MAX_INPUT_MAP_WIDTH];

static FIX_MAP *p_line_buff_ping[MAX_SUPPORTED_KER_SIZE];

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

#define PADDING_SUPPORT 1


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

	int o_h, o_w, omap, imap, row, new_width, o_w_x8;
	int pitch, in_row, out_row, r;
	STATUS_E status = FAILED;

	o_h = (in_height + 2 * pad - ker_size + 1 + stride - 1) / stride;
	o_w = (in_width  + 2 * pad - ker_size + 1 + stride - 1) / stride;

	// output width which is multiple of 8
	o_w_x8 = o_w + (8 - o_w % 8);
	// reset the output maps buffer; only the portion that belongs to this core.
	memset(p_output + start_map * o_h * o_w, 0, o_h * o_w * no_maps * sizeof(FIX_MAP));

	switch(ker_size) {
		case 3:
#if PADDING_SUPPORT
			new_width = in_width + 2 * pad - 2;
			new_width = ((new_width & 0x1) == 0)? new_width : (new_width + 1);

			pitch = in_width + 2 * pad;
			pitch = pitch % 2 == 0? pitch : pitch + 1;
			for(r = 0; r < 3; r++) {
				p_line_buff_ping[r]  = (FIX_MAP *)line_buff_ping + r * pitch;
			}

			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					in_row = -pad;
					memset(line_buff_ping, 0, 3 * pitch * sizeof(FIX_MAP));
					for(out_row = 0; out_row < o_h; out_row++) {
						if(is_a_ge_zero_and_a_lt_b(in_row, in_height - 2)) {
							// need to laod all K rows	starting at line_buff[0]
							for(r = 0; r < 3; r++) {
								memcpy(p_line_buff_ping[r] + pad, p_input + (imap * in_height + in_row + r) * in_width, in_width * sizeof(FIX_MAP));
							}
						} else {
							// Need to load only few rows
							if(in_row < 0) { // need to load K + in_row number of rows starting at line buffer no
								for(r = 0; r < 3 + in_row; r++) {
									memcpy(p_line_buff_ping[-in_row + r] + pad, p_input + (imap * in_height + r) * in_width, in_width * sizeof(FIX_MAP));
								}
							} else { // bottom end of input map
								// reset the line buffers to mimic zero padding since it is overwritten by the input maps after initial reset.
								memset(line_buff_ping, 0, 3 * pitch * sizeof(FIX_MAP));
								for(r = 0; r < in_height - in_row; r++) {
									memcpy(p_line_buff_ping[r] + pad, p_input + (imap * in_height + in_row + r) * in_width, in_width * sizeof(FIX_MAP));
								}
							}
						}
						IMG_conv_3x3_i16s_c16s(p_line_buff_ping[0],
							(FIX_MAP *)private_temp_buff,
							new_width,
							pitch,
							p_weight + (omap * no_inputs + imap) * 9,
							shift
							);
						// throw unnecessary pixels to mimic column stride
						if(stride != 1) {
							strided_move((FIX_MAP *)private_temp_buff, o_w, stride);
						}

						memcpy(private_conv_buff, p_output + (omap * o_h + out_row) * o_w, o_w * sizeof(FIX_MAP));
						DSP_add16((FIX_MAP *)private_conv_buff, (FIX_MAP *)private_temp_buff, (FIX_MAP *)private_conv_buff, o_w_x8);
						memcpy(p_output + (omap * o_h + out_row) * o_w, private_conv_buff, o_w * sizeof(FIX_MAP));

						in_row += stride;
					}
				}
				// add bias
				DSP_vs_add_unroll_8(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
#else
			REL_ASSERT(in_width % 2 == 0);
			// The output width for  IMG_conv_3x3_i16s_c16s API must be multiple of 2.
			new_width = in_width - 2;
			new_width = ((new_width & 0x1) == 0)? new_width : (new_width + 1);
			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					for(row = 0; row < in_height - ker_size + 1; row += stride) {
						IMG_conv_3x3_i16s_c16s(p_input + (imap * in_height + row ) * in_width,
							(FIX_MAP *)private_temp_buff,
							new_width,
							in_width,
							p_weight + (omap * no_inputs + imap) * ker_size * ker_size,
							shift
							);
						// throw unnecessary pixels to mimic column stride
						if(stride != 1) {
							strided_move((FIX_MAP *)private_temp_buff, o_w, stride);
						}
						// add the output corresponding to one input map.
						// FIXME: This may cause the overflow. Need to saturate.
						memcpy(private_conv_buff, p_output + (omap * o_h + row / stride) * o_w, o_w * sizeof(FIX_MAP));
						DSP_add16((FIX_MAP *)private_conv_buff, (FIX_MAP *)private_temp_buff, (FIX_MAP *)private_conv_buff, o_w_x8);
						memcpy(p_output + (omap * o_h + row / stride) * o_w, private_conv_buff, o_w * sizeof(FIX_MAP));
					}
				}
				// add bias
				DSP_vs_add_unroll_8(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
#endif // PADDING_SUPPORT
			break;
		case 5:
#if PADDING_SUPPORT
			new_width = in_width + 2 * pad - 4;
			new_width = ((new_width & 0x1) == 0)? new_width : (new_width + 1);
			pitch = in_width + 2 * pad;
			pitch = pitch % 2 == 0? pitch : pitch + 1;

			for(r = 0; r < 5; r++) {
				p_line_buff_ping[r]  = (FIX_MAP *)line_buff_ping + r * pitch;
			}

			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					in_row = -pad;
					memset(line_buff_ping, 0, 5 * pitch * sizeof(FIX_MAP));
					for(out_row = 0; out_row < o_h; out_row++) {
						if(is_a_ge_zero_and_a_lt_b(in_row, in_height - 4)) {
							// need to laod all K rows	starting at line_buff[0]
							for(r = 0; r < 5; r++) {
								memcpy(p_line_buff_ping[r] + pad, p_input + (imap * in_height + in_row + r) * in_width, in_width * sizeof(FIX_MAP));
							}
						} else {
							// Need to load only few rows
							if(in_row < 0) { // need to load K + in_row number of rows starting at line buffer no
								for(r = 0; r < 5 + in_row; r++) {
									memcpy(p_line_buff_ping[-in_row + r] + pad, p_input + (imap * in_height + r) * in_width, in_width * sizeof(FIX_MAP));
								}
							} else { // bottom end of input map
								// reset the line buffers to mimic zero padding since it is overwritten by the input maps after initial reset.
								memset(line_buff_ping, 0, 5 * pitch * sizeof(FIX_MAP));
								for(r = 0; r < in_height - in_row; r++) {
									memcpy(p_line_buff_ping[r] + pad, p_input + (imap * in_height + in_row + r) * in_width, in_width * sizeof(FIX_MAP));
								}
							}
						}
						IMG_conv_5x5_i16s_c16s(p_line_buff_ping[0],
							(FIX_MAP *)private_temp_buff,
							new_width,
							pitch,
							p_weight + (omap * no_inputs + imap) * 25,
							shift
							);
						// throw unnecessary pixels to mimic column stride
						if(stride != 1) {
							strided_move((FIX_MAP *)private_temp_buff, o_w, stride);
						}

						memcpy(private_conv_buff, p_output + (omap * o_h + out_row) * o_w, o_w * sizeof(FIX_MAP));
						DSP_add16((FIX_MAP *)private_conv_buff, (FIX_MAP *)private_temp_buff, (FIX_MAP *)private_conv_buff, o_w_x8);
						memcpy(p_output + (omap * o_h + out_row) * o_w, private_conv_buff, o_w * sizeof(FIX_MAP));

						in_row += stride;
					}
				}
				// add bias
				DSP_vs_add_unroll_8(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
#else
			// The input and output width for  IMG_conv_5x5_i16s_c16s API must be multiple of 2.
			REL_ASSERT(in_width % 2 == 0);
			new_width = ((in_width & 0x1) == 0)? in_width : in_width + 1;
			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					for(row = 0; row < in_height - ker_size + 1; row += stride) {
						IMG_conv_5x5_i16s_c16s(p_input + (imap * in_height + row ) * in_width,
							(FIX_MAP *)private_temp_buff,
							new_width - 4,
							// FIXME: if input_width is odd, then new_width = in_width + 1, the DSPLIB API will access rows at wrong offset
							// from second row onwards leading to wrong output! Take care of this by padding extra column in the end when supporting padding.
							in_width,
							p_weight + (omap * no_inputs + imap) * ker_size * ker_size,
							shift
							);
						// throw unnecessary pixels
						if(stride != 1) {
							strided_move((FIX_MAP *)private_temp_buff, o_w, stride);
						}
						// add the output corresponding to one input map.
						// FIXME: This may cause the overflow. Need to saturate.
						memcpy(private_conv_buff, p_output + (omap * o_h + row / stride) * o_w, o_w * sizeof(FIX_MAP));
						DSP_add16((FIX_MAP *)private_conv_buff, (FIX_MAP *)private_temp_buff, (FIX_MAP *)private_conv_buff, o_w_x8);
						memcpy(p_output + (omap * o_h + row / stride) * o_w, private_conv_buff, o_w * sizeof(FIX_MAP));
					}
				}
				// add bias
				DSP_vs_add_unroll_8(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
#endif // PADDING_SUPPORT
			break;
		case 7:
#if PADDING_SUPPORT
			new_width = in_width + 2 * pad - 6;
			new_width = (new_width % 8 == 0)? new_width : new_width + (8 - new_width % 8);

			pitch = in_width + 2 * pad;
			pitch = pitch % 4 == 0? pitch : pitch + (4 - pitch % 4);

			for(r = 0; r < 7; r++) {
				p_line_buff_ping[r]  = (FIX_MAP *)line_buff_ping + r * pitch;
			}

			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					in_row = -pad;
					memset(line_buff_ping, 0, 7 * pitch * sizeof(FIX_MAP));
					for(out_row = 0; out_row < o_h; out_row++) {
						if(is_a_ge_zero_and_a_lt_b(in_row, in_height - 6)) {
							// need to laod all K rows	starting at line_buff[0]
							for(r = 0; r < 7; r++) {
								memcpy(p_line_buff_ping[r] + pad, p_input + (imap * in_height + in_row + r) * in_width, in_width * sizeof(FIX_MAP));
							}
						} else {
							// Need to load only few rows
							if(in_row < 0) { // need to load K + in_row number of rows starting at line buffer no
								for(r = 0; r < 7 + in_row; r++) {
									memcpy(p_line_buff_ping[-in_row + r] + pad, p_input + (imap * in_height + r) * in_width, in_width * sizeof(FIX_MAP));
								}
							} else { // bottom end of input map
								// reset the line buffers to mimic zero padding since it is overwritten by the input maps after initial reset.
								memset(line_buff_ping, 0, 7 * pitch * sizeof(FIX_MAP));
								for(r = 0; r < in_height - in_row; r++) {
									memcpy(p_line_buff_ping[r] + pad, p_input + (imap * in_height + in_row + r) * in_width, in_width * sizeof(FIX_MAP));
								}
							}
						}
						IMG_conv_7x7_i16s_c16s(p_line_buff_ping[0],
							(FIX_MAP *)private_temp_buff,
							new_width,
							pitch,
							p_weight + (omap * no_inputs + imap) * 49,
							shift
							);
						// throw unnecessary pixels to mimic column stride
						if(stride != 1) {
							strided_move((FIX_MAP *)private_temp_buff, o_w, stride);
						}

						memcpy(private_conv_buff, p_output + (omap * o_h + out_row) * o_w, o_w * sizeof(FIX_MAP));
						DSP_add16((FIX_MAP *)private_conv_buff, (FIX_MAP *)private_temp_buff, (FIX_MAP *)private_conv_buff, o_w_x8);
						memcpy(p_output + (omap * o_h + out_row) * o_w, private_conv_buff, o_w * sizeof(FIX_MAP));

						in_row += stride;
					}
				}
				// add bias
				DSP_vs_add_unroll_8(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
#else
			// The DSPLIIB API requrired the input width be factor of 4 and output width be factor of 8
			// Since padding is not supported as of now, restrict the input width to be factor of 4
			// TODO: handle this requirement while adding support for padding by padding necessary extra columns in the right size of the image.
			REL_ASSERT(in_width % 4 == 0);
			new_width = in_width - 6;
			new_width = ((new_width & 0x7) == 0)? new_width : (new_width + 8 - new_width % 8);
			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					for(row = 0; row < in_height - ker_size + 1; row += stride) {
						IMG_conv_7x7_i16s_c16s(p_input + (imap * in_height + row ) * in_width,
							(FIX_MAP *)private_temp_buff,
							new_width,
							in_width,
							p_weight + (omap * no_inputs + imap) * ker_size * ker_size,
							shift
							);
						// throw unnecessary pixels
						if(stride != 1) {
							strided_move((FIX_MAP *)private_temp_buff, o_w, stride);
						}
						// add the output corresponding to one input map.
						// FIXME: This may cause the overflow. Need to saturate.
						memcpy(private_conv_buff, p_output + (omap * o_h + row / stride) * o_w, o_w * sizeof(FIX_MAP));
						DSP_add16((FIX_MAP *)private_conv_buff, (FIX_MAP *)private_temp_buff, (FIX_MAP *)private_conv_buff, o_w_x8);
						memcpy(p_output + (omap * o_h + row / stride) * o_w, private_conv_buff, o_w * sizeof(FIX_MAP));
					}
				}
				// add bias
				DSP_vs_add_unroll_8(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
#endif
			break;
		case 11:
#if PADDING_SUPPORT
			new_width = in_width + 2 * pad - 6;
			new_width = (new_width % 8 == 0)? new_width : new_width + (8 - new_width % 8);

			pitch = in_width + 2 * pad;
			pitch = pitch % 8 == 0? pitch : pitch + (8 - pitch % 8);

			for(r = 0; r < 11; r++) {
				p_line_buff_ping[r]  = (FIX_MAP *)line_buff_ping + r * pitch;
			}

			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					in_row = -pad;
					memset(line_buff_ping, 0, 11 * pitch * sizeof(FIX_MAP));
					for(out_row = 0; out_row < o_h; out_row++) {
						if(is_a_ge_zero_and_a_lt_b(in_row, in_height - 10)) {
							// need to laod all K rows	starting at line_buff[0]
							for(r = 0; r < 11; r++) {
								memcpy(p_line_buff_ping[r] + pad, p_input + (imap * in_height + in_row + r) * in_width, in_width * sizeof(FIX_MAP));
							}
						} else {
							// Need to load only few rows
							if(in_row < 0) { // need to load K + in_row number of rows starting at line buffer no
								for(r = 0; r < 11 + in_row; r++) {
									memcpy(p_line_buff_ping[-in_row + r] + pad, p_input + (imap * in_height + r) * in_width, in_width * sizeof(FIX_MAP));
								}
							} else { // bottom end of input map
								// reset the line buffers to mimic zero padding since it is overwritten by the input maps after initial reset.
								memset(line_buff_ping, 0, 11 * pitch * sizeof(FIX_MAP));
								for(r = 0; r < in_height - in_row; r++) {
									memcpy(p_line_buff_ping[r] + pad, p_input + (imap * in_height + in_row + r) * in_width, in_width * sizeof(FIX_MAP));
								}
							}
						}
						IMG_conv_11x11_i16s_c16s(p_line_buff_ping[0],
							(FIX_MAP *)private_temp_buff,
							new_width,
							pitch,
							p_weight + (omap * no_inputs + imap) * 121,
							shift
							);
						// throw unnecessary pixels to mimic column stride
						if(stride != 1) {
							strided_move((FIX_MAP *)private_temp_buff, o_w, stride);
						}

						memcpy(private_conv_buff, p_output + (omap * o_h + out_row) * o_w, o_w * sizeof(FIX_MAP));
						DSP_add16((FIX_MAP *)private_conv_buff, (FIX_MAP *)private_temp_buff, (FIX_MAP *)private_conv_buff, o_w_x8);
						memcpy(p_output + (omap * o_h + out_row) * o_w, private_conv_buff, o_w * sizeof(FIX_MAP));

						in_row += stride;
					}
				}
				// add bias
				DSP_vs_add_unroll_8(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
#else
			// The input and output width for  IMG_conv_5x5_i16s_c16s API must be multiple of 8.
			REL_ASSERT(in_width % 8 == 0);
			new_width = in_width - 10;
			new_width = ((new_width & 0x3) == 0)? new_width : (new_width + 4 - new_width % 4);
			for(omap = start_map; omap < start_map + no_maps; omap++) {
				for(imap = 0; imap < no_inputs; imap++) {
					for(row = 0; row < in_height - ker_size + 1; row += stride) {
						IMG_conv_11x11_i16s_c16s(p_input + (imap * in_height + row ) * in_width,
							(FIX_MAP *)private_temp_buff,
							new_width,
							in_width,
							p_weight + (omap * no_inputs + imap) * ker_size * ker_size,
							shift
							);
						// throw unnecessary pixels
						if(stride != 1) {
							strided_move((FIX_MAP *)private_temp_buff, o_w, stride);
						}
						// add the output corresponding to one input map.
						// FIXME: This may cause the overflow. Need to saturate.
						memcpy(private_conv_buff, p_output + (omap * o_h + row / stride) * o_w, o_w * sizeof(FIX_MAP));
						DSP_add16((FIX_MAP *)private_conv_buff, (FIX_MAP *)private_temp_buff, (FIX_MAP *)private_conv_buff, o_w_x8);
						memcpy(p_output + (omap * o_h + row / stride) * o_w, private_conv_buff, o_w * sizeof(FIX_MAP));
					}
				}
				// add bias
				DSP_vs_add_unroll_8(p_output + omap * o_h * o_w, p_bias[omap], o_w * o_h);
			}
#endif
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
