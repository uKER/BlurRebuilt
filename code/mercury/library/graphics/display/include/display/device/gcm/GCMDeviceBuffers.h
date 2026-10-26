#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_BUFFERS_H
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_BUFFERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Surface/GCM/GCMSurface.h"
#include "Display/Texture/GCM/GCMTexture2D.h"

namespace Display
{
namespace GCM
{

class C_GCMDevice;

//! C_BackBuffer

class C_BackBuffer
{
friend class C_GCMDevice;
public:

	C_BackBuffer();
	~C_BackBuffer();

	const U32 GetWidth() const;
	const U32 GetHeight() const;
	const E_Multisample GetMultisample() const;
	
	const E_Format GetColourFormat() const;
	const C_Surface* GetColourSurface() const;
	const C_Texture2D* GetColourTexture() const;
	C_Surface* GetColourSurface();
	C_Texture2D* GetColourTexture();
	
	const E_Format GetDepthStencilFormat() const;
	const C_Surface* GetDepthStencilSurface() const;
	const C_Texture2D* GetDepthStencilTexture() const;
	C_Surface* GetDepthStencilSurface();
	C_Texture2D* GetDepthStencilTexture();

	void SetWidth(const U32 width);
	void SetHeight(const U32 height);
	void SetMultisample(const E_Multisample multisample);
	void SetColourFormat(const E_Format format);
	void SetDepthStencilFormat(const E_Format format);

private:

	Display2::C_RenderBuffer		m_RenderBuffer;
};

//! C_FrontBuffer

class C_FrontBuffer
{
friend class C_GCMDevice;
public:

	C_FrontBuffer();
	~C_FrontBuffer();

	const U32 GetWidth() const;
	const U32 GetHeight() const;
	const E_Format GetFormat() const;
	const C_Surface* GetSurface() const;
	const C_Texture2D* GetTexture() const;
	C_Surface* GetSurface();
	C_Texture2D* GetTexture();

	void SetWidth(const U32 width);
	void SetHeight(const U32 height);
	void SetFormat(const E_Format format);

private:

	Display2::C_RenderBuffer		m_RenderBuffer;
};

//! C_RenderTarget

class C_RenderTarget
{
friend class C_GCMDevice;
public:

	C_RenderTarget();
	~C_RenderTarget();

	const U32 GetWidth() const;
	const U32 GetHeight() const;
	const C_Surface* GetColourSurface() const;
	const C_Surface* GetDepthStencilSurface() const;

private:

	const C_Surface*				m_pColourSurface;
	const C_Surface*				m_pDepthStencilSurface;
};

}; // namespace GCM
}; // namespace Display

#include "Display/Device/GCM/GCMDeviceBuffers.inl"

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_BUFFERS_H
