#ifndef _DEBUG_CONTROL_H_
#define _DEBUG_CONTROL_H_
#include <stdio.h>
#include <assert.h>

#define REL_INFO(ARGS...)	printf(ARGS)
#define REL_ASSERT(ARGS...) assert(ARGS)

#define DBG_INFO(ARGS...) printf(ARGS)
#define DBG_ASSERT(ARGS...)	assert(ARGS)

#define DBG_MAPS(x)

#endif // _DEBUG_CONTROL_H_
