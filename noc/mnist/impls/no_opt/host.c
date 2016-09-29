/*
 * 50 L1 maps
 * 128 L2 maps
 * 10 L1-to-L2 edges per L2 map
 * Unroll-factor 1
 * Filter2D not unrolled
 * Buffers - 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <e-hal.h>

#include "papi.h"
#include "common.h"

//function to get wall clock time
long_long gettime(){
	return PAPI_get_virt_usec();
}

//functions declarations
void random_shards(data_t **L1_kernel, data_t **L2_kernel, unsigned **L1_L2_edge_map);
void init_constants(uint16_t **L1_maps, uint16_t **L2_maps, data_t **L1_kernel, float *L1_kernel_scale, data_t **L2_kernel, float *L2_kernel_scale, unsigned **L1_L2_edge_map);
void init_test_image(data_t *image);
unsigned get_global_address(unsigned target_pe, unsigned offset);
void load_stack(unsigned i, unsigned j, unsigned coreid, e_epiphany_t *dev, unsigned timesteps);
unsigned l1map_global_addr(unsigned pe, unsigned offset);
unsigned l2kernel_global_addr(unsigned pe, unsigned offset);
unsigned l2kernel_scale_global_addr(unsigned pe, unsigned offset);
void filter2D(uint8_t *kernel, uint16_t *src, uint8_t *src1, float *dest, unsigned kernel_width, unsigned src_width, float scale_fix, int isL1);
void normalize(float *input, unsigned input_size);
void subsample(float *input, uint16_t *dest, unsigned input_size, unsigned factor);

data_t *image, **L1_kernel, **L2_kernel;
uint16_t **L1_maps, **L2_maps;
uint16_t **L1_maps_parr, **L2_maps_parr;
float *L1_kernel_scale, *L2_kernel_scale, **L1_kernel_scale_shard, **L2_kernel_scale_shard;
unsigned **L1_L2_edge_map, **L1_L2_edge_kernel;
unsigned *l1map_c, *l2map_c;
data_t **L1_kernel_shard, **L2_kernel_shard;
unsigned **l1map_addr_shard, **l2kernel_addr_shard, **l2kernel_scale_addr_shard;

unsigned **L1_shard_map, **L2_shard_map;

int main(int argc, char **argv){

	unsigned timesteps = atoi(argv[1]);

	printf("Total Deep Learning timesteps = %d\n",timesteps);
	fflush(stdout);

	//declare variables and events to monitor
	unsigned i,j,k,curr_address;
	long_long t0, t1;

	/****** sequential solver ******/
	//malloc vectors
	image = (data_t *)malloc(SIZE_IMAGE*sizeof(data_t));
	L1_maps = (uint16_t **)malloc(L1_MAPS*sizeof(uint16_t *));
	L1_maps_parr = (uint16_t **)malloc(L1_MAPS*sizeof(uint16_t *));
	L2_maps = (uint16_t **)malloc(L2_MAPS*sizeof(uint16_t *));
	L2_maps_parr = (uint16_t **)malloc(L2_MAPS*sizeof(uint16_t *));
	L1_kernel = (data_t **)malloc(L1_MAPS*sizeof(data_t *));
	L1_kernel_scale = (float *)malloc(L1_MAPS*sizeof(float));
	L2_kernel = (data_t **)malloc(L2_MAPS*sizeof(data_t *));
	L2_kernel_scale = (float *)malloc(L2_MAPS*sizeof(float));
	L1_L2_edge_map = (unsigned **)malloc(L2_MAPS*sizeof(unsigned *));
