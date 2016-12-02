#ifndef _STRUCT_DEFS_H_
#define _STRUCT_DEFS_H_
/*
* DESCRIPTION: This file contains definitions of all major data structures and enumerations that 
* the MXP deep learning application uses.
*
* FILENAME: struct_defs.h 
*
* AUTHOR: Gopalakrishna Hegde
*
* DATE: 15 Oct 2016
*/
#include <stdint.h>

//=============================================================
// Various pixel representations
typedef unsigned char FIX_U8;
typedef unsigned short FIX_U16;
typedef signed char FIX_S8;
typedef float FLT_32;
typedef signed short FIX_S16;
typedef int32_t FIX_S32;

typedef FIX_S16 FIX_MAP;
typedef FIX_S16 FIX_KER;
typedef FLT_32 FLT_MAP;
typedef FLT_32 FLT_KER;
//=============================================================
//=============================================================
// Application status
typedef enum {
	FAILED,
	SUCCESS,
	INIT_DONE,
	MALLOC_FAIL,
	SP_MALLOC_FAIL,
	UNSUPPORTED_FEATURE,
	// Add extra status here
	STATUS_INVALID
} STATUS_E;
//=============================================================
//=============================================================
typedef enum {
	FLOAT_POINT,
	FIXED_POINT
} LYR_ARITH_MODE_E;
//=============================================================
//=============================================================
// test status flags
typedef enum {
	TEST_PASS,
	TEST_ABORT,
	TEST_FAIL
} TEST_STATUS_E;
//=============================================================
//=============================================================
// Structure for performance counters 
typedef struct {
	// Start timestamp in clock cycles
	uint64_t start_time;
	// End timestamp in clock cycles
	uint64_t end_time;

} PERF_CNTR_T;
//=============================================================
//=============================================================
// test data mismatch info
typedef struct {
	int mis_map;
	int mis_row;
	int mis_col;
	TEST_STATUS_E flag;
} CMP_STATUS_T;
//=============================================================

#endif // _STRUCT_DEFS_H_

