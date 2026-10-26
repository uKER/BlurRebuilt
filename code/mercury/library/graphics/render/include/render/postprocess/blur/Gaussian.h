#ifndef RENDER_POSTPROCESS_GAUSSIANBLUR_H
#define RENDER_POSTPROCESS_GAUSSIANBLUR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

class C_Manager;

class C_GaussianBlur : public I_PostProcess
{
public:

	C_GaussianBlur(C_Manager& manager) : I_PostProcess(manager), m_fDeviation(2.0f), m_bGamma(false), m_bBilinear(false) {}
	~C_GaussianBlur() {};

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}
	void Update() {}

	void SetDeviation(F32 fDeviation) const;
	void SetGammaEnable(const bool bGamma) const;
	void SetBilinear(const bool bBilinear) const;

	void Display() const;
	
private:
	void SetShader() const;
	void SetParameters() const;

	mutable Display::C_PixelShaderParameterArray		m_Taps;

	mutable F32											m_fDeviation;
	mutable bool										m_bGamma;
	mutable bool										m_bBilinear;

};

INLINE void C_GaussianBlur::SetDeviation(F32 fDeviation) const
{
	m_fDeviation = fDeviation;
}

INLINE void C_GaussianBlur::SetGammaEnable(const bool bGamma) const
{
	if(bGamma != m_bGamma)
	{
		m_bGamma = bGamma;
		SetShader();
		SetParameters();
	}
}

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_GAUSSIANBLUR_H
