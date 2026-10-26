#ifndef __SHADER_FOGSETTINGS_H__
#define __SHADER_FOGSETTINGS_H__

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

// structures from Sorcery
struct DepthFogSettings
{
	XtF32	Density;
	XtF32	NearDistance;
	XtF32	Scale;
};

struct HeightFogSettings
{
	XtF32	Height;
	XtF32	Density;
	XtF32	Falloff;
	XtF32	Scale;
};

struct FogSettings
{
	XtRgba				FogColour;
	DepthFogSettings	DepthFog;
	HeightFogSettings	HeightFog;
};

// global structure containing the current lighting settings
struct WorldFogSettings
{
	Maths::C_RGBA_F32	m_vFogColour;
	Maths::HVec4		m_vDepthFogSettings;	// density, neardistance, scale, 0
	Maths::HVec4		m_vHeightFogSettings;	// height, density, falloff, scale
	Maths::C_RGBA_F32	m_vFogIntensities;		// 0,0,0,0
};

/// implementation of I_EnvironmentSettings for C_TextureSkyDome
class FogEnvSettings : public FX::I_EnvironmentSettings
{
public:
	/// constructor
	FogEnvSettings(FogSettings *pData);

	/// construct by lerping two other FogEnvSettings
	FogEnvSettings(const FogEnvSettings *pA, const FogEnvSettings *pB, float f);

	/// cause these environment settings to be applied to the scene
	void Set();

	/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
	I_EnvironmentSettings *Lerp(const I_EnvironmentSettings *p, float f) const;

	/// GUID for this settings block
	Util::C_String Name() const;

	/// associate this block of environment settings with the given debug variable or debug variable page
	void AssociateDebugVariables(const Util::C_String &sName);

	static void InitialiseParameters();

public:

	static WorldFogSettings m_FogSettings;

private:

	FX::EnvColourParam	m_vFogColour;
	FX::EnvFloatParam	m_fDepthFogDensity;
	FX::EnvFloatParam	m_fDepthFogNearDistance;
	FX::EnvFloatParam	m_fDepthFogScale;
	FX::EnvFloatParam	m_fHeightFogHeight;
	FX::EnvFloatParam	m_fHeightFogDensity;
	FX::EnvFloatParam	m_fHeightFogFalloff;
	FX::EnvFloatParam	m_fHeightFogScale;
	FX::EnvColourParam	m_vFogIntensities;

};

} // namespace

#endif
