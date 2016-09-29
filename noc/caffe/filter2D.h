void filter2D(KERNEL_T *kernel, MAP_T *src, INTERMEDIATE_T *dest, 
	unsigned kernel_width, unsigned src_height, unsigned src_width, SCALE_T scale){

	unsigned row,col,kernel_row,kernel_col,i,j;
	int cntr[kernel_width];
	unsigned o_cntr = 0;
	unsigned conv_height = src_height - kernel_width + 1;
	unsigned conv_width = src_width - kernel_width + 1;
	for (row=0;row<conv_height;row++){
		for (i=0;i<kernel_width;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<conv_width;col++){
			
			INTERMEDIATE_T sop = 0.0f;

			for (kernel_row=0;kernel_row<kernel_width;kernel_row++){
				for (kernel_col=0;kernel_col<kernel_width;kernel_col++){
					sop += kernel[kernel_row*kernel_width+kernel_col]*src[cntr[kernel_row]+kernel_col];			
				}
			}

			sop = sop*scale;
			*(dest + o_cntr) = sop;

			o_cntr++;
			for (i=0;i<kernel_width;i++)
				cntr[i]++;
		}
	}
}
