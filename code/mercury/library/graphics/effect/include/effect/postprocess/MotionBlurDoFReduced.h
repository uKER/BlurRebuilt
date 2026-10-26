#ifndef EFFECT_POSTPROCESS_MOTIONBLURDOFREDUCED_H
#define EFFECT_POSTPROCESS_MOTIONBLURDOFREDUCED_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>
#include <Render/Texture/Downsample.h>

#include <Effect/PostProcess/DoFBase.h>
#include <Effect/PostProcess/MotionBlurDoF.h>

namespace PostProcess
{

class C_MotionBlurDoFReduced : public I_PostProcess, public C_DoFBase
{
public:

	C_MotionBlurDoFReduced(C_Manager& manager);
	~C_MotionBlurDoFReduced();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	void Enable() { m_bEnabled = true; }
	void Disable() { m_bEnabled = false; }
	void SetComposite(const bool bComposite) const { m_bComposite = bComposite; }
	void SetMethod(const E_MotionBlurDoFMethod eMethod) const;
	void SetSamples(const E_MotionBlurDoFSamples eSamples) const;

	void SetTextureBuffer(U8* pTextureBuffer, const U32 iTextureBufferSize);
	U32 GetTextureBufferRequiredSize() const;

	void Display() const;

	F32 GetMotionAmount() const { return m_vMotionAmount.Length(); }

	Display::C_Texture2D* GetTexture();

private:

	bool CreateRenderTarget();
	void DestroyRenderTarget();

	void SetConstants() const;

	void CopyStencilBufferToAlpha() const;
	void DownsampleStencilBufferToAlpha() const;

private:

	mutable bool									m_bEnabled;
	mutable bool									m_bComposite;

	mutable E_MotionBlurDoFMethod					m_eMethod;
	mutable E_MotionBlurDoFSamples					m_eSamples;
	mutable U32										m_nShader;
	mutable U32										m_nSamples;
	mutable F32										m_fSamplesMinusOneRcp;
	mutable Maths::HVec3							m_vMotionAmount;

	Display::C_Texture2D							m_NoiseTexture;
	Display::C_Texture2D							m_BlackTexture;

	RenderTextures									m_ColourMaskRenderTextures;
	RenderTextures									m_ReducedRenderTextures;

	U8*												m_pTextureBuffer;
	U32												m_iTextureBufferSize;

	Display::C_PixelShader							m_BlurShader[kMotionBlurDoFShaderCount];
	Display::C_PixelShader							m_WriteMaskShader;
	Display::C_PixelShader							m_CopyStencilShader;
	Display::C_PixelShader							m_CompositeShader;
	Display::C_PixelShader							m_DownsampleStencilShader;

	mutable Display::C_PixelShaderParameterArray	m_MatrixParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_DepthScaleParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_StepScaleParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_PixelScaleParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_Settings1Parameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_Settings2Parameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_NoiseParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameterArray	m_FilterParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_AlphaParameter[kMotionBlurDoFShaderCount];

	Render::Texture::C_Downsample					m_Downsample;
};

INLINE void C_MotionBlurDoFReduced::SetMethod(const E_MotionBlurDoFMethod eMethod) const
{
	m_eMethod = eMethod;
}

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_MOTIONBLURDOFREDUCED_H
