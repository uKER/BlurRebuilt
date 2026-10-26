#ifndef SPUDAMAGEDOUBLEBUFFER_H
#define SPUDAMAGEDOUBLEBUFFER_H

namespace SpuDamage
{

//! Double buffer for SPU-update of damage lattice.
template <typename _Type>
struct SPUDoubleBuffer
{
public:
	//! Default c'tor.
	__forceinline explicit SPUDoubleBuffer() :
		m_current(0);
	
	//! Return a pointer to the current buffer.
	//@return Pointer to the current used buffer.
	_Type*				GetCurrentBuffer()
	{ return &m_buffers[m_current]; }
	//!< Swap the buffers.
	__forceinline void SwapBuffers() __restrict
	{ m_current ^= 0x1; }
	
private:
	_Type		m_buffers[2];				//!< Inlined storage for the double buffers.
	uint16_t	m_current;					//!< Index of current buffer.
};

}

#endif // #ifndef SPUDAMAGEDOUBLEBUFFER_H
