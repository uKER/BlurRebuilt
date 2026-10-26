#ifndef AMAX_GRAPHICS_ROADTRACKENVIRONMENTSETTINGS_H
#define AMAX_GRAPHICS_ROADTRACKENVIRONMENTSETTINGS_H

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentFloat.h>
#include <FX/EnvironmentColour.h>

#include <Xt/XtGame.h>

namespace Game
{

struct RoadTrackSettings
{
	XtF32	fTrackReflectionBrightness;
	XtF32	fTrackReflectionContrast;
	XtF32	fReflectionBlurX;
	XtF32	fReflectionBlurY;
	XtF32	fReflectionBlurDesviation;
	XtF32	fReflectionBrightness;
	XtF32	fCubeMapSmoothness;
	XtF32	fTrackReflectionScale;
	XtF32	fWaterReflectionScale;
	XtF32	fTrackReflectionOffset;
	bool	BlackBorderReflection;

	XtRgb	FakeSunLightColour;
	XtF32	fFakeSunLightMinSize;
	XtF32	fFakeSunLightMaxSize;
	XtVec3	FakeSunLightPosition;
	bool	FakeSunLightDepthTest;
};

/// implementation of I_EnvironmentSettings for Post FX
class RoadTrackEnvSettings : public FX::I_EnvironmentSettings
{
public:

	RoadTrackEnvSettings(RoadTrackSettings* pData);
	RoadTrackEnvSettings(const RoadTrackEnvSettings* pA, const RoadTrackEnvSettings* pB, float f);

	Util::C_String			Name() const { return "RoadTrack"; }

	I_EnvironmentSettings*	Lerp(const I_EnvironmentSettings* p, float f) const;

	void					Set();
	void					AssociateDebugVariables(const Util::C_String& sName);

	static void				InitialiseParameters();

private:

	FX::EnvFloatParam	m_fTrackReflectionBrightness;
	FX::EnvFloatParam	m_fTrackReflectionContrast;
	FX::EnvFloatParam	m_fReflectionBlurX;
	FX::EnvFloatParam	m_fReflectionBlurY;
	FX::EnvFloatParam	m_fReflectionBlurDesviation;
	FX::EnvFloatParam	m_fReflectionBrightness;
	FX::EnvFloatParam	m_fCubeMapSmoothness;
	FX::EnvFloatParam	m_fTrackReflectionScale;
	FX::EnvFloatParam	m_fWaterReflectionScale;
	FX::EnvFloatParam	m_fTrackReflectionOffset;
	bool				m_BlackBorderReflection;

	FX::EnvColourParam			m_FakeSunLightColour;
	FX::EnvFloatParam			m_fFakeSunLightMinSize;
	FX::EnvFloatParam			m_fFakeSunLightMaxSize;
	FX::EnvFloatParam			m_fFakeSunLightPositionX;
	FX::EnvFloatParam			m_fFakeSunLightPositionY;
	FX::EnvFloatParam			m_fFakeSunLightPositionZ;
	bool						m_FakeSunLightDepthTest;
};

} // namespace 

#endif // AMAX_GRAPHICS_ROADTRACKENVIRONMENTSETTINGS_H
