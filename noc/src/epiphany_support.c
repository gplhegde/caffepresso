#include "common.h"
#include "epiphany_support.h"

// Computes the actual patch height/width needed, with ghost pixels, to compute the layer's output maps patch NxM at a time
void patch (
	unsigned *in_patch_width, 	// actual input patch height (with ghost pixels) -- computed by this routine
	unsigned *in_patch_height, 	// actual input patch width (with ghost pixels) -- computed by this routine 
	unsigned patch_width,		// final patch width
	unsigned patch_height, 		// final patch height
	unsigned map_width,			// layer's output map width
	unsigned map_height,		// layer's output map height
	unsigned kernel_dim,		// convolution kernel dimension (assuming KxK square kernel)
	unsigned conv_stride,		// convolution stride 
	unsigned conv_pad,			// padding before any convolution
	unsigned subsampling_factor,// subsampling factor in layer
	unsigned pooling_stride		// layer's pooling stride
) {

	// Sanity check to ensure only supported patch dimensions is selected.
	if ((map_width % patch_width != 0) || (map_height % patch_height != 0)){
		printf("Only patch dimnesions with perfect multiples of map resolutions are supported!.\n");
		int p = possiblePatchCombinations(map_width,map_height);
		unsigned *pw = (unsigned *)malloc(p*sizeof(unsigned));
		unsigned *ph = (unsigned *)malloc(p*sizeof(unsigned));
		unsigned *np = (unsigned *)malloc(p*sizeof(unsigned));
		computePotentialPatchSizes(pw,ph,np,map_width,map_height);
		int i;
		printf("%d possible patch WxH combinations : \n",p);
		for (i=0;i<p;i++) {
			printf("[%d] W x H = %d x %d, %d patches total\n",i+1,pw[i],ph[i],np[i]);
		}
		printf("Exiting..\n");
		free(pw);free(ph);free(np);
		exit(1);
	}

	unsigned curr_patch_height = patch_height;
	unsigned curr_patch_width = patch_width;
		
	curr_patch_width = curr_patch_width * subsampling_factor * pooling_stride;
	curr_patch_height = curr_patch_height * subsampling_factor * pooling_stride;

	curr_patch_width = curr_patch_width * conv_stride;
	curr_patch_height = curr_patch_height * conv_stride;
	
	curr_patch_width = curr_patch_width + kernel_dim - 1 + 2 * conv_pad;
	curr_patch_height = curr_patch_height + kernel_dim - 1 + 2 * conv_pad;

	*in_patch_width = curr_patch_width;
	*in_patch_height = curr_patch_height;
	
	return;
}

// This function returns the total number of patch combinations for input map width/height
int possiblePatchCombinations (
	unsigned map_width,
	unsigned map_height
) {
	int i, a = 0, b = 0;
	for (i=1;i<=map_width;i++) {
		if (map_width%i == 0) {
			a++;
		}
	}
	for (i=1;i<=map_height;i++) {
		if (map_height%i == 0) {
			b++;
		}
	}
	return a*b;
}

// This function computes the possible patch size combinations given an output map dimensions.
// The function also returns the total number of possibilities for patch width/height dimensions.
void computePotentialPatchSizes (
	unsigned *possible_patch_widths, 	// pointer to possible patch widths
	unsigned *possible_patch_heights, 	// pointer to possible patch heights
	unsigned *num_patches, 			// pointer to number of patches for each combination
	unsigned map_width, 			// output map width
	unsigned map_height 			// output map height
) {
	int i, j;
	int count = 0;
	for (i=1;i<=map_width;i++) {
		if (map_width%i == 0) {
			for (j=1;j<=map_height;j++) {
				if (map_height%j == 0) {
					possible_patch_widths[count] = i;
					possible_patch_heights[count] = j;
					num_patches[count] = (map_width/i)*(map_height/j);
					count++;
				}
			}
		}
	}
	return;
}

