#ifndef DISPLAY_D3D10INDEXBUFFER_H
#define DISPLAY_D3D10INDEXBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/D3D10/D3D10.h"

namespace NewDisplay
{
namespace D3D10
{

class C_IndexBuffer
{
public:

	C_IndexBuffer();
	~C_IndexBuffer();

	const bool IsCreated() const;

	const U32 GetSize();
	const U32 GetOffset();
	const U32 GetFlags();
	const S32 GetSystemID();
	const E_Format GetFormat();
	
	void SetSize(const U32 size);
	void SetOffset(const U32 offset);
	void SetFlags(const U32 flags);
	void SetSystemID(const S32 systemID);
	void SetFormat(const E_Format format);

	void* Lock(const U32 offset, const U32 size);
	void* Lock(const U32 size);
	void* Lock();
	void Unlock();

public:

	ID3D10Buffer*	m_pd3dBuffer;
	E_Format		m_Format;

private:

#ifdef DEVICE_DEBUG
	bool			m_Mapped;
#endif

	struct C_Header
	{
		C_Header() : m_Flags(0), m_SystemID(-1)
		{
		}

		U32 m_Flags;
		S32 m_SystemID;
	};

#ifdef DEVICE_DEBUG
	C_Header* m_pHeader;

	INLINE C_Header* GetHeader()
	{
		return m_pHeader ? m_pHeader : m_pHeader = new C_Header;
	}
#endif
};

#include "D3D10IndexBuffer.inl"

}; // namespace D3D10
}; // namespace NewDisplay

#endif // DISPLAY_D3D10INDEXBUFFER_H
