// [Blur1PCDX11] Native PC memory allocation path.
#ifndef DISPLAY2_PCD3D11_MEMORY_INL
#define DISPLAY2_PCD3D11_MEMORY_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

INLINE U32 C_MemoryBuffer::GetSize() const
{
	return 0;
}

INLINE U32 C_MemoryBuffer::GetAlign() const
{
	return 0;
}

INLINE U32 C_MemoryBuffer::GetFlags() const
{
	return 0;
}

INLINE U8* C_MemoryBuffer::GetAddress() const
{
	return NULL;
}

INLINE E_MemoryBuffer C_MemoryBuffer::GetType() const
{
	return E_MemoryBuffer_Unknown;
}

} // namespace Display2

#endif // DISPLAY2_PCD3D11_MEMORY_INL
