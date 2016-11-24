#include <stdio.h>
#include <stdint.h>
#include "app_profile.h"
#include <ti/csl/csl_tsc.h>

#define MAX_TIMER_CNT	(0xFFFFFFFFFFFFFFFF)
#define DSP_FREQ_IN_MHZ			1400

void GET_TIME(uint64_t *time) {
	*time = CSL_tscRead();
}

void PRINT_RUNTIME(char *disp_string, uint64_t start_time) {
	uint64_t end_time, total_time;
	end_time = CSL_tscRead();

	// check for single overflow condition. This is not possible since the counter will not overflow for ~417 years with 1.4GHz clock rate!
	if(end_time < start_time) {
		total_time = MAX_TIMER_CNT - start_time + end_time;
	} else {
		total_time = end_time - start_time;
	}

	printf("%s = %f usec", disp_string, (float)total_time / DSP_FREQ_IN_MHZ);
}
