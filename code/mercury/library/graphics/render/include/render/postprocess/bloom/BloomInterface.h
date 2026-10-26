#ifndef RENDER_POSTPROCESS_BLOOM_INTERFACE_H
#define RENDER_POSTPROCESS_BLOOM_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Blur/Gaussian.h>
#include <Render/PostProcess/Misc/SunRays.h>
#include <Render/PostProcess/Scale/GaussianScale.h>

namespace PostProcess
{

struct BloomParameters
{
	BloomParameters();
	~BloomParameters();

	void	Update();
	
	bool	bBloomGamma;
	U32		nDownsamples;
	F32		fBloomDeviation;
	bool	bDownsample[4];
	bool	bBloom[4];
	F32		fDownsampleScale[4];
	F32		fBloomScale[4];
	F32		fCombinedScale[4];
};

class I_Bloom : public I_PostProcess
{
public:

	I_Bloom(C_Manager& manager);
	~I_Bloom();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	void SetBloomGamma(const bool bBloomGamma) const;
	bool GetBloomGamma() const;

	virtual const Display::C_Texture* Display(const BloomParameters& parameters) const = 0;

protected:

	void CalculateBloomTextures(const BloomParameters& parameters, RenderTexturesConst& bloomTextures) const;
	
	void UpdateParameters(BloomParameters& parameters) const;

protected:

	Display::C_PixelShader					m_BloomCombine;
	Display::C_PixelShader					m_BloomCombineGamma;
	
	C_GaussianScale							m_BloomDownsample;
	C_GaussianBlur							m_BloomFilter;

	mutable bool							m_bBloomGamma;
};

INLINE void I_Bloom::SetBloomGamma(const bool bBloomGamma) const
{
	if(m_bBloomGamma != bBloomGamma)
	{
		m_bBloomGamma = bBloomGamma;
	
		m_BloomFilter.SetGammaEnable(bBloomGamma);
		m_BloomDownsample.SetGammaEnable(bBloomGamma);
	}
}

INLINE bool I_Bloom::GetBloomGamma() const
{
	return m_bBloomGamma;
}

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_BLOOM_INTERFACE_H
