#ifndef DISPLAY_GLDEVICEBUFFERS_H
#define DISPLAY_GLDEVICEBUFFERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Surface/GL/GLSurface.h"
#include "Display/Texture/GL/GLTexture2D.h"

namespace NewDisplay
{
namespace GL
{

class C_GLDevice;
class C_PCGLDevice;

class C_BackBuffer
{
friend class C_GLDevice;
friend class C_PCGLDevice;
public:

	C_BackBuffer();
	~C_BackBuffer();

	const U32 GetWidth() const;
	const U32 GetHeight() const;
	const E_Multisample GetMultisample() const;
	const E_Format GetColourFormat() const;
	const E_Format GetDepthStencilFormat() const;

private:

	U32					m_Width;
	U32					m_Height;
	E_Multisample		m_Multisample;
	E_Format			m_ColourFormat;
	C_Surface			m_ColourSurface;
	E_Format			m_DepthStencilFormat;
	C_Surface			m_DepthStencilSurface;
};

class C_FrontBuffer
{
friend class C_GLDevice;
friend class C_PCGLDevice;
public:

	C_FrontBuffer();
	~C_FrontBuffer();

	const C_Texture2D* GetTexture() const;
	const U32 GetWidth() const;
	const U32 GetHeight() const;
	const E_Format GetFormat() const;
	const E_Multisample GetMultisample() const;

private:

	void SwapTexture();

private:

	U32					m_Texture;
	U32					m_Width;
	U32					m_Height;
	E_Format			m_Format;
	E_Multisample		m_Multisample;
	C_Texture2D			m_Textures[2];
};

class C_RenderTarget
{
friend class C_GLDevice;
friend class C_PCGLDevice;
public:

	C_RenderTarget();
	~C_RenderTarget();

	const C_Surface* GetColourSurface();
	const C_Surface* GetDepthStencilSurface();

private:

	const C_Surface*	m_pColourSurface;
	const C_Surface*	m_pDepthStencilSurface;
};

#include "GLDeviceBuffers.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLDEVICEBUFFERS_H
