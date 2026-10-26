#ifndef RENDER_POSTPROCESS_BLOOM_GHOST_H
#define RENDER_POSTPROCESS_BLOOM_GHOST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Bloom/BloomInterface.h>
#include <Render/PostProcess/Blur/Gaussian.h>
#include <Render/PostProcess/Misc/SunRays.h>
#include <Render/PostProcess/Scale/GaussianScale.h>

namespace PostProcess
{

class C_BloomGhost : public I_Bloom
{
public:

	C_BloomGhost(C_Manager& manager);
	~C_BloomGhost();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	const Display::C_Texture* Display(const BloomParameters& parameters) const;

	void SetScales(const Display::C_Float2& vScale0, const Display::C_Float2& vScale1, const Display::C_Float2& vScale2, const Display::C_Float2& vScale3) const;
	void SetColours(const Display::C_Float4& vColour0, const Display::C_Float4& vColour1, const Display::C_Float4& vColour2, const Display::C_Float4& vColour3) const;

private:

	Display::C_PixelShaderParameter			m_BloomCombineBloomSettings;
	Display::C_PixelShaderParameter			m_BloomCombineGammaBloomSettings;

	Display::C_PixelShader					m_BloomGhost;
	Display::C_PixelShader					m_BloomGhostGamma;

	Display::C_PixelShaderParameter			m_BloomGhostScale01;
	Display::C_PixelShaderParameter			m_BloomGhostScale23;
	Display::C_PixelShaderParameter			m_BloomGhostColour0;
	Display::C_PixelShaderParameter			m_BloomGhostColour1;
	Display::C_PixelShaderParameter			m_BloomGhostColour2;
	Display::C_PixelShaderParameter			m_BloomGhostColour3;
	Display::C_PixelShaderParameter			m_BloomGhostGammaScale01;
	Display::C_PixelShaderParameter			m_BloomGhostGammaScale23;
	Display::C_PixelShaderParameter			m_BloomGhostGammaColour0;
	Display::C_PixelShaderParameter			m_BloomGhostGammaColour1;
	Display::C_PixelShaderParameter			m_BloomGhostGammaColour2;
	Display::C_PixelShaderParameter			m_BloomGhostGammaColour3;

	mutable	Display::C_Float4				m_Scales01;
	mutable	Display::C_Float4				m_Scales23;
	mutable Display::C_Float4				m_Colour0;
	mutable Display::C_Float4				m_Colour1;
	mutable Display::C_Float4				m_Colour2;
	mutable Display::C_Float4				m_Colour3;

	Display::C_Texture2D					m_MaskTexture;

};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_BLOOM_GHOST_H
