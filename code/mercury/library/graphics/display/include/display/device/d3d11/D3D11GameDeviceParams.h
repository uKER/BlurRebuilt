// [Blur1PCDX11] Native D3D11 game facade declarations.
#ifndef DISPLAY_DEVICE_D3D11_GAME_DEVICE_PARAMS_H
#define DISPLAY_DEVICE_D3D11_GAME_DEVICE_PARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"

namespace Display
{
namespace D3D11
{

class C_DeviceParams
{
public:

	C_DeviceParams();
	~C_DeviceParams();

	void Setup(const U32 fbWidth, const U32 fbHeight, const U32 bbWidth, const U32 bbHeight, const E_Multisample bbMultisample, const U32 wbWidth, const U32 wbHeight, const E_Multisample wbMultisample);

public:

	ID3D11Device*	m_pd3dDevice;
	IDXGIFactory1*			m_pd3dObject;


	#ifdef PLATFORM_PC
	U32					m_Width;
	U32					m_Height;
	E_Multisample		m_Multisample;
	E_Format			m_ColourFormat;
	E_Format			m_DepthStencilFormat;
	U32					m_RefreshRate;		// [Blur2PC] retail PC: the chosen mode's refresh rate (0 = default)
	bool				m_Windowed;
	S8					m_CmdShow;
	LPCTSTR				m_pName;
	HDC					m_hDC;
	HWND				m_hWnd;
	HINSTANCE			m_hInstance;
	bool				m_UseREF;
	U32					m_BeginVerticesSize;
	U32					m_FreeBeginVerticesSize;
	E_PresentInterval	m_PresentInterval;
	// [Blur2PC] retail Blur PC (Blur.exe 1.2, constructor 0x005b69f0) adds the console window-buffer block
	// after the PC fields (+0x44..+0x5c): the size, formats and multisample of the display windows
	U32					m_WindowBufferWidth;
	U32					m_WindowBufferHeight;
	E_Format			m_WindowBufferColourFormat;
	E_Format			m_WindowBufferDepthStencilFormat;
	E_Multisample		m_WindowBufferMultisample;
	U32					m_ScreenWidth;					// Width/Height of the actual display setting
	U32					m_ScreenHeight;
	#endif
};

}; // namespace D3D11
}; // namespace Display

#endif // DISPLAY_DEVICE_D3D11_GAME_DEVICE_PARAMS_H
