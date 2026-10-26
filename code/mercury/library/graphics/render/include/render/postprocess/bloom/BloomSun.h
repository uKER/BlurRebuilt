#ifndef RENDER_POSTPROCESS_BLOOM_SUN_H
#define RENDER_POSTPROCESS_BLOOM_SUN_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Bloom/BloomInterface.h>
#include <Render/PostProcess/Blur/Gaussian.h>
#include <Render/PostProcess/Misc/SunRays.h>
#include <Render/PostProcess/Scale/GaussianScale.h>

namespace PostProcess
{

class C_BloomSun : public I_Bloom
{
public:

	C_BloomSun(C_Manager& manager);
	~C_BloomSun();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	void SetBloomSun(const bool bBloomSun) const;
	void SetBloomSunPosition(const Display::C_Float2& vSunPosition) const;
	void SetBloomSunSize(const Display::C_Float2& vSunSize) const;
	void SetBloomSunIntensity(const F32 fSunIntensity) const;

	const Display::C_Texture* Display(const BloomParameters& parameters) const;

public:

	Display::C_PixelShaderParameter			m_BloomCombineSunBloomSettings;
	Display::C_PixelShaderParameter			m_BloomCombineSunSunSettings;
	Display::C_PixelShaderParameter			m_BloomCombineSunGammaBloomSettings;
	Display::C_PixelShaderParameter			m_BloomCombineSunGammaSunSettings;

	C_SunRays								m_SunRays;

	mutable bool							m_bBloomSun;

};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_BLOOM_SUN_H
