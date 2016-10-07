void init_test_image(IMAGE_T *image){
	int i;
	for (i=0;i<IMAGE_SIZE;i++){
		image[i] = (IMAGE_T)(rand()%2); // randomly initialize image
	}
}
