void patch(unsigned in_patch_height, unsigned in_patch_width, unsigned *out_patch_height, unsigned *out_patch_width, unsigned num_layers, unsigned out_layer_height, unsigned out_layer_width, unsigned subsampling_factors, unsigned kernel_dims, unsigned pooling_stride, unsigned conv_stride, unsigned conv_pad) {

	if ((out_layer_height % in_patch_height != 0) && (out_layer_width % in_patch_width != 0)){
		printf("Only patch dimnesions with perfect multiples of out_layer resolutions are supported! Please correct\n");
		exit(1);
	}

	unsigned i;
	unsigned curr_patch_height = in_patch_height;
	unsigned curr_patch_width = in_patch_width;
	for (i=num_layers;i>0;i--){
		
		curr_patch_height = curr_patch_height*subsampling_factors*pooling_stride;
		curr_patch_width = curr_patch_width*subsampling_factors*pooling_stride;
		curr_patch_height = curr_patch_height * conv_stride;
		curr_patch_width = curr_patch_width * conv_stride;
		curr_patch_height = curr_patch_height + kernel_dims - 1 + 2*conv_pad;
		curr_patch_width = curr_patch_width + kernel_dims - 1 + 2*conv_pad;
	}

	*out_patch_height = curr_patch_height;
	*out_patch_width = curr_patch_width;
	
	return;
}
