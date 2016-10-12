#ifndef _DEBUG_CONTROL_H_
#define _DEBUG_CONTROL_H_
#include <stdio.h>
#include <assert.h>

#define REL_INFO(ARGS...)	printf(ARGS)
#define REL_ASSERT(ARGS...) assert(ARGS)

#if defined(MXP_PROFILE) && !defined(VBX_SIMULATOR)
#define DBG_INFO(ARGS...)
#define DBG_ASSERT(ARGS...)
#else
#define DBG_INFO(ARGS...) printf(ARGS)
#define DBG_ASSERT(ARGS...)	assert(ARGS)
#endif

#ifdef VBX_SIMULATOR
#define DBG_MAPS(x)		x
#else
#define DBG_MAPS(x)
#endif

#endif // _DEBUG_CONTROL_H_
