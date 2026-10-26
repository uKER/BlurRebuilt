#ifndef AMAX_GRAPHICS_LIGHTFLARESETTINGS_H
#define AMAX_GRAPHICS_LIGHTFLARESETTINGS_H

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentFloat.h>
#include <FX/EnvironmentColour.h>

#include <Xt/XtGame.h>

namespace Game
{

struct LightFlareSettings
{
	XtF32	fGlobalSize;
	XtF32	fGlobalSizeX;
	XtF32	fGlobalSizeY;
	XtRgb	vGlobalColour;
	XtF32	fReflectionMinSizeX;
	XtF32	fReflectionMinSizeY;
	XtF32	fReflectionMaxSizeX;
	XtF32	fReflectionMaxSizeY;
	XtF32	fEnvironmentSizeX;
	XtF32	fEnvironmentSizeY;
	XtRgb	vReflectionColour;
	XtRgb	vEnvironmentColour;
	XtF32	fNearFadeDistant;
	XtF32	fFarFadeDistant;
	XtF32	fEnvironmentNearFadeDistant;
	XtF32	fEnvironmentFarFadeDistant;
	XtF32	fSizeDistantFactor;
	XtF32	fIntensityDistantFactor;
	XtF32	fCarGlobalSize;
	XtF32	fCarGlobalSizeX;
	XtF32	fCarGlobalSizeY;
	XtRgb	vCarGlobalColour;
	XtF32	fCarReflectionMinSizeX;
	XtF32	fCarReflectionMinSizeY;
	XtF32	fCarReflectionMaxSizeX;
	XtF32	fCarReflectionMaxSizeY;
	XtF32	fCarEnvironmentSizeX;
	XtF32	fCarEnvironmentSizeY;
	XtRgb	vCarReflectionColour;
	XtRgb	vCarWhiteFlaresColour;
	XtRgb	vCarRedFlaresColour;
};

class LightFlareEnvSettings : public FX::I_EnvironmentSettings
{
public:

	LightFlareEnvSettings(LightFlareSettings* pData);
	LightFlareEnvSettings(const LightFlareEnvSettings* pA, const LightFlareEnvSettings* pB, float f);

	Util::C_String			Name() const { return "LightFlare"; }

	I_EnvironmentSettings*	Lerp(const I_EnvironmentSettings* p, float f) const;

	void					Set();
	void					AssociateDebugVariables(const Util::C_String& sName);

private:

	FX::EnvFloatParam	m_fGlobalSize;
	FX::EnvFloatParam	m_fGlobalSizeX;
	FX::EnvFloatParam	m_fGlobalSizeY;
	FX::EnvColourParam	m_vGlobalColour;
	FX::EnvFloatParam	m_fReflectionMinSizeX;
	FX::EnvFloatParam	m_fReflectionMinSizeY;
	FX::EnvFloatParam	m_fReflectionMaxSizeX;
	FX::EnvFloatParam	m_fReflectionMaxSizeY;
	FX::EnvColourParam	m_vReflectionColour;
	FX::EnvFloatParam	m_fEnvironmentSizeX;
	FX::EnvFloatParam	m_fEnvironmentSizeY;
	FX::EnvColourParam	m_vEnvironmentColour;
	FX::EnvFloatParam	m_fNearFadeDistant;
	FX::EnvFloatParam	m_fFarFadeDistant;
	FX::EnvFloatParam	m_fEnvironmentNearFadeDistant;
	FX::EnvFloatParam	m_fEnvironmentFarFadeDistant;
	FX::EnvFloatParam	m_fSizeDistantFactor;
	FX::EnvFloatParam	m_fIntensityDistantFactor;
	FX::EnvFloatParam	m_fCarGlobalSize;
	FX::EnvFloatParam	m_fCarGlobalSizeX;
	FX::EnvFloatParam	m_fCarGlobalSizeY;
	FX::EnvColourParam	m_vCarGlobalColour;
	FX::EnvFloatParam	m_fCarReflectionMinSizeX;
	FX::EnvFloatParam	m_fCarReflectionMinSizeY;
	FX::EnvFloatParam	m_fCarReflectionMaxSizeX;
	FX::EnvFloatParam	m_fCarReflectionMaxSizeY;
	FX::EnvColourParam	m_vCarReflectionColour;
	FX::EnvFloatParam	m_fCarEnvironmentSizeX;
	FX::EnvFloatParam	m_fCarEnvironmentSizeY;
	FX::EnvColourParam	m_vCarWhiteFlaresColour;
	FX::EnvColourParam	m_vCarRedFlaresColour;

};

} // namespace 

#endif // AMAX_GRAPHICS_LIGHTFLARESETTINGS_H
