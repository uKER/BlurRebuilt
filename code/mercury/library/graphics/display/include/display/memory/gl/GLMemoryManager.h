#ifndef DISPLAY_GLMEMORYMANAGER_H
#define DISPLAY_GLMEMORYMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Memory/MemoryManagerInterface.h"

namespace NewDisplay
{
namespace GL
{

class C_GLMemoryBuffer : public I_MemoryBuffer
{
public:

	C_GLMemoryBuffer();
	~C_GLMemoryBuffer();

	const bool IsCreated() const;

	const U32 GetSize() const;
	const U32 GetAlign() const;
	const U32 GetFlags() const;
	U8* GetAddress() const;

protected:

	bool CreateAddress();
	void DestroyAddress();

protected:

	U32		m_Size;
	U32		m_Align;
	U32		m_Flags;
};

}; // namespace GL
}; // namespace NewDisplay

#include "GLMemoryManager.inl"

#endif // DISPLAY_GLMEMORYMANAGER_H
