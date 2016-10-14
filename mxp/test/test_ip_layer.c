#include "unit_test.h"
#include "inner_prod_layer.h"
#include "misc_utils.h"
#include <math.h>

FL_MAP_PIXEL *pRefFltOutput;

void compute_ip_ref(IP_LYR_CTX_T *pCtx, FL_MAP_PIXEL *pFloatInput) {
	int out, in;
	FL_MAP_PIXEL sum;
	for(out = 0; out < pCtx->ipInfo.nOutput; out++) {
		sum = 0.0;
		for(in = 0; in < pCtx->ipInfo.nInput; in++) {
			sum += pCtx->pFloatWeight[out * pCtx->ipInfo.nInput + in] * pFloatInput[in];
		}
		pRefFltOutput[out] = sum + pCtx->pFloatBias[out];
	}
}

CMP_STATUS_T compare_ip_out(IP_LYR_CTX_T *pCtx, FL_MAP_PIXEL *pOutput) {
	CMP_STATUS_T status;
	status.misMap = -1;
	status.misRow = -1;
	status.misCol = -1;
	status.flag = TEST_PASS;

	for(int out = 0; out < pCtx->ipInfo.nOutput; out++) {
		if(fabs(pOutput[out] - pRefFltOutput[out]) > ERR_THRESHOLD) {
			status.misMap = 1;
			status.misRow = 1;
			status.misCol = out;
			status.flag = TEST_FAIL;
			return status;
		}
	}
	return status;
}

TEST_STATUS_E test_ip_layer() {
	int noInputs, noOutputs;
	int nMapFracBits, nKerFracBits;
	FL_MAP_PIXEL *pFltInput;
	FP_MAP_PIXEL *pFixInput;
	
	CMP_STATUS_T status;

	IP_LYR_CTX_T ipCtx;

	printf("Testing Inner product Layer\n");
	noInputs = 16;
	noOutputs = 32;
	nMapFracBits = 13;
	nKerFracBits = 13;

	ipCtx.ipInfo = (IP_INFO_T){noInputs, noOutputs, nKerFracBits, nMapFracBits};
	ipCtx.lyrArithMode = FLOAT_POINT; 
	ipCtx.optType = SCALAR;

	// input and output buffer allocation	
	ipCtx.pFloatOutput = malloc(noOutputs * sizeof(FL_MAP_PIXEL));
	ipCtx.pFixOutput = malloc(noOutputs * sizeof(FP_MAP_PIXEL));
	ipCtx.pFloatWeight = malloc(noInputs * noOutputs * sizeof(FL_KERNEL));
	ipCtx.pFixWeight = malloc(noInputs * noOutputs * sizeof(FP_KERNEL));
	ipCtx.pFloatBias = malloc(noOutputs *sizeof(FL_KERNEL));
	ipCtx.pFixBias = malloc(noOutputs *sizeof(FP_KERNEL));

	pRefFltOutput = malloc(noOutputs * sizeof(FL_MAP_PIXEL));
	pFltInput = malloc(noInputs * sizeof(FL_MAP_PIXEL));
	pFixInput = malloc(noInputs * sizeof(FP_MAP_PIXEL));

	// random input
	generate_random_data(pFltInput, noInputs, 123);
	generate_random_data(ipCtx.pFloatWeight, noInputs * noOutputs, 345);
	generate_random_data(ipCtx.pFloatBias, noOutputs, 321);

	float_to_fix_data(pFltInput, noInputs, nMapFracBits, pFixInput);
	float_to_fix_data(ipCtx.pFloatWeight, noInputs * noOutputs, nKerFracBits, ipCtx.pFixWeight);
	float_to_fix_data(ipCtx.pFloatBias, noOutputs, nMapFracBits, ipCtx.pFixBias);


	inner_prod_layer(&ipCtx, pFltInput, pFixInput);
	ipCtx.lyrArithMode = FIXED_POINT; 
	//ipCtx.optType = VECTOR_MXP;
	inner_prod_layer(&ipCtx, pFltInput, pFixInput);

	print_float_img(ipCtx.pFloatOutput, 1, noOutputs);

	compute_ip_ref(&ipCtx, pFltInput);
	print_float_img(pRefFltOutput, 1, noOutputs);

	status = compare_ip_out(&ipCtx, ipCtx.pFloatOutput);
	printf("Comparing floating point output\n");
	check_cmp_status(&status);

	fix16_to_float_data(ipCtx.pFixOutput, noOutputs, nMapFracBits, ipCtx.pFloatOutput);
	print_float_img(ipCtx.pFloatOutput, 1, noOutputs);
	status = compare_ip_out(&ipCtx, ipCtx.pFloatOutput);
	check_cmp_status(&status);

	free(ipCtx.pFloatOutput);
	free(ipCtx.pFixOutput);
	free(ipCtx.pFloatWeight);
	free(ipCtx.pFixWeight);
	free(ipCtx.pFloatBias);
	free(ipCtx.pFixBias);

	free(pRefFltOutput);
	free(pFixInput);
	free(pFltInput);
	return status.flag;
}
