#ifndef __EFFECT_LIGHTINGSETTINGS_H__
#define __EFFECT_LIGHTINGSETTINGS_H__

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentFloat.h>
#include <FX/ENvironmentColour.h>

#include <xt/xtGame.h>

namespace Debug
{
	class String;
};

namespace EnvironmentSettings
{

extern Maths::HVec4 g_vIntensities;

// structures from Sorcery
struct LightingInfo
{
	XtRgba	SunColour;
	XtRgba	SkyColour;
	XtRgba	BakedColour;
	XtRgba	EmissiveColour;
};

// global structure containing the current lighting settings
struct WorldLightingSettings
{
	Maths::C_RGBA_F32	m_vSunColour;
	Maths::C_RGBA_F32	m_vSkyColour;
	Maths::C_RGBA_F32	m_vBakedColour;
	Maths::C_RGBA_F32	m_vEmissiveColour;
	Maths::C_RGBA_F32	m_vIntensities;			// 1,1,1,1
};

/// implementation of I_EnvironmentSettings for C_TextureSkyDome
class LightingEnvSettings : public FX::I_EnvironmentSettings
{
public:
	/// constructor
	LightingEnvSettings(LightingInfo *pData);

	/// construct by lerping two other LightingEnvSettings
	LightingEnvSettings(const LightingEnvSettings *pA, const LightingEnvSettings *pB, float f);

	/// cause these environment settings to be applied to the scene
	void Set();

	/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
	I_EnvironmentSettings *Lerp(const I_EnvironmentSettings *p, float f) const;

	/// GUID for this settings block
	Util::C_String Name() const;

	/// associate this block of environment settings with the given debug variable or debug variable page
	void AssociateDebugVariables(const Util::C_String &sName);

	static void InitialiseParameters();

private:
	FX::EnvColourParam	m_vSunColour;
	FX::EnvColourParam	m_vSkyColour;
	FX::EnvColourParam	m_vBakedColour;
	FX::EnvColourParam	m_vEmissiveColour;
	FX::EnvColourParam	m_vIntensities;

public:
	static WorldLightingSettings m_LightingSettings;
};

} // namespace

#endif
