/*
 * mem_manager.h
 *
 *  Created on: 18 Nov 2016
 *      Author: Gopalakrishna Hegde
 */

#ifndef _MEM_MANAGER_H_
#define _MEM_MANAGER_H_
#include <stdio.h>

void * shared_malloc(size_t size);
void * private_malloc(size_t size);
void * ext_malloc(size_t size);
void reset_mem_manager();
void shared_free(void *ptr);
void private_free(void *ptr);
void ext_free(void *ptr);
#endif /* INC_MEM_MANAGER_H_ */
