#include "stdafx.h"

#include <Render/State/Fog.h>

namespace Render
{

::Debug::Colour		g_FogColour("Graphics.Render.RenderState.Fog.Colour", 0.5f, 0.5f, 0.5f, 1.0f, ::Debug::Gamma(2.2f));

::Debug::Bool		g_FogDepthEnable("Graphics.Render.RenderState.Depth.Enable", true);
::Debug::Float		g_FogDepthDensity("Graphics.Render.RenderState.Depth.Density", 0.001f, ::Debug::Range(0.f, 1.0f), ::Debug::Step(0.001f));
::Debug::Float		g_FogDepthNearDistance("Graphics.Render.RenderState.Depth.NearDistance", 0.0f, ::Debug::Range(0.f, 200.0f), ::Debug::Step(1.0f));

::Debug::Bool		g_FogHeightEnable("Graphics.Render.RenderState.HeightFog.Enable", true);
::Debug::Float		g_FogHeight("Graphics.Render.RenderState.HeightFog.Height", 0.0f, ::Debug::Range(-100.f, 100.0f), ::Debug::Step(1.0f));
::Debug::Float		g_FogHeightDensity("Graphics.Render.RenderState.HeightFog.Density", 0.001f, ::Debug::Range(0.f, 1.0f), ::Debug::Step(0.001f));
::Debug::Float		g_FogHeightFallOff("Graphics.Render.RenderState.HeightFog.FallOff", 10.0f, ::Debug::Range(0.f, 100.0f), ::Debug::Step(1.0f));


C_Fog::C_Fog()
{
	m_Colour = Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 1.0f);
	
	m_eDepth = E_DepthFog_Disable;
	m_eDepthFalloff = E_FogFalloff_Exp;
	m_fDepthDensity = 0.0f;
	m_fDepthNearDistance = 0.0f;
	
	m_eHeight = E_HeightFog_Disable;
	m_eHeightFalloff = E_FogFalloff_Exp;
	m_fHeightDensity = 0.0f;
	m_fHeightPosition = 0.0f;
	m_fHeightPositionFalloff = 0.0f;
}

C_Fog::~C_Fog()
{
}

void C_Fog::Update()
{
	m_Colour = Maths::C_RGBA_F32(g_FogColour.Value().r,	g_FogColour.Value().g, g_FogColour.Value().b, g_FogColour.Value().a);
	
	m_eDepth = E_DepthFog_Euclidean;
	m_eDepthFalloff = E_FogFalloff_Exp;
	m_fDepthDensity = g_FogDepthDensity;
}

MathsStorage::Float4 C_Fog::GetFogColour() const
{
	MathsStorage::Float4 constant;
	Maths::Store(m_Colour, &constant);
	return constant;
}

MathsStorage::Float4 C_Fog::GetDepthFogShaderParameters() const
{
	MathsStorage::Float4 constant;
	constant.x = m_fDepthDensity;
	constant.y = m_fDepthNearDistance;
#ifndef CFG_SPEED
	constant.z = m_eDepth != E_DepthFog_Disable && g_FogDepthEnable ? 1.0f : 0.0f;
#else
	constant.z = m_eDepth != E_DepthFog_Disable ? 1.0f : 0.0f;
#endif
	return constant;
}

MathsStorage::Float4 C_Fog::GetHeightFogShaderParameters() const
{
	MathsStorage::Float4 constant;
	constant.x = m_fHeightPosition;
	constant.y = m_fHeightDensity;
	if(m_fHeightPositionFalloff > 0.0f)
		constant.z = 1.0f / m_fHeightPositionFalloff;
	else
		constant.z = 0.0f;
#ifndef CFG_SPEED
	constant.w = m_eHeight == E_HeightFog_Enable && g_FogHeightEnable ? 1.0f : 0.0f;
#else
	constant.w = m_eHeight == E_HeightFog_Enable ? 1.0f : 0.0f;
#endif
	return constant;
}

}



