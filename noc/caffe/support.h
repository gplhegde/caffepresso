void init_test_image(IMAGE_T *image){
	int i;
	for (i=0;i<IMAGE_WIDTH*IMAGE_HEIGHT;i++){
		image[i] = (IMAGE_T)(rand()%2);
	}
}
/*
void random_shards(KERNEL_T **L1_kernel, KERNEL_T **L2_kernel){

	int i,j,k;

	//global mallocs
	L1_kernel_shard = (KERNEL_T **)malloc(NUM_PES*sizeof(KERNEL_T *));
	L2_kernel_shard = (KERNEL_T **)malloc(NUM_PES*sizeof(KERNEL_T *));
	L1_kernel_scale_shard = (SCALE_T **)malloc(NUM_PES*sizeof(SCALE_T *));
	L2_kernel_scale_shard = (SCALE_T **)malloc(NUM_PES*sizeof(SCALE_T *));
	for (i=0;i<NUM_PES;i++){
		L1_kernel_shard[i] = (KERNEL_T *)malloc(L1_MAX_MAPS_PER_ECORE*L1_KERNEL_SIZE*sizeof(KERNEL_T));
		L2_kernel_shard[i] = (KERNEL_T *)malloc(L2_MAX_MAPS_PER_ECORE*L2_KERNEL_SIZE*sizeof(KERNEL_T));
		L1_kernel_scale_shard[i] = (SCALE_T *)malloc(L1_MAX_MAPS_PER_ECORE*sizeof(SCALE_T));
		L2_kernel_scale_shard[i] = (SCALE_T *)malloc(L2_MAX_MAPS_PER_ECORE*sizeof(SCALE_T));
	}
	
	//local variables
	unsigned *placement_L1 = (unsigned *)malloc(L1_MAPS*sizeof(unsigned));
	unsigned *placement_L2 = (unsigned *)malloc(L2_MAPS*sizeof(unsigned));
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
		int success = 0;
		while (!success){
			if (l1map_c[pe] < L1_MAX_MAPS_PER_ECORE){
				placement_L1[i] = pe;
				j = l1map_c[pe]*L1_KERNEL_SIZE;
				for (k=j;k<j+L1_KERNEL_SIZE;k++){
					L1_kernel_shard[pe][k] = L1_kernel[i][k%L1_KERNEL_SIZE];
				}
				L1_shard_map[pe][l1map_c[pe]] = i;
				L1_kernel_scale_shard[pe][l1map_c[pe]] = L1_kernel_scale[i];
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
		int success = 0;
		while (!success){
			if (l2map_c[pe] < L2_MAX_MAPS_PER_ECORE){
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
}
*/
void init_weights(KERNEL_T ***kernels, SCALE_T **scales, unsigned *maps, unsigned *kw){

	int m,i,j;	
	for(m=0;m<NUM_LAYERS;m++) {
		for (j=0;j<maps[m];j++){
			SCALE_T scale = 0.0f;
			for (i=0;i<kw[m]*kw[m];i++){
				kernels[m][j][i] = (KERNEL_T)(rand()%256); //between 0 and 255			
				scale += kernels[m][j][i];
			}
			scales[m][j] = 1/scale;
		}
	}
}
