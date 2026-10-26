#include "stdafx.h"
#include "Deferred/Prepass/Shadow.h"

#include <Display/Buffer/VertexDeclarations.h>
#include <Render/Shaders/Constants.h>
#include <Render/State/RenderState.h>
#include <System/Misc/Random.h>

EXTERN_SHADER(psShadowPrepassSimple);
EXTERN_SHADER(psShadowPrepassJitter);
EXTERN_SHADER(psShadowPrepassPCF);
EXTERN_SHADER(psShadowPrepassVariance);
EXTERN_SHADER(psShadowPrepassExponential);
EXTERN_SHADER(psShadowFadePrepassSimple);
EXTERN_SHADER(psShadowFadePrepassJitter);
EXTERN_SHADER(psShadowFadePrepassPCF);
EXTERN_SHADER(psShadowFadePrepassVariance);
EXTERN_SHADER(psShadowFadePrepassExponential);

namespace Deferred
{

const U32 g_NoiseTextureSize = 64;

C_Shadow::C_Shadow() : Display::I_Renderer("Shadow")
{
};

C_Shadow::~C_Shadow()
{
	DestroyRenderer();
}

bool C_Shadow::OnCreate()
{
	RESOURCE_MEMORY_ID("Deferred::C_Shadow");

	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowPrepassSimple", 0, &m_PixelShaders[0], REF_SHADER(psShadowPrepassSimple)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowPrepassJitter", 0, &m_PixelShaders[1], REF_SHADER(psShadowPrepassJitter)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowPrepassPCF", 0, &m_PixelShaders[2], REF_SHADER(psShadowPrepassPCF)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowPrepassVariance", 0, &m_PixelShaders[3], REF_SHADER(psShadowPrepassVariance)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowPrepassExponential", 0, &m_PixelShaders[4], REF_SHADER(psShadowPrepassExponential)))
		return false;

	for (U32 i = 0; i < kShadowShaderCount; ++i)
	{
		m_SizeParameter[i]			= m_PixelShaders[i].GetParameter(CP_RENDER_SETTINGS, "g_vSize");
		m_SettingsParameter[i]		= m_PixelShaders[i].GetParameter(CP_RENDER_SETTINGS_1, "g_vSettings");
		m_DepthParameter[i]			= m_PixelShaders[i].GetParameter(CP_RENDER_SETTINGS_2, "g_vDepth");
		m_WorldMatrixParameter[i]	= m_PixelShaders[i].GetParameterArray(CP_RENDER_SETTINGS_4, "g_mWorld", 4);
		m_ShadowMatrixParameter[i]	= m_PixelShaders[i].GetParameterArray(CP_RENDER_SETTINGS_8, "g_mShadow", 4);
	}

	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowFadePrepassSimple", 0, &m_FadePixelShaders[0], REF_SHADER(psShadowFadePrepassSimple)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowFadePrepassJitter", 0, &m_FadePixelShaders[1], REF_SHADER(psShadowFadePrepassJitter)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowFadePrepassPCF", 0, &m_FadePixelShaders[2], REF_SHADER(psShadowFadePrepassPCF)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowFadePrepassVariance", 0, &m_FadePixelShaders[3], REF_SHADER(psShadowFadePrepassVariance)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psShadowFadePrepassExponential", 0, &m_FadePixelShaders[4], REF_SHADER(psShadowFadePrepassExponential)))
		return false;

	for (U32 i = 0; i < kShadowShaderCount; ++i)
	{
		m_FadeSizeParameter[i]			= m_FadePixelShaders[i].GetParameter(CP_RENDER_SETTINGS, "g_vSize");
		m_FadeSettingsParameter[i]		= m_FadePixelShaders[i].GetParameter(CP_RENDER_SETTINGS_1, "g_vSettings");
		m_FadeDepthParameter[i]			= m_FadePixelShaders[i].GetParameter(CP_RENDER_SETTINGS_2, "g_vDepth");
		m_FadeParameter[i]				= m_FadePixelShaders[i].GetParameter(CP_RENDER_SETTINGS_3, "g_vFade");
		m_FadeWorldMatrixParameter[i]	= m_FadePixelShaders[i].GetParameterArray(CP_RENDER_SETTINGS_4, "g_mWorld", 4);
		m_FadeShadowMatrix0Parameter[i]	= m_FadePixelShaders[i].GetParameterArray(CP_RENDER_SETTINGS_8, "g_mShadow0", 4);
		m_FadeShadowMatrix1Parameter[i]	= m_FadePixelShaders[i].GetParameterArray(CP_RENDER_SETTINGS_12, "g_mShadow1", 4);
	}

	// noise texture
	{
		if (!Display::Device()->CreateTexture2D(g_NoiseTextureSize, g_NoiseTextureSize, 1, Display::E_Usage_Managed, Display::E_Format_A8R8G8B8, &m_NoiseTexture))
			return false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_NoiseTexture, "_ShadowNoise");
		#endif

		Util::C_Random random;
		random.Seed(123456);

		// fill the noise texture with random values
		Display::C_LockedRect rect = m_NoiseTexture.Lock(0);
		if(rect.pBits)
		{
			for(U32 y = 0; y < g_NoiseTextureSize; ++y)
			{	
				U32 *pData = reinterpret_cast<U32*>(rect.pBits);
				for(U32 x = 0; x < g_NoiseTextureSize; ++x)
				{
					const F32 fAngle	= random.AngRand();
					const F32 fSin		= sinf(fAngle) * 0.5f + 0.5f;
					const F32 fCos		= cosf(fAngle) * 0.5f + 0.5f;
					const U8 iSin		= (U8)(fSin * 255.0f);
					const U8 iCos		= (U8)(fCos * 255.0f);
					pData[x] = (iSin << 24) + (iCos << 16) + (iSin << 8) + iCos;
				}
				rect.pBits += rect.pitch;
			}
			m_NoiseTexture.Unlock(0);
		}
	}

