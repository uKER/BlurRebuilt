#ifndef AMAX_GRAPHICS_CARLIGHTSETTINGS_H
#define AMAX_GRAPHICS_CARLIGHTSETTINGS_H

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentFloat.h>
#include <FX/EnvironmentColour.h>

#include <Xt/XtGame.h>

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

namespace Game
{

struct CarLightSettings
{
	XtF32	fSHLightFactor;
	XtF32	fSHLightMax;
	XtF32	fStaticLightCubemapFactor;
	XtF32	fDynamicLightCubemapFactor;
	XtF32	fLightCubemapMax;

	XtRgb	FakeLightColour1;
	XtF32	fFakeLightSize1;
	XtVec3	FakeLightPosition1;

	XtRgb	FakeLightColour2;
	XtF32	fFakeLightSize2;
	XtVec3	FakeLightPosition2;

	XtRgb	FakeLightColour3;
	XtF32	fFakeLightSize3;
	XtVec3	FakeLightPosition3;

	XtF32	fFakeSunBlobShadowIntensity;
	XtVec3	FakeSunBlobShadowPosition;
};

/// implementation of I_EnvironmentSettings for Car Light
class CarLightEnvSettings : public FX::I_EnvironmentSettings
{
public:

	CarLightEnvSettings(CarLightSettings* pData);
	CarLightEnvSettings(const CarLightEnvSettings* pA, const CarLightEnvSettings* pB, float f);

	Util::C_String			Name() const { return "CarLightSettings"; }

	I_EnvironmentSettings*	Lerp(const I_EnvironmentSettings* p, float f) const;

	void					Set();
	void					AssociateDebugVariables(const Util::C_String& sName);

private:

	FX::EnvFloatParam			m_fSHLightFactor;
	FX::EnvFloatParam			m_fSHLightMax;
	FX::EnvFloatParam			m_fStaticLightCubemapFactor;
	FX::EnvFloatParam			m_fDynamicLightCubemapFactor;
	FX::EnvFloatParam			m_fLightCubemapMax;

	FX::EnvColourParam			m_FakeLightColour1;
	FX::EnvFloatParam			m_fFakeLightSize1;
	FX::EnvFloatParam			m_fFakeLightPositionX1;
	FX::EnvFloatParam			m_fFakeLightPositionY1;
	FX::EnvFloatParam			m_fFakeLightPositionZ1;

	FX::EnvColourParam			m_FakeLightColour2;
	FX::EnvFloatParam			m_fFakeLightSize2;
	FX::EnvFloatParam			m_fFakeLightPositionX2;
	FX::EnvFloatParam			m_fFakeLightPositionY2;
	FX::EnvFloatParam			m_fFakeLightPositionZ2;

	FX::EnvColourParam			m_FakeLightColour3;
	FX::EnvFloatParam			m_fFakeLightSize3;
	FX::EnvFloatParam			m_fFakeLightPositionX3;
	FX::EnvFloatParam			m_fFakeLightPositionY3;
	FX::EnvFloatParam			m_fFakeLightPositionZ3;

	FX::EnvFloatParam			m_fFakeSunBlobShadowIntensity;
	FX::EnvFloatParam			m_fFakeSunBlobShadowPositionX;
	FX::EnvFloatParam			m_fFakeSunBlobShadowPositionY;
	FX::EnvFloatParam			m_fFakeSunBlobShadowPositionZ;
};

} // namespace 

#endif // AMAX_GRAPHICS_SHADOWSETTINGS_H
