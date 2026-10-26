#include "stdafx.h"

#include <Render/State/RenderState.h>
#include <Render/Shaders/Constants.h>

#include <Render/PostProcess/HDR/HDRInterface.h>
#include <Render/Render/RenderSettings.h>

#include <Display/Context/Context.h>

namespace Render
{

C_RenderState::C_RenderState() :
	m_fViewDistance(500.0f)	
{
	SetFadeIn(0.0f, 0.1f, MathsStorage::Float4());
	SetGain(1.f);
}

void C_RenderState::Update()
{
	m_Fog.Update();
}

::Debug::Bool g_debugDepthBufferShowBands("Graphics.Render.DepthBuffer.ShowBands", true);
::Debug::Float g_debugDepthBufferDistance("Graphics.Render.DepthBuffer.Distance", 10.0f, ::Debug::Range(0.0f, 1000.0f));

MathsStorage::Float4 C_RenderState::GetDepthScaleShaderParameter() const
{
	return GetDepthScaleShaderParameter(Display::Device()->GetView()->GetViewSettings3D().GetNearClip(), Display::Device()->GetView()->GetViewSettings3D().GetFarClip());
}

MathsStorage::Float4 C_RenderState::GetDepthScaleShaderParameter(const F32 nearClip, const F32 farClip) const
{
	// use in conjunction with psCommon.h shader function SampleDepthBuffer

	MathsStorage::Float4 parameter;

	F32 zNear;
	F32 zFar;

	if(Display::Device()->GetDepthFlip())
	{
		zNear = farClip;
		zFar = nearClip;
	}
	else
	{
		zNear = nearClip;
		zFar = farClip;
	}

	parameter.x = zFar / (zFar - zNear);
	parameter.y = zFar * zNear / (zNear - zFar);
	
	parameter.z = g_debugDepthBufferDistance;
	parameter.w = g_debugDepthBufferShowBands ? 1.0f : 0.0f;

	return parameter;
}

MathsStorage::Float4 C_RenderState::GetBlendShaderParamater() const
{
	Display::E_BlendMode eBlendMode = Display::Device()->GetBlendMode();
	MathsStorage::Float4 settings;

	switch(eBlendMode)
	{
		case Display::E_BlendMode_Additive:
		case Display::E_BlendMode_AdditiveSmooth:
		case Display::E_BlendMode_Subtractive:
			settings = MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f);
			break;
		case Display::E_BlendMode_AlphaPreMultiply:
			settings = MathsStorage::Float4(1.0f, 1.0f, 0.0f, 0.0f);
			break;
		case Display::E_BlendMode_Multiply:
			settings = MathsStorage::Float4(1.0f, 0.0f, 1.0f, 0.0f);
			break;
		case Display::E_BlendMode_AlphaAdditive:
		case Display::E_BlendMode_AlphaSubtractive:
			settings = MathsStorage::Float4(1.0f, 0.0f, 0.0f, 1.0f);
			break;
		case Display::E_BlendMode_SolidAlphaMask:
		case Display::E_BlendMode_Alpha:
		case Display::E_BlendMode_Solid:
		case Display::E_BlendMode_SolidAlphaTest:
			settings = MathsStorage::Float4(1.0f, 0.0f, 0.0f, 0.0f);
			break;
	}

	return settings;
}


bool C_RenderState::ShaderHDR() const
{
	return m_pHDR && m_pHDR->GetType() != PostProcess::E_HighDynamicRangeType_PostProcess && Render::C_RenderSettings::HDREnabled();
}

void C_RenderState::SetFadeIn(const F32 fFadeInBegin, const F32 fFadeInEnd) const
{
	Display::C_Float4 vViewPosition = Display::Device()->GetView()->GetViewSettings3D().GetViewPosition();

	SetFadeIn(fFadeInBegin, fFadeInEnd, vViewPosition);
}

void C_RenderState::SetFadeIn(const F32 fFadeInBegin, const F32 fFadeInEnd, const MathsStorage::Float4& vFadeInPosition) const
{
	if(fFadeInBegin != m_vFadeInSettings.z || fFadeInEnd != m_vFadeInSettings.w)
	{
		if(fFadeInBegin >= 0.0f)
		{
			if(fFadeInEnd > fFadeInBegin)
			{
				m_vFadeInSettings.x = 1.0f / (fFadeInEnd - fFadeInBegin);
				m_vFadeInSettings.y = -fFadeInBegin / (fFadeInEnd - fFadeInBegin);
				m_vFadeInSettings.z = fFadeInBegin;
				m_vFadeInSettings.w = fFadeInEnd;
				
				m_vFadeInPosition = vFadeInPosition;
			}
			else
				LOGWARN(g_Log, "RenderState: Invalid FadeInEnd distance. Must be greater that FadeInBegin!\n");
		}
		else
			LOGWARN(g_Log, "RenderState: Invalid FadeInBegin distance. Must be greater or equal to zero!\n");
	}
}

void C_RenderState::SetFadeInShaderParameters(Display::C_Context* pContext) const
{
	if(pContext)
		pContext->SetVertexShaderConstantF(CV_RENDER_FADEIN_SETTINGS, (F32*)&m_vFadeInSettings, 2);
	else
		Display::Device()->SetVertexShaderConstantF(CV_RENDER_FADEIN_SETTINGS, (F32*)&m_vFadeInSettings, 2);
}

void C_RenderState::SetGain(const F32 fGain) const
{
	m_fGain = fGain;
}

MathsStorage::Float4 C_RenderState::GetGainShaderParameters() const
{
	return MathsStorage::Float4(m_fGain, 1.f / m_fGain, 0.0f, 0.0f);
}

void C_RenderState::SetGainShaderParameters(Display::C_Context* pContext) const
{
	const Display::C_Float4 psGain = GetGainShaderParameters();
	if(pContext)
		pContext->SetVertexShaderConstantF(CP_RENDER_POSTPROCESS_LUMINANCE, (F32*)&psGain, 1);
	else
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_LUMINANCE, (F32*)&psGain, 1);
}

void C_RenderState::SetViewDistance(const F32 fViewDistance)
{
	m_fViewDistance = fViewDistance;
}

};
