void pool(INTERMEDIATE_T *src, unsigned window, unsigned src_width){
	unsigned row,col,i,j;
	int cntr[window];
	unsigned o_cntr = 0;
	unsigned corr = src_width - src_width%window;

	unsigned denom = window*window;

	for (row=0;row<corr;row+=window){
		for (i=0;i<window;i++)
			cntr[i] = (row+i)*src_width;
		//printf("Row = %d, cntr[0] = %d\n",row,cntr[0]);
		for (col=0;col<corr;col+=window){
			//printf("Col = %d, cntr[1] = %d\n",col,cntr[1]);
			float sum = 0.0f;
			for (i=0;i<window;i++){
				for (j=0;j<window;j++){
					sum += src[cntr[i]+j];
				}
			}
			
			sum = sum/denom;
			
			for (i=0;i<window;i++){
				for (j=0;j<window;j++){
					src[cntr[i]+j] = sum;
				}
			}

			for (i=0;i<window;i++)
				cntr[i] += window;
			/*
			sum += src[cntr[0]] + src[cntr[0]+1] + src[cntr[0]+2] + src[cntr[0]+3] + src[cntr[0]+4] + src[cntr[0]+5] + src[cntr[0]+6] + src[cntr[0]+7] + src[cntr[0]+8] + src[cntr[0]+9];
			sum += src[cntr[1]] + src[cntr[1]+1] + src[cntr[1]+2] + src[cntr[1]+3] + src[cntr[1]+4] + src[cntr[1]+5] + src[cntr[1]+6] + src[cntr[1]+7] + src[cntr[1]+8] + src[cntr[1]+9];
			sum += src[cntr[2]] + src[cntr[2]+1] + src[cntr[2]+2] + src[cntr[2]+3] + src[cntr[2]+4] + src[cntr[2]+5] + src[cntr[2]+6] + src[cntr[2]+7] + src[cntr[2]+8] + src[cntr[2]+9];
			sum += src[cntr[3]] + src[cntr[3]+1] + src[cntr[3]+2] + src[cntr[3]+3] + src[cntr[3]+4] + src[cntr[3]+5] + src[cntr[3]+6] + src[cntr[3]+7] + src[cntr[3]+8] + src[cntr[3]+9];
			sum += src[cntr[4]] + src[cntr[4]+1] + src[cntr[4]+2] + src[cntr[4]+3] + src[cntr[4]+4] + src[cntr[4]+5] + src[cntr[4]+6] + src[cntr[4]+7] + src[cntr[4]+8] + src[cntr[4]+9];								
			sum += src[cntr[5]] + src[cntr[5]+1] + src[cntr[5]+2] + src[cntr[5]+3] + src[cntr[5]+4] + src[cntr[5]+5] + src[cntr[5]+6] + src[cntr[5]+7] + src[cntr[5]+8] + src[cntr[5]+9];				
			sum += src[cntr[6]] + src[cntr[6]+1] + src[cntr[6]+2] + src[cntr[6]+3] + src[cntr[6]+4] + src[cntr[6]+5] + src[cntr[6]+6] + src[cntr[6]+7] + src[cntr[6]+8] + src[cntr[6]+9];
			sum += src[cntr[7]] + src[cntr[7]+1] + src[cntr[7]+2] + src[cntr[7]+3] + src[cntr[7]+4] + src[cntr[7]+5] + src[cntr[7]+6] + src[cntr[7]+7] + src[cntr[7]+8] + src[cntr[7]+9];
			sum += src[cntr[8]] + src[cntr[8]+1] + src[cntr[8]+2] + src[cntr[8]+3] + src[cntr[8]+4] + src[cntr[8]+5] + src[cntr[8]+6] + src[cntr[8]+7] + src[cntr[8]+8] + src[cntr[8]+9];
			sum += src[cntr[9]] + src[cntr[9]+1] + src[cntr[9]+2] + src[cntr[9]+3] + src[cntr[9]+4] + src[cntr[9]+5] + src[cntr[9]+6] + src[cntr[9]+7] + src[cntr[9]+8] + src[cntr[9]+9];
			sum = sum*0.01f;//divide by 100
			//sum = sum > 0.0f ? sum : 0.0f;

			} else {
				sum += src[cntr[0]] + src[cntr[0]+1] + src[cntr[0]+2] + src[cntr[0]+3] + src[cntr[0]+4] + src[cntr[0]+5];
				sum += src[cntr[1]] + src[cntr[1]+1] + src[cntr[1]+2] + src[cntr[1]+3] + src[cntr[1]+4] + src[cntr[1]+5];
				sum += src[cntr[2]] + src[cntr[2]+1] + src[cntr[2]+2] + src[cntr[2]+3] + src[cntr[2]+4] + src[cntr[2]+5];
				sum += src[cntr[3]] + src[cntr[3]+1] + src[cntr[3]+2] + src[cntr[3]+3] + src[cntr[3]+4] + src[cntr[3]+5];
				sum += src[cntr[4]] + src[cntr[4]+1] + src[cntr[4]+2] + src[cntr[4]+3] + src[cntr[4]+4] + src[cntr[4]+5];
				sum += src[cntr[5]] + src[cntr[5]+1] + src[cntr[5]+2] + src[cntr[5]+3] + src[cntr[5]+4] + src[cntr[5]+5];
				sum = sum*0.0277777777f;//divide by 36.. precision lost..
				//sum = sum > 0.0f ? sum : 0.0f;
			}
			dest[o_cntr] = sum;
			o_cntr++;
			*/
		}
	}
}


void subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned stride, unsigned src_width){
	unsigned row,col,i,j;
	int cntr[stride];
	unsigned o_cntr = 0;

	unsigned corr = src_width - src_width%stride;

	for (row=0;row<corr;row+=stride){
		for (i=0;i<stride;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<corr;col+=stride){

			dest[o_cntr] = src[row*src_width+col];
			o_cntr++;

			for (i=0;i<stride;i++)
				cntr[i] += stride;
		}
	}
}