#ifndef DISPLAY_MEMORY_MEMORY_LINEAR_BUFFER_H
#define DISPLAY_MEMORY_MEMORY_LINEAR_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#if !defined(PLATFORM_PC)

#include <Btl/Allocators/Allocator.h>
#include <Btl/Allocators/Pool.h>
#include <Btl/Utility/Threads.h>
#if PLATFORM_SPU
#include <Display/Device/Gcm/GCMDeviceEnums.h>
#else
#include <Display/Device/Device.h>
#endif

namespace Display
{

#if PLATFORM_SPU
using namespace GCM;
#endif

class C_LinearBuffer
{
public:
	C_LinearBuffer();
	~C_LinearBuffer();

	bool	Create(const U32 size, const U32 alignment, const E_AllocHeap eHeap);
	void	Destroy();	

	void*	Allocate(const U32 size, const U32 alignment);
	void	Free();

	U32		Used() const;

private:

	U8*							m_pBase;
	U8*							m_pTop;
	U8*							m_pPosition;
	E_AllocHeap					m_eHeap;

};

}; // namespace Display

#endif // PLATFORM_PC

#endif // DISPLAY_MEMORY_MEMORY_LINEAR_BUFFER_H
