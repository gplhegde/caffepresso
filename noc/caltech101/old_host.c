#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <e-hal.h>

#include "papi.h"
#include "parameters.h"
#include "address.h"
#include "types.h"
#include "filter2D.h"
#include "pool_subsample.h"
#include "patch.h"

#define ERR_TOL 0.1

//function to get wall clock time
long_long gettime(){
	return PAPI_get_virt_usec();
}

//functions declarations
void random_shards(KERNEL_T **L1_kernel, KERNEL_T **L2_kernel, KERNEL_T **L3_kernel);
void init_weights(KERNEL_T **L1_kernel, SCALE_T *L1_kernel_scale, KERNEL_T **L2_kernel, SCALE_T *L2_kernel_scale, KERNEL_T **L3_kernel, SCALE_T *L3_kernel_scale);
void init_test_image(IMAGE_T *image);
//MEM_ADDR_T l1map_global_addr(unsigned pe, unsigned offset);
void pool_subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned window, unsigned stride, unsigned src_width, int isMaxPool);
void load_stack(unsigned i, unsigned j, unsigned coreid, e_epiphany_t *dev, unsigned timesteps);

//classification functions
static inline float dot_prod(float *pActivation , float *pWeight, int vLen);
static inline float dot_prod_rotate_weightVec(float *pFeatureVec, float *pWeightVec, int iN, int circShift);
static int index_of_max(float *pVec, int vLen);
void elm_classifier(float *pInputNeurons, int nInputNeurons, int nHiddenNeurons, int nOutputNeurons, float *pInputWt, float *pHiddenBias, float *pOutputWt);

IMAGE_T *image;
KERNEL_T **L1_kernel, **L2_kernel, **L3_kernel;
MAP_T  **L1_maps, **L2_maps, **L3_maps, **L1_maps_parr, **L2_maps_parr, **L3_maps_parr;
SCALE_T *L1_kernel_scale, *L2_kernel_scale, *L3_kernel_scale, **L1_kernel_scale_shard, **L2_kernel_scale_shard, **L3_kernel_scale_shard;
unsigned *l1map_c, *l2map_c, *l3map_c;
KERNEL_T **L1_kernel_shard, **L2_kernel_shard, **L3_kernel_shard;
MEM_ADDR_T **l1map_addr_shard, **l2map_addr_shard;

unsigned **L1_shard_map, **L2_shard_map, **L3_shard_map; //to rebuild L1/L2/L3 maps after compute on Epiphany is done

