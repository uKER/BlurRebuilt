#include "stdafx.h"
#include "Display/Device/GCM/GCMDeviceParams.h"

namespace Display
{
namespace GCM
{

C_DeviceParams::C_DeviceParams()
{
	m_HiDef								= true;
	m_WideScreen						= true;
	m_UniqueBackBuffer					= true;
	m_BackBufferWidth					= 1280;
	m_BackBufferHeight					= 720;
	m_BackBufferColourFormat			= E_Format_A8R8G8B8;
	m_BackBufferDepthStencilFormat		= E_Format_D24S8;
	m_BackBufferMultisample				= E_Multisample_None;
	m_FrontBufferWidth					= 1280;
	m_FrontBufferHeight					= 720;
	m_FrontBufferFormat					= E_Format_A8R8G8B8;
	m_WindowBufferWidth					= 1280;
	m_WindowBufferHeight				= 720;
	m_WindowBufferColourFormat			= E_Format_A8R8G8B8;
	m_WindowBufferDepthStencilFormat	= E_Format_D24S8;
	m_WindowBufferMultisample			= E_Multisample_4;
	m_CmdBufMemorySize					= 0;
	m_VideoMemorySize					= 0;
	m_SystemMemorySize					= 0;
	m_MainDeviceMemorySize				= 0;
	m_LocalDeviceMemorySize				= 0;
	m_BeginVerticesSize					= (1024*256)+(1024*1024*2);
	m_FreeBeginVerticesSize				= (1024*256);
}

C_DeviceParams::~C_DeviceParams()
{
}

void C_DeviceParams::Setup(const U32 fbWidth, const U32 fbHeight, const U32 bbWidth, const U32 bbHeight, const E_Multisample bbMultisample, const U32 wbWidth, const U32 wbHeight, const E_Multisample wbMultisample)
{
	m_FrontBufferWidth			= fbWidth;
	m_FrontBufferHeight			= fbHeight;

	m_BackBufferWidth			= bbWidth;
	m_BackBufferHeight			= bbHeight;
	m_BackBufferMultisample		= bbMultisample;

	m_WindowBufferWidth			= wbWidth;
	m_WindowBufferHeight		= wbHeight;
	m_WindowBufferMultisample	= wbMultisample;
}

}; // namespace GCM
}; // namespace Display