	return true;
}

void C_Shadow::OnDestroy()
{
	for (U32 i = 0; i < kShadowShaderCount; ++i)
	{
		Display::Device()->Destroy(&m_PixelShaders[i]);
		Display::Device()->Destroy(&m_FadePixelShaders[i]);
	}

	Display::Device()->Destroy(&m_NoiseTexture);
}

void C_Shadow::RenderSimpleShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias)
{
	PERF_EVENT("RenderSimpleShadowMap");

	const Display::C_Float4	vSettings = Display::C_Float4(fDepthBias, 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_Simple], (F32*)&vSettings);

#if PLATFORM_PS3
	Bind(pShadowMapSetup, pDepthTexture, E_ShadowType_Simple, Display::E_TextureFilter_Linear);
#else
	Bind(pShadowMapSetup, pDepthTexture, E_ShadowType_Simple, Display::E_TextureFilter_Point);
#endif

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup, pDepthTexture);
}

void C_Shadow::RenderJitteredShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias, const F32 fJitterScale)
{
	PERF_EVENT("RenderJitteredShadowMap");

	const F32 fScreenWidth = (F32)Display::Device()->GetViewport().GetW();
	const F32 fScreenHeight = (F32)Display::Device()->GetViewport().GetH();
	const Display::C_Float4	vSettings = Display::C_Float4(fDepthBias, fJitterScale, fScreenWidth / (F32)g_NoiseTextureSize, fScreenHeight / (F32)g_NoiseTextureSize);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_Jitter], (F32*)&vSettings);

#if PLATFORM_PS3
	Bind(pShadowMapSetup, pDepthTexture, E_ShadowType_Jitter, Display::E_TextureFilter_Linear);
#else
	Bind(pShadowMapSetup, pDepthTexture, E_ShadowType_Jitter, Display::E_TextureFilter_Point);
#endif

	Display::Device()->SetTexture(3, &m_NoiseTexture);
	Display::Device()->SetTextureAddress(3, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureFilter(3, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(3, Display::E_Bool_False);

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup, pDepthTexture);
}

void C_Shadow::RenderPCFShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias)
{
	PERF_EVENT("RenderPCFShadowMap");

	const Display::C_Float4	vSettings = Display::C_Float4(fDepthBias, 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_PCF], (F32*)&vSettings);

#if PLATFORM_PS3
	Bind(pShadowMapSetup, pDepthTexture, E_ShadowType_PCF, Display::E_TextureFilter_Linear);
#else
	Bind(pShadowMapSetup, pDepthTexture, E_ShadowType_PCF, Display::E_TextureFilter_Point);
#endif

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup, pDepthTexture);
}

void C_Shadow::RenderVarianceShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fBias, const F32 fPower)
{
	PERF_EVENT("RenderVarianceShadowMap");

	const Display::C_Float4	vSettings = Display::C_Float4(fBias, fPower, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_Variance], (F32*)&vSettings);

	Bind(pShadowMapSetup, pDepthTexture, E_ShadowType_Variance, Display::E_TextureFilter_Linear);

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup, pDepthTexture);
}

