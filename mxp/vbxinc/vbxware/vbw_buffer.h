/* VECTORBLOX MXP SOFTWARE DEVELOPMENT KIT
 *
 * Copyright (C) 2012-2016 VectorBlox Computing Inc., Vancouver, British Columbia, Canada.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of VectorBlox Computing Inc. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This agreement shall be governed in all respects by the laws of the Province
 * of British Columbia and by the laws of Canada.
 *
 * This file is part of the VectorBlox MXP Software Development Kit.
 *
 */


/**
 * @file
 */
#ifndef BUFFER_H
#define BUFFER_H
#include "vbx.h"
#ifdef __cplusplus
extern "C"{
#endif
	/**
	 * @defgroup rotating_prefetcher Rotating Prefetcher
	 * @ingroup VBXware
	 * @{
	 */

	/**
	 * @brief A rotating prefetcher object
	 *
	 * A rotating prefetcher object contains state information that makes it easy to prefetch
	 * a sliding window. The object is created with rotating_prefetcher(). Memory is prefetched
	 * with rp_fetch_next(), and buffers are accessed with rp_get_buffer().
	 *
	 */
	struct rotating_prefetcher_t{
		vbx_void_t* data;
		size_t buffer_size;
		int num_buffers;
		int current_buffer;
  		int chunk_size;
		int chunk_increment;
		int full/*bool*/;
		void* current_in;
		void* last_in;
	};
	typedef struct rotating_prefetcher_t rotating_prefetcher_t;

	static inline rotating_prefetcher_t
	rotating_prefetcher(int num_buffers, size_t chunk_size,
	                    void* first_in,void* last_in, ssize_t chunk_increment)
	{
		rotating_prefetcher_t self;
		self.num_buffers=num_buffers+1;
		self.buffer_size=chunk_size;
		size_t malloc_size=self.buffer_size*self.num_buffers;
		self.data=vbx_sp_malloc(malloc_size);

		self.current_buffer=0;

		self.chunk_increment=chunk_increment;
		self.chunk_size=chunk_size;

		self.current_in=first_in;
		self.last_in=last_in;
		self.full=0;
		return self;
	}
	static inline void rp_rotate(rotating_prefetcher_t* self)
	{
		self->current_buffer++;
		if (self->current_buffer>=self->num_buffers){
			self->full=1;
			self->current_buffer=0;
		}
	}

	static inline void rp_fetch(rotating_prefetcher_t* self)
	{
		char* cur_buf=(char*)self->data+self->current_buffer*self->buffer_size;
		if(self->current_in < self->last_in){
			if((size_t)self->current_in + self->chunk_size  > (size_t)self->last_in ){
				self->chunk_size=(size_t)self->last_in - (size_t) self->current_in;
			}

			vbx_dma_to_vector(cur_buf,self->current_in,self->chunk_size);
		}
		self->current_in=(char*)self->current_in+self->chunk_increment;
		rp_rotate(self);
	}
	static inline void* rp_get_buffer(rotating_prefetcher_t* self, int nth_buf)
	{
		int actual_buffer=nth_buf;
		if( self->full){
			actual_buffer+= self->current_buffer  ;
		}
		/*assume this loop is faster than a modulus operation*/
		while(actual_buffer >= self->num_buffers){
			actual_buffer -= self->num_buffers;
		}

		return (void*)((char*)self->data + self->buffer_size*actual_buffer);
	}
	/**
	 * @brief reset the rotating prefetcher object
	 *
	 * Invalidates all buffers and makes self ready to prefetch again
	 *
	 * @param self the object to reset
	 */
	static inline void rp_reset(rotating_prefetcher_t* self)
	{
		self->current_buffer=0;
		self->full=0;
	}
	/**
	 * @}
	 */

	/**
	 * @defgroup double_buffer Double Buffer
	 * @ingroup VBXware
	 * @{
	 */

	/* //double buffer logic */
	/* /\** */
	/*  * @brief Object containing state for switching between buffers during double buffering */
	/*  * */
	/*  * To use this object create one with the double_buffer() function, and use db_read_ahead() */
	/*  * to do the double buffering. */
	/*  *\/ */

	/* struct double_buffer_t{ */
	/* 	int which_buffer; */
	/* 	vbx_byte_t* buffers[2]; */
	/* 	void* last; */
	/* 	int init; */
	/* 	size_t size; */
	/* }; */
	/* typedef struct double_buffer_t double_buffer_t; */
	/* static inline void db_reset(struct double_buffer_t* db) */
	/* { */
	/* 	db->init=0; */
	/* 	db->which_buffer=0; */
	/* } */
	/* /\** */
	/*  * @brief Create a double buffer object. */
	/*  * */
	/*  * Buffers are allocated and the object is initialized */
	/*  * */
	/*  * @param size length in bytes of each buffer */
	/*  *\/ */
	/* static inline struct double_buffer_t double_buffer(size_t size){ */
	/* 	struct double_buffer_t db; */
	/* 	db.buffers[0]=(vbx_byte_t*)vbx_sp_malloc(size*2); */
	/* 	db.buffers[1]=db.buffers[0]+size; */
	/* 	db_reset(&db); */
	/* 	db.size=size; */
	/* 	return db; */
	/* } */
	/* /\** */
	/*  * @brief Get a scratchpad buffer with data transfered from main memory, */
	/*  *        and prepare a buffer for later use. */
	/*  * */
	/*  * The Function is given two pointers into main memory, srca and srcb */
	/*  * srca points to the memory that you want to access now, and srcb points */
	/*  * to the memory that you want to begin dma now, and access next time. */
	/*  * */
	/*  * @param db Double Buffer object to work with */
	/*  * @param srca Main memory ptr to copy data into scratchpad now */
	/*  * @param srcb Main memory ptr to copy data into scratchpad later */
	/*  * @return scratchpad pointer to a a copy of data pointed to by srcb */
	/*  *\/ */
	/* static inline vbx_void_t* db_read_ahead(struct double_buffer_t* db, void* srca,void* srcb) */
	/* { */
	/* 	int b=db->which_buffer; */
	/* 	if(db->last!=srca){ */
	/* 		db_reset(db); */
	/* 	} */
	/* 	if(!db->init) { */
	/* 		vbx_dma_to_vector( db->buffers[b], srca, db->size); */
	/* 		b = !b; */
	/* 		db->init=1; */
	/* 	} */
	/* 	vbx_dma_to_vector( db->buffers[b], srcb, db->size); */
	/* 	db->last= srcb; */
	/* 	b = !b; */
	/* 	db->which_buffer=b; */
	/* 	return db->buffers[b]; */
	/* } */

	/* static inline vbx_void_t* db_rotate(struct double_buffer_t* db) */
	/* { */
	/* 	int b=!db->which_buffer; */
	/* 	return db->buffers[b]; */
	/* } */
	/* /\** */
	/*  * @} */
	/*  *\/ */
#ifdef __cplusplus
	}
#endif

#endif //BUFFER_H
