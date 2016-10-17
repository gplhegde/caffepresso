#include <stdio.h>
#include "debug_control.h"
#include "unit_test.h"
#include "vbx.h"

int test_layers();
void mxp_init();

int main(void) {
	mxp_init();
	test_layers();
	printf("Application complete\n");
	return 0;
}


int test_layers() {
	TEST_STATUS_E status;
#if 1
	status = test_pool_layer();
	if(status != TEST_PASS) {
		REL_INFO("Pool layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}
#endif
#if 0
	status = test_conv_layer();
	if(status != TEST_PASS) {
		REL_INFO("Conv layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}
#endif
#if 0
	status = test_ip_layer();
	if(status != TEST_PASS) {
		REL_INFO("Inner product layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}
#endif
	return status;
}

void mxp_init() {
	// Simulator init
	REL_INFO("Initializing MXP Simulator\n");
	vbxsim_init(16,     //vector_lanes
		64,     //KB scratchpad_size
		256,    //max masked waves
		16,     //fractional_bits (word)
		15,     //fractional_bits (half)
		4);     //fractional_bits (byte)
}

