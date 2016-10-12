// Copyright (C) 2012-2014 VectorBlox Computing, Inc.
//
#ifndef __VBX_LIB_XIL_H
#ifndef __VBX_LIB_H
#error This file should not be included directly, include vbx_lib.h instead
#endif
#define __VBX_LIB_XIL_H

// ------------------------------------

#define VBX_OP_DMA_TO_HOST   0
#define VBX_OP_DMA_TO_VECTOR 1
#define VBX_OP_SET_VL        2

// opcode extension flag; use acc since it's not used by other opcodes
#define VBX_OP_EXT          (1 << MASKED_BIT)
// extended opcodes
#define VBX_OP_GET_PARAM     0
#define VBX_OP_SET_PARAM     1
#define VBX_OP_SYNC          2
#define VBX_OP_GET_MASK      3
// ------------------------------------

 __attribute__((always_inline)) inline static void _vbx_sync ()
{
	uint32_t t;
	VBX_INSTR_SINGLE((((VBX_OP_SYNC) << (VBX_OPCODE_SHIFT)) | (VBX_OP_EXT)),t);
}

__attribute__((always_inline)) inline static void _vbx_dma_to_host( void *EXT, vbx_void_t *INT, int LENGTH )
{
	VBX_INSTR_QUAD(((VBX_OP_DMA_TO_HOST) << (VBX_OPCODE_SHIFT)), \
	               (VBX_DMA_ADDR(EXT,LENGTH)), \
	               (INT), \
	               (LENGTH));
}
__attribute__((always_inline)) inline static void _vbx_dma_to_vector( vbx_void_t *INT, void* EXT, int LENGTH )
{
	VBX_INSTR_QUAD(((VBX_OP_DMA_TO_VECTOR) << (VBX_OPCODE_SHIFT)), \
	               (VBX_DMA_ADDR(EXT,LENGTH)), \
	               (INT), \
	               (LENGTH));
}

__attribute__((always_inline)) static inline void VBX_DMA_SET_2D(uint32_t ROWS,uint32_t EXT_INCR,uint32_t INT_INCR)
{
	VBX_INSTR_QUAD((((VBX_OP_DMA_TO_VECTOR) << (VBX_OPCODE_SHIFT)) | VBX_MODE_SV | MOD_2D),
	               (INT_INCR),
	               (ROWS),
	               ((ROWS)*(INT_INCR)));
	VBX_INSTR_QUAD((((VBX_OP_DMA_TO_HOST) << (VBX_OPCODE_SHIFT)) | VBX_MODE_SV),
	               (EXT_INCR),
	               0,
	               0);
}

__attribute__((always_inline)) inline static void _vbx_dma_to_host_2D( void* EXT,void* INT, size_t ROW_LEN, uint32_t ROWS,
                          uint32_t EXT_INCR, uint32_t INT_INCR)
{
	VBX_DMA_SET_2D(ROWS, EXT_INCR, INT_INCR);
	VBX_INSTR_QUAD((((VBX_OP_DMA_TO_HOST) << (VBX_OPCODE_SHIFT)) | MOD_2D),
	               (VBX_DMA_ADDR(EXT,EXT_INCR*ROWS)),
	               (INT),
	               (ROW_LEN));
}
__attribute__((always_inline)) inline static void _vbx_dma_to_vector_2D(void* INT,void* EXT, uint32_t ROW_LEN, uint32_t ROWS,
                           uint32_t INT_INCR,uint32_t EXT_INCR)
{
	VBX_DMA_SET_2D(ROWS, EXT_INCR, INT_INCR);
	VBX_INSTR_QUAD((((VBX_OP_DMA_TO_VECTOR) << (VBX_OPCODE_SHIFT)) | MOD_2D),
	               (VBX_DMA_ADDR(EXT,ROW_LEN)),
	               (INT),
	               (ROW_LEN));
}


#define VBX_GET(ADDRESS_REG, RETURN_REG)  \
	VBX_INSTR_SINGLE((((VBX_OP_GET_PARAM) << (VBX_OPCODE_SHIFT)) | (VBX_OP_EXT) | ((ADDRESS_REG) << SIGNED_BIT)), \
	                 RETURN_REG)

#define VBX_GET_MASK(RETURN_REG)  \
	VBX_INSTR_SINGLE((((VBX_OP_GET_MASK) << (VBX_OPCODE_SHIFT)) | (VBX_OP_EXT)), \
	                 RETURN_REG)

#define VBX_SET(ADDRESS_REG, VALUE_REG)	\
	VBX_INSTR_DOUBLE((((VBX_OP_SET_PARAM) << (VBX_OPCODE_SHIFT)) | (VBX_OP_EXT) | ((ADDRESS_REG) << SIGNED_BIT)), \
	                 (VALUE_REG))

#define VBX_SET_VL(MODIFIERS,LENGTHA,LENGTHB) \
	VBX_INSTR_QUAD((((VBX_OP_SET_VL) << (VBX_OPCODE_SHIFT)) | (MODIFIERS)), \
	               (LENGTHA), \
	               (LENGTHB), \
	               (0))

#define VBX_SET_VL3D(MODIFIERS,LENGTHA,LENGTHB) \
	 VBX_SET_VL(((MODIFIERS) | (MOD_3D)), (LENGTHA), (LENGTHB))


void vbx_set_reg( int REGADDR, int  VALUE )
{
	VBX_SET( REGADDR, VALUE );
}

void vbx_get_reg( int REGADDR, int *VALUE )
{
	int val;
	VBX_GET( REGADDR, val );
	*VALUE = val;
}
#endif// __VBX_LIB_XIL_H
