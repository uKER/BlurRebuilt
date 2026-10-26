#ifndef EFFECT_POSTPROCESS_MOTIONBLUR_H
#define EFFECT_POSTPROCESS_MOTIONBLUR_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>

namespace PostProcess
{

enum E_MotionBlurMethod
{
	E_MotionBlurMethod_Alpha = 0,
	E_MotionBlurMethod_Stencil,
	E_MotionBlurMethod_Count
};

enum E_MotionBlurSamples
{
	E_MotionBlurSamples_5 = 0,
	E_MotionBlurSamples_7,
	E_MotionBlurSamples_9,
	E_MotionBlurSamples_11,
	E_MotionBlurSamples_Count
};

const U32 kMotionBlurShaderCount = E_MotionBlurMethod_Count * E_MotionBlurSamples_Count;

class C_MotionBlur : public I_PostProcess
{
public:

	C_MotionBlur( C_Manager& manager );
	~C_MotionBlur();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	void Enable() { m_bEnabled = true; }
	void Disable() { m_bEnabled = false; }
	void SetMethod(const E_MotionBlurMethod eMethod) const { m_eMethod = eMethod; }
	void SetSamples(const E_MotionBlurSamples eSamples) const;

	void Display() const;

	F32 GetMotionAmount() const { return m_vMotionAmount.Length(); }

private:

	bool CreateRenderTarget();
	void DestroyRenderTarget();

	void SetConstants() const;

	void CopyStencilBufferToAlpha() const;

private:

	mutable bool									m_bEnabled;

	mutable E_MotionBlurMethod						m_eMethod;
	mutable E_MotionBlurSamples						m_eSamples;
	mutable U32										m_nShader;
	mutable U32										m_nSamples;
	mutable F32										m_fSamplesMinusOneRcp;
	mutable Maths::HVec3							m_vMotionAmount;

	Display::C_Texture2D							m_NoiseTexture;

	Display::C_RenderTexture2D						m_ColourMaskRenderTexture;

	Display::C_PixelShader							m_BlurShader[kMotionBlurShaderCount];
	Display::C_PixelShader							m_WriteMaskShader;
	Display::C_PixelShader							m_CopyStencilShader;

	mutable Display::C_PixelShaderParameterArray	m_MatrixParameter[kMotionBlurShaderCount];
	mutable Display::C_PixelShaderParameter			m_DepthScaleParameter[kMotionBlurShaderCount];
	mutable Display::C_PixelShaderParameter			m_StepScaleParameter[kMotionBlurShaderCount];
	mutable Display::C_PixelShaderParameter			m_SettingsParameter[kMotionBlurShaderCount];
	mutable Display::C_PixelShaderParameter			m_NoiseParameter[kMotionBlurShaderCount];

};

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_MOTIONBLUR_H
