#include <stdio.h>
#include <stdlib.h>

#define IMAGE_T unsigned 
#define MAP_T unsigned 
#define KERNEL_T unsigned 
#define SCALE_T unsigned 
#define INTERMEDIATE_T unsigned 

#define NUM_KERNELS_PER_LAYER 4

void patch(IMAGE_T *image, unsigned image_dim, unsigned in_patch_dim, unsigned *out_patch_dim, unsigned num_layers, unsigned out_layer_dim, unsigned *subsampling_factors, unsigned *kernel_dims) {

	if (out_layer_dim % in_patch_dim != 0){
		printf("Only patch dimnesions with perfect multiples of out_layer resolutions are supported! Please correct\n");
		exit(1);
	}

	printf("Image dimensions = %dx%d\n",image_dim,image_dim);
	printf("Requested patch size = %dx%d\n",in_patch_dim,in_patch_dim);
	printf("Number of layers = %d\n",num_layers);

	unsigned i;
	unsigned curr_patch_dim = in_patch_dim;
	printf("Patch dim final = %dx%d\n",curr_patch_dim,curr_patch_dim);
	for (i=num_layers;i>0;i--){
		
		curr_patch_dim = curr_patch_dim + kernel_dims[i-1] - 1;
		printf("Patch dim after layer %d = %dx%d\n",(i-1),curr_patch_dim,curr_patch_dim);

	}

	printf("Actual patch size = %dx%d\n",curr_patch_dim,curr_patch_dim);
	*out_patch_dim = curr_patch_dim;
	
	return;
}

