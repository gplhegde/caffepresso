/* VECTORBLOX MXP SOFTWARE DEVELOPMENT KIT
 *
 * Copyright (C) 2012-2016 VectorBlox Computing Inc., Vancouver, British Columbia, Canada.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of VectorBlox Computing Inc. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This agreement shall be governed in all respects by the laws of the Province
 * of British Columbia and by the laws of Canada.
 *
 * This file is part of the VectorBlox MXP Software Development Kit.
 *
 */


#ifndef __VBX_TEST_H
#define __VBX_TEST_H
#include "vbx.h"
#include "vbx_port.h"
#ifdef __cplusplus
extern "C" {
#endif




#define MAX_PRINT_LENGTH 16

char *vbx_eng(double value, int digits);

void vbx_mxp_print_params();

double vbx_print_scalar_time_per(vbx_timestamp_t time_start,
								 vbx_timestamp_t time_stop,
								 double cycles_divisor,
								 char *divisor_str);

double vbx_print_scalar_time(vbx_timestamp_t time_start,
							 vbx_timestamp_t time_stop);

double vbx_print_vector_time_per(vbx_timestamp_t time_start,
								 vbx_timestamp_t time_stop,
								 double cycles_divisor,
								 char *divisor_str,
								 double scalar_time);

double vbx_print_vector_time(vbx_timestamp_t time_start,
							 vbx_timestamp_t time_stop,
							 double scalar_time);

int lfsr_32(int previous_value);

void test_inc_array_byte( int8_t *d, int size, int seed, int increase );
void test_init_array_byte( int8_t *d, int size, int seed );
void test_init_matrix_byte( int8_t *d, int height, int width, int seed );
void test_zero_array_byte( int8_t *d, int size);
void test_copy_array_byte( int8_t *out, int8_t *in, int size);
void test_print_hex_array_byte( int8_t *d, int size );
void test_print_hex_matrix_byte( int8_t *d, int row, int col, int width );
void test_print_array_byte( int8_t *d, int size );
void test_print_matrix_byte( int8_t *d, int row, int col, int width );
int test_verify_array_byte( int8_t *scalar_out, int8_t *vector_out, int size );
int test_range_array_byte( int8_t *scalar_out, int8_t *vector_out, int size, int range );

void test_inc_array_half( int16_t *d, int size, int seed, int increase );
void test_init_array_half( int16_t *d, int size, int seed );
void test_init_matrix_half( int16_t *d, int height, int width, int seed );
void test_zero_array_half( int16_t *d, int size);
void test_copy_array_half( int16_t *out, int16_t *in, int size);
void test_print_hex_array_half( int16_t *d, int size );
void test_print_hex_matrix_half( int16_t *d, int row, int col, int width );
void test_print_array_half( int16_t *d, int size );
void test_print_matrix_half( int16_t *d, int row, int col, int width );
int test_verify_array_half( int16_t *scalar_out, int16_t *vector_out, int size );
int test_range_array_half( int16_t *scalar_out, int16_t *vector_out, int size, int range );

void test_inc_array_word( int32_t *d, int size, int seed, int increase );
void test_init_array_word( int32_t *d, int size, int seed );
void test_init_matrix_word( int32_t *d, int height, int width, int seed );
void test_zero_array_word( int32_t *d, int size);
void test_copy_array_word( int32_t *out, int32_t *in, int size);
void test_print_hex_array_word( int32_t *d, int size );
void test_print_hex_matrix_word( int32_t *d, int row, int col, int width );
void test_print_array_word( int32_t *d, int size );
void test_print_matrix_word( int32_t *d, int row, int col, int width );
int test_verify_array_word( int32_t *scalar_out, int32_t *vector_out, int size );
int test_range_array_word( int32_t *scalar_out, int32_t *vector_out, int size, int range );

void test_inc_array_ubyte( uint8_t *d, int size, int seed, int increase );
void test_init_array_ubyte( uint8_t *d, int size, int seed );
void test_init_matrix_ubyte( uint8_t *d, int height, int width, int seed );
void test_zero_array_ubyte( uint8_t *d, int size);
void test_copy_array_ubyte( uint8_t *out, uint8_t *in, int size);
void test_print_hex_array_ubyte( uint8_t *d, int size );
void test_print_hex_matrix_ubyte( uint8_t *d, int row, int col, int width );
void test_print_array_ubyte( uint8_t *d, int size );
void test_print_matrix_ubyte( uint8_t *d, int row, int col, int width );
int test_verify_array_ubyte( uint8_t *scalar_out, uint8_t *vector_out, int size );
int test_range_array_ubyte( uint8_t *scalar_out, uint8_t *vector_out, int size, int range );

void test_inc_array_uhalf( uint16_t *d, int size, int seed, int increase );
void test_init_array_uhalf( uint16_t *d, int size, int seed );
void test_init_matrix_uhalf( uint16_t *d, int height, int width, int seed );
void test_zero_array_uhalf( uint16_t *d, int size);
void test_copy_array_uhalf( uint16_t *out, uint16_t *in, int size);
void test_print_hex_array_uhalf( uint16_t *d, int size );
void test_print_hex_matrix_uhalf( uint16_t *d, int row, int col, int width );
void test_print_array_uhalf( uint16_t *d, int size );
void test_print_matrix_uhalf( uint16_t *d, int row, int col, int width );
int test_verify_array_uhalf( uint16_t *scalar_out, uint16_t *vector_out, int size );
int test_range_array_uhalf( uint16_t *scalar_out, uint16_t *vector_out, int size, int range );

void test_inc_array_uword( uint32_t *d, int size, int seed, int increase );
void test_init_array_uword( uint32_t *d, int size, int seed );
void test_init_matrix_uword( uint32_t *d, int height, int width, int seed );
void test_zero_array_uword( uint32_t *d, int size);
void test_copy_array_uword( uint32_t *out, uint32_t *in, int size);
void test_print_hex_array_uword( uint32_t *d, int size );
void test_print_hex_matrix_uword( uint32_t *d, int row, int col, int width );
void test_print_array_uword( uint32_t *d, int size );
void test_print_matrix_uword( uint32_t *d, int row, int col, int width );
int test_verify_array_uword( uint32_t *scalar_out, uint32_t *vector_out, int size );
int test_range_array_uword( uint32_t *scalar_out, uint32_t *vector_out, int size, int range );

#if VBX_SIMULATOR
#define VBX_TEST_PASS()       VBX_S{ printf("\nTest passed!\n"); }VBX_E
#define VBX_TEST_FAIL(errors) VBX_S{ printf("\nTest failed with %d errors.\n", \
                                            (int)(errors));}VBX_E
#else
#define VBX_TEST_PASS()       VBX_S{ printf("\nTest passed!\n"); putchar(4); }VBX_E
#define VBX_TEST_FAIL(errors) VBX_S{ printf("\nTest failed with %d errors.\n", \
                                            (int)(errors)); putchar(4); }VBX_E
#endif

#define VBX_TEST_END(errors) \
	VBX_S{ \
		if (errors) { \
			VBX_TEST_FAIL(errors); \
		} else { \
			VBX_TEST_PASS(); \
		} \
	}VBX_E

// --------------------------------------------------------

int vbx_test_init();
#if __ARM_ARCH_7A__
int vbx_zynq_remap_ddr_uncached();
int vbx_zynq_set_instr_port_normal_uncached();
int vbx_zynq_set_instr_port_device_memory();
#define SEV() __asm__ __volatile__( "SEV\n" )
#endif

// --------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif //__VBX_TEST_H