void C_Shadow::RenderExponentialShadowMap(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const F32 fQ, const F32 fK, const F32 fScale, const F32 fBias)
{
	PERF_EVENT("RenderExponentialShadowMap");

	const Display::C_Float4	vSettings = Display::C_Float4(fQ, fK, fScale, fBias);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_Variance], (F32*)&vSettings);

	Bind(pShadowMapSetup, pDepthTexture, E_ShadowType_Variance, Display::E_TextureFilter_Point);

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup, pDepthTexture);
}

void C_Shadow::RenderSimpleShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias)
{
	PERF_EVENT("RenderSimpleShadowMapFade");

	const Display::C_Float4	vSettings = Display::C_Float4(fDepthBias, 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_Simple], (F32*)&vSettings);

#if PLATFORM_PS3
	Bind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture, E_ShadowType_Simple, Display::E_TextureFilter_Linear);
#else
	Bind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture, E_ShadowType_Simple, Display::E_TextureFilter_Point);
#endif

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture);
}

void C_Shadow::RenderJitteredShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias, const F32 fJitterScale)
{
	PERF_EVENT("RenderJitteredShadowMapFade");

	const F32 fScreenWidth = (F32)Display::Device()->GetViewport().GetW();
	const F32 fScreenHeight = (F32)Display::Device()->GetViewport().GetH();
	const Display::C_Float4	vSettings = Display::C_Float4(fDepthBias, fJitterScale, fScreenWidth / (F32)g_NoiseTextureSize, fScreenHeight / (F32)g_NoiseTextureSize);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_Jitter], (F32*)&vSettings);

#if PLATFORM_PS3
	Bind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture, E_ShadowType_Jitter, Display::E_TextureFilter_Linear);
#else
	Bind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture, E_ShadowType_Jitter, Display::E_TextureFilter_Point);
#endif

	Display::Device()->SetTexture(3, &m_NoiseTexture);
	Display::Device()->SetTextureAddress(3, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureFilter(3, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(3, Display::E_Bool_False);

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture);
}

void C_Shadow::RenderPCFShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fDepthBias)
{
	PERF_EVENT("RenderPCFShadowMapFade");

	const Display::C_Float4	vSettings = Display::C_Float4(fDepthBias, 0.0f, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_PCF], (F32*)&vSettings);

#if PLATFORM_PS3
	Bind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture, E_ShadowType_PCF, Display::E_TextureFilter_Linear);
#else
	Bind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture, E_ShadowType_PCF, Display::E_TextureFilter_Point);
#endif

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture);
}

void C_Shadow::RenderVarianceShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fBias, const F32 fPower)
{
	PERF_EVENT("RenderVarianceShadowMapFade");

	const Display::C_Float4	vSettings = Display::C_Float4(fBias, fPower, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_Variance], (F32*)&vSettings);

	Bind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture, E_ShadowType_Variance, Display::E_TextureFilter_Linear);

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture);
}

void C_Shadow::RenderExponentialShadowMapFade(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const F32 fQ, const F32 fK, const F32 fScale, const F32 fBias)
{
	PERF_EVENT("RenderExponentialShadowMapFade");

	const Display::C_Float4	vSettings = Display::C_Float4(fQ, fK, fScale, fBias);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter[E_ShadowType_Variance], (F32*)&vSettings);

	Bind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture, E_ShadowType_Variance, Display::E_TextureFilter_Point);

	Display::Quad::DrawFullScreen();

	Unbind(pShadowMapSetup0, pShadowMapSetup1, pDepthTexture);
}

