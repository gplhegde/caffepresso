#ifndef _SIM_IMAGE_H_
#define _SIM_IMAGE_H_

#include<opencv/highgui.h>
#include<opencv/cxcore.h>
#include<opencv/cv.h>
#include "struct_defs.h"
/** \brief Write image to a file
*
* @param[in] name	Name of the output image file.
* @param[in] imgData	Pointer to gray scale image data buffer.
* @param[in] nRows	Number of rows in the image.
* @param[in] nCols	Number of columns in the image.
*/
void write_image(const char *name, char *imgData, int nRows, int nCols);

/** \brief Display image 
*
* @param[in] name	Image name to be displayed.
* @param[in] imgData	Pointer to gray scale image data buffer.
* @param[in] nRows	Number of rows in the image.
* @param[in] nCols	Number of columns in the image.
*/
void show_image(const char *name, char *imgData, int nRows, int nCols);

uint8_t * read_gray_image(const char *fileName, int *H, int *W);

#endif // _SIM_IMAGE_H_
