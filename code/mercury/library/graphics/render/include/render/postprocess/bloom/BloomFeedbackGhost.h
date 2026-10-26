#ifndef RENDER_POSTPROCESS_BLOOM_FEEDBACK_GHOST_H
#define RENDER_POSTPROCESS_BLOOM_FEEDBACK_GHOST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Bloom/BloomInterface.h>
#include <Render/PostProcess/Blur/Gaussian.h>
#include <Render/PostProcess/Scale/GaussianScale.h>

namespace PostProcess
{

class C_BloomFeedbackGhost : public I_Bloom
{
public:

	C_BloomFeedbackGhost(C_Manager& manager);
	~C_BloomFeedbackGhost();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	const Display::C_Texture* Display(const BloomParameters& parameters) const;

	void SetFeedbackClear() const;
	void SetFeedbackFade(const F32 fFeedbackFade) const;

	void SetScales(const Display::C_Float2& vScale0, const Display::C_Float2& vScale1, const Display::C_Float2& vScale2, const Display::C_Float2& vScale3) const;
	void SetColours(const Display::C_Float4& vColour0, const Display::C_Float4& vColour1, const Display::C_Float4& vColour2, const Display::C_Float4& vColour3) const;

	const Display::C_Texture2D* GetFeedbackTexture(const S32 iView) const;

private:

	Display::C_PixelShaderParameter	m_BloomCombineFeedbackBloomSettings;
	Display::C_PixelShaderParameter	m_BloomCombineFeedbackFeedbackSettings;
	Display::C_PixelShaderParameter	m_BloomCombineFeedbackGammaBloomSettings;
	Display::C_PixelShaderParameter	m_BloomCombineFeedbackGammaFeedbackSettings;

	RenderTextures					m_FeedbackTextures;

	mutable F32						m_FeedbackFade;
	mutable bool					m_FeedbackClear;
	mutable	Display::C_Float4		m_Scales01;
	mutable	Display::C_Float4		m_Scales23;
	mutable Display::C_Float4		m_Colour0;
	mutable Display::C_Float4		m_Colour1;
	mutable Display::C_Float4		m_Colour2;
	mutable Display::C_Float4		m_Colour3;

	Display::C_PixelShader			m_BloomGhost;
	Display::C_PixelShader			m_BloomGhostGamma;

	Display::C_PixelShaderParameter	m_BloomGhostScale01;
	Display::C_PixelShaderParameter	m_BloomGhostScale23;
	Display::C_PixelShaderParameter	m_BloomGhostColour0;
	Display::C_PixelShaderParameter	m_BloomGhostColour1;
	Display::C_PixelShaderParameter	m_BloomGhostColour2;
	Display::C_PixelShaderParameter	m_BloomGhostColour3;
	Display::C_PixelShaderParameter	m_BloomGhostGammaScale01;
	Display::C_PixelShaderParameter	m_BloomGhostGammaScale23;
	Display::C_PixelShaderParameter	m_BloomGhostGammaColour0;
	Display::C_PixelShaderParameter	m_BloomGhostGammaColour1;
	Display::C_PixelShaderParameter	m_BloomGhostGammaColour2;
	Display::C_PixelShaderParameter	m_BloomGhostGammaColour3;

	Display::C_Texture2D			m_MaskTexture;

};

INLINE const Display::C_Texture2D* C_BloomFeedbackGhost::GetFeedbackTexture(const S32 iView) const
{
	if(iView > -1 && iView < m_FeedbackTextures.Size())
		return &m_FeedbackTextures[iView]->GetTexture();
	else
		return NULL;
}

INLINE void C_BloomFeedbackGhost::SetFeedbackClear() const
{
	m_FeedbackClear = true;
}

INLINE void C_BloomFeedbackGhost::SetFeedbackFade(const F32 fFeedbackFade) const
{
	m_FeedbackFade = fFeedbackFade;
}

INLINE void C_BloomFeedbackGhost::SetScales(const Display::C_Float2& vScale0, const Display::C_Float2& vScale1, const Display::C_Float2& vScale2, const Display::C_Float2& vScale3) const
{
	m_Scales01.x = vScale0.x;
	m_Scales01.y = vScale0.y;
	m_Scales01.z = vScale1.x;
	m_Scales01.w = vScale1.y;
	m_Scales23.x = vScale2.x;
	m_Scales23.y = vScale2.y;
	m_Scales23.z = vScale3.x;
	m_Scales23.w = vScale3.y;
}

INLINE void C_BloomFeedbackGhost::SetColours(const Display::C_Float4& vColour0, const Display::C_Float4& vColour1, const Display::C_Float4& vColour2, const Display::C_Float4& vColour3) const
{
	m_Colour0 = vColour0;
	m_Colour1 = vColour1;
	m_Colour2 = vColour2;
	m_Colour3 = vColour3;
}

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_BLOOM_FEEDBACK_GHOST_H
