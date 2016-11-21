#ifndef _DEBUG_CONTROL_H_
#define _DEBUG_CONTROL_H_
#include <stdio.h>
#include <assert.h>

#define REL_INFO(ARGS...)	printf(ARGS)
#define REL_ASSERT(ARGS...) assert(ARGS)

#if defined(DSP_PROFILE)
#define DBG_INFO(ARGS...)
#define DBG_ASSERT(ARGS...)
#else
#define DBG_INFO(ARGS...) printf(ARGS)
#define DBG_ASSERT(ARGS...)	assert(ARGS)

#define MASTER_PRINT(ARGS...) \
	if(DNUM == MASTER_CORE) printf(ARGS)
#endif


#endif // _DEBUG_CONTROL_H_