int merge_layers(CNN_LYR_NODE_T *cnnLayers, int nLayers, CNN_LYR_NODE_T *mergedLayers) {

	// useful for dereferencing void pointers
	CONV_POOL_LYR_CTX_T *pConvPoolCtx;
	CONV_LYR_CTX_T *pConvCtx;
	POOL_LYR_CTX_T *pPoolCtx;
	ACT_LYR_CTX_T *pActCtx;
	IP_LYR_CTX_T * pIpCtx;
	SMAX_LYR_CTX_T *pSmaxCtx;

	int layers = 0, lyr, hit = 0;
	for(lyr=0;lyr<nLayers;lyr++) {
		switch(cnnLayers[lyr].lyrType) {
			case CONV:
				if (cnnLayers[lyr+1].lyrType == POOL) { // found a merge candidate
					printf("*MERGE* [L%d] CONV_POOL\n",layers);
					mergedLayers[layers].lyrType = CONV_POOL; // set layerType
					mergedLayers[layers].pLyrCtx = (CONV_POOL_LYR_CTX_T *)malloc(sizeof(CONV_POOL_LYR_CTX_T));
					pConvPoolCtx = (CONV_POOL_LYR_CTX_T *)mergedLayers[layers].pLyrCtx;
					pConvCtx = (CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx;
					pPoolCtx = (POOL_LYR_CTX_T *)cnnLayers[lyr+1].pLyrCtx;
					pConvPoolCtx->convInfo = (CONV_INFO_T){
						.mapH = pConvCtx->convInfo.mapH,
						.mapW = pConvCtx->convInfo.mapW,
						.K = pConvCtx->convInfo.K,
						.stride = pConvCtx->convInfo.stride,
						.pad = pConvCtx->convInfo.pad,
						.nInMaps = pConvCtx->convInfo.nInMaps,
						.nOutMaps = pConvCtx->convInfo.nOutMaps,
						.stride = pConvCtx->convInfo.stride,
						.pad = pConvCtx->convInfo.pad
					};
					pConvPoolCtx->poolInfo = (POOL_INFO_T) {
						.mapH = pPoolCtx->poolInfo.mapH,
						.mapW = pPoolCtx->poolInfo.mapW,
						.nMaps = pPoolCtx->poolInfo.nMaps,	
						.winSize = pPoolCtx->poolInfo.winSize,
						.stride = pPoolCtx->poolInfo.stride,
						.pad = pPoolCtx->poolInfo.pad,
						.poolType = pPoolCtx->poolInfo.poolType
					};
					hit = 1;
				} else {
					printf("*MERGE* [L%d] CONV\n",layers);
					mergedLayers[layers].lyrType = CONV; // set layerType
					mergedLayers[layers].pLyrCtx = (CONV_LYR_CTX_T *)malloc(sizeof(CONV_LYR_CTX_T));
					pConvCtx = (CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx;
					pConvCtx->convInfo = (CONV_INFO_T){
						.mapH = ((CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->convInfo.mapH,
						.mapW = ((CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->convInfo.mapW,
						.K = ((CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->convInfo.K,
						.stride = ((CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->convInfo.stride,
						.pad = ((CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->convInfo.pad,
						.nInMaps = ((CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->convInfo.nInMaps,
						.nOutMaps = ((CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->convInfo.nOutMaps,
						.stride = ((CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->convInfo.stride,
						.pad = ((CONV_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->convInfo.pad
					};
					layers++;
				}
				break;
			case POOL:
				if (hit) {
					hit = 0; // reset hit
				} else {
					printf("*MERGE* [L%d] POOL\n",layers);
					mergedLayers[layers].lyrType = POOL; // set layerType
					mergedLayers[layers].pLyrCtx = (POOL_LYR_CTX_T *)malloc(sizeof(POOL_LYR_CTX_T));
					pPoolCtx = (POOL_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx;
					((POOL_LYR_CTX_T *)mergedLayers[layers].pLyrCtx)->poolInfo = (POOL_INFO_T) {
						.mapH = pPoolCtx->poolInfo.mapH,
						.mapW = pPoolCtx->poolInfo.mapW,
						.nMaps = pPoolCtx->poolInfo.nMaps,	
						.winSize = pPoolCtx->poolInfo.winSize,
						.stride = pPoolCtx->poolInfo.stride,
						.pad = pPoolCtx->poolInfo.pad,
						.poolType = pPoolCtx->poolInfo.poolType
					};
				}
				layers++;
				break;
			case ACT:
				printf("*MERGE* [L%d] ACT\n",layers);
				mergedLayers[layers].lyrType = ACT; // set layerType
				mergedLayers[layers].pLyrCtx = (ACT_LYR_CTX_T *)malloc(sizeof(ACT_LYR_CTX_T));
				pActCtx = (ACT_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx;
				((ACT_LYR_CTX_T *)mergedLayers[layers].pLyrCtx)->actInfo = (ACT_INFO_T) {
					.nMaps = pActCtx->actInfo.nMaps,
					.mapH = pActCtx->actInfo.mapH,
					.mapW = pActCtx->actInfo.mapW,
					.actType = pActCtx->actInfo.actType
				};
				layers++;
				break;
			case INNER_PROD:
				printf("*MERGE* [L%d] INNER_PROD\n",layers);
				mergedLayers[layers].lyrType = INNER_PROD; // set layerType
				mergedLayers[layers].pLyrCtx = (IP_LYR_CTX_T *)malloc(sizeof(IP_LYR_CTX_T));
				((IP_LYR_CTX_T *)mergedLayers[layers].pLyrCtx)->ipInfo = (IP_INFO_T) {
					.nInput = ((IP_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->ipInfo.nInput,
					.nOutput = ((IP_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->ipInfo.nOutput
				};
				layers++;
				break;
			case SOFTMAX:
				printf("*MERGE* [L%d] SOFTMAX\n",layers);
				mergedLayers[layers].lyrType = SOFTMAX; // set layerType
				mergedLayers[layers].pLyrCtx = (SMAX_LYR_CTX_T *)malloc(sizeof(SMAX_LYR_CTX_T));
				((SMAX_LYR_CTX_T *)mergedLayers[layers].pLyrCtx)->nInputs = ((SMAX_LYR_CTX_T *)cnnLayers[lyr].pLyrCtx)->nInputs;
				layers++;
				break;
			default:
				REL_INFO("Unsupported layer\n");
				return UNSUPPORTED_FEATURE;
		}
	}
	return layers;
}

// TODO: write a function to read in an image
// use opencv?
void read_in_image(const char* filename, int width, int height, IMAGE_T *image) {

}

// randomly instantiate a gray image, pixel value between 0 (black) and 255 (white)
void init_random_image(int width, int height, IMAGE_T *image) {
	int i, j;
	for (i=0;i<height;i++) {
		for (j=0;j<width;j++) {
			image[i*width+j] = rand()%256;
		}
	}
}