void C_Shadow::Bind(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture, const U32 iShader, const Display::E_TextureFilter eFilter) const
{
	Maths::HMat44 mTexture;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);
		mTemp._11 = +2.0f;
		mTemp._22 = -2.0f;
		mTemp._41 = -1.0f;
		mTemp._42 = +1.0f;
		mTexture = Maths::Load<Maths::HMat44>(&mTemp);
	}

	const Maths::HMat44 mDeviceToWorld = Display::Device()->GetViewSettings3D().GetWorldToDevice().Inverse();

	Display::C_Matrix mWorld;
	Maths::Store(mTexture * mDeviceToWorld, &mWorld);	

	Display::Device()->SetPixelShaderParameterArrayF(m_WorldMatrixParameter[iShader], (F32*)&mWorld);

	const F32 fShadowWidth = (F32)pShadowMapSetup->m_pTexture->GetWidth();
	const F32 fShadowHeight = (F32)pShadowMapSetup->m_pTexture->GetHeight();
	const Display::C_Float4	vSize		= Display::C_Float4(fShadowWidth, fShadowHeight, 1.0f / fShadowWidth, 1.0f / fShadowHeight);
	Display::Device()->SetPixelShaderParameterF(m_SizeParameter[iShader], (F32*)&vSize);

	const Display::C_Float4	vDepth		= Render::C_RenderState::Instance().GetDepthScaleShaderParameter();	
	Display::Device()->SetPixelShaderParameterF(m_DepthParameter[iShader], (F32*)&vDepth);

	Display::Device()->SetPixelShaderParameterArrayF(m_ShadowMatrixParameter[iShader], (F32*)&pShadowMapSetup->m_mShadow);

	Display::Device()->SetPixelShader(&m_PixelShaders[iShader]);

	Display::Device()->SetTexture(0, pDepthTexture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

	Display::Device()->SetTexture(1, pShadowMapSetup->m_pTexture);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, eFilter);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);
}

void C_Shadow::Unbind(const C_ShadowMapSetup* pShadowMapSetup, const Display::C_Texture2D* pDepthTexture) const
{
	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);

	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);
}

void C_Shadow::Bind(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture, const U32 iShader, const Display::E_TextureFilter eFilter) const
{
	Maths::HMat44 mTexture;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);
		mTemp._11 = +2.0f;
		mTemp._22 = -2.0f;
		mTemp._41 = -1.0f;
		mTemp._42 = +1.0f;
		mTexture = Maths::Load<Maths::HMat44>(&mTemp);
	}

	const Maths::HMat44 mDeviceToWorld = Display::Device()->GetViewSettings3D().GetWorldToDevice().Inverse();

	Display::C_Matrix mWorld;
	Maths::Store(mTexture * mDeviceToWorld, &mWorld);	

	Display::Device()->SetPixelShaderParameterArrayF(m_FadeWorldMatrixParameter[iShader], (F32*)&mWorld);

	const F32 fShadowWidth = (F32)pShadowMapSetup0->m_pTexture->GetWidth();
	const F32 fShadowHeight = (F32)pShadowMapSetup0->m_pTexture->GetHeight();
	const Display::C_Float4	vSize		= Display::C_Float4(fShadowWidth, fShadowHeight, 1.0f / fShadowWidth, 1.0f / fShadowHeight);
	Display::Device()->SetPixelShaderParameterF(m_FadeSizeParameter[iShader], (F32*)&vSize);

	const Display::C_Float4	vDepth		= Render::C_RenderState::Instance().GetDepthScaleShaderParameter();	
	Display::Device()->SetPixelShaderParameterF(m_FadeDepthParameter[iShader], (F32*)&vDepth);

	const Display::C_Float4	vFade		= Display::C_Float4(pShadowMapSetup1->m_fBegin, 1.0f / (pShadowMapSetup0->m_fEnd - pShadowMapSetup1->m_fBegin), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_FadeParameter[iShader], (F32*)&vFade);

	Display::Device()->SetPixelShaderParameterArrayF(m_FadeShadowMatrix0Parameter[iShader], (F32*)&pShadowMapSetup0->m_mShadow);
	Display::Device()->SetPixelShaderParameterArrayF(m_FadeShadowMatrix1Parameter[iShader], (F32*)&pShadowMapSetup1->m_mShadow);

	Display::Device()->SetPixelShader(&m_FadePixelShaders[iShader]);

	Display::Device()->SetTexture(0, pDepthTexture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

	Display::Device()->SetTexture(1, pShadowMapSetup0->m_pTexture);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, eFilter);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);

	Display::Device()->SetTexture(2, pShadowMapSetup1->m_pTexture);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(2, eFilter);
	Display::Device()->SetTextureGamma(2, Display::E_Bool_False);
}

void C_Shadow::Unbind(const C_ShadowMapSetup* pShadowMapSetup0, const C_ShadowMapSetup* pShadowMapSetup1, const Display::C_Texture2D* pDepthTexture) const
{
	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);

	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	Display::Device()->SetTexture(2, 0);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(2, Display::E_Bool_Default);
}

}; // namespace Deferred