//	L1_L2_edge_kernel = (unsigned **)malloc(L2_MAPS*sizeof(unsigned *));
	L1_shard_map = (unsigned **)malloc(NUM_PES*sizeof(unsigned *));
	L2_shard_map = (unsigned **)malloc(NUM_PES*sizeof(unsigned *));

	for (i=0;i<NUM_PES;i++){
		L1_shard_map[i] = (unsigned *)malloc(L1_MAX_MAPS*sizeof(unsigned));
		L2_shard_map[i] = (unsigned *)malloc(L2_MAX_MAPS*sizeof(unsigned));
	}

	for (i=0;i<L1_MAPS;i++){
		L1_maps[i] = (uint16_t *)malloc(L1_MAP_SIZE*sizeof(uint16_t));
		L1_maps_parr[i] = (uint16_t *)malloc(L1_MAP_SIZE*sizeof(uint16_t));
		L1_kernel[i] = (data_t *)malloc(L1_KERNEL_SIZE*sizeof(data_t));
	}
	for (i=0;i<L2_MAPS;i++){	
		L2_maps[i] = (uint16_t *)malloc(L2_MAP_SIZE*sizeof(uint16_t));
		L2_maps_parr[i] = (uint16_t *)malloc(L2_MAP_SIZE*sizeof(uint16_t));
		L2_kernel[i] = (data_t *)malloc(L2_KERNEL_SIZE*sizeof(data_t));
		L1_L2_edge_map[i] = (unsigned *)malloc(L1_TO_L2*sizeof(unsigned));
	}

	//initialize data
	init_test_image(image);
	init_constants(L1_maps,L2_maps,L1_kernel,L1_kernel_scale,L2_kernel,L2_kernel_scale,L1_L2_edge_map);

	//partition maps into 16 random shards
	random_shards(L1_kernel, L2_kernel, L1_L2_edge_map);

	int t;
	uint16_t *dummy = 0;//null dummy pointer
	//to store intermediate results
	unsigned conv_size = 22;//IMG_WIDTH - L1_KERNEL_WIDTH + 1;
	float *filter2D_out = (float *)malloc(conv_size*conv_size*sizeof(float));

	//start taking note of time, and start event counters
	t0=gettime();

	//compute the deep learning stack sequentially
	for (t=0;t<timesteps;t++){
		//do Layer 1
		for (i=0;i<L1_MAPS;i++){
			filter2D(L1_kernel[i],dummy,image,filter2D_out,L1_KERNEL_WIDTH,IMG_WIDTH,L1_kernel_scale[i],1);
			normalize(filter2D_out, conv_size);
			subsample(filter2D_out,L1_maps[i],conv_size,DOWN_FAC1);
		}
		
		//do Layer 2
		unsigned in_size = 11;//conv_size/DOWN_FAC1;//11 
		conv_size = 7;//in_size - L2_KERNEL_WIDTH + 1;//7
		float *filter2D_out_L2 = (float *)malloc(conv_size*conv_size*sizeof(float));
		for (i=0;i<L2_MAPS;i++){
			float accum[conv_size*conv_size];
			for (j=0;j<conv_size*conv_size;j++)
				accum[j] = 0;
			for (j=0;j<L1_TO_L2;j++){
				filter2D(L2_kernel[i],L1_maps[L1_L2_edge_map[i][j]],image,filter2D_out_L2,L2_KERNEL_WIDTH,in_size,L2_kernel_scale[i],0);
				for (k=0;k<conv_size*conv_size;k++){
					accum[k] += filter2D_out_L2[k];
				}
			}
			float third_scale = 0.4f;//(L1_TO_L2)/(L2_KERNEL_SIZE);
			for (k=0;k<conv_size*conv_size;k++){
				accum[k] = accum[k]*third_scale;
				if (accum[k] <= 0)
					accum[k] = 0;
			}
			
			float *naccum = (float *)malloc((conv_size+1)*(conv_size+1)*sizeof(float));
			naccum[0] = accum[0];
			for (k=0;k<conv_size;k++)
				naccum[k+1] = accum[k];
			for (j=0;j<conv_size;j++){
				for (k=0;k<conv_size+1;k++){
					if (k == 0)
						naccum[(j+1)*(conv_size+1)+k] = accum[j*conv_size];
					else
						naccum[(j+1)*(conv_size+1)+k] = accum[j*conv_size+k-1];
				}
			}
			subsample(naccum,L2_maps[i],8,DOWN_FAC2);
		}
	}
	
	//stop taking note of time, and stop event counters
	t1=gettime();

	printf("[SEQUENTIAL]Runtime=%lld\n",t1-t0);
	fflush(stdout);

	/*******************************************************************/
	/************************ parallel solver **************************/
	/*******************************************************************/

	e_platform_t platform;
	e_epiphany_t dev;

	//initialize device
	e_init(NULL);
	e_reset_system();
	e_get_platform_info(&platform);

	// Open a workgroup
	e_open(&dev, 0, 0, platform.rows, platform.cols);

