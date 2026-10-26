#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_PARAMS_H
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_PARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"

namespace Display
{
namespace GCM
{

//! C_DeviceParams

class C_DeviceParams
{
public:

	C_DeviceParams();
	~C_DeviceParams();

	void Setup(const U32 fbWidth, const U32 fbHeight, const U32 bbWidth, const U32 bbHeight, const E_Multisample bbMultisample, const U32 wbWidth, const U32 wbHeight, const E_Multisample wbMultisample);

public:

	bool						m_HiDef;
	bool						m_WideScreen;
	bool						m_UniqueBackBuffer;
	U32							m_BackBufferWidth;
	U32							m_BackBufferHeight;
	E_Multisample				m_BackBufferMultisample;
	E_Format					m_BackBufferColourFormat;
	E_Format					m_BackBufferDepthStencilFormat;
	U32							m_FrontBufferWidth;
	U32							m_FrontBufferHeight;
	E_Format					m_FrontBufferFormat;
	U32							m_WindowBufferWidth;
	U32							m_WindowBufferHeight;
	E_Multisample				m_WindowBufferMultisample;
	E_Format					m_WindowBufferColourFormat;
	E_Format					m_WindowBufferDepthStencilFormat;
	U32							m_CmdBufMemorySize;
	U32							m_VideoMemorySize;
	U32							m_SystemMemorySize;
	U32							m_MainDeviceMemorySize;
	U32							m_LocalDeviceMemorySize;
	U32							m_BeginVerticesSize;
	U32							m_FreeBeginVerticesSize;
};

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_PARAMS_H
