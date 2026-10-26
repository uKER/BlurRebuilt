#include "stdafx.h"
#include "Display/Device/GCM/GCMDeviceState.h"

namespace NewDisplay
{
namespace GCM
{

C_State::C_State()
{
	m_AlphaEnable						= false;
	m_AlphaFunc							= E_CmpFunc_Always;
	m_AlphaRef							= 0;
	m_BlendEnable						= false;
	m_BlendDst							= E_Blend_Zero;
	m_BlendMode							= E_BlendMode_Solid;
	m_BlendOp							= E_BlendOp_Add;
	m_BlendSrc							= E_Blend_One;
	m_ClearDepth						= 1.f;
	m_ClearStencil						= 0;
	m_ColourWriteEnable					= E_ColourWriteEnable_All;
	m_CullEnable						= false;
	m_CullMode							= E_CullMode_Ccw;
	m_DepthBias							= 0.f;
	m_FillMode							= E_FillMode_Solid;
	m_FlipZ								= false;
	m_SlopeScaledBias					= 0.f;
	m_ZEnable							= true;
	m_ZFunc								= E_CmpFunc_LessEqual;
	m_ZWriteEnable						= true;
}

C_State::~C_State()
{
}

}; // namespace GCM
}; // namespace NewDisplay
