#ifndef RENDER_POSTPROCESS_BLOOM_FEEDBACK_H
#define RENDER_POSTPROCESS_BLOOM_FEEDBACK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Bloom/BloomInterface.h>
#include <Render/PostProcess/Blur/Gaussian.h>
#include <Render/PostProcess/Scale/GaussianScale.h>

namespace PostProcess
{

class C_BloomFeedback : public I_Bloom
{
public:

	C_BloomFeedback(C_Manager& manager);
	~C_BloomFeedback();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	const Display::C_Texture* Display(const BloomParameters& parameters) const;

	void SetFeedbackClear() const;
	void SetFeedbackFade(const F32 fFeedbackFade) const;

	const Display::C_Texture2D* GetFeedbackTexture(const S32 iView) const;

private:

	Display::C_PixelShaderParameter	m_BloomCombineFeedbackBloomSettings;
	Display::C_PixelShaderParameter	m_BloomCombineFeedbackFeedbackSettings;
	Display::C_PixelShaderParameter	m_BloomCombineFeedbackGammaBloomSettings;
	Display::C_PixelShaderParameter	m_BloomCombineFeedbackGammaFeedbackSettings;

	RenderTextures					m_FeedbackTextures;

	mutable bool					m_FeedbackClear;
	mutable F32						m_FeedbackFade;

};

INLINE const Display::C_Texture2D* C_BloomFeedback::GetFeedbackTexture(const S32 iView) const
{
	if(iView < m_FeedbackTextures.Size())
		return &m_FeedbackTextures[iView]->GetTexture();
	else
		return NULL;
}

INLINE void C_BloomFeedback::SetFeedbackFade(const F32 fFeedbackFade) const
{
	m_FeedbackFade = fFeedbackFade;
}

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_BLOOM_FEEDBACK_H
