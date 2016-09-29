#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <e-hal.h>

#include "papi.h"

#include "parameters.h"
#include "types.h"
#include "address.h"

//function to get wall clock time
long_long gettime(){
	return PAPI_get_virt_usec();
}

int main(int argc, char **argv){

	int num_maps = atoi(argv[1]);

	//declare variables and events to monitor
	unsigned i,j,k,curr_address;
	unsigned init[8192] = {0};
	long_long t0, t1;

	e_platform_t platform;
	e_epiphany_t dev;

	//initialize device
	e_init(NULL);
	e_reset_system();
	e_get_platform_info(&platform);

	// Open a workgroup
	e_open(&dev, 0, 0, platform.rows, platform.cols);

	// CORE initializations
	unsigned done = 0xbeefdead;
	for (i = 0; i < platform.rows; i++){
		for (j = 0; j < platform.cols; j++){
			unsigned coreid = i*platform.rows + j;
			e_write(&dev,i,j,0x0,&init,8192*sizeof(unsigned));
			e_write(&dev,i,j,GLOBAL_CONSTANTS_ADDR,&num_maps,sizeof(unsigned));
			//for reseting done flag
			if (i == 0 && j == 0)	
				e_write(&dev,i,j,DONE_ADDR,&done,sizeof(unsigned));
		}
	}

	//load srec
	e_load_group("pe.srec", &dev, 0, 0, platform.rows, platform.cols, E_FALSE);
	
	//start taking note of time, and start event counters
	t0=gettime();

	e_start_group(&dev);

	done = 0;
	while (done != 0xdeadbeef){
		e_read(&dev,0,0,DONE_ADDR,&done,sizeof(unsigned));
	}

	//stop taking note of time, and stop event counters
	t1=gettime();

	printf("[PARALLEL]Runtime=%lld\n",t1-t0);
	fflush(stdout);

	// Close the workgroup
	e_close(&dev);

	// Finalize e-platform connection
	e_finalize();
	
	return 0;
}
