#ifndef DEFERRED_PREPASS_SHADOWS_H
#define DEFERRED_PREPASS_SHADOWS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Deferred
{

enum E_ShadowType
{
	E_ShadowType_Simple = 0,
	E_ShadowType_Jitter,
	E_ShadowType_PCF,
	E_ShadowType_Variance,
	E_ShadowType_Exponential,
	E_ShadowType_Count
};

const U32 kShadowShaderCount = E_ShadowType_Count;

struct C_ShadowMapSetup
{
	Display::C_Matrix			m_mShadow;
	const Display::C_Texture2D*	m_pTexture;
	F32							m_fBegin;
	F32							m_fEnd;
};

class C_Shadow : public Display::I_Renderer
{
public:

	C_Shadow();
	~C_Shadow();

	bool OnCreate();
	void OnDestroy();

	void RenderSimpleShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias = 0.0f);
	void RenderJitteredShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias = 0.0f, const F32 fJitterScale = 1.0f);
	void RenderPCFShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias = 0.0f);
	void RenderVarianceShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fBias, const F32 fPower);
	void RenderExponentialShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fQ, const F32 fK, const F32 fScale, const F32 fBias);

	void RenderSimpleShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias = 0.0f);
	void RenderJitteredShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias = 0.0f, const F32 fJitterScale = 1.0f);
	void RenderPCFShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias = 0.0f);
	void RenderVarianceShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fBias, const F32 fPower);
	void RenderExponentialShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fQ, const F32 fK, const F32 fScale, const F32 fBias);

private:

	void Bind(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const U32 iShader, const Display::E_TextureFilter eFilter) const;
	void Unbind(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture) const;

	void Bind(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const U32 iShader, const Display::E_TextureFilter eFilter) const;
	void Unbind(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture) const;

private:

	Display::C_PixelShader					m_PixelShaders[kShadowShaderCount];
	
	Display::C_PixelShaderParameter			m_SizeParameter[kShadowShaderCount];
	Display::C_PixelShaderParameter			m_SettingsParameter[kShadowShaderCount];
	Display::C_PixelShaderParameter			m_DepthParameter[kShadowShaderCount];
	Display::C_PixelShaderParameterArray	m_WorldMatrixParameter[kShadowShaderCount];
	Display::C_PixelShaderParameterArray	m_ShadowMatrixParameter[kShadowShaderCount];

	Display::C_PixelShader					m_FadePixelShaders[kShadowShaderCount];
	
	Display::C_PixelShaderParameter			m_FadeSizeParameter[kShadowShaderCount];
	Display::C_PixelShaderParameter			m_FadeSettingsParameter[kShadowShaderCount];
	Display::C_PixelShaderParameter			m_FadeDepthParameter[kShadowShaderCount];
	Display::C_PixelShaderParameter			m_FadeParameter[kShadowShaderCount];
	Display::C_PixelShaderParameterArray	m_FadeWorldMatrixParameter[kShadowShaderCount];
	Display::C_PixelShaderParameterArray	m_FadeShadowMatrix0Parameter[kShadowShaderCount];
	Display::C_PixelShaderParameterArray	m_FadeShadowMatrix1Parameter[kShadowShaderCount];

	Display::C_Texture2D					m_NoiseTexture;

};

}; // namespace Deferred

#endif // DEFERRED_PREPASS_SHADOWS_H
