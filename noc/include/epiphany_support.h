#ifndef _EPIPHANY_SUPPORT_H_
#define _EPIPHANY_SUPPORT_H_

#include "caffe_frontend.h"
#include "cnn_layers.h"
#include "types.h"
#include "debug_control.h"

void computePotentialPatchSizes(unsigned *possible_patch_widths, unsigned *possible_patch_heights, unsigned *num_patches, unsigned map_width, unsigned map_height);

// Computes the actual patch height/width needed, with ghost pixels, to compute the layer's output maps patch NxM at a time
void patch (unsigned *in_patch_width, unsigned *in_patch_height, unsigned patch_width, unsigned patch_height, unsigned map_width, unsigned map_height, unsigned kernel_dim, unsigned conv_stride, unsigned conv_pad, unsigned subsampling_factor, unsigned pooling_stride);

// This function returns the total number of patch combinations for input map width/height
int possiblePatchCombinations(unsigned map_width, unsigned map_height);

// This function computes the possible patch size combinations given an output map dimensions.
void computePotentialPatchSizes(unsigned *possible_patch_widths, unsigned *possible_patch_heights, unsigned *num_patches, unsigned map_width, unsigned map_height);

// This function merges Caffe layers so as to reduce off-chip data transfer between layers.
// Currently supports CONV + POOL = CONV_POOL merge transformations only.
int merge_layers(CNN_LYR_NODE_T *cnnLayers, int nLayers, CNN_LYR_NODE_T *mergedLayers);

// initialize a random image
void init_random_image(int width, int height, IMAGE_T *image);

// read in an image from file
void read_in_image(const char* filename, int width, int height, IMAGE_T *image);

#endif // _EPIPHANY_SUPPORT_H_