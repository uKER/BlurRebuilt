#ifndef __SKYDOMEENVSETTINGS_H__
#define __SKYDOMEENVSETTINGS_H__

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentString.h>
#include <FX/EnvironmentFloat.h>
#include <FX/ENvironmentColour.h>

#include <xt/xtGame.h>

namespace Debug
{
	class String;
};

namespace Effect
{
namespace Sky
{

struct SkyDome
{
	XtSz8	textureName;
	XtF32	rotation;
	XtF32	intensity;
	XtF32	fogCurve;
	XtRgba	fogColour;
	XtF32	blackHeight;
	XtF32	blackSize;
};

class C_TexturedSkyDome;

/// implementation of I_EnvironmentSettings for C_TextureSkyDome
class SkyDomeEnvSettings : public FX::I_EnvironmentSettings
{
public:
	/// constructor
	SkyDomeEnvSettings(SkyDome *pData, C_TexturedSkyDome *pTarget);

	/// construct by lerping two other SkyDomeEnvSettings
	SkyDomeEnvSettings(const SkyDomeEnvSettings *pA, const SkyDomeEnvSettings *pB, float f);

	/// cause these environment settings to be applied to the scene
	void Set();

	/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
	I_EnvironmentSettings *Lerp(const I_EnvironmentSettings *p, float f) const;

	/// GUID for this settings block
	Util::C_String Name() const;

	/// associate this block of environment settings with the given debug variable or debug variable page
	void AssociateDebugVariables(const Util::C_String &sName);

private:
	FX::EnvFloatParam	m_Rotation;
	FX::EnvFloatParam	m_Intensity;
	FX::EnvFloatParam	m_FogCurve;
	FX::EnvColourParam	m_FogColour;
	FX::EnvFloatParam	m_BlackHeight;
	FX::EnvFloatParam	m_BlackSize;
	FX::EnvStringParam	m_Texture;
	Util::C_String	m_sPreviousTextureName;

	C_TexturedSkyDome*	m_pTarget;
};

} // namespace

} // namespace

#endif