//	unsigned huh[8192] = {0};

	// CORE initializations
	unsigned done = 0xbeefdead;
	for (i = 0; i < platform.rows; i++){
		for (j = 0; j < platform.cols; j++){
			unsigned coreid = i*platform.rows + j;
			load_stack(i,j,coreid,&dev,timesteps);
			//e_write(&dev,i,j,0x0,&huh,8192*sizeof(unsigned));
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

	//verify functionality
	for (i=0;i<platform.rows;i++){
		for (j=0;j<platform.cols;j++){
			uint16_t read[L1_MAX_MAPS*L1_MAP_SIZE];
			uint16_t read1[L2_MAX_MAPS*L2_MAP_SIZE];
			e_read(&dev,i,j,L1_MAPS_ADDR,read,L1_MAX_MAPS*L1_MAP_SIZE*sizeof(uint16_t));
			e_read(&dev,i,j,L2_MAPS_ADDR,read1,L2_MAX_MAPS*L2_MAP_SIZE*sizeof(uint16_t));
			int k;
			for (k=0;k<l1map_c[i*4+j];k++){
				unsigned id = L1_shard_map[i*4+j][k];
				int n;
				for (n=0;n<L1_MAP_SIZE;n++)
					L1_maps_parr[id][n] = read[k*L1_MAP_SIZE+n];
			}
			for (k=0;k<l2map_c[i*4+j];k++){
				unsigned id = L2_shard_map[i*4+j][k];
				int n;
				for (n=0;n<L2_MAP_SIZE;n++)
					L2_maps_parr[id][n] = read1[k*L2_MAP_SIZE+n];
			}
		}
	}

	int cnt_f = 0, cnt_p = 0;
	for (i=0;i<L1_MAPS;i++){
		int mf = 0;
		for (j=0;j<L1_MAP_SIZE;j++){
			if (L1_maps[i][j] != L1_maps_parr[i][j]){
				cnt_f++;
				mf = 1;
				//printf("L1: [map %d,%d] seq = %u, parr = %u\n",i,cnt_f,L1_maps[i][j],L1_maps_parr[i][j]);
			} else
				cnt_p++;
		}
//		if (mf)
//			printf("map %d fails\n",i);
	}
	
	printf("L1 fail = %d, pass = %d\n",cnt_f,cnt_p);

	cnt_f = 0, cnt_p = 0;
	for (i=0;i<L2_MAPS;i++){
		int mf = 0;
		for (j=0;j<L2_MAP_SIZE;j++){
			if (L2_maps[i][j] != L2_maps_parr[i][j]){
				cnt_f++;
				mf = 1;
				//printf("L2: [%d] seq = %u, parr = %u\n",cnt_f,L2_maps[i][j],L2_maps_parr[i][j]);
			} else
				cnt_p++;
		}
//		if (mf)
//			printf("map %d fails\n",i);
	}
	
	printf("L2 fail = %d, pass = %d\n",cnt_f,cnt_p);
	
	// Close the workgroup
	e_close(&dev);

	// Finalize e-platform connection
	e_finalize();
	
	return 0;
}

void init_test_image(data_t *image){

	int i;
	for (i=0;i<SIZE_IMAGE;i++){
		image[i] = ((data_t) rand()%2);//0 or 1 black-and-white image
		//image[i] = 1;
	}
}

void init_constants(uint16_t **L1_maps, uint16_t **L2_maps, data_t **L1_kernel, float *L1_kernel_scale, data_t **L2_kernel, float *L2_kernel_scale, unsigned **L1_L2_edge_map){

	int m,i;	
	for(m=0;m<L1_MAPS;m++) {
		//initialize L1_maps to zeros
		for (i=0;i<L1_MAP_SIZE;i++)
			L1_maps[m][i] = 0;
		//initialize L1_kernel randomly
		float scale = 0;
		for (i=0;i<L1_KERNEL_SIZE;i++){
			L1_kernel[m][i] = ((data_t) rand()%255); //between 0 and 255
			//L1_kernel[m][i] = 2; 
			scale += L1_kernel[m][i];
		}
		L1_kernel_scale[m] = 1/scale;
	}

	for (m=0;m<L2_MAPS;m++){
		//intialize L2_maps to zeros
		for (i=0;i<L2_MAP_SIZE;i++)
			L2_maps[m][i] = 0;
		//intialize L2_kernel randomly
		float scale = 0;
		for (i=0;i<L2_KERNEL_SIZE;i++){
			L2_kernel[m][i] = ((data_t) rand()%255); //between 0 and 255
			scale += L2_kernel[m][i];
		}
		L2_kernel_scale[m] = 1/scale;
		//initialize connection matrix
		for (i=0;i<L1_TO_L2;i++){
			L1_L2_edge_map[m][i] = rand()%L1_MAPS;
		}
	}
}

void random_shards(data_t **L1_kernel, data_t **L2_kernel, unsigned **L1_L2_edge_map){

	int i,j,k;

	//global mallocs
	L1_kernel_shard = (data_t **)malloc(NUM_PES*sizeof(data_t *));
	L2_kernel_shard = (data_t **)malloc(NUM_PES*sizeof(data_t *));
	L1_kernel_scale_shard = (float **)malloc(NUM_PES*sizeof(float *));
	L2_kernel_scale_shard = (float **)malloc(NUM_PES*sizeof(float *));
	l1map_addr_shard = (unsigned **)malloc(NUM_PES*sizeof(unsigned *));
	for (i=0;i<NUM_PES;i++){
		L1_kernel_shard[i] = (data_t *)malloc(L1_MAX_MAPS*L1_KERNEL_SIZE*sizeof(data_t));
		L2_kernel_shard[i] = (data_t *)malloc(L2_MAX_MAPS*L2_KERNEL_SIZE*sizeof(data_t));
		L1_kernel_scale_shard[i] = (float *)malloc(L1_MAX_MAPS*sizeof(float));
		L2_kernel_scale_shard[i] = (float *)malloc(L2_MAX_MAPS*sizeof(float));
		l1map_addr_shard[i] = (unsigned *)malloc(L1_TO_L2*L2_MAX_MAPS*sizeof(unsigned));
	}
	
	//local variables
	unsigned *placement_L1 = (unsigned *)malloc(L1_MAPS*sizeof(unsigned));
	unsigned *placement_L2 = (unsigned *)malloc(L2_MAPS*sizeof(unsigned));
	unsigned *l1map_addr = (unsigned *)malloc(L1_MAPS*sizeof(unsigned));
	l1map_c = (unsigned *)malloc(NUM_PES*sizeof(unsigned));
	l2map_c = (unsigned *)malloc(NUM_PES*sizeof(unsigned));

	//init counts
	for (i=0;i<NUM_PES;i++){
		l1map_c[i] = 0;
		l2map_c[i] = 0;
	}

	//assign random placement for each L1 map
	for (i=0;i<L1_MAPS;i++){
		unsigned pe = ((unsigned)rand()%NUM_PES);
		//we need to check that this PE is not over the limit
		unsigned orig_pe = pe;
		BOOL success = 0;
		while (!success){
			if (l1map_c[pe] < L1_MAX_MAPS){
				placement_L1[i] = pe;
				j = l1map_c[pe]*L1_KERNEL_SIZE;
				for (k=j;k<j+L1_KERNEL_SIZE;k++){
					L1_kernel_shard[pe][k] = L1_kernel[i][k%L1_KERNEL_SIZE];
				}
				L1_shard_map[pe][l1map_c[pe]] = i;
				L1_kernel_scale_shard[pe][l1map_c[pe]] = L1_kernel_scale[i];
				l1map_addr[i] = l1map_global_addr(pe,l1map_c[pe]);
				l1map_c[pe]++;
				success = 1;
			} else {
				pe = (pe+1)%NUM_PES;
				if (pe == orig_pe){
					printf("Cannot find PE to place L1 map in. Exiting.\n");
					exit(1);
				}
			}
		}
	}

	//assign random placement for each L2 map
	for (i=0;i<L2_MAPS;i++){
		unsigned pe = ((unsigned)rand()%NUM_PES);
		//we need to check that this PE is not over the limit
		unsigned orig_pe = pe;
		BOOL success = 0;
		while (!success){
			if (l2map_c[pe] < L2_MAX_MAPS){
				placement_L2[i] = pe;
				j = l2map_c[pe]*L2_KERNEL_SIZE;
				for (k=j;k<j+L2_KERNEL_SIZE;k++){
					L2_kernel_shard[pe][k] = L2_kernel[i][k%L2_KERNEL_SIZE];
				}
				L2_shard_map[pe][l2map_c[pe]] = i;
				L2_kernel_scale_shard[pe][l2map_c[pe]] = L2_kernel_scale[i];
				l2map_c[pe]++;
				success = 1;
			} else {
				pe = (pe+1)%NUM_PES;
				if (pe == orig_pe){
					printf("Cannot find PE to place L2 map in. Exiting.\n");
					exit(1);
				}
			}
		}
	}

	//assign L1 -> L2 edge stuff
	for (i=0;i<NUM_PES;i++){
		int index = 0;
		for (j=0;j<L2_MAPS;j++){
			if (placement_L2[j] == i){
				for (k=0;k<L1_TO_L2;k++){
					l1map_addr_shard[i][index] = l1map_addr[L1_L2_edge_map[j][k]];
					index++;
				}
			}
		}
	}
}

unsigned l1map_global_addr(unsigned pe, unsigned offset){
	
	int col = pe%4;
	int row = (pe-col)/4;
	unsigned z = L1_MAPS_ADDR + L1_MAP_SIZE*offset*sizeof(uint16_t);

	unsigned address = 0x80800000 + row*(0x4000000) + col*(0x100000) + z;

	return address;
}

unsigned l2kernel_global_addr(unsigned pe, unsigned offset){
	
	int col = pe%4;
	int row = (pe-col)/4;
	unsigned z = L2_KERNEL_ADDR + L2_KERNEL_SIZE*offset;

	unsigned address = 0x80800000 + row*(0x4000000) + col*(0x100000) + z;

	return address;
}

unsigned l2kernel_scale_global_addr(unsigned pe, unsigned offset){
	
	int col = pe%4;
	int row = (pe-col)/4;
	unsigned z = L2_KERNEL_SCALE_ADDR + 4*offset;

	unsigned address = 0x80800000 + row*(0x4000000) + col*(0x100000) + z;

	return address;
}


void load_stack(unsigned i, unsigned j, unsigned coreid, e_epiphany_t *dev, unsigned timesteps) {

	//for switching between debugging prints on/off
	unsigned de = 16;

	unsigned curr_address = 0x3ffc;
	e_write(dev,i,j,curr_address,&timesteps,sizeof(unsigned));
	if (coreid==de)
		printf("Writing timesteps to 0x%x\n",curr_address);
	
	curr_address += 4;
	e_write(dev,i,j,curr_address,&l1map_c[coreid],sizeof(unsigned));
	if (coreid==de)
		printf("Writing l1map_count to 0x%x\n",curr_address);

	curr_address += 4;
	e_write(dev,i,j,curr_address,&l2map_c[coreid],sizeof(unsigned));
	if (coreid==de)
		printf("Writing l2map_count to 0x%x\n",curr_address);
	
	//curr_address += 4;
	curr_address = BASE_ADDR;
	e_write(dev,i,j,curr_address,image,SIZE_IMAGE*sizeof(data_t));
	if (coreid==de)
		printf("Writing image to 0x%x\n",curr_address);
	
/*	e_write(dev,i,j,curr_address,window,WINDOW_SIZE*sizeof(float));
	if (coreid==de)
		printf("Writing window to 0x%x\n",curr_address);

	curr_address += sizeof(float)*WINDOW_SIZE;
*/	
	//curr_address += sizeof(uint8_t)*SIZE_IMAGE;
	curr_address = L1_KERNEL_ADDR;
	e_write(dev,i,j,curr_address,L1_kernel_shard[coreid],l1map_c[coreid]*L1_KERNEL_SIZE*sizeof(uint8_t));
	if (coreid==de)
		printf("Writing L1_kernel to 0x%x\n",curr_address);
	
	//curr_address += sizeof(uint8_t)*L1_MAX_MAPS*L1_KERNEL_SIZE;
	curr_address = L1_KERNEL_SCALE_ADDR;
	e_write(dev,i,j,curr_address,L1_kernel_scale_shard[coreid],l1map_c[coreid]*sizeof(float));
	if (coreid==de)
		printf("Writing L1_kernel_scale to 0x%x\n",curr_address);
	
	//curr_address += sizeof(float)*L1_MAX_MAPS;
	curr_address = L2_KERNEL_ADDR;
	e_write(dev,i,j,curr_address,L2_kernel_shard[coreid],l2map_c[coreid]*L2_KERNEL_SIZE*sizeof(uint8_t));
	if (coreid==de)
		printf("Writing L2_kernel to 0x%x\n",curr_address);
	
	//curr_address += sizeof(uint8_t)*L2_MAX_MAPS*L2_KERNEL_SIZE;
	curr_address = L2_KERNEL_SCALE_ADDR;
	e_write(dev,i,j,curr_address,L2_kernel_scale_shard[coreid],l2map_c[coreid]*sizeof(float));
	if (coreid==de)
		printf("Writing L2_kernel_scale to 0x%x\n",curr_address);

	//curr_address += sizeof(float)*L2_MAX_MAPS;
	//curr_address += sizeof(uint16_t)*L1_MAX_MAPS*L1_MAP_SIZE;
	//curr_address += sizeof(uint16_t)*L2_MAX_MAPS*L2_MAP_SIZE;
	curr_address = L2_L1_MAP_ADDR;
	e_write(dev,i,j,curr_address,l1map_addr_shard[coreid],l2map_c[coreid]*L1_TO_L2*sizeof(unsigned));
	if (coreid==de)
		printf("Writing l1map_addr to 0x%x\n",curr_address);
	
	//curr_address += sizeof(unsigned)*L2_MAX_MAPS*L1_TO_L2;
	//curr_address += sizeof(uint16_t)*UNROLL_FACTOR*L1_MAP_SIZE;
//	curr_address = L2_L2_KERNEL_ADDR;
//	e_write(dev,i,j,curr_address,l2kernel_addr_shard[coreid],l2map_c[coreid]*L1_TO_L2*sizeof(unsigned));
//	if (coreid==de)
//		printf("Writing l2kernel_addr to 0x%x\n",curr_address);
	
	//curr_address += sizeof(unsigned)*L2_MAX_MAPS*L1_TO_L2;
	//curr_address += sizeof(uint8_t)*UNROLL_FACTOR*L2_KERNEL_SIZE;
//	curr_address = L2_L2_KERNEL_SCALE_ADDR;
//	e_write(dev,i,j,curr_address,l2kernel_scale_addr_shard[coreid],l2map_c[coreid]*L1_TO_L2*sizeof(unsigned));
//	if (coreid==de)
//		printf("Writing l2kernel_scale_addr to 0x%x\n",curr_address);
}

void filter2D(uint8_t *kernel, uint16_t *src, uint8_t *src1, float *dest, unsigned kernel_width, unsigned src_width, float scale_fix, int isL1){

	unsigned row,col,kernel_row,kernel_col,i;
	int cntr[kernel_width];
	unsigned o_cntr = 0;
	unsigned conv_size = src_width - kernel_width + 1;
	for (row=0;row<conv_size;row++){
		for (i=0;i<kernel_width;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<conv_size;col++){
			float sop = 0.0f;
			for (kernel_row=0;kernel_row<kernel_width;kernel_row++){
				for (kernel_col=0;kernel_col<kernel_width;kernel_col++){
					if (isL1)
						sop += kernel[kernel_row*kernel_width+kernel_col]*src1[cntr[kernel_row]+kernel_col];
					else
						sop += kernel[kernel_row*kernel_width+kernel_col]*src[cntr[kernel_row]+kernel_col];
				}
			}
			sop = sop*scale_fix;
			if (isL1)
				*(dest + o_cntr) = sop > 0 ? sop : 0;//non-linearity
			else 
				*(dest + o_cntr) = sop;//non-linearity

			o_cntr++;
			
			for (i=0;i<kernel_width;i++)
				cntr[i]++;
		}
	}
}

void normalize(float *input, unsigned input_size){
	int i;
	float max = 0.0f;
	unsigned total = input_size*input_size;
	for (i=0;i<total;i++){
		if (input[i] > max)
			max = input[i];
	}
	float scale = 255.0/max;
	for (i=0;i<total;i++)
		input[i] = (input[i]*scale);
}

void subsample(float *input, uint16_t *dest, unsigned input_size, unsigned factor){
	int i,j;
	int cntr = 0, cntr1 = 0;
	int s = input_size/factor;
	for (i=0;i<s;i++){
		for (j=0;j<s;j++){
			*(dest + cntr) = ((uint16_t)(input[cntr1]+0.5f));
			cntr++;
			cntr1 += 2;
		}
	}
}
