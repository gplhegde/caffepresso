#ifndef _APP_PROFILE_H_
#define _APP_PROFILE_H_
#include "vbx.h"
#include "vbx_port.h"

void GET_TIME(vbx_timestamp_t *time);

void PRINT_RUNTIME(char *dispString, vbx_timestamp_t startTime);

#endif // _APP_PROFILE_H_
