#ifndef DISPLAY_D3D10PIXELSHADER_H
#define DISPLAY_D3D10PIXELSHADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/D3D10/D3D10.h"

namespace NewDisplay
{
namespace D3D10
{

class C_PixelShader
{
public:

	C_PixelShader();
	~C_PixelShader();

	const bool IsCreated() const;

	const U32 GetSize();
	const U32 GetOffset();
	const Util::C_String GetFileName();
	const U64 GetFileTime();
	const S32 GetSystemID();

	void SetSize(const U32 size);
	void SetOffset(const U32 offset);
	void SetFileName(const char* fileName);
	void SetFileTime(const U64 fileTime);
	void SetSystemID(const S32 systemID);

public:

	ID3D10PixelShader* m_pd3dPixelShader;

private:

	struct C_Header
	{
		C_Header() : m_Size(0), m_FileName(""), m_FileTime(0), m_SystemID(-1)
		{
		}

		U32				m_Size;
		Util::C_String	m_FileName;
		U64				m_FileTime;
		S32				m_SystemID;
	};

#ifdef DEVICE_DEBUG
	C_Header* m_pHeader;

	INLINE C_Header* GetHeader()
	{
		return m_pHeader ? m_pHeader : m_pHeader = new C_Header;
	}
#endif
};

#include "D3D10PixelShader.inl"

}; // namespace D3D10
}; // namespace NewDisplay

#endif // DISPLAY_D3D10PIXELSHADER_H