int main(int argc, char **argv){

	unsigned timesteps = atoi(argv[1]);

	printf("Total Deep Learning timesteps = %d\n",timesteps);	

	//declare variables and events to monitor
	unsigned i,j,k;
	long_long t0, t1;

	//malloc vectors
	image = (IMAGE_T *)malloc(IMAGE_SIZE*sizeof(IMAGE_T));
	L1_maps = (MAP_T **)malloc(L1_MAPS*sizeof(MAP_T *));
	L1_maps_parr = (MAP_T **)malloc(L1_MAPS*sizeof(MAP_T *));
	L2_maps = (MAP_T **)malloc(L2_MAPS*sizeof(MAP_T *));
	L2_maps_parr = (MAP_T **)malloc(L2_MAPS*sizeof(MAP_T *));
	L1_kernel = (KERNEL_T **)malloc(L1_MAPS*sizeof(KERNEL_T *));
	L1_kernel_scale = (SCALE_T *)malloc(L1_MAPS*sizeof(SCALE_T));
	L2_kernel = (KERNEL_T **)malloc(L2_MAPS*sizeof(KERNEL_T *));
	L2_kernel_scale = (SCALE_T *)malloc(L2_MAPS*sizeof(SCALE_T));	
	L1_shard_map = (unsigned **)malloc(NUM_PES*sizeof(unsigned *));
	L2_shard_map = (unsigned **)malloc(NUM_PES*sizeof(unsigned *));

	for (i=0;i<NUM_PES;i++){
		L1_shard_map[i] = (unsigned *)malloc(L1_MAX_MAPS_PER_ECORE*sizeof(unsigned));
		L2_shard_map[i] = (unsigned *)malloc(L2_MAX_MAPS_PER_ECORE*sizeof(unsigned));
	}

	for (i=0;i<L1_MAPS;i++){
		L1_maps[i] = (MAP_T *)malloc(L1_MAP_SIZE*sizeof(MAP_T));
		L1_maps_parr[i] = (MAP_T *)malloc(L1_MAP_SIZE*sizeof(MAP_T));
		L1_kernel[i] = (KERNEL_T *)malloc(L1_KERNEL_SIZE*sizeof(KERNEL_T));
	}
	for (i=0;i<L2_MAPS;i++){	
		L2_maps[i] = (MAP_T *)malloc(L2_MAP_SIZE*sizeof(MAP_T));
		L2_maps_parr[i] = (MAP_T *)malloc(L2_MAP_SIZE*sizeof(MAP_T));
		L2_kernel[i] = (KERNEL_T *)malloc(L2_KERNEL_SIZE*sizeof(KERNEL_T));
	}

	/********************************** INITIALIZATION OF ARRAYS **************************************/
	init_test_image(image);
	init_weights(L1_kernel,L1_kernel_scale,L2_kernel,L2_kernel_scale);
	//partition maps into 16 random shards
	random_shards(L1_kernel, L2_kernel);
	//read in weights for classification stage -- i've randomly initialized them for now..
	float *pInputWt = (float *)malloc(L2_MAPS*L2_MAP_SIZE*8*sizeof(float));
	float *pOutputWt = (float *)malloc(NUM_OUTPUT_NEURONS*NUM_HIDDEN_NEURONS*sizeof(float));
	float *pHiddenBias = (float *)malloc(NUM_HIDDEN_NEURONS*sizeof(float));

	for (i=0;i<L2_MAPS*L2_MAP_SIZE*8;i++)
		pInputWt[i] = rand()%5;
	for (i=0;i<NUM_OUTPUT_NEURONS*NUM_HIDDEN_NEURONS;i++)
		pOutputWt[i] = rand()%3;
	for (i=0;i<NUM_HIDDEN_NEURONS;i++)
		pHiddenBias[i] = rand()%2;

	/****************************************** CPU-ONLY solver ******************************************/
	int t;
	//start taking note of time, and start event counters
	t0=gettime();

	for (t=0;t<timesteps;t++){
		//accum vector for L1 maps, since it is fully-connected + it is add-convolve-once		
		MAP_T *accum_L1 = (MAP_T *)malloc(L1_MAP_SIZE*sizeof(MAP_T));//for L2, since it is fully-connected
		for (i=0;i<L1_MAP_SIZE;i++)
			accum_L1[i] = 0;

		/****************** LAYER 1 *********************/
		//to store intermediate results
		unsigned conv_size = IMAGE_WIDTH - L1_KERNEL_WIDTH + 1; //143
		INTERMEDIATE_T *filter2D_out = (INTERMEDIATE_T *)malloc(conv_size*conv_size*sizeof(INTERMEDIATE_T));
		for (i=0;i<L1_MAPS;i++){
			filter2D(L1_kernel[i],image,filter2D_out,L1_KERNEL_WIDTH,IMAGE_WIDTH,L1_kernel_scale[i]);
			pool_subsample(filter2D_out,L1_maps[i],WINDOW_SIZE1,DOWN_FAC1,conv_size,1); //isL1
			for (j=0;j<L1_MAP_SIZE;j++)
				accum_L1[j] += L1_maps[i][j];
		}
		/****************** LAYER 2 *********************/	
		conv_size = L1_MAP_WIDTH - L2_KERNEL_WIDTH + 1; //20
		INTERMEDIATE_T *filter2D_out_L2 = (INTERMEDIATE_T *)malloc(conv_size*conv_size*sizeof(INTERMEDIATE_T));
		for (i=0;i<L2_MAPS;i++){
			filter2D(L2_kernel[i],accum_L1,filter2D_out_L2,L2_KERNEL_WIDTH,L1_MAP_WIDTH,L2_kernel_scale[i]);
			pool_subsample(filter2D_out_L2,L2_maps[i],WINDOW_SIZE2,DOWN_FAC2,conv_size,0); //isL2
		}
	}
	
	//stop taking note of time, and stop event counters
	t1=gettime();

	printf("[SEQUENTIAL]Runtime=%lld\n",t1-t0);
	
	/****************************************** CPU-EPIPHANY solver ******************************************/

	//device handlers
	e_platform_t platform;
	e_epiphany_t dev;

	//initialize device
	e_init(NULL);
	e_reset_system();
	e_get_platform_info(&platform);

	// Open a workgroup
	e_open(&dev, 0, 0, platform.rows, platform.cols);

	// CORE initializations
	FLAG_T done = 0xbeefdead;
	for (i = 0; i < platform.rows; i++){
		for (j = 0; j < platform.cols; j++){
			unsigned coreid = i*platform.rows + j;
			load_stack(i,j,coreid,&dev,timesteps);			
			//for reseting done flag
			if (i == 0 && j == 0)	
				e_write(&dev,i,j,DONE_ADDR,&done,sizeof(FLAG_T));
		}
	}

	//load srec
	e_load_group("pe.srec", &dev, 0, 0, platform.rows, platform.cols, E_FALSE);

	//create patches and load into DRAM
	//patch(image,image_dim,patch_dim,out_patch_dim,num_layers,final_map_dim,subsampling_factors,kernel_dims);
	e_mem_t emem;
	unsigned *image_test = (unsigned *)malloc(8*8*sizeof(unsigned));
	for (i=0;i<64;i++)
		image_test[i] = 1;
	e_alloc(&emem,0x01000000,256);

	//start taking note of time, and start event counters
	t0=gettime();

	e_start_group(&dev);

	done = 0;
	while (done != 0xdeadbeef){
		e_read(&dev,0,0,DONE_ADDR,&done,sizeof(unsigned));
	}
	
	/*************************** Verify functional correctness *****************************/
#ifdef VERIFY
	for (i=0;i<platform.rows;i++){
		for (j=0;j<platform.cols;j++){
			MAP_T read[L1_MAX_MAPS_PER_ECORE*L1_MAP_SIZE];
			MAP_T read1[L2_MAX_MAPS_PER_ECORE*L2_MAP_SIZE];
			MAP_T read2[L3_MAX_MAPS_PER_ECORE*L3_MAP_SIZE];
			e_read(&dev,i,j,L1_MAPS_ADDR,read,L1_MAX_MAPS_PER_ECORE*L1_MAP_SIZE*sizeof(MAP_T));
			e_read(&dev,i,j,L2_MAPS_ADDR,read1,L2_MAX_MAPS_PER_ECORE*L2_MAP_SIZE*sizeof(MAP_T));
			e_read(&dev,i,j,L3_MAPS_ADDR,read2,L3_MAX_MAPS_PER_ECORE*L3_MAP_SIZE*sizeof(MAP_T));
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
			for (k=0;k<l3map_c[i*4+j];k++){
				unsigned id = L3_shard_map[i*4+j][k];
				int n;
				for (n=0;n<L3_MAP_SIZE;n++)
					L3_maps_parr[id][n] = read2[k*L3_MAP_SIZE+n];
			}
		}
	}

	int cnt_f = 0, cnt_p = 0;
	for (i=0;i<L1_MAPS;i++){
		int mf = 0;
		for (j=0;j<L1_MAP_SIZE;j++){
			if (fabs(L1_maps[i][j]-L1_maps_parr[i][j]) > ERR_TOL){
				cnt_f++;
				mf = 1;
				printf("L1: [map %d,%d] seq = %f, parr = %f\n",i,cnt_f,L1_maps[i][j],L1_maps_parr[i][j]);
			} else {
				cnt_p++;
			}
		}
	}
	
	printf("L1 fail = %d, pass = %d\n",cnt_f,cnt_p);

	cnt_f = 0, cnt_p = 0;
	for (i=0;i<L2_MAPS;i++){
		int mf = 0;
		for (j=0;j<L2_MAP_SIZE;j++){
			if (fabs(L2_maps[i][j] != L2_maps_parr[i][j]) > ERR_TOL){
				cnt_f++;
				mf = 1;
				//printf("L2: [%d] seq = %u, parr = %u\n",cnt_f,L2_maps[i][j],L2_maps_parr[i][j]);
			} else {
				cnt_p++;
			}
		}
	}
	
	printf("L2 fail = %d, pass = %d\n",cnt_f,cnt_p);

	cnt_f = 0, cnt_p = 0;
	for (i=0;i<L3_MAPS;i++){
		int mf = 0;
		for (j=0;j<L3_MAP_SIZE;j++){
			if (fabs(L3_maps[i][j] != L3_maps_parr[i][j]) > ERR_TOL){
				cnt_f++;
				mf = 1;
				//printf("L2: [%d] seq = %u, parr = %u\n",cnt_f,L2_maps[i][j],L2_maps_parr[i][j]);
			} else {
				cnt_p++;
			}
		}
	}
	
	printf("L3 fail = %d, pass = %d\n",cnt_f,cnt_p);
#endif

	//flatten L3_maps
	float *L3_maps_flattened = (float *)malloc(L3_MAPS*L3_MAP_SIZE*sizeof(float));
	MAP_T read[L3_MAX_MAPS_PER_ECORE*L3_MAP_SIZE];

	/****************************** CLASSIFICATION *****************************/
	for (t=0;t<timesteps;t++){
		//read L2 maps out
		for (i=0;i<platform.rows;i++){
			for (j=0;j<platform.cols;j++){			
				e_read(&dev,i,j,L3_MAPS_ADDR,read,L3_MAX_MAPS_PER_ECORE*L3_MAP_SIZE*sizeof(MAP_T));
				int k;
				for (k=0;k<l3map_c[i*4+j];k++){
					unsigned id = L3_shard_map[i*4+j][k];
					int n;
					for (n=0;n<L3_MAP_SIZE;n++)
						L3_maps_parr[id][n] = read[k*L3_MAP_SIZE+n];
				}
			}
		}

		//allocate to flattened array
		for (i=0;i<L3_MAPS;i++){
			for (j=0;j<L3_MAP_SIZE;j++){
				L3_maps_flattened[i*L3_MAPS+j] = (float)L3_maps_parr[i][j];
			}
		}
		
		elm_classifier(L3_maps_flattened,L3_MAPS*L3_MAP_SIZE,NUM_HIDDEN_NEURONS,NUM_OUTPUT_NEURONS,pInputWt,pHiddenBias,pOutputWt);
	}

	//stop taking note of time, and stop event counters
	t1=gettime();

	printf("[PARALLEL]Runtime=%lld\n",t1-t0);

	//free DRAM memory
	e_free(&emem);

	// Close the workgroup
	e_close(&dev);

	// Finalize e-platform connection
	e_finalize();


	return 0;
}

