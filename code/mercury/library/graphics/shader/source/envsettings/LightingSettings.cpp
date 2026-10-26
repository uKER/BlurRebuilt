#include "stdafx.h"
#include "shader/envsettings/LightingSettings.h"

#include <Btl/Containers/Vector.h>
#include <Display.h>
#include <System/Debug/Variables.h>

#include <Fx/Fx.h>
#include <Fx/EnvironmentSettings.h>

XT_TYPE(EnvironmentSettings::LightingInfo, "LightingInfo")
	XT_FIELD(SunColour,		"SunColour")
	XT_FIELD(SkyColour,		"SkyColour")
	XT_FIELD(BakedColour,	"BakedColour")
	XT_FIELD(EmissiveColour,"EmissiveColour")
XT_END_TYPE

namespace EnvironmentSettings
{

static FX::ParameterHandle g_LightingInfo;

Maths::HVec4 g_vIntensities(1.f,1.f,1.f,1.f);

inline Maths::HVec4 Load(const XtRgba& v)
{
	return Maths::Load<Maths::HVec4>((const MathsStorage::Float4*)&v);
}

WorldLightingSettings LightingEnvSettings::m_LightingSettings;

/// constructor
LightingEnvSettings::LightingEnvSettings(LightingInfo *pData)
	: m_vSunColour(Load(pData->SunColour))
	, m_vSkyColour(Load(pData->SkyColour))
	, m_vBakedColour(Load(pData->BakedColour))
	, m_vEmissiveColour(Load(pData->EmissiveColour))
	, m_vIntensities(g_vIntensities)
{
}

/// construct by lerping two other LightingEnvSettings
LightingEnvSettings::LightingEnvSettings(const LightingEnvSettings *pA, const LightingEnvSettings *pB, float f)
	: m_vSunColour(pA->m_vSunColour, pB->m_vSunColour, f)
	, m_vSkyColour(pA->m_vSkyColour, pB->m_vSkyColour, f)
	, m_vBakedColour(pA->m_vBakedColour, pB->m_vBakedColour, f)
	, m_vEmissiveColour(pA->m_vEmissiveColour, pB->m_vEmissiveColour, f)
	, m_vIntensities(pA->m_vIntensities, pB->m_vIntensities, f)
{
}

void LightingEnvSettings::InitialiseParameters()
{
	g_LightingInfo = FX::GetSharedParameter("g_LightingInfo");
}

/// cause these environment settings to be applied to the scene
void LightingEnvSettings::Set()
{
	m_LightingSettings.m_vSunColour = m_vSunColour;
	m_LightingSettings.m_vSkyColour = m_vSkyColour;
	m_LightingSettings.m_vBakedColour = m_vBakedColour;
	m_LightingSettings.m_vEmissiveColour = m_vEmissiveColour;
	m_LightingSettings.m_vIntensities = m_vIntensities;

	FX::SetParameter(g_LightingInfo, &m_LightingSettings.m_vSunColour);
}

/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
FX::I_EnvironmentSettings *LightingEnvSettings::Lerp(const FX::I_EnvironmentSettings *p, float f) const
{
	assert(Name() == p->Name());
	return new LightingEnvSettings(this, (LightingEnvSettings*)p, f);
}

/// GUID for this settings block
Util::C_String LightingEnvSettings::Name() const
{
	return "LightingEnvSettings";
}

/// associate this block of environment settings with the given debug variable or debug variable page
void LightingEnvSettings::AssociateDebugVariables(const Util::C_String &sName)
{
	m_vSunColour.AssociateDebugVariables(sName + ".SunColour");
	m_vSkyColour.AssociateDebugVariables(sName + ".SkyColour");
	m_vBakedColour.AssociateDebugVariables(sName + ".BakedColour");
	m_vEmissiveColour.AssociateDebugVariables(sName + ".EmissiveColour");
	m_vIntensities.AssociateDebugVariables(sName + ".Intensities");
}

}
