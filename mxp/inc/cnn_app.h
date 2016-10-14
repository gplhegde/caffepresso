#ifndef _CNN_APP_H_
#define _CNN_APP_H_
#include "struct_defs.h"
APP_STATUS_E main_cnn_app_init();

APP_STATUS_E main_cnn_app(char *pImage, int *pLabel);

#endif // _CNN_APP_H_
