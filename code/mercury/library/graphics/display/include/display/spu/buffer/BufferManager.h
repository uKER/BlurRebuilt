#ifndef DISPLAY_SPU_BUFFER_MANGER_H
#define DISPLAY_SPU_BUFFER_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif



#include "Display/Spu/Buffer/MemoryBuffer.h"



namespace Display
{
namespace Spu
{



bool CreateMemoryBuffer(const Display::E_ResourceType type, const U32 flags, const U32 size, const U32 labelNumber, C_MemoryBuffer * pMemoryBuffer);
void DestroyMemoryBuffer(C_MemoryBuffer * pMemoryBuffer);
void BeginMemoryBuffer(C_MemoryBuffer * pMemoryBuffer);
void EndMemoryBuffer(C_MemoryBuffer * pMemoryBuffer);
U8 * AllocateFromMemoryBuffer(const U32 sizeInBytes, const U32 alignment, C_MemoryBuffer * pMemoryBuffer);



template <int T>
class C_MemoryMultiBuffer
{

public:

	C_MemoryMultiBuffer();
	~C_MemoryMultiBuffer();

	bool				Create(const Display::E_ResourceType type, const U32 flags, const U32 size, const U32 labelNumber);
	void				Destroy();

	void				Begin();
	void				End();

	U8 *				Allocate(const U32 sizeInBytes, const U32 alignment);

	C_MemoryBuffer *	Get();

public:

	C_MemoryBuffer		m_Buffers[T];
	U32					m_BufferIndex;

};



typedef C_MemoryMultiBuffer<2> C_MemoryDoubleBuffer;



}; // namespace Spu
}; // namespace Display



#include "Display/Spu/Buffer/BufferManager.inl"



#endif // DISPLAY_SPU_BUFFER_MANAGER_H
