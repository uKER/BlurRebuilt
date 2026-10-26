#include "stdafx.h"
#include "Effect/Sky/SkyDomeEnvSettings.h"
#include "Effect/Sky/TexturedSkyDome.h"

#include <Btl/Containers/Vector.h>
#include <Display.h>
#include <System/Debug/Variables.h>

#include <Fx/Fx.h>
#include <Fx/EnvironmentSettings.h>

XT_TYPE(Effect::Sky::SkyDome, "SkySettings")
	XT_FIELD(textureName,	"texture")
	XT_FIELD(rotation,		"rotation")
	XT_FIELD(intensity,		"intensity")
	XT_FIELD(fogColour,		"fogColour")
	XT_FIELD(fogCurve,		"fogCurve")
	XT_FIELD(blackHeight,	"blackHeight")
	XT_FIELD(blackSize,		"blackSize")
XT_END_TYPE

namespace Effect
{

namespace Sky
{

extern Debug::Float g_debugTextureFogPower;
extern Debug::Float g_debugTextureBlackHeight;
extern Debug::Float g_debugTextureBlackSize;

inline Maths::HVec4 Load(const XtRgba& v)
{
	return Maths::Load<Maths::HVec4>((const MathsStorage::Float4*)&v);
}

/// constructor
SkyDomeEnvSettings::SkyDomeEnvSettings(SkyDome *pData, C_TexturedSkyDome *pTarget)
	: m_Rotation(pData->rotation)
	, m_Intensity(pData->intensity)
	, m_FogCurve(pData->fogCurve)
	, m_FogColour(Load(pData->fogColour))
	, m_BlackHeight(pData->blackHeight)
	, m_BlackSize(pData->blackSize)
	, m_Texture(pData->textureName)
	, m_pTarget(pTarget)
{
}

/// construct by lerping two other SkyDomeEnvSettings
SkyDomeEnvSettings::SkyDomeEnvSettings(const SkyDomeEnvSettings *pA, const SkyDomeEnvSettings *pB, float f)
	: m_Rotation(pA->m_Rotation, pB->m_Rotation, f)
	, m_Intensity(pA->m_Intensity, pB->m_Intensity, f)
	, m_FogColour(pA->m_FogColour, pB->m_FogColour, f)
	, m_FogCurve(pA->m_FogCurve, pB->m_FogCurve, f)
	, m_BlackHeight(pA->m_BlackHeight, pB->m_BlackHeight, f)
	, m_BlackSize(pA->m_BlackSize, pB->m_BlackSize, f)
	, m_Texture(pA->m_Texture, pB->m_Texture, f)
{
	if (f > 0)
		m_pTarget = pB->m_pTarget;
	else
		m_pTarget = pA->m_pTarget;
}

/// cause these environment settings to be applied to the scene
void SkyDomeEnvSettings::Set()
{
	m_pTarget->SetFogColour(m_FogColour);
	m_pTarget->SetIntensity(m_Intensity);
	m_pTarget->SetRotation(m_Rotation);
	g_debugTextureFogPower		= m_FogCurve;
	if (m_BlackHeight == 0 && m_BlackSize == 0)
	{
		g_debugTextureBlackHeight = 3.0f;
		g_debugTextureBlackSize = 1.0f;
	}
	else
	{
		g_debugTextureBlackHeight	= m_BlackHeight;
		g_debugTextureBlackSize		= m_BlackSize;
	}

	Util::C_String sTexture = m_Texture;
	if (sTexture != m_pTarget->GetCurrentTextureFilename())
	{
		m_pTarget->SetTexture(sTexture);
		m_pTarget->ReloadTexture();
	}
}

/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
FX::I_EnvironmentSettings *SkyDomeEnvSettings::Lerp(const FX::I_EnvironmentSettings *p, float f) const
{
	assert(Name() == p->Name());
	return new SkyDomeEnvSettings(this, (SkyDomeEnvSettings*)p, f);
}

/// GUID for this settings block
Util::C_String SkyDomeEnvSettings::Name() const
{
	return "SkyDomeEnvSettings";
}

/// associate this block of environment settings with the given debug variable or debug variable page
void SkyDomeEnvSettings::AssociateDebugVariables(const Util::C_String &sName)
{
	m_Rotation.AssociateDebugVariables(sName + ".Sky.Rotation");
	m_Intensity.AssociateDebugVariables(sName + ".Sky.Intensity");
	m_FogCurve.AssociateDebugVariables(sName + ".Sky.FogCurve");
	m_FogColour.AssociateDebugVariables(sName + ".Sky.FogColour");
	m_Texture.AssociateDebugVariables(sName + ".Sky.Texture");
}

} // namespace Sky

} // namespace Effect


