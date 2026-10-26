#ifndef __POSTPROCESSENVSETTINGS_H__
#define __POSTPROCESSENVSETTINGS_H__

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentFloat.h>
#include <Xt/XtGame.h>

namespace PostProcess
{

struct Settings
{
	XtF32 middleGrey;
	XtF32 exposureMin;
	XtF32 exposureMax;
	XtF32 bloomScale;
	XtF32 bloomScale0;
	XtF32 bloomScale1;
	XtF32 bloomScale2;
	XtF32 bloomScale3;
	XtF32 bloomThreshold;
};

class C_Manager;

/// implementation of I_EnvironmentSettings for Post Processing
class PostProcessEnvSettings : public FX::I_EnvironmentSettings
{
public:
	/// constructor
	PostProcessEnvSettings(Settings *pData);

	/// construct by lerping two other PostProcessEnvSettings
	PostProcessEnvSettings(const PostProcessEnvSettings *pA, const PostProcessEnvSettings *pB, float f);

	/// cause these environment settings to be applied to the scene
	void Set();

	/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
	I_EnvironmentSettings *Lerp(const I_EnvironmentSettings *p, float f) const;

	/// GUID for this settings block
	Util::C_String Name() const;

	/// associate this block of environment settings with the given debug variable or debug variable page
	void AssociateDebugVariables(const Util::C_String &sName);

private:
	FX::EnvFloatParam	m_MiddleGrey;
	FX::EnvFloatParam	m_ExposureMin;
	FX::EnvFloatParam	m_ExposureMax;
	FX::EnvFloatParam	m_BloomScale;
	FX::EnvFloatParam	m_BloomScale0;
	FX::EnvFloatParam	m_BloomScale1;
	FX::EnvFloatParam	m_BloomScale2;
	FX::EnvFloatParam	m_BloomScale3;
	FX::EnvFloatParam	m_BloomThreshold;
};

} // namespace

#endif