void filter2D(KERNEL_T *kernel,MAP_T *src,INTERMEDIATE_T *dest,unsigned kernel_width,unsigned src_width,SCALE_T scale){

	unsigned row,col,kernel_row,kernel_col,i,j;
	int cntr[kernel_width];
	unsigned o_cntr = 0;
	unsigned conv_size = src_width - kernel_width + 1;
	for (row=0;row<conv_size;row++){
		for (i=0;i<kernel_width;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<conv_size;col++){
			
			INTERMEDIATE_T sop = 0;

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

int main(void){
	
	unsigned image_dim = 24;
	unsigned num_layers = 2;
	unsigned patch_dim = 10;
	unsigned final_map_dim = 20;

	IMAGE_T *image = (IMAGE_T *)malloc(image_dim*image_dim*sizeof(IMAGE_T));

	int i,j,k,m;
	for (i=0;i<image_dim*image_dim;i++){
		image[i] = i;//rand()%3;
	}

	for (i=0;i<image_dim;i++){
	//	printf("[%d]\t",i);
		for (j=0;j<image_dim;j++){
	//		printf("%d\t",image[i*image_dim+j]);
		}
	//	printf("\n");
	}

	unsigned *out_patch_dim = (unsigned *)malloc(sizeof(unsigned));
	unsigned *subsampling_factors = (unsigned *)malloc(num_layers*sizeof(unsigned));
	subsampling_factors[0] = 2; subsampling_factors[1] = 2;
	unsigned *kernel_dims = (unsigned *)malloc(num_layers*sizeof(unsigned));
	kernel_dims[0] = 3; kernel_dims[1] = 3;
	KERNEL_T **kernels = (KERNEL_T **)malloc(num_layers*NUM_KERNELS_PER_LAYER*sizeof(KERNEL_T *));
	kernels[0] = (KERNEL_T *)malloc(kernel_dims[0]*kernel_dims[0]*sizeof(KERNEL_T));
	kernels[1] = (KERNEL_T *)malloc(kernel_dims[0]*kernel_dims[0]*sizeof(KERNEL_T));
	kernels[2] = (KERNEL_T *)malloc(kernel_dims[0]*kernel_dims[0]*sizeof(KERNEL_T));
	kernels[3] = (KERNEL_T *)malloc(kernel_dims[0]*kernel_dims[0]*sizeof(KERNEL_T));
	kernels[4] = (KERNEL_T *)malloc(kernel_dims[1]*kernel_dims[1]*sizeof(KERNEL_T));
	kernels[5] = (KERNEL_T *)malloc(kernel_dims[1]*kernel_dims[1]*sizeof(KERNEL_T));
	kernels[6] = (KERNEL_T *)malloc(kernel_dims[1]*kernel_dims[1]*sizeof(KERNEL_T));
	kernels[7] = (KERNEL_T *)malloc(kernel_dims[1]*kernel_dims[1]*sizeof(KERNEL_T));

	for (i=0;i<kernel_dims[0];i++){
		for (j=0;j<kernel_dims[0];j++){
			kernels[0][i*kernel_dims[0]+j] = rand()%3;
			kernels[1][i*kernel_dims[0]+j] = rand()%3;
			kernels[2][i*kernel_dims[0]+j] = rand()%3;
			kernels[3][i*kernel_dims[0]+j] = rand()%3;
		}
	}
	for (i=0;i<kernel_dims[1];i++){
		for (j=0;j<kernel_dims[1];j++){
			kernels[4][i*kernel_dims[1]+j] = rand()%5;
			kernels[5][i*kernel_dims[1]+j] = rand()%5;
			kernels[6][i*kernel_dims[1]+j] = rand()%5;
			kernels[7][i*kernel_dims[1]+j] = rand()%5;
		}
	}

	/******************* non-patch solve **********************/
	int conv_size = image_dim - kernel_dims[0] + 1;
	unsigned *dest = (unsigned *)malloc(conv_size*conv_size*sizeof(unsigned));
	unsigned *accum = (unsigned *)malloc(conv_size*conv_size*sizeof(unsigned));
	for (i=0;i<conv_size*conv_size;i++)
		accum[i] = 0;
	for (i=0;i<NUM_KERNELS_PER_LAYER;i++){
		filter2D(kernels[i],image,dest,kernel_dims[0],image_dim,1);
		for (j=0;j<conv_size*conv_size;j++)
			accum[j] += dest[j];
	}
	
	int conv_size1 = conv_size - kernel_dims[1] + 1;
	unsigned *map0 = (unsigned *)malloc(conv_size1*conv_size1*sizeof(unsigned));
	unsigned *map1 = (unsigned *)malloc(conv_size1*conv_size1*sizeof(unsigned));
	unsigned *map2 = (unsigned *)malloc(conv_size1*conv_size1*sizeof(unsigned));
	unsigned *map3 = (unsigned *)malloc(conv_size1*conv_size1*sizeof(unsigned));
	filter2D(kernels[4],accum,map0,kernel_dims[1],conv_size,1);
	filter2D(kernels[5],accum,map1,kernel_dims[1],conv_size,1);
	filter2D(kernels[6],accum,map2,kernel_dims[1],conv_size,1);
	filter2D(kernels[7],accum,map3,kernel_dims[1],conv_size,1);

	
	/******************* patch solve **************************/
	patch(image,image_dim,patch_dim,out_patch_dim,num_layers,final_map_dim,subsampling_factors,kernel_dims);

	int num_patches = final_map_dim*final_map_dim/(patch_dim*patch_dim);
	int patches_per_dim = final_map_dim/patch_dim;
	printf("Num patches = %d\n",num_patches);

	int patch_row,patch_col;
	unsigned *patch_map = (unsigned *)malloc(final_map_dim*final_map_dim*sizeof(unsigned));
	for (i=0;i<patches_per_dim;i++){
		for (j=0;j<patches_per_dim;j++){
			unsigned *patch = (unsigned *)malloc((*out_patch_dim)*(*out_patch_dim)*sizeof(unsigned));
			//construct patch
			for (patch_row=0;patch_row<*out_patch_dim;patch_row++){
//				printf("[%d]\t",patch_row);
				for (patch_col=0;patch_col<*out_patch_dim;patch_col++){
					int index = i*image_dim*patch_dim + j*patch_dim;
					patch[patch_row*(*out_patch_dim)+patch_col] = image[index+patch_row*image_dim+patch_col];
//					printf("%d\t",patch[patch_row*(*out_patch_dim)+patch_col]);
				}
//				printf("\n");
			}
			unsigned conv_size = *out_patch_dim;
			unsigned src_dim = *out_patch_dim;
			unsigned *src = (unsigned *)malloc(src_dim*src_dim*sizeof(unsigned));
			for (k=0;k<src_dim*src_dim;k++){//initial src is patch..
				src[k] = patch[k];
			}
			for (k=0;k<num_layers;k++){
				conv_size = conv_size - kernel_dims[k] + 1;
				unsigned *dest = (unsigned *)malloc(conv_size*conv_size*sizeof(unsigned));
				unsigned *p_accum = (unsigned *)malloc(conv_size*conv_size*sizeof(unsigned));
				for (m=0;m<NUM_KERNELS_PER_LAYER;m++){
					filter2D(kernels[m],src,dest,kernel_dims[k],src_dim,1);	
				}				
				src = dest;
				src_dim = conv_size;
			}
			if (src_dim != patch_dim){
				printf("src dim does not match final map dim!\n");
			}
			for (k=0;k<patch_dim;k++){
				for (m=0;m<patch_dim;m++){
					int index = i*final_map_dim*patch_dim + j*patch_dim;
					patch_map[index+k*final_map_dim+m] = src[k*patch_dim+m];
				}
			}
		}
	}

	/*
	printf("Original-------------\n");
	for (i=0;i<final_map_dim;i++){
		printf("[%d]\t",i);
		for (j=0;j<final_map_dim;j++){
			printf("%d\t",map[i*final_map_dim+j]);
		}
		printf("\n");
	}

	printf("Patched-------------\n");
	for (i=0;i<final_map_dim;i++){
		printf("[%d]\t",i);
		for (j=0;j<final_map_dim;j++){
			printf("%d\t",patch_map[i*final_map_dim+j]);
		}
		printf("\n");
	}
	*/

	unsigned err = 0;
	for (i=0;i<final_map_dim;i++){
		for (j=0;j<final_map_dim;j++){
			if (patch_map[i*final_map_dim+j] != map[i*final_map_dim+j])
				err++;
		}
	}

	if (!err)
		printf("SUCCESS!\n");
	else 
		printf("FAILURES = %d\n",err);

	return 0;
}
