#ifndef __VBX_LIB_SIM_H
#ifndef __VBX_LIB_H
#error This file should not be included directly, include vbx_lib.h instead
#endif
#define __VBX_LIB_SIM_H
void VBX_SET( uint32_t reg, uint32_t value );

void VBX_SET_VL( uint32_t mode, uint32_t new_vl1, uint32_t new_vl2 );
void VBX_SET_VL3D( uint32_t mode, uint32_t new_vl1, uint32_t new_vl2 );
uint32_t _VBX_GET1( uint32_t reg );
inline static void _vbx_sync ()
{
	void vbx_sim_sync(void);
	vbx_sim_sync();
}
#define VBX_GET(reg, value) value = _VBX_GET1(reg)
inline static void _vbx_dma_to_host( void *EXT, vbx_void_t *INT, int LENGTH )
{
	void sim_dma_to_host(void* to,void* from,size_t num_bytes);
	sim_dma_to_host(EXT,INT,LENGTH);
}
inline static void _vbx_dma_to_vector( vbx_void_t *INT, void* EXT, int LENGTH )
{
	void sim_dma_to_vector(void* to,void* from,size_t num_bytes);
	sim_dma_to_vector(INT,EXT,LENGTH);
}

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
void vbx_get_mask( int* val);
#define VBX_GET_MASK(val) vbx_get_mask(&(val))
#endif // __VBX_LIB_SIM_H
