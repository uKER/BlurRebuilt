#ifndef DISPLAY_GLSURFACE_H
#define DISPLAY_GLSURFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GL/GL.h"
#include "Display/Resource/Resource.h"

namespace NewDisplay
{
namespace GL
{

class C_SurfaceParams
{
public:

	C_SurfaceParams();
	~C_SurfaceParams();
};

class C_Surface : public C_Resource
{
public:

	C_Surface();
	~C_Surface();

	const bool IsCreated() const;

	const U32 GetWidth() const;
	const U32 GetHeight() const;
	const E_Format GetFormat() const;
	const U32 GetSize();
	U8* GetAddress();
	const U32 GetFlags();
	const S32 GetSystemID();

	void SetSize(const U32 size);
	void SetAddress(U8* pAddress);
	void SetFlags(const U32 flags);
	void SetSystemID(const S32 systemID);

public:

	GLuint m_glID;
	
private:
	
	struct C_Header
	{
		C_Header()
		{
		#if defined(PLATFORM_PS3) || defined(DEVICE_DEBUG)
			m_Size		= 0;
			m_Flags		= 0;
		#endif
		#if defined(PLATFORM_PS3)
			m_pAddress	= NULL;
		#endif
		#if defined(DEVICE_DEBUG)
			m_SystemID	= -1;
		#endif
		}
	#if defined(PLATFORM_PS3) || defined(DEVICE_DEBUG)
		U32 m_Size;
		U32 m_Flags;
	#endif
	#if defined(PLATFORM_PS3)
		U8* m_pAddress;
	#endif
	#if defined(DEVICE_DEBUG)
		S32 m_SystemID;
	#endif
	};

	C_Header* m_pHeader;

	INLINE C_Header* GetHeader()
	{
		return m_pHeader ? m_pHeader : m_pHeader = new C_Header;
	}
};

#include "GLSurface.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLSURFACE_H
