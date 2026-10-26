#ifndef RENDER_POSTPROCESS_GUASSIANSCALE_H
#define RENDER_POSTPROCESS_GUASSIANSCALE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

class C_GaussianScale : public I_PostProcess
{
public:

	C_GaussianScale( C_Manager& manager ) : I_PostProcess( manager ), m_fMultiplier(1.0f), m_Downsample(2), m_bConstants(true), m_bGamma(false) {}
	~C_GaussianScale() {}

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}

	void Update() {}

	void SetMultiplier(F32 fMultiplier) const;
	void SetDownsample(S32 downsample) const;
	void SetGammaEnable(const bool bGamma) const;
	void SetConstants(const bool bConstants) const;
	
	void Display() const;

private:
	void SetShader() const;
	void SetParameters() const;

	mutable Display::C_PixelShaderParameterArray		m_Taps;

	mutable F32											m_fMultiplier;
	mutable S32											m_Downsample;
	mutable bool										m_bConstants;
	mutable bool										m_bGamma;
};

INLINE void C_GaussianScale::SetMultiplier( F32 fMultiplier) const
{
	m_fMultiplier = fMultiplier;
}

INLINE void C_GaussianScale::SetDownsample(S32 downsample) const
{
	m_Downsample = downsample;
}

INLINE void C_GaussianScale::SetGammaEnable(const bool bGamma) const
{
	if(bGamma != m_bGamma)
	{
		m_bGamma = bGamma;
		SetShader();
		SetParameters();
	}
}

INLINE void C_GaussianScale::SetConstants(const bool bConstants) const
{
	m_bConstants = bConstants;
}

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_GUASSIANSCALE_H
