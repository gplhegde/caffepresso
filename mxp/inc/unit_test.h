#ifndef _UNIT_TEST_H
#define _UNIT_TEST_H
#include "struct_defs.h"

#define ERR_THRESHOLD (1e-2)

TEST_STATUS_E test_pool_layer();
TEST_STATUS_E test_conv_layer();

#endif // _UNIT_TEST_H
