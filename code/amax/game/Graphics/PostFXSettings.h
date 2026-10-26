#ifndef AMAX_GRAPHICS_POSTFXENVIRONMENTSETTINGS_H
#define AMAX_GRAPHICS_POSTFXENVIRONMENTSETTINGS_H

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentFloat.h>
#include <FX/EnvironmentColour.h>
#include <FX/EnvironmentString.h>

#include <Graphics/PostFXLensFlare.h>

#include <Render/Utils/Curve.h>

#include <Xt/XtGame.h>

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

namespace Game
{

enum BlurType
{
	BlurType_MotionBlur,
	BlurType_DepthOfField,
};

struct PostFxMatrix;

struct PostFxSettings
{
	XtArray<PostFxMatrix*>	matrices;
	PostFxMatrix*			root;
	XtSz8					lookupTexture;
	Render::Curve			bloomThresholdCurve;
	PostFxLensFlares		lensFlares;
	XtF32					fFilmGrainIntensity;
	XtF32					fFilmGrainLuminance;
	XtF32					fVignetteWeight;
	XtF32					fTunnelAmount;
	XtF32					fBloomFeedback;
	BlurType				blurType;
	XtU32					uid;
};

/// implementation of I_EnvironmentSettings for Post FX
class PostFxEnvSettings : public FX::I_EnvironmentSettings
{
public:

	PostFxEnvSettings(PostFxSettings* pData);
	PostFxEnvSettings(const PostFxEnvSettings* pA, const PostFxEnvSettings* pB, float f);

	Util::C_String			Name() const { return "PostFX"; }

	I_EnvironmentSettings*	Lerp(const I_EnvironmentSettings* p, float f) const;

	void					Set();
	void					AssociateDebugVariables(const Util::C_String& sName);
	
	Maths::Mat44			m_ColourMatrix;
	FX::EnvStringParam		m_LookupTexture;
	FX::EnvFloatParam		m_FilmGrainIntensity;
	FX::EnvFloatParam		m_FilmGrainLuminance;
	FX::EnvFloatParam		m_VignetteWeight;
	FX::EnvFloatParam		m_TunnelAmount;
	FX::EnvFloatParam		m_ColourSep;
	FX::EnvFloatParam		m_Blockiness;
	FX::EnvFloatParam		m_ScreenOverlay;
	FX::EnvFloatParam		m_BloomFeedback;
	Display::C_Float2		m_FlareScales[4];
	Display::C_Float4		m_FlareColours[4];
	BlurType				m_BlurType;
	Render::C_HermiteCurve	m_BloomThresholdCurve;
	U32						m_UID;

};

} // namespace 

#endif // AMAX_GRAPHICS_HDRENVIRONMENTSETTINGS_H
