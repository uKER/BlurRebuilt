#ifndef EFFECT_POSTPROCESS_MOTIONBLURVELOCITY_H
#define EFFECT_POSTPROCESS_MOTIONBLURVELOCITY_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>

namespace PostProcess
{

enum E_MotionBlurVelocityShader
{
	E_MotionBlurVelocityShader_5 = 0,
	E_MotionBlurVelocityShader_7,
	E_MotionBlurVelocityShader_9,
	E_MotionBlurVelocityShader_11,
	E_MotionBlurVelocityShader_Count
};

class C_MotionBlurVelocity : public I_PostProcess
{
public:

	C_MotionBlurVelocity( C_Manager& manager );
	~C_MotionBlurVelocity();

	// I_PostProcess interface
	bool	Create();
	bool	CreateVolatile();
	void	Destroy();
	void	DestroyVolatile();

	void	Update();

	void	Display() const;

	// member functions
	void	SetPixelShader(const E_MotionBlurVelocityShader eShader) const;
	void	SetScaleBias(const F32 fScale, const F32 fBias) const;
	void	SetVelocityBuffer(const Display::C_Texture2D* pVelocityBuffer) const;

private:

	Display::C_PixelShader				m_PixelShaders[E_MotionBlurVelocityShader_Count];
	Display::C_PixelShaderParameter		m_Parameters[E_MotionBlurVelocityShader_Count];

	mutable const Display::C_Texture2D*	m_pVelocityBuffer;

	mutable E_MotionBlurVelocityShader	m_eShader;

	mutable F32							m_fScale;
	mutable F32							m_fBias;

};

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_MOTIONBLURVELOCITY_H
