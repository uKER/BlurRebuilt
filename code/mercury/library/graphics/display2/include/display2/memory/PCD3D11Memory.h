// [Blur1PCDX11] Native PC memory allocation path.
#ifndef DISPLAY2_PCD3D11_MEMORY_H
#define DISPLAY2_PCD3D11_MEMORY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

enum E_MemoryBuffer
{
	E_MemoryBuffer_Unknown,
};

class C_MemoryBuffer
{
public:

	C_MemoryBuffer();
	
	U32 GetSize() const;
	U32 GetAlign() const;
	U32 GetFlags() const;
	U8* GetAddress() const;
	E_MemoryBuffer GetType() const;
	
	bool Create(const U32 size, const U32 align, const U32 flags, const E_MemoryBuffer type, U8* pAddress = NULL);
	void Destroy();

	bool Inside(const U32 size, const U8* pAddress) const;
};

} // namespace Display2

#include "Display2/Memory/PCD3D11Memory.inl"

#endif // DISPLAY2_PCD3D11_MEMORY_H
