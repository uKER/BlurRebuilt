#ifndef DISPLAY_GLINDEXBUFFER_H
#define DISPLAY_GLINDEXBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Resource/Resource.h"
#include "Display/Device/GL/GL.h"
#include "Display/Resource/Resource.h"

namespace NewDisplay
{
namespace GL
{

class C_IndexBuffer : public C_Resource
{
public:

	C_IndexBuffer();
	~C_IndexBuffer();

	const bool IsCreated() const;

	const U32 GetSize();
	U8* GetAddress();
	const U32 GetFlags();
	const E_Format GetFormat();
	const S32 GetSystemID();

	void SetSize(const U32 size);
	void SetAddress(U8* pAddress);
	void SetFlags(const U32 flags);
	void SetFormat(const E_Format format);
	void SetSystemID(const S32 systemID);

	void* Lock(const U32 offset, const U32 size);
	void* Lock(const U32 size);
	void* Lock();
    void Unlock();

public:

	E_Format	m_Format;
	GLuint		m_Name;
};

#include "GLIndexBuffer.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLINDEXBUFFER_H
