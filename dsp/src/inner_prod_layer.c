#include "inner_prod_layer.h"
#include "struct_defs.h"

STATUS_E dsp_ip_layer(FIX_MAP *p_input,	// pointer to input features
	FIX_KER *p_weight,	// pointer to weight matrix stored in [no_outputs][no_inputs] manner
	FIX_KER *p_bias,	// pointer to bias units
	int no_inputs,		// number of input units to this layer
	int no_outputs,		// number of output units
	int shift,			// shift used to convert the dot product to 16b. Perform conversion before adding bias
	FIX_MAP *p_output	// pointer to output features.
	) {

	STATUS_E status = FAILED;
	// TODO 
	// Refer to https://github.com/gplhegde/caffepresso/blob/master/mxp/src/inner_prod_layer.c:scalar_fix_ip_layer function for
	// implementation details.
	return status;
}
