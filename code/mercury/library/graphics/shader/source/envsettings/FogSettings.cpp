#include "stdafx.h"
#include "shader/envsettings/FogSettings.h"

#include <Btl/Containers/Vector.h>
#include <Display.h>
#include <System/Debug/Variables.h>

#include <Fx/Fx.h>
#include <Fx/EnvironmentSettings.h>

XT_TYPE(EnvironmentSettings::DepthFogSettings, "DepthFog")
	XT_FIELD(Density,		"Density")
	XT_FIELD(NearDistance,	"NearDistance")
	XT_FIELD(Scale,			"Scale")
XT_END_TYPE

XT_TYPE(EnvironmentSettings::HeightFogSettings, "HeightFog")
	XT_FIELD(Height,		"Height")
	XT_FIELD(Density,		"Density")
	XT_FIELD(Falloff,		"Falloff")
	XT_FIELD(Scale,			"Scale")
XT_END_TYPE

XT_TYPE(EnvironmentSettings::FogSettings, "FogSettings")
	XT_FIELD(FogColour,		"FogColour")
	XT_FIELD(DepthFog,		"DepthFog")
	XT_FIELD(HeightFog,		"HeightFog")
XT_END_TYPE

namespace EnvironmentSettings
{

static FX::ParameterHandle g_FogSettings;

inline Maths::HVec4 Load(const XtRgba& v)
{
	return Maths::Load<Maths::HVec4>((const MathsStorage::Float4*)&v);
}

WorldFogSettings FogEnvSettings::m_FogSettings;

/// constructor
FogEnvSettings::FogEnvSettings(FogSettings *pData)
	: m_vFogColour(Load(pData->FogColour))
	, m_fDepthFogDensity(pData->DepthFog.Density)
	, m_fDepthFogScale(pData->DepthFog.Scale)
	, m_fDepthFogNearDistance(pData->DepthFog.NearDistance)
	, m_fHeightFogHeight(pData->HeightFog.Height)
	, m_fHeightFogDensity(pData->HeightFog.Density)
	, m_fHeightFogFalloff(pData->HeightFog.Falloff)
	, m_fHeightFogScale(pData->HeightFog.Scale)
	, m_vFogIntensities( Maths::HVec4(0,0,0,0))
{
}

/// construct by lerping two other FogEnvSettings
FogEnvSettings::FogEnvSettings(const FogEnvSettings *pA, const FogEnvSettings *pB, float f)
	: m_vFogColour			 (pA->m_vFogColour			 ,pB->m_vFogColour			 ,f)
	, m_fDepthFogDensity	 (pA->m_fDepthFogDensity	 ,pB->m_fDepthFogDensity	 ,f)
	, m_fDepthFogScale		 (pA->m_fDepthFogScale		 ,pB->m_fDepthFogScale		 ,f)
	, m_fDepthFogNearDistance(pA->m_fDepthFogNearDistance,pB->m_fDepthFogNearDistance,f)
	, m_fHeightFogHeight	 (pA->m_fHeightFogHeight	 ,pB->m_fHeightFogHeight	 ,f)
	, m_fHeightFogDensity	 (pA->m_fHeightFogDensity	 ,pB->m_fHeightFogDensity	 ,f)
	, m_fHeightFogFalloff	 (pA->m_fHeightFogFalloff	 ,pB->m_fHeightFogFalloff	 ,f)
	, m_fHeightFogScale	 	 (pA->m_fHeightFogScale	 	 ,pB->m_fHeightFogScale	 	 ,f)
	, m_vFogIntensities		 (pA->m_vFogIntensities		 ,pB->m_vFogIntensities		 ,f)
{
}

void FogEnvSettings::InitialiseParameters()
{
	g_FogSettings = FX::GetSharedParameter("g_FogSettings");
}

/// cause these environment settings to be applied to the scene
void FogEnvSettings::Set()
{
	m_FogSettings.m_vFogColour = m_vFogColour;
	m_FogSettings.m_vDepthFogSettings = Maths::HVec4((F32)m_fDepthFogDensity * (F32)m_fDepthFogScale, (F32)m_fDepthFogNearDistance, 0.0f, 0.0f);
	m_FogSettings.m_vHeightFogSettings = Maths::HVec4((F32)m_fHeightFogHeight, (F32)m_fHeightFogDensity * (F32)m_fHeightFogScale, (F32)m_fHeightFogFalloff, 0.0f);
	m_FogSettings.m_vFogIntensities = m_vFogIntensities;

	FX::SetParameter(g_FogSettings, &m_FogSettings.m_vFogColour);
}

/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
FX::I_EnvironmentSettings *FogEnvSettings::Lerp(const FX::I_EnvironmentSettings *p, float f) const
{
	assert(Name() == p->Name());
	return new FogEnvSettings(this, (FogEnvSettings*)p, f);
}

/// GUID for this settings block
Util::C_String FogEnvSettings::Name() const
{
	return "FogEnvSettings";
}

/// associate this block of environment settings with the given debug variable or debug variable page
void FogEnvSettings::AssociateDebugVariables(const Util::C_String &sName)
{
	m_vFogColour.AssociateDebugVariables(sName + ".FogColour");
	m_fDepthFogDensity.AssociateDebugVariables(sName + ".DepthFog.Density");
	m_fDepthFogNearDistance.AssociateDebugVariables(sName + ".DepthFog.NearDistance");
	m_fDepthFogScale.AssociateDebugVariables(sName + ".DepthFog.Scale");
	m_fHeightFogHeight.AssociateDebugVariables(sName + ".HeightFog.Height");
	m_fHeightFogDensity.AssociateDebugVariables(sName + ".HeightFog.Density");
	m_fHeightFogFalloff.AssociateDebugVariables(sName + ".HeightFog.Falloff");
	m_fHeightFogScale.AssociateDebugVariables(sName + ".HeightFog.Scale");
}

} // namespace


