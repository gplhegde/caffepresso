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
* DATE: 31 Oct 2015
*/
//#include "app_config.h"
#include "vbx.h"

#ifndef MAX
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#endif

//=============================================================
// Various pixel representations
typedef unsigned char PIXEL_U8;
typedef unsigned short PIXEL_U16;
typedef signed char PIXEL_S8;
typedef signed short PIXEL_S16;
typedef float FL_IMG_PIXEL;
typedef float FL_MAP_PIXEL;
typedef float FL_KERNEL;
typedef float FL_FC_WEIGHT;
typedef float FL_FC_BIAS;
typedef PIXEL_S16	FP_IMG_PIXEL;	       // Fixed point input image pixel representation
typedef PIXEL_S16	FP_MAP_PIXEL;	       // Fixed point feature map pixel
typedef signed short 	FP_KERNEL;
typedef signed short 	FP_FC_WEIGHT;
typedef signed short 	FP_FC_BIAS;

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
} APP_STATUS_E;
//=============================================================
//=============================================================
// Map arrangement mode
typedef enum {
	MAP_ISOLATED,
	MAP_CONCAT
} MAP_LAYOUT_E;
//=============================================================
//=============================================================
typedef enum {
	FLOAT_POINT,
	FIXED_POINT
} LYR_ARITH_MODE_E;
//=============================================================
//=============================================================
typedef enum {
	SCALAR,
	VECTOR_NEON,
	VECTOR_MXP
} OPT_TYPE_E;
//=============================================================
//=============================================================
// Structure for performance counters 
typedef struct {
	// Start timestamp in clock cycles
	uint64_t startTime;
	// End timestamp in clock cycles
	uint64_t endTime;

} PERF_CNTR_T;
//=============================================================
//=============================================================
// test status flags
typedef enum {
	TEST_PASS,
	TEST_ABORT,
	TEST_FAIL	
} TEST_STATUS_E;

// test data mismatch info
typedef struct {
	int misMap;
	int misRow;
	int misCol;
	TEST_STATUS_E flag;
} CMP_STATUS_T;


#endif // _STRUCT_DEFS_H_

