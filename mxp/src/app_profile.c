#include <stdio.h>
#include "app_profile.h"
#if defined(MXP_PROFILE) && !defined(USE_MXP_SIM)
#include "vbx_test.h"
#endif

void GET_TIME(vbx_timestamp_t *time) {
#if defined(MXP_PROFILE) && !defined(USE_MXP_SIM)
	*time = vbx_timestamp();
#endif
}

void PRINT_RUNTIME(char *dispString, vbx_timestamp_t startTime) {

#if defined(MXP_PROFILE) && !defined(USE_MXP_SIM)
	vbx_timestamp_t endTime = vbx_timestamp();
	vbx_print_scalar_time(startTime, endTime);
#endif
}
