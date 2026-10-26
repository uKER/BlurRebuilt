#ifndef DISPLAY_WINDOW_GCM_GCM_WINDOW_H
#define DISPLAY_WINDOW_GCM_GCM_WINDOW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Surface/GCM/GCMSurface.h"
#include "Display/Texture/GCM/GCMTexture2D.h"
#include "Display/Window/WindowBase.h"

namespace Display
{
namespace GCM
{

//! C_Window

class C_Window
{
public:

	C_Window(const Maths::C_Rect& coords);
	~C_Window();

	void OverlapBuffers(C_Surface* pColourSurface = NULL, C_Surface* pDepthStencilSurface = NULL, C_Texture* pColourTexture = NULL, C_Texture* pDepthStencilTexture = NULL);

	U32 GetWidth() const;
	U32 GetHeight() const;
	E_Multisample GetMultisample() const;
	E_Format GetColourFormat() const;
	E_Format GetDepthStencilFormat() const;

	C_Surface* GetColourSurface(const U32 i = 0);
	C_Surface* GetDepthStencilSurface();
	
	C_Texture2D* GetColourTexture(const U32 i = 0);
	C_Texture2D* GetDepthStencilTexture();

	void Resize(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat, const E_Format depthStencilFormat);

public:

	Maths::C_Rect				m_Coords;

	C_Surface*					m_pColourSurface;
	C_Surface*					m_pDepthStencilSurface;

	C_Texture*					m_pColourTexture;
	C_Texture*					m_pDepthStencilTexture;

	Display2::C_RenderBuffer	m_RenderBuffer;
};

}; // namespace GCM
}; // namespace Display

#include "Display/Window/GCM/GCMWindow.inl"

#endif // DISPLAY_WINDOW_GCM_GCM_WINDOW_H
