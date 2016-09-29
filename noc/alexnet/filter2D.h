void filter2D(KERNEL_T *kernel, MAP_T *src, INTERMEDIATE_T *dest, 
	unsigned kernel_width, unsigned src_width, SCALE_T scale){

	unsigned row,col,kernel_row,kernel_col,i,j;
	int cntr[kernel_width];
	unsigned o_cntr = 0;
	unsigned conv_size = src_width - kernel_width + 1;
	for (row=0;row<conv_size;row++){
		for (i=0;i<kernel_width;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<conv_size;col++){
			
			INTERMEDIATE_T sop = 0.0f;

			sop += (kernel[0])*(src[cntr[0]]);
			sop += (kernel[1])*(src[cntr[0]+1]);
			sop += (kernel[2])*(src[cntr[0]+2]);
			sop += (kernel[3])*(src[cntr[0]+3]);
			sop += (kernel[4])*(src[cntr[0]+4]);
			sop += (kernel[5])*(src[cntr[0]+5]);
			sop += (kernel[6])*(src[cntr[0]+6]);
			sop += (kernel[7])*(src[cntr[0]+7]);
			sop += (kernel[8])*(src[cntr[0]+8]);

			sop += (kernel[9])*(src[cntr[1]]);
			sop += (kernel[10])*(src[cntr[1]+1]);
			sop += (kernel[11])*(src[cntr[1]+2]);
			sop += (kernel[12])*(src[cntr[1]+3]);
			sop += (kernel[13])*(src[cntr[1]+4]);
			sop += (kernel[14])*(src[cntr[1]+5]);
			sop += (kernel[15])*(src[cntr[1]+6]);
			sop += (kernel[16])*(src[cntr[1]+7]);
			sop += (kernel[17])*(src[cntr[1]+8]);

			sop += (kernel[18])*(src[cntr[2]]);
			sop += (kernel[19])*(src[cntr[2]+1]);
			sop += (kernel[20])*(src[cntr[2]+2]);
			sop += (kernel[21])*(src[cntr[2]+3]);
			sop += (kernel[22])*(src[cntr[2]+4]);
			sop += (kernel[23])*(src[cntr[2]+5]);
			sop += (kernel[24])*(src[cntr[2]+6]);
			sop += (kernel[25])*(src[cntr[2]+7]);
			sop += (kernel[26])*(src[cntr[2]+8]);

			sop += (kernel[27])*(src[cntr[3]]);
			sop += (kernel[28])*(src[cntr[3]+1]);
			sop += (kernel[29])*(src[cntr[3]+2]);
			sop += (kernel[30])*(src[cntr[3]+3]);
			sop += (kernel[31])*(src[cntr[3]+4]);
			sop += (kernel[32])*(src[cntr[3]+5]);
			sop += (kernel[33])*(src[cntr[3]+6]);
			sop += (kernel[34])*(src[cntr[3]+7]);
			sop += (kernel[35])*(src[cntr[3]+8]);

			sop += (kernel[36])*(src[cntr[4]]);
			sop += (kernel[37])*(src[cntr[4]+1]);
			sop += (kernel[38])*(src[cntr[4]+2]);
			sop += (kernel[39])*(src[cntr[4]+3]);
			sop += (kernel[40])*(src[cntr[4]+4]);
			sop += (kernel[41])*(src[cntr[4]+5]);
			sop += (kernel[42])*(src[cntr[4]+6]);
			sop += (kernel[43])*(src[cntr[4]+7]);
			sop += (kernel[44])*(src[cntr[4]+8]);

			sop += (kernel[45])*(src[cntr[5]]);
			sop += (kernel[46])*(src[cntr[5]+1]);
			sop += (kernel[47])*(src[cntr[5]+2]);
			sop += (kernel[48])*(src[cntr[5]+3]);
			sop += (kernel[49])*(src[cntr[5]+4]);
			sop += (kernel[50])*(src[cntr[5]+5]);
			sop += (kernel[51])*(src[cntr[5]+6]);
			sop += (kernel[52])*(src[cntr[5]+7]);
			sop += (kernel[53])*(src[cntr[5]+8]);

			sop += (kernel[54])*(src[cntr[6]]);
			sop += (kernel[55])*(src[cntr[6]+1]);
			sop += (kernel[56])*(src[cntr[6]+2]);
			sop += (kernel[57])*(src[cntr[6]+3]);
			sop += (kernel[58])*(src[cntr[6]+4]);
			sop += (kernel[59])*(src[cntr[6]+5]);
			sop += (kernel[60])*(src[cntr[6]+6]);
			sop += (kernel[61])*(src[cntr[6]+7]);
			sop += (kernel[62])*(src[cntr[6]+8]);

			sop += (kernel[63])*(src[cntr[7]]);
			sop += (kernel[64])*(src[cntr[7]+1]);
			sop += (kernel[65])*(src[cntr[7]+2]);
			sop += (kernel[66])*(src[cntr[7]+3]);
			sop += (kernel[67])*(src[cntr[7]+4]);
			sop += (kernel[68])*(src[cntr[7]+5]);
			sop += (kernel[69])*(src[cntr[7]+6]);
			sop += (kernel[70])*(src[cntr[7]+7]);
			sop += (kernel[71])*(src[cntr[7]+8]);

			sop += (kernel[72])*(src[cntr[8]]);
			sop += (kernel[73])*(src[cntr[8]+1]);
			sop += (kernel[74])*(src[cntr[8]+2]);
			sop += (kernel[75])*(src[cntr[8]+3]);
			sop += (kernel[76])*(src[cntr[8]+4]);
			sop += (kernel[77])*(src[cntr[8]+5]);
			sop += (kernel[78])*(src[cntr[8]+6]);
			sop += (kernel[79])*(src[cntr[8]+7]);
			sop += (kernel[80])*(src[cntr[8]+8]);

			sop = sop*scale;
			*(dest + o_cntr) = sop;

			o_cntr++;
			for (i=0;i<kernel_width;i++)
				cntr[i]++;
		}
	}
}
