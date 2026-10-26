#ifndef EFFECT_POSTPROCESS_DOFFGBG_H
#define EFFECT_POSTPROCESS_DOFFGBG_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>
#include <Render/Texture/GaussianBlur.h>
#include <Effect/PostProcess/DoFBase.h>

namespace PostProcess
{

struct C_DepthOfFieldFgBgSettings
{
	C_DepthOfFieldFgBgSettings() : m_fNearStartDistance(0.0f), m_fNearEndDistance(0.0f), m_fFarStartDistance(0.0f), m_fFarEndDistance(0.0f), m_fFarScale(1.0f) {}

	F32 m_fNearStartDistance;
	F32 m_fNearEndDistance;
	F32 m_fFarStartDistance;
	F32 m_fFarEndDistance;
	F32 m_fFarScale;
};

class C_DoFFgBg : public I_PostProcess, public C_DoFBase
{
public:

	C_DoFFgBg(C_Manager& manager);
	~C_DoFFgBg();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	void SetTextureBuffer(U8* pTextureBuffer, const U32 iTextureBufferSize);
	U32 GetTextureBufferRequiredSize() const;

	void ConvertDepthOfFieldParameters(const F32 fPlaneInFocus, const F32 fAperture, F32& fNearStartDistance, F32& fNearEndDistance, F32& fFarStartDistance, F32& fFarEndDistance) const;
	void SetDepthOfField(const S32 iViewIndex, const F32 fPlaneInFocus, const F32 fAperture);
	void SetDepthOfField(const S32 iViewIndex, const F32 fNearStartDistance, const F32 fNearEndDistance, const F32 fFarStartDistance, const F32 fFarEndDistance, const F32 fFarScale);

	void Enable() { m_bEnabled = true; }
	void Disable() { m_bEnabled = false; }

	void Display() const;

private:

	mutable bool					m_bEnabled;

	Display::C_PixelShader			m_BlurShader;
	Display::C_PixelShader			m_CombineShader;
	Display::C_PixelShader			m_DownsampleShader;
	Display::C_PixelShader			m_NearShader;

	Display::C_PixelShaderParameter	m_BlurSettingsParameter;
	Display::C_PixelShaderParameter	m_CombineSettingsParameter;
	Display::C_PixelShaderParameter	m_CombineScaleParameter;
	Display::C_PixelShaderParameter	m_CombineBiasParameter;
	Display::C_PixelShaderParameter	m_CombineFarParameter;
	Display::C_PixelShaderParameter	m_CombineDepthParameter;
	Display::C_PixelShaderParameter	m_DownsampleSettingsParameter;
	Display::C_PixelShaderParameter	m_DownsampleDepthParameter;

	Render::Texture::C_GaussianBlur	m_GaussianBlurUtil;

	RenderTextures					m_DownsampleRenderTextures;
	RenderTextures					m_LargeBlurRenderTextures;
	RenderTextures					m_SmallBlurRenderTextures;

	U8*								m_pTextureBuffer;
	U32								m_iTextureBufferSize;

	Btl::Vector<C_DepthOfFieldFgBgSettings> m_DepthOfFieldSettings;

};

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_DOFFGBG_H
