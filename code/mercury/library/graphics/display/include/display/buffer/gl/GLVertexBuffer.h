#ifndef DISPLAY_GLVERTEXBUFFER_H
#define DISPLAY_GLVERTEXBUFFER_H

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

class C_VertexBuffer : public C_Resource
{
public:

	C_VertexBuffer();
	~C_VertexBuffer();

	const bool IsCreated() const;

	const U32 GetSize();
	U8* GetAddress();
	const U32 GetFlags();
	const S32 GetSystemID();

	void SetSize(const U32 size);
	void SetAddress(U8* pAddress);
	void SetFlags(const U32 flags);
	void SetSystemID(const S32 systemID);

	void* Lock(const U32 offset, const U32 size);
	void* Lock(const U32 size);
	void* Lock();
    void Unlock();

public:

	U32		m_Stride;
	GLuint	m_Name;
};

#include "GLVertexBuffer.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLVERTEXBUFFER_H
