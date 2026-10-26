#ifndef RENDER_POSTPROCESS_BLOOM_H
#define RENDER_POSTPROCESS_BLOOM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Bloom/BloomInterface.h>
#include <Render/PostProcess/Blur/Gaussian.h>
#include <Render/PostProcess/Misc/SunRays.h>
#include <Render/PostProcess/Scale/GaussianScale.h>

namespace PostProcess
{

class C_Bloom : public I_Bloom
{
public:

	C_Bloom(C_Manager& manager);
	~C_Bloom();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}
	const Display::C_Texture* Display(const BloomParameters& parameters) const;

private:

	Display::C_PixelShaderParameter			m_BloomCombineBloomSettings;
	Display::C_PixelShaderParameter			m_BloomCombineGammaBloomSettings;

};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_BLOOM_H
