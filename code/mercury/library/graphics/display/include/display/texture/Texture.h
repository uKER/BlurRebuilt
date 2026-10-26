#ifndef DISPLAY_TEXTURE_TEXTURE_H
#define DISPLAY_TEXTURE_TEXTURE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"

#if defined(DEVICE_PCD3D11)
#include "Display/Device/D3D11/D3D11Resources.h" // [Blur1PCDX11] Native resources.
#elif defined(DEVICE_PS3GCM)
#include "Display/Texture/GCM/GCMTexture.h"
#elif defined(DEVICE_PCD3D10)
#include "Display/Texture/D3D10/D3D10Texture.h"
#elif defined(DEVICE_PCGL) || defined(DEVICE_PS3GL)
#include "Display/Texture/GL/GLTexture.h"
#else
#pragma message("Unsupported device define when compiling Texture.h")
#endif

namespace Display
{

//! C_RefTexture

struct C_RefTexture : public C_Texture
{
	C_RefTexture();

	S32 RefCount() const;
	S32 Acquire();
	S32 Release();

	const Util::C_String& GetFileName() const;
	void SetFileName(const Util::C_String& fileName);

	S32					m_RefCount;
	Util::C_String		m_FileName;
};

//! C_RefTexture2D

struct C_RefTexture2D : public C_RefTexture
{
};

//! C_RefTexture3D

struct C_RefTexture3D : public C_RefTexture
{
};

//! C_RefTextureCube

struct C_RefTextureCube : public C_RefTexture
{
};

}; // namespace Display

#include "Display/Texture/Texture.inl"

#endif // DISPLAY_TEXTURE_TEXTURE_H
