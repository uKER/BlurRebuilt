#ifndef RENDER_POSTPROCESS_MOTIONBLUR_H
#define RENDER_POSTPROCESS_MOTIONBLUR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

class C_Motion : public I_PostProcess
{
public:

	C_Motion(C_Manager& manager) : I_PostProcess(manager), m_fShutterAngle(0.5f) {}
	~C_Motion() {};

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}
	void Update() {}

	void Display() const;
	F32 GetMotionAmount() const { return m_vMotionAmount.Length(); }
	void SetShutterAngle(F32 amount) { m_fShutterAngle = amount; }

protected:
	void SetNumSamples(U32 nSamples) const;
	
	mutable F32						m_fShutterAngle;
private:
	void SetConstants() const;
	

	mutable U32						m_nSamples;
	mutable F32						m_fSamplesMinusOneRcp;
	mutable Maths::HVec3			m_vMotionAmount;
	
	Display::C_PixelShader		m_Shader;
};

INLINE void C_Motion::SetNumSamples(U32 nSamples) const
{
	m_nSamples = nSamples;
	m_fSamplesMinusOneRcp = 1.0f / (F32)(nSamples - 1);
}

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_GUASSIANBLUR_H
