#ifndef EFFECT_POSTPROCESS_MOTIONBLURREDUCED_H
#define EFFECT_POSTPROCESS_MOTIONBLURREDUCED_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>
#include <Render/Texture/Downsample.h>

#include <Effect/PostProcess/MotionBlur/MotionBlur.h>

namespace PostProcess
{

class C_MotionBlurReduced : public I_PostProcess
{
public:

	C_MotionBlurReduced(C_Manager& manager);
	~C_MotionBlurReduced();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	void Enable() { m_bEnabled = true; }
	void Disable() { m_bEnabled = false; }
	void SetComposite(const bool bComposite) const { m_bComposite = bComposite; }
	void SetMethod(const E_MotionBlurMethod eMethod) const { m_eMethod = eMethod; }
	void SetSamples(const E_MotionBlurSamples eSamples) const;

	void SetTextureBuffer(U8* pTextureBuffer, const U32 iTextureBufferSize);
	U32 GetTextureBufferRequiredSize() const;

	void Display() const;

	F32 GetMotionAmount() const { return m_vMotionAmount.Length(); }

	Display::C_Texture2D* GetTexture();

private:

	void SetConstants() const;

	void CopyStencilBufferToAlpha() const;
	void DownsampleStencilBufferToAlpha() const;

private:

	mutable bool									m_bEnabled;
	mutable bool									m_bComposite;

	mutable E_MotionBlurMethod						m_eMethod;
	mutable E_MotionBlurSamples						m_eSamples;
	mutable U32										m_nShader;
	mutable U32										m_nSamples;
	mutable F32										m_fSamplesMinusOneRcp;
	mutable Maths::HVec3							m_vMotionAmount;

	Display::C_Texture2D							m_NoiseTexture;
	Display::C_Texture2D							m_BlackTexture;

	RenderTextures									m_RenderTextures;

	U8*												m_pTextureBuffer;
	U32												m_iTextureBufferSize;

	Display::C_PixelShader							m_BlurShader[kMotionBlurShaderCount];
	Display::C_PixelShader							m_CopyStencilShader;
	Display::C_PixelShader							m_CompositeShader;
	Display::C_PixelShader							m_DownsampleStencilShader;

	mutable Display::C_PixelShaderParameterArray	m_MatrixParameter[kMotionBlurShaderCount];
	mutable Display::C_PixelShaderParameter			m_DepthScaleParameter[kMotionBlurShaderCount];
	mutable Display::C_PixelShaderParameter			m_StepScaleParameter[kMotionBlurShaderCount];
	mutable Display::C_PixelShaderParameter			m_SettingsParameter[kMotionBlurShaderCount];
	mutable Display::C_PixelShaderParameter			m_NoiseParameter[kMotionBlurShaderCount];

	Render::Texture::C_Downsample					m_Downsample;

};

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_MOTIONBLURREDUCED_H
