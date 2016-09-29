void patch(unsigned in_patch_height, unsigned in_patch_width, unsigned *out_patch_height, unsigned *out_patch_width, unsigned num_layers, unsigned out_layer_height, unsigned out_layer_width, unsigned subsampling_factors, unsigned kernel_dims, unsigned pooling_stride, unsigned conv_stride, unsigned conv_pad) {

	if ((out_layer_height % in_patch_height != 0) && (out_layer_width % in_patch_width != 0)){
		printf("Only patch dimnesions with perfect multiples of out_layer resolutions are supported! Please correct\n");
		exit(1);
	}

//	printf("Image dimensions = %dx%d\n",image_dim,image_dim);
//	printf("Requested patch size = %dx%d\n",in_patch_dim,in_patch_dim);
//	printf("Number of layers = %d\n",num_layers);

	unsigned i;
	unsigned curr_patch_height = in_patch_height;
	unsigned curr_patch_width = in_patch_width;
//	printf("Patch dim final = %dx%d\n",curr_patch_dim,curr_patch_dim);
	for (i=num_layers;i>0;i--){
		
		curr_patch_height = curr_patch_height*subsampling_factors*pooling_stride;
		curr_patch_width = curr_patch_width*subsampling_factors*pooling_stride;
		curr_patch_height = curr_patch_height * conv_stride;
		curr_patch_width = curr_patch_width * conv_stride;
		curr_patch_height = curr_patch_height + kernel_dims - 1 + 2*conv_pad;
		curr_patch_width = curr_patch_width + kernel_dims - 1 + 2*conv_pad;
//		printf("Patch dim after layer %d = %dx%d\n",(i-1),curr_patch_dim,curr_patch_dim);

	}

//	printf("Actual patch size = %dx%d\n",curr_patch_dim,curr_patch_dim);
	*out_patch_height = curr_patch_height;
	*out_patch_width = curr_patch_width;
	
	return;
}
