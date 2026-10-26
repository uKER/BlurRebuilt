#ifndef DISPLAY_DEVICE_STENCIL_STATE_H
#define DISPLAY_DEVICE_STENCIL_STATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceStates.h"
#include "Display/Device/DeviceTypes.h"

namespace Display
{

//! C_StencilState

struct C_StencilState
{
	C_StencilState()
	{
		m_StencilEnable			= E_Bool_False;
		m_StencilFunc			= E_CmpFunc_Equal;
		m_StencilRef			= 0xff;
		m_StencilMask			= 0xff;
		m_StencilWriteMask		= 0xff;
		m_StencilFail			= E_StencilOp_Keep;
		m_StencilDepthFail		= E_StencilOp_Keep;
		m_StencilDepthPass		= E_StencilOp_Keep;
	}

	E_Bool			m_StencilEnable;
	E_CmpFunc		m_StencilFunc;
	U32				m_StencilRef;
	U32				m_StencilMask;
	U32				m_StencilWriteMask;
	E_StencilOp		m_StencilFail;
	E_StencilOp		m_StencilDepthFail;
	E_StencilOp		m_StencilDepthPass;
};

}; // namespace Display

#endif // DISPLAY_DEVICE_STENCIL_STATE_H
