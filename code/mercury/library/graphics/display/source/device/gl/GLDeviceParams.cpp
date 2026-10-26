#include "stdafx.h"
#include "Display/Device/GL/GLDeviceParams.h"

namespace NewDisplay
{
namespace GL
{

C_DeviceParams::C_DeviceParams()
{
	m_VSync					= true;
	m_Multisample			= E_Multisample_4;
	m_ColourFormat			= E_Format_A8R8G8B8;
	m_DepthStencilFormat	= E_Format_D24S8;
#ifdef PLATFORM_PC
	m_Width					= 640;
	m_Height				= 480;
#endif
#ifdef PLATFORM_PS3
	m_Width					= 1280;
	m_Height				= 720;
	m_MaxSPUs				= 1;
	m_HostMemorySize		= 0;
#endif
#ifdef PLATFORM_PC
	m_Windowed				= true;
	m_CmdShow				= 1;
	m_pName					= NULL;
	m_hDC					= NULL;
	m_hWnd					= NULL;
	m_hInstance				= NULL;
#endif
}

C_DeviceParams::~C_DeviceParams()
{
}

}; // namespace GL
}; // namespace NewDisplay
