// [Blur1PCDX11] Native PC memory allocation path.
#include "StdAfx.h"
#include "Display2/Memory/PCD3D11Memory.h"

namespace Display2
{

// C_MemoryBuffer

C_MemoryBuffer::C_MemoryBuffer() 
{
}

bool C_MemoryBuffer::Create(const U32 size, const U32 align, const U32 flags, const E_MemoryBuffer type, U8* pAddress)
{
	return true;
}

void C_MemoryBuffer::Destroy()
{
}

bool C_MemoryBuffer::Inside(const U32 size, const U8* pAddress) const
{
	return true;
}

} // namespace Display2
