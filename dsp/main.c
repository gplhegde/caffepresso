#include <stdio.h>
#include <string.h>
#include "debug_control.h"
#include "cnn_app.h"
#include "caffe_proto_params.h"

#ifndef DEVICE_K2H
#error "Device not specified"
#endif
#include <ti/csl/cslr_device.h>




int main() {
	uint8_t *p_image;
	int label, img_width, img_height;

	//main_cnn_app_init();

	//main_cnn_app(pImage, &label);

	printf("Application complete\n");

	return 0;
}





