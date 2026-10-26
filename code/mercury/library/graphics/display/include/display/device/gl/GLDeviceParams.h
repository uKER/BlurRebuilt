#ifndef DISPLAY_GLDEVICEPARAMS_H
#define DISPLAY_GLDEVICEPARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"

namespace NewDisplay
{
namespace GL
{

class C_DeviceParams
{
public:

	C_DeviceParams();
	~C_DeviceParams();

public:

	bool				m_VSync;
	U32					m_Width;
	U32					m_Height;
	E_Multisample		m_Multisample;
	E_Format			m_ColourFormat;
	E_Format			m_DepthStencilFormat;
#ifdef PLATFORM_PS3
	U32		m_MaxSPUs;
	U32		m_HostMemorySize;
#endif
#ifdef PLATFORM_PC
	bool				m_Windowed;
	S8					m_CmdShow;
	LPCTSTR				m_pName;
	HDC					m_hDC;
	HWND				m_hWnd;
	HINSTANCE			m_hInstance;
#endif
};

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLDEVICEPARAMS_H
