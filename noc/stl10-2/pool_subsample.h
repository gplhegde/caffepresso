void subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned stride, unsigned src_height, unsigned src_width){
	unsigned row,col,i,j;
	int cntr[stride];
	unsigned o_cntr = 0;

	unsigned corr_width = src_width - src_width%stride;
	unsigned corr_height = src_height - src_height%stride;

	for (row=0;row<corr_height;row+=stride){
		for (i=0;i<stride;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<corr_width;col+=stride){

			//2x2 unrolled max-pool
			float c0,c1;
			c0 = src[cntr[0]] > src[cntr[0]+1] ? src[cntr[0]] : src[cntr[0]+1];
			c1 = src[cntr[1]] > src[cntr[1]+1] ? src[cntr[1]] : src[cntr[1]+1];

			c0 = c0 > c1 ? c0 : c1;

			dest[o_cntr] = c0;
			o_cntr++;

			for (i=0;i<stride;i++)
				cntr[i] += stride;
		}
	}
}
