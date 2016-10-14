#include "sim_image.h"
#include "debug_control.h"

void write_image(const char *name, char *imgData, int nRows, int nCols) {
#if 0
	CvSize size;

	size.height = nRows;
	size.width  = nCols;
	IplImage * outputImg = cvCreateImageHeader(size, IPL_DEPTH_8U, 1);

	outputImg->imageData = imgData;
	cvSaveImage(name, outputImg, NULL);
#endif
}

void show_image(const char *name, char *imgData, int nRows, int nCols) {
#if 0
	CvSize size;

	size.height = nRows;
	size.width = nCols;
	IplImage * dispImg = cvCreateImageHeader(size, IPL_DEPTH_8U, 1);
	dispImg->imageData = imgData;
	cvShowImage(name, dispImg);
#endif
}

uint8_t * read_gray_image(const char *fileName, int *H, int *W) {
	uint8_t *data;
#if 0
	CvSize orgSize;

	IplImage* colorImg = cvLoadImage(fileName, CV_LOAD_IMAGE_COLOR);
	if (NULL == colorImg) {
		DBG_INFO("cannot load image\n");
		return NULL;
	}
	orgSize.height = colorImg->height;
	orgSize.width = colorImg->width;
	*H = colorImg->height;
	*W = colorImg->width;
	if(NULL == (data = (uint8_t *) malloc(colorImg->height*colorImg->width*sizeof(uint8_t)))) {
		DBG_INFO("Malloc failed\n");
		return NULL;
	}
	
	IplImage *grayImg = cvCreateImage(cvGetSize(colorImg), IPL_DEPTH_8U, 1);
	cvCvtColor(colorImg, grayImg, CV_RGB2GRAY);
	memcpy(data, grayImg->imageData, (colorImg->height*colorImg->width*sizeof(uint8_t)));
#endif
	return data;
}
