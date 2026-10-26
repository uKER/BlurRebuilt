#ifndef DISPLAY_SPU_BUFFER_MANAGER_INL
#define DISPLAY_SPU_BUFFER_MANAGER_INL

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



template <int T>
C_MemoryMultiBuffer<T>::C_MemoryMultiBuffer()
{
	m_BufferIndex = 0;
}

template <int T>
C_MemoryMultiBuffer<T>::~C_MemoryMultiBuffer()
{
}

template <int T>
bool C_MemoryMultiBuffer<T>::Create(const Display::E_ResourceType type, const U32 flags, const U32 size, const U32 labelNumber)
{
	bool succeeded = true;
	for (U32 bufferNo = 0; bufferNo < T; ++bufferNo)
	{
		succeeded = succeeded && CreateMemoryBuffer(type, flags, size, labelNumber + bufferNo, &m_Buffers[bufferNo]);
	}
	return succeeded;
}

template <int T>
void C_MemoryMultiBuffer<T>::Destroy()
{
	for (U32 bufferNo = 0; bufferNo < T; ++bufferNo)
	{
		DestroyMemoryBuffer( &m_Buffers[bufferNo] );
	}
}

template <int T>
void C_MemoryMultiBuffer<T>::Begin()
{
	BeginMemoryBuffer( &m_Buffers[m_BufferIndex] );
}

template <int T>
void C_MemoryMultiBuffer<T>::End()
{
	EndMemoryBuffer( &m_Buffers[m_BufferIndex] );

	m_BufferIndex = (m_BufferIndex + 1) % T;;
}

template <int T>
C_MemoryBuffer * C_MemoryMultiBuffer<T>::Get()
{
	return &m_Buffers[m_BufferIndex];
}



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_BUFFER_MANAGER_INL