MEM_ADDR_T l1map_global_addr(unsigned pe, unsigned offset){
	
	int col = pe%4;
	int row = (pe-col)/4;
	unsigned z = L1_MAPS_ADDR + L1_MAP_SIZE*offset*sizeof(MAP_T);

	unsigned address = 0x80800000 + row*(0x4000000) + col*(0x100000) + z;

	return address;
}

void load_stack(unsigned i, unsigned j, unsigned coreid, e_epiphany_t *dev, unsigned timesteps) {
	
	unsigned curr_address = GLOBAL_CONSTANTS_ADDR;

#ifdef DEBUG
	if (coreid == 0)
		printf("Writing timesteps to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,&timesteps,sizeof(GLOBAL_CONSTANTS_T));
	curr_address += 4;

#ifdef DEBUG
	if (coreid == 0)
		printf("Writing l1map_count to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,&l1map_c[coreid],sizeof(GLOBAL_CONSTANTS_T));
	curr_address += 4;

#ifdef DEBUG
	if (coreid==0)
		printf("Writing l2map_count to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,&l2map_c[coreid],sizeof(GLOBAL_CONSTANTS_T));	
	curr_address += 4;

#ifdef DEBUG
	if (coreid==0)
		printf("Writing l3map_count to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,&l3map_c[coreid],sizeof(GLOBAL_CONSTANTS_T));	
	curr_address = IMAGE_ADDR;

#ifdef DEBUG
	if (coreid==0)
		printf("Writing image to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,image,IMAGE_SIZE*sizeof(IMAGE_T));
	curr_address = L1_KERNEL_ADDR;

#ifdef DEBUG
	if (coreid==0)
		printf("Writing L1_kernel to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,L1_kernel_shard[coreid],l1map_c[coreid]*L1_KERNEL_SIZE*sizeof(KERNEL_T));
	curr_address = L1_KERNEL_SCALE_ADDR;

#ifdef DEBUG
	if (coreid==0)
		printf("Writing L1_kernel_scale to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,L1_kernel_scale_shard[coreid],l1map_c[coreid]*sizeof(SCALE_T));
	curr_address = L2_KERNEL_ADDR;

#ifdef DEBUG	
	if (coreid==0)
		printf("Writing L2_kernel to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,L2_kernel_shard[coreid],l2map_c[coreid]*L2_KERNEL_SIZE*sizeof(KERNEL_T));
	curr_address = L2_KERNEL_SCALE_ADDR;

#ifdef DEBUG
	if (coreid==0)
		printf("Writing L2_kernel_scale to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,L2_kernel_scale_shard[coreid],l2map_c[coreid]*sizeof(SCALE_T));
	curr_address = L3_KERNEL_ADDR;

#ifdef DEBUG	
	if (coreid==0)
		printf("Writing L3_kernel to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,L3_kernel_shard[coreid],l3map_c[coreid]*L3_KERNEL_SIZE*sizeof(KERNEL_T));
	curr_address = L3_KERNEL_SCALE_ADDR;

#ifdef DEBUG
	if (coreid==0)
		printf("Writing L3_kernel_scale to 0x%x\n",curr_address);
#endif
	e_write(dev,i,j,curr_address,L3_kernel_scale_shard[coreid],l3map_c[coreid]*sizeof(SCALE_T));
}

void elm_classifier(float *pInputNeurons, int nInputNeurons, int nHiddenNeurons, int nOutputNeurons, float *pInputWt, 
	float *pHiddenBias, float *pOutputWt) {
	
	float *pHiddenNeuron, *pOutputNeuron;
	int v, n, label, nWeightVec, nFullyUsedVec, remHiddenNeurons;

	pHiddenNeuron = (float *)malloc(nHiddenNeurons * sizeof(float));
	pOutputNeuron = (float *)malloc(nOutputNeurons * sizeof(float));

	nWeightVec = ceil(nHiddenNeurons / nInputNeurons);
	nFullyUsedVec = floor(nHiddenNeurons / nInputNeurons);
	
	// Input layer computations
	for (v = 0; v < nFullyUsedVec; v++) {
		for (n = 0; n < nInputNeurons; n++) {
			pHiddenNeuron[v * nInputNeurons + n] = dot_prod_rotate_weightVec(pInputNeurons,
				pInputWt + v * nInputNeurons, nInputNeurons, n);
		}
	}

	remHiddenNeurons = nHiddenNeurons - nInputNeurons * nFullyUsedVec;
	
	for ( n = 0; n < remHiddenNeurons; n++) {
		
		pHiddenNeuron[nFullyUsedVec * nInputNeurons + n] = dot_prod_rotate_weightVec(pInputNeurons,
			pInputWt + nFullyUsedVec * nInputNeurons, nInputNeurons, n);
	}
	
	// Add bias to hidden neurons and do nonlinear activation
	for (n = 0;n < nHiddenNeurons;n++){
		pHiddenNeuron[n] += pHiddenBias[n];
		pHiddenNeuron[n] = 1 / ( 1 + exp(-pHiddenNeuron[n]));
	}	

	// Output layer
	for ( n = 0; n < nOutputNeurons; n++) {
		pOutputNeuron[n] = dot_prod(pHiddenNeuron, pOutputWt + n * nHiddenNeurons, nHiddenNeurons);
	}
	
	//Classify
	label = index_of_max(pOutputNeuron, nOutputNeurons);
	//printf("THE PREDICTED LABEL FOR THE GIVEN INPUT IS = %d\n", label);

	//free(pHiddenNeuron);
	//free(pOutputNeuron);
}

static inline float dot_prod(float *pActivation , float *pWeight, int vLen) {
	int e;
	float sop;

	sop = 0;
	for (e = 0; e < vLen; e++) {
		sop += pActivation[e] * pWeight[e];
	}

	return sop;
}

static inline float dot_prod_rotate_weightVec(float *pFeatureVec, float *pWeightVec, int iN, int circShift) {
	float sop;
	int n;

	sop = 0;
	for (n = 0; n < iN - circShift; n++) {
		sop += pFeatureVec[n + circShift] * pWeightVec[n];
	}
	for ( n = 0; n < circShift; n++) {
		sop += pFeatureVec[n] * pWeightVec[iN + n - circShift];
	}

	return sop;	
}

static int index_of_max(float *pVec, int vLen) {
	float max;
	int maxIndex, e;

	max = -10000000;
	maxIndex = -1;
	for ( e = 0; e < vLen; e++) {
		if (pVec[e] > max) {
			max = pVec[e];
			maxIndex = e;
		}
	}
	if ( maxIndex >= 0) {
	} else {
		//printf("No value greater than %f is not found in the array\n", max);
	}

	return maxIndex;
}

