#ifndef DISPLAY_MEMORY_GCM_GCM_MEMORY_MANAGER_H
#define DISPLAY_MEMORY_GCM_GCM_MEMORY_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Memory/MemoryManagerInterface.h"

namespace Display
{
namespace GCM
{

//! E_GCMMemoryBufferType

enum E_GCMMemoryBufferType
{
	E_GCMMemoryBufferType_Main,
	E_GCMMemoryBufferType_Local,
	E_GCMMemoryBufferType_CmdBuf,
};

//! C_GCMMemoryBuffer

class C_GCMMemoryBuffer : public I_MemoryBuffer
{
public:

	C_GCMMemoryBuffer();
	~C_GCMMemoryBuffer();

	const bool IsCreated() const;

	const U32 GetSize() const;
	const U32 GetAlign() const;
	const U32 GetFlags() const;
	U8* GetAddress() const;

	void SetType(const E_GCMMemoryBufferType type);
	void SetOffset(const U32 offset);
	void SetAddress(U8* pAddress);

protected:

	bool CreateAddress();
	void DestroyAddress();

protected:

	U32						m_Size;
	U32						m_Align;
	U32						m_Flags;
	U32						m_Offset;
	bool					m_Created;
	U8*						m_pAddress;
	E_GCMMemoryBufferType	m_Type;
};

}; // namespace GCM
}; // namespace Display

#include "Display/Memory/GCM/GCMMemoryManager.inl"

#endif // DISPLAY_MEMORY_GCM_GCM_MEMORY_MANAGER_H
