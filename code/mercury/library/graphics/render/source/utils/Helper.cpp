#include "stdafx.h"
#include "Render/Utils/Helper.h"
#include "Render/State/RenderState.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Render
{

void EnablePostEffectStates(const bool disableMotionBlur)
{
	#if defined(PLATFORM_PC)
	/*if(disableMotionBlur)
	{
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	}
	else
	{
		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Red | Display::E_ColourWriteEnable_Green | Display::E_ColourWriteEnable_Blue);
	}*/

	// enable the stencil surface
	Display::Device()->SetStencilEnable( Display::E_Bool_True );

	// always pass the stencil cmp
	Display::Device()->SetStencilFunc( Display::E_CmpFunc_Always );

	// setup the stencil write values

	// write to all bits of the stencil buffer
	Display::Device()->SetStencilWriteMask( 0xff );

	// we should always pass so just configure what to write when we do
	Display::Device()->SetStencilPass(Display::E_StencilOp_Replace );
	Display::Device()->SetStencilDepthFail(Display::E_StencilOp_Keep );

	if(disableMotionBlur)
	{
		Display::Device()->SetStencilRef(0xff);
	}
	else
	{
		Display::Device()->SetStencilRef(0x00);
	}

	//Display::Device()->SetStencilPass(Display::E_StencilOp_Zero );	

	#endif

	#ifdef PLATFORM_PS3
	Display::C_StencilState stencilState;
	stencilState.m_StencilEnable		= Display::E_Bool_True;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
	stencilState.m_StencilRef			= disableMotionBlur ? 0xff : 0x00;
	stencilState.m_StencilMask			= 0xff;
	stencilState.m_StencilWriteMask		= 0xff;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail 	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass 	= Display::E_StencilOp_Replace;
	Display::Device()->SetStencilState(stencilState);
	#endif

	// if we are using hdr u32 we need to enable luminance alpha writes
	const Render::C_RenderState* pRenderState = &Render::C_RenderState::Instance();
	if(pRenderState->HDR() && pRenderState->HDR()->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
	{
		Display::Device()->SetLuminanceWrite(Display::E_LuminanceWrite_Enabled);
	}
}

void DisablePostEffectStates()
{
	#if defined(PLATFORM_PC)
	//Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
	#endif

	#ifdef PLATFORM_PS3
	Display::Device()->SetStencilEnable(Display::E_Bool_False);
	#endif
	
	const Render::C_RenderState* pRenderState = &Render::C_RenderState::Instance();
	if(pRenderState->HDR() && pRenderState->HDR()->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
	{
		Display::Device()->SetLuminanceWrite(Display::E_LuminanceWrite_Default);
	}
}

bool g_CurrentMotionBlurState = false;
Ut::Vector<bool> g_StoredMotionBlurStates;

void BeginPostEffectStates(const bool disableMotionBlur)
{
	g_StoredMotionBlurStates.PushBack(g_CurrentMotionBlurState);
	g_CurrentMotionBlurState = g_CurrentMotionBlurState;

	EnablePostEffectStates(disableMotionBlur);
}

void EndPostEffectStates()
{
	assert(g_StoredMotionBlurStates.Size() > 0);
	bool disableMotionBlur = g_StoredMotionBlurStates.Back();
	g_StoredMotionBlurStates.PopBack();

	if(g_StoredMotionBlurStates.Size() > 0)
	{
		EnablePostEffectStates(disableMotionBlur);
	}
	else
	{
		DisablePostEffectStates();
	}	
}

void GetZScaleParameters( MathsStorage::Float4& fvec, const Display::C_ViewSettings3D& viewSettings3D )
{
	F32 zNear;
	F32 zFar;

	if(Display::Device()->GetDepthFlip())
	{
		zNear = viewSettings3D.GetFarClip();
		zFar = viewSettings3D.GetNearClip();
	}
	else
	{
		zNear = viewSettings3D.GetNearClip();
		zFar = viewSettings3D.GetFarClip();
	}

	fvec.x = zFar / (zFar - zNear);
	fvec.y = zFar * zNear / (zNear - zFar);
	fvec.z = 0.0f;
	fvec.w = 0.0f;
}

}; // namespace Render
