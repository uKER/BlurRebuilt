// [Blur1PCDX11] Native D3D11 game facade declarations.
#ifndef DISPLAY_WINDOW_D3D11_D3D11_WINDOW_H
#define DISPLAY_WINDOW_D3D11_D3D11_WINDOW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/D3D11/D3D11Resources.h"
#include "Display/Device/D3D11/D3D11Resources.h"
#include "Display/Window/WindowBase.h"
#ifdef PLATFORM_PC
#include "Display/Device/D3D11/D3D11Resources.h"
#endif

namespace Display
{
namespace D3D11
{

//! C_Window

#ifdef PLATFORM_PC
// [Blur2PC] Retail Blur PC's display window (Blur.exe 1.2, 0xf0 bytes, constructor 0x005ac0f0): the scene of a
// splitscreen view is drawn into a multisampled colour target and depth-stencil surface, then copied to the
// back buffer (C_PCD3D11GameDevice::RestoreWindow). Offsets are retail's; member names after 0x18 are ours.
class C_Window
{
public:

	C_Window(const Maths::C_Rect& coords);
	~C_Window();

	E_Multisample GetMultisample() const;

	C_Surface* GetColourSurface(const U32 i = 0);
	C_Surface* GetDepthStencilSurface();

	C_Texture2D* GetColourTexture(const U32 i = 0);
	const C_Texture2D* GetDepthStencilTexture();
	C_Texture2D* GetMaskTexture();

	// retail 0x005ac150
	void Resize(const U32 width, const U32 height, const E_Multisample multisample, const U32 screenWidth, const U32 screenHeight, const E_Format colourFormat, const E_Format depthStencilFormat);

public:

	Maths::C_Rect				m_Coords;				// +0x00

	U32							m_Width;				// +0x10 window buffer size of this window
	U32							m_Height;				// +0x14
	E_Multisample				m_Multisample;			// +0x18
	U32							m_ScreenWidth;			// +0x1c the window's size on screen
	U32							m_ScreenHeight;			// +0x20

	C_TileSetup					m_TileSetup;			// +0x24 never set on PC

	E_Format					m_ColourFormat;			// +0x30
	E_Format					m_DepthStencilFormat;	// +0x34
	C_RenderTexture2D			m_ColourRenderTexture;	// +0x38 multisampled scene colour
	C_Surface					m_DepthStencilSurface;	// +0x8c multisampled depth-stencil
	C_RenderTexture2D			m_MaskRenderTexture;	// +0x94 A8R8G8B8 stencil mask (C_PCD3D11GameDevice::RenderWindowMask)
	const C_Texture2D*			m_pDepthTexture;		// +0xe8 set every frame by the game (C_GameWorld::BeginFrame)
	bool						m_ClearDepthStencil;	// +0xec SetWindow clears depth and stencil too
	bool						m_MaskEnable;			// +0xed RenderWindowMask draws the mask (graphical detail > 1)
};
#else
class C_Window
{
public:

	C_Window(const Maths::C_Rect& coords);
	~C_Window();

	E_Multisample GetMultisample() const;

	C_Surface* GetColourSurface(const U32 i = 0);
	C_Surface* GetDepthStencilSurface();
	
	C_Texture2D* GetColourTexture(const U32 i = 0);
	C_Texture2D* GetDepthStencilTexture();

	void Resize(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat, const E_Format depthStencilFormat);

public:

	Maths::C_Rect				m_Coords;
	
	U32							m_Width;
	U32							m_Height;
	E_Multisample				m_Multisample;

	C_TileSetup					m_TileSetup;

	E_Format					m_ColourFormat;
	C_Surface					m_ColourSurfaceWithAA;
	C_Surface					m_ColourSurfaceWithoutAA;
	C_Texture2D					m_ColourTexture;

	E_Format					m_DepthStencilFormat;
	C_Surface					m_DepthStencilSurfaceWithAA;
	C_Surface					m_DepthStencilSurfaceWithoutAA;
	C_Texture2D					m_DepthStencilTexture;
};
#endif // PLATFORM_PC

}; // namespace D3D11
}; // namespace Display

#include "Display/Window/D3D11/D3D11Window.inl"

#endif // DISPLAY_WINDOW_D3D11_D3D11_WINDOW_H
