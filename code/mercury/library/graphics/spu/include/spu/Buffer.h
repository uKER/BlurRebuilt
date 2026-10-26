#ifndef GRAPHICS_SPU_BUFFER_H
#define GRAPHICS_SPU_BUFFER_H

#if _MSC_VER > 1000
#pragma once
#endif

#if defined(PLATFORM_SPU)

#include <spu_intrinsics.h>

namespace Spu
{

template <typename T>
class MemoryDoubleBuffer
{
public:

	MemoryDoubleBuffer(const U32 count)
	{
		const U32 size = sizeof(T) * count;
		m_pAddress0 = (U8*)memalign(0x80, size);
		m_pAddress1 = (U8*)memalign(0x80, size);
		m_Count = count;
		m_Index = 0;
	}

	operator T*()				{ return (T*)m_pAddress0; }
	
	T* GetBuffer0()				{ return (T*)m_pAddress0; }
	T* GetBuffer1()				{ return (T*)m_pAddress1; }
	const T* GetBuffer0() const	{ return (T*)m_pAddress0; }
	const T* GetBuffer1() const	{ return (T*)m_pAddress1; }

	U32 GetCount() const		{ return m_Count; }
	U32 GetIndex() const		{ return m_Index; }

	void SwapBuffers()
	{
		U8* pTmp = m_pAddress0;
		m_pAddress0 = m_pAddress1;
		m_pAddress1 = pTmp;
		m_Index = 1 - m_Index;
	}

private:
	
	U8*	m_pAddress0;
	U8*	m_pAddress1;
	U32	m_Count;
	U32	m_Index;

};

}; // namespace Spu

#endif // PLATFORM_SPU

#endif // GRAPHICS_SPU_BUFFER_H
