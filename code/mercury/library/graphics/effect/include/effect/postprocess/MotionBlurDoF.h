#ifndef EFFECT_POSTPROCESS_MOTIONBLURDOF_H
#define EFFECT_POSTPROCESS_MOTIONBLURDOF_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Btl/Containers/vector.h>
#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>
#include <Effect/PostProcess/DoFBase.h>

namespace PostProcess
{

enum E_MotionBlurDoFMethod
{
	E_MotionBlurDoFMethod_Alpha = 0,
	E_MotionBlurDoFMethod_Stencil,
	E_MotionBlurDoFMethod_Count
};

enum E_MotionBlurDoFSamples
{
	E_MotionBlurDoFSamples_5 = 0,
	E_MotionBlurDoFSamples_7,
	E_MotionBlurDoFSamples_9,
	E_MotionBlurDoFSamples_11,
	E_MotionBlurDoFSamples_Count
};

const U32 kMotionBlurDoFShaderCount = E_MotionBlurDoFMethod_Count * E_MotionBlurDoFSamples_Count;

class C_MotionBlurDoF : public I_PostProcess, public C_DoFBase
{
public:

	C_MotionBlurDoF( C_Manager& manager );
	~C_MotionBlurDoF();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	void Enable() { m_bEnabled = true; }
	void Disable() { m_bEnabled = false; }
	void SetMethod(const E_MotionBlurDoFMethod eMethod) const;
	void SetSamples(const E_MotionBlurDoFSamples eSamples) const;

	void Display() const;

	F32 GetMotionAmount() const { return m_vMotionAmount.Length(); }

private:

	bool CreateRenderTarget();
	void DestroyRenderTarget();

	void SetConstants() const;

	void CopyStencilBufferToAlpha() const;

private:

	mutable bool									m_bEnabled;

	mutable E_MotionBlurDoFMethod					m_eMethod;
	mutable E_MotionBlurDoFSamples					m_eSamples;
	mutable U32										m_nShader;
	mutable U32										m_nSamples;
	mutable F32										m_fSamplesMinusOneRcp;
	mutable Maths::HVec3							m_vMotionAmount;

	Display::C_Texture2D							m_NoiseTexture;

	Display::C_RenderTexture2D						m_ColourMaskRenderTexture;

	Display::C_PixelShader							m_BlurShader[kMotionBlurDoFShaderCount];
	Display::C_PixelShader							m_WriteMaskShader;
	Display::C_PixelShader							m_CopyStencilShader;

	mutable Display::C_PixelShaderParameterArray	m_MatrixParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_DepthScaleParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_StepScaleParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_PixelScaleParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_Settings1Parameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_Settings2Parameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameter			m_NoiseParameter[kMotionBlurDoFShaderCount];
	mutable Display::C_PixelShaderParameterArray	m_FilterParameter[kMotionBlurDoFShaderCount];

};

INLINE void C_MotionBlurDoF::SetMethod(const E_MotionBlurDoFMethod eMethod) const
{
	m_eMethod = eMethod;
}

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_MOTIONBLURDOF_H
