#ifndef AMAX_GRAPHICS_SHADOWSETTINGS_H
#define AMAX_GRAPHICS_SHADOWSETTINGS_H

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentFloat.h>
#include <FX/EnvironmentColour.h>

#include <Xt/XtGame.h>

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

namespace Game
{

struct ShadowSettings
{
	XtF32	fShadowSize;
	XtF32	fShadowHeight;
	XtF32	fDropShadowIntensity;
	XtF32	fDropShadowHeight;
	XtF32	fIndividualShadowMaxIntensity;
	XtF32	fMultiShadowIntensity;
	XtF32	fFadeRange;
	XtF32	fMaxShadows;
};

/// implementation of I_EnvironmentSettings for Shadows
class ShadowEnvSettings : public FX::I_EnvironmentSettings
{
public:

	ShadowEnvSettings(ShadowSettings* pData);
	ShadowEnvSettings(const ShadowEnvSettings* pA, const ShadowEnvSettings* pB, float f);

	Util::C_String			Name() const { return "ShadowSettings"; }

	I_EnvironmentSettings*	Lerp(const I_EnvironmentSettings* p, float f) const;

	void					Set();
	void					AssociateDebugVariables(const Util::C_String& sName);

private:

	FX::EnvFloatParam			m_fShadowSize;
	FX::EnvFloatParam			m_fShadowHeight;
	FX::EnvFloatParam			m_fDropShadowIntensity;
	FX::EnvFloatParam			m_fDropShadowHeight;
	FX::EnvFloatParam			m_fIndividualShadowMaxIntensity;
	FX::EnvFloatParam			m_fMultiShadowIntensity;
	FX::EnvFloatParam			m_fFadeRange;
	FX::EnvFloatParam			m_fMaxShadows;
};

} // namespace 

#endif // AMAX_GRAPHICS_SHADOWSETTINGS_H
