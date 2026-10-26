#ifndef EFFECT_POSTPROCESS_MOTIONBLURDOFVELOCITY_H
#define EFFECT_POSTPROCESS_MOTIONBLURDOFVELOCITY_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>

#include <Effect/PostProcess/MotionBlurDoF.h>
#include <Effect/PostProcess/DoFBase.h>


namespace PostProcess
{

enum E_MotionBlurDoFVelocityShader
{
	E_MotionBlurDoFVelocityShader_5 = 0,
	E_MotionBlurDoFVelocityShader_7,
	E_MotionBlurDoFVelocityShader_9,
	E_MotionBlurDoFVelocityShader_11,
	E_MotionBlurDoFVelocityShader_Count
};

class C_MotionBlurDoFVelocity : public I_PostProcess, public C_DoFBase
{
public:

	C_MotionBlurDoFVelocity( C_Manager& manager );
	~C_MotionBlurDoFVelocity();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	void Enable() { m_bEnabled = true; }
	void Disable() { m_bEnabled = false; }

	void Display() const;

	void SetPixelShader(const E_MotionBlurDoFVelocityShader eShader) const;
	void SetScaleBias(const F32 fScale, const F32 fBias) const;
	void SetVelocityBuffer(const Display::C_Texture2D* pVelocityBuffer) const;

	void SetConstants() const;

public:

	virtual const Display::C_Texture2D*				CreateVelocityBuffer() const;

private:

	mutable bool									m_bEnabled;

	mutable E_MotionBlurDoFVelocityShader			m_eShader;
	mutable U32										m_nSamples;
	mutable F32										m_fSamplesMinusOneRcp;

	C_DepthOfFieldSettings*							m_pViewportSettings;

	Display::C_PixelShader							m_PixelShaders[E_MotionBlurDoFVelocityShader_Count];

	mutable Display::C_PixelShaderParameter			m_DepthScaleParameter[E_MotionBlurDoFVelocityShader_Count];
	mutable Display::C_PixelShaderParameter			m_StepScaleParameter[E_MotionBlurDoFVelocityShader_Count];
	mutable Display::C_PixelShaderParameter			m_PixelScaleParameter[E_MotionBlurDoFVelocityShader_Count];
	mutable Display::C_PixelShaderParameter			m_Settings1Parameter[E_MotionBlurDoFVelocityShader_Count];
	mutable Display::C_PixelShaderParameter			m_Settings2Parameter[E_MotionBlurDoFVelocityShader_Count];
	mutable Display::C_PixelShaderParameterArray	m_FilterParameter[E_MotionBlurDoFVelocityShader_Count];

	mutable const Display::C_Texture2D*				m_pVelocityBuffer;

	mutable F32										m_fScale;
	mutable F32										m_fBias;

};

INLINE void C_MotionBlurDoFVelocity::SetPixelShader(const E_MotionBlurDoFVelocityShader eShader) const
{
	m_eShader = eShader;
	switch (m_eShader)
	{
		case E_MotionBlurDoFVelocityShader_5:  m_nSamples = 5;  break;
		case E_MotionBlurDoFVelocityShader_7:  m_nSamples = 7;  break;
		case E_MotionBlurDoFVelocityShader_9:  m_nSamples = 9;  break;
		case E_MotionBlurDoFVelocityShader_11: m_nSamples = 11; break;
	}

	m_fSamplesMinusOneRcp = 1.0f / (F32)(m_nSamples - 1);
}

INLINE void C_MotionBlurDoFVelocity::SetScaleBias(const F32 fScale, const F32 fBias) const
{
	m_fScale = fScale;
	m_fBias = fBias;
}

INLINE void C_MotionBlurDoFVelocity::SetVelocityBuffer(const Display::C_Texture2D* pVelocityBuffer) const
{
	m_pVelocityBuffer = pVelocityBuffer;
}

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_MOTIONBLURDOFVELOCITY_H
