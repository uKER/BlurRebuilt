#ifndef DISPLAY_GLVERTEXSHADER_H
#define DISPLAY_GLVERTEXSHADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Resource/Resource.h"
#include "Display/Device/GL/GL.h"

namespace NewDisplay
{
namespace GL
{

class C_VertexShader : public C_Resource
{
public:

	C_VertexShader();
	~C_VertexShader();

	const bool IsCreated() const;

	const U32 GetSize();
	U8* GetAddress();
	const Util::C_String GetFileName();
	const U64 GetFileTime();
	const S32 GetSystemID();

	void SetSize(const U32 size);
	void SetAddress(U8* pAddress);
	void SetFileName(const char* fileName);
	void SetFileTime(const U64 fileTime);
	void SetSystemID(const S32 systemID);

public:

	CGprogram m_cgProgram;

private:

	Util::C_String	m_FileName;
	U64				m_FileTime;
};

#include "GLVertexShader.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLVERTEXSHADER_H
