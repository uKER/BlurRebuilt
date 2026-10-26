#ifndef DISPLAY_GLPIXELSHADER_H
#define DISPLAY_GLPIXELSHADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Resource/Resource.h"
#include "Display/Device/GL/GL.h"

namespace NewDisplay
{
namespace GL
{

class C_PixelShader : public C_Resource
{
public:

	C_PixelShader();
	~C_PixelShader();

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

	const bool IsCreated() const;

public:

	CGprogram m_cgProgram;

private:

	Util::C_String	m_FileName;
	U64				m_FileTime;
};

#include "GLPixelShader.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLPIXELSHADER_H
