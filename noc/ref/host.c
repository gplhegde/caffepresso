#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <e-hal.h>
#include "time.h"
#include "papi.h"

//function to get wall clock time
long_long gettime(){
	return PAPI_get_virt_usec();
}

int main(int argc, char** argv)
{
	// setup PAPI
	long_long start, end;

	// size of transfer
	int NUM_ROW = atoi(argv[1]);
	int NUM_COL = atoi(argv[2]);
	int NUM_KERNEL_ROW = atoi(argv[3]);
	int NUM_KERNEL_COL = atoi(argv[4]);
	printf("Inputs NUM_ROW=%d, NUM_COL=%d, NUM_KERNEL_ROW=%d,NUM_KERNEL_COL=%d\n",
			NUM_ROW, NUM_COL, NUM_KERNEL_ROW, NUM_KERNEL_COL);

	// Local Variables
	e_platform_t platform;
	e_epiphany_t dev;
	e_mem_t emem;

	//initialize device
	e_init(NULL);
	e_reset_system();
	e_get_platform_info(&platform);
	
	// Open a workgroup
	e_open(&dev, 0, 0, platform.rows, platform.cols);

	// Load PE side and start SMVM
	e_load_group("pe.srec", &dev, 0, 0, platform.rows, platform.cols, E_FALSE);
	int src_x, src_y;
	for(src_x=0;src_x<platform.rows;src_x++) {
		for(src_y=0;src_y<platform.cols;src_y++) {
			e_write(&dev, src_x, src_y, (off_t)0x7100, &NUM_ROW, sizeof(unsigned));
			e_write(&dev, src_x, src_y, (off_t)0x7104, &NUM_COL, sizeof(unsigned));
			e_write(&dev, src_x, src_y, (off_t)0x7108, &NUM_KERNEL_ROW, sizeof(unsigned));
			e_write(&dev, src_x, src_y, (off_t)0x710C, &NUM_KERNEL_COL, sizeof(unsigned));
		}
	}
	start=gettime();
	e_start_group(&dev);

	// Read Timer values
	unsigned done = 0;
	while(done != 1)
	{
		done=1;
		e_read(&dev, 0, 0, 0x7000, &done, sizeof(unsigned));
	} 
	end=gettime();

	usleep(10000);
	printf("PAPI Time=%lldus\n",(end-start));
	
	unsigned clocks;
	e_read(&dev, 0, 0, (off_t)0x7004, &clocks, sizeof(unsigned));
	printf("Counter Clocks: clocks=%d cycles\n", (int)clocks);

	// Close the workgroup
	e_close(&dev);
	
	//Finalize e-platform connection
	e_finalize();

	return 0;
}
