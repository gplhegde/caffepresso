/*
 * config.h
 *
 *  Created on: 29 Dec 2016
 *      Author: hgashok
 */

#ifndef CONFIG_H_
#define CONFIG_H_
#include <assert.h>
#define DSP_FREQ_GHZ	(1.35167993)

#define DSP_FREQ_MHZ	(1351.67993)

#define FIXED_POINT_GEMM	1

#define MASTER_CORE_ID (0)

#define L1_CACHE_LINE_SIZE (64)

#define SHARED_MEM_SEM	(2)

#define NO_GEMM_CORES 8

#define REL_ASSERT(ARGS...) assert(ARGS)

#define REL_INFO(ARGS...)	printf(ARGS)

typedef int16_t FIX_MAP;

typedef float FLT_MAP;

#if FIXED_POINT_GEMM
typedef short DTYPE;
#else
typedef float DTYPE;
#endif
#endif /* CONFIG_H_ */
