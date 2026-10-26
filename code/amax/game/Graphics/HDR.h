#ifndef AMAX_GRAPHICS_HDR_H
#define AMAX_GRAPHICS_HDR_H

#if _MSC_VER > 1000
#pragma once
#endif

#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
#define HDR_USE_U32
#endif

#if defined(PLATFORM_PS3)
#define TONEMAP_COMBINE
#define TONEMAP_COMBINE_BUFFER_SIZE 8 // 4 splitscreen views double buffered
#endif

#include <Render/PostProcess/HDR/HDR.h>

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include <Graphics/PostFxLensFlare.h>

#include <Render/Utils/Curve.h>

struct ToneMapParameters : public PostProcess::HighDynamicRangeParameters
{
	Maths::Mat44	mColour;
	F32				fFilmGrainIntensity;
	F32				fFilmGrainLuminance;
	F32				fVignetteWeight;
	F32				fTunnelAmount;
};

enum E_ToneMapShader
{
	E_ToneMapShader_Default = 0,
	E_ToneMapShader_ReducedAlpha,
	E_ToneMapShader_MotionBlur,
	E_ToneMapShader_ReducedAlphaMotionBlur,
	E_ToneMapShader_DepthOfField,
	E_ToneMapShader_LargeScreenshot,
	E_ToneMapShader_ExtraFX,
	E_ToneMapShader_Count
};

namespace PostProcess
{
	class C_Manager;
}

class C_HighDynamicRange : public PostProcess::C_HighDynamicRange
{
public:

	C_HighDynamicRange(PostProcess::C_Manager& manager);
	~C_HighDynamicRange();

	bool Create();
	bool CreateVolatile();
	void Destroy();
	void DestroyVolatile();

	void Display() const;

	void SetUserParameters(const MathsStorage::Float4& settings1, const MathsStorage::Float4& settings2) const;

	void ManualCalculateLuminance() const;
	
	ToneMapParameters* GetParameters(const S32 iViewIndex);

	void SetToneMapShader(const E_ToneMapShader eToneMapShader) const;
	E_ToneMapShader GetToneMapShader() const;

	void SetMotionBlurTexture(const Display::C_Texture2D* pMotionBlurTexture) { m_pMotionBlurTexture = pMotionBlurTexture; }
	void SetLensEffectTexture(const Display::C_Texture2D* pLensEffectTexture) { m_pLensEffectTexture = pLensEffectTexture; }
	

	void SetLargeScreenshotBloomScale(const F32 fZoom, const F32 fX, const F32 fY, const F32 fBright);

	void ReloadNoiseTexture();
	void SetNoiseTextureName(const Util::C_String& textureName) { m_NoiseTextureName = textureName; }
	Util::C_String GetNoiseTextureName() const { return m_NoiseTextureName; }

	bool CreateLookupTexture();
	void ReloadLookupTexture();
	void SetLookupTextureName(const Util::C_String& textureName) { m_LookupTextureName = textureName; }
	Util::C_String GetLookupTextureName() const { return m_LookupTextureName; }

	void ReloadBloomCurve(const Render::C_HermiteCurve& curve, const U32 m_UID);
	F32 ColourSep() const;
	F32 Blockiness() const;
	F32 ScreenOverlay() const;
	bool NeedExtraFX() const;

	Display::C_Texture2D& GetOverlayTexture() { return m_OverlayTexture; }
	void SetOverlay(int viewport, float amount);

protected:

	Display::C_PixelShader					m_PixelShader[E_ToneMapShader_Count];

	Display::C_PixelShaderParameter			m_Settings1Parameter[E_ToneMapShader_Count];
	Display::C_PixelShaderParameter			m_Settings2Parameter[E_ToneMapShader_Count];
	Display::C_PixelShaderParameterArray	m_ColourMatrixParameter[E_ToneMapShader_Count];
	Display::C_PixelShaderParameter			m_FilmGrainParameter[E_ToneMapShader_Count];
	Display::C_PixelShaderParameter			m_SpecialFXParameter[E_ToneMapShader_Count];
	Display::C_PixelShaderParameter			m_BloomScaleParameter[E_ToneMapShader_Count];
	Display::C_PixelShaderParameter			m_ColourSepParameter[E_ToneMapShader_Count];

	mutable E_ToneMapShader					m_eToneMapShader;

	const Display::C_Texture2D*				m_pMotionBlurTexture;
	const Display::C_Texture2D*				m_pLensEffectTexture;

	Display::C_Texture2D					m_NoiseTexture;
	Util::C_String							m_NoiseTextureName;
	Display::C_Texture2D					m_BlackTexture;

	Display::C_Texture3D					m_LookupTexture;
#ifdef TONEMAP_COMBINE
	Maths::Mat44							m_ColourMatArray[TONEMAP_COMBINE_BUFFER_SIZE];
	Display::C_Texture3D					m_LookupTextureArray[TONEMAP_COMBINE_BUFFER_SIZE];
	Display::C_Texture3D					m_LookupTextureGamma;
	float									m_LookupTextureGammaValue;
#endif
	Util::C_String							m_LookupTextureName;
#if !defined(CFG_GOLD)
	Display::C_Texture3D					m_IdentityTexture;
#endif
	
	Display::C_Texture2D					m_BloomCurveTexture;
	U32										m_UID;
	Display::C_Texture2D					m_OverlayTexture;

	float									m_OverlayAmount[4];
};

C_HighDynamicRange* GetHighDynamicRange();
void SetHighDynamicRange(C_HighDynamicRange* pHighDynamicRange);
bool HighDynamicRangeExists();

#endif // AMAX_GRAPHICS_HDR_H
