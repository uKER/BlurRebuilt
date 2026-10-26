#include "stdafx.h"
#include "Effect/PostProcess/DoFFgBg.h"

#include "Render/PostProcess/Util.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"

EXTERN_SHADER(psDoFFgBgBlur);
EXTERN_SHADER(psDoFFgBgCombine);
EXTERN_SHADER(psDoFFgBgDownsample);
EXTERN_SHADER(psDoFFgBgNear);

namespace PostProcess
{

Debug::Bool		g_debugDoFFgBgOverride			("Graphics.Effect.PostProcess.DepthOfFieldFgBg.Override",			false);
Debug::Bool		g_debugDoFFgBgBaseOverride		("Graphics.Effect.PostProcess.DepthOfFieldFgBg.BaseOverride",		false);
Debug::Float	g_debugDoFFgBgNearStartDistance	("Graphics.Effect.PostProcess.DepthOfFieldFgBg.NearStartDistance",	0.00f,		Debug::Range(0.0f, 4000.0f));
Debug::Float	g_debugDoFFgBgNearEndDistance	("Graphics.Effect.PostProcess.DepthOfFieldFgBg.NearEndDistance",	0.01f,		Debug::Range(0.0f, 4000.0f));
Debug::Float	g_debugDoFFgBgFarStartDistance	("Graphics.Effect.PostProcess.DepthOfFieldFgBg.FarStartDistance",	3999.99f,	Debug::Range(0.0f, 4000.0f));
Debug::Float	g_debugDoFFgBgFarEndDistance	("Graphics.Effect.PostProcess.DepthOfFieldFgBg.FarEndDistance",		4000.00f,	Debug::Range(0.0f, 4000.0f));
Debug::Float	g_debugDoFFgBgFarScale			("Graphics.Effect.PostProcess.DepthOfFieldFgBg.FarScale",			1.00f,		Debug::Range(0.0f, 1.0f));

Debug::Float	g_debugDoFFgBgDistance0			("Graphics.Effect.PostProcess.DepthOfFieldFgBg.Distance0",			0.30f,		Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugDoFFgBgDistance1			("Graphics.Effect.PostProcess.DepthOfFieldFgBg.Distance1",			0.30f,		Debug::Range(0.0f, 1.0f));

Debug::Float	g_debugDoFFgBgMaxCoC			("Graphics.Effect.PostProcess.DepthOfFieldFgBg.MaxCoC",				0.01f,		Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugDoFFgBgFarClipScale		("Graphics.Effect.PostProcess.DepthOfFieldFgBg.FarClipScale",		0.10f,		Debug::Range(0.0f, 1.0f));

extern Debug::Bool g_debugDepthOfField;

extern Debug::Logger& g_Log;

C_DoFFgBg::C_DoFFgBg(C_Manager& manager) : I_PostProcess(manager), m_bEnabled(true), m_pTextureBuffer(0), m_iTextureBufferSize(0)
{
}

C_DoFFgBg::~C_DoFFgBg()
{
	m_DepthOfFieldSettings.Clear();
}

bool C_DoFFgBg::Create()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFFgBgBlur", 0, &m_BlurShader, REF_SHADER(psDoFFgBgBlur)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFFgBgCombine", 0, &m_CombineShader, REF_SHADER(psDoFFgBgCombine)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFFgBgDownsample", 0, &m_DownsampleShader, REF_SHADER(psDoFFgBgDownsample)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDoFFgBgNear", 0, &m_NearShader, REF_SHADER(psDoFFgBgNear)))
		return false;

	m_BlurSettingsParameter			= m_BlurShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_CombineSettingsParameter		= m_CombineShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_CombineScaleParameter			= m_CombineShader.GetParameter(CP_RENDER_SETTINGS_1, "g_vScale");
	m_CombineBiasParameter			= m_CombineShader.GetParameter(CP_RENDER_SETTINGS_2, "g_vBias");
	m_CombineFarParameter			= m_CombineShader.GetParameter(CP_RENDER_SETTINGS_3, "g_vFar");
	m_CombineDepthParameter			= m_CombineShader.GetParameter(CP_RENDER_SETTINGS_4, "g_vDepth");
	m_DownsampleSettingsParameter	= m_DownsampleShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");
	m_DownsampleDepthParameter		= m_DownsampleShader.GetParameter(CP_RENDER_SETTINGS_1, "g_vDepth");

	return true;
}

bool C_DoFFgBg::CreateVolatile()
{
	Display::C_SurfaceParams surfaceParams;

	U8* pCurrentTextureBuffer = m_pTextureBuffer;

	const Display::E_Format eFormat = Display::E_Format_Unknown;

	U8* pQuarterSizeAddress	= m_pTextureBuffer;
	U8* pDownsampleAddress	= m_pTextureBuffer;
	U8* pSmallBlurAddress	= m_pTextureBuffer;
	U8* pLargeBlurAddress	= m_pTextureBuffer;

	U32 iQuarterTextureSize = m_iTextureBufferSize;
	U32 iSixteenthTextureSize = m_iTextureBufferSize;

	U32 flags = Display::E_Usage_Colour | Display::E_Usage_Uint;
	if(m_pTextureBuffer)
	{
		flags |= Display::E_Usage_Indirect;
		iQuarterTextureSize = m_Manager.GetMaximumTextureSize(2, 2, eFormat);
		iSixteenthTextureSize = m_Manager.GetMaximumTextureSize(4, 4, eFormat);
		U32 iTotalSize = iSixteenthTextureSize * 3;
		if(iTotalSize > m_iTextureBufferSize)
		{
			LOGERROR(g_Log, "Texture buffer size not large enough\n");
			return false;
		}
		pSmallBlurAddress = pDownsampleAddress + iSixteenthTextureSize;
		pLargeBlurAddress = pSmallBlurAddress + iSixteenthTextureSize;
	}

	if(!m_Manager.CreateUserRenderTextures(4, 4, flags, eFormat, &surfaceParams, m_DownsampleRenderTextures, pDownsampleAddress, iSixteenthTextureSize))
		return false;
	if(!m_Manager.CreateUserRenderTextures(4, 4, flags, eFormat, &surfaceParams, m_SmallBlurRenderTextures, pSmallBlurAddress, iSixteenthTextureSize))
		return false;
	if(!m_Manager.CreateUserRenderTextures(4, 4, flags, eFormat, &surfaceParams, m_LargeBlurRenderTextures, pLargeBlurAddress, iQuarterTextureSize))
		return false;

	return true;
}

void C_DoFFgBg::Destroy()
{
	Display::Device()->Destroy(&m_BlurShader);
	Display::Device()->Destroy(&m_CombineShader);
	Display::Device()->Destroy(&m_DownsampleShader);
	Display::Device()->Destroy(&m_NearShader);
}

void C_DoFFgBg::DestroyVolatile()
{
	m_Manager.DestroyUserRenderTextures(m_DownsampleRenderTextures);
	m_Manager.DestroyUserRenderTextures(m_SmallBlurRenderTextures);
	m_Manager.DestroyUserRenderTextures(m_LargeBlurRenderTextures);
}

void C_DoFFgBg::SetTextureBuffer(U8* pTextureBuffer, const U32 iTextureBufferSize)
{
	m_pTextureBuffer = pTextureBuffer;
	m_iTextureBufferSize = iTextureBufferSize;
}

U32 C_DoFFgBg::GetTextureBufferRequiredSize() const
{
	const Display::E_Format eFormat = Display::E_Format_Unknown;
	const U32 iQuarterTextureSize = m_Manager.GetMaximumTextureSize(2, 2, eFormat);
	const U32 iSixteenthTextureSize = m_Manager.GetMaximumTextureSize(4, 4, eFormat);

	return iSixteenthTextureSize * 3;
}

void C_DoFFgBg::ConvertDepthOfFieldParameters(const F32 fPlaneInFocus, const F32 fAperture, F32& fNearStartDistance, F32& fNearEndDistance, F32& fFarStartDistance, F32& fFarEndDistance) const
{
	const F32 fNearClip = Display::Device()->GetViewSettings3D().GetNearClip();
	const F32 fFarClip = Display::Device()->GetViewSettings3D().GetFarClip() * g_debugDoFFgBgFarClipScale;

	const F32 fK			= g_debugDoFMinCoC / (fAperture + 0.00001f);
	const F32 fOnePlusK		= 1.0f + fK;
	const F32 fOneMinusK	= (1.0f - fK > 0.0f) ? 1.0f - fK : 0.00001f;

	const F32 fNearCoC		= fAperture * abs(1.0f - (fPlaneInFocus / fNearClip));
	const F32 fFarCoC		= fAperture * abs(1.0f - (fPlaneInFocus / fFarClip));

	fNearEndDistance		= fPlaneInFocus / fOnePlusK;
	fFarStartDistance		= fPlaneInFocus / fOneMinusK;

	fNearStartDistance = (g_debugDoFFgBgMaxCoC / fNearCoC) * (fNearClip - fNearEndDistance) + fNearEndDistance;
	if(fNearStartDistance > fNearEndDistance * 0.80f)
		fNearStartDistance = fNearEndDistance * 0.80f;
	fFarEndDistance = (g_debugDoFFgBgMaxCoC / fFarCoC) * (fFarClip - fFarStartDistance) + fFarStartDistance;
	if(fFarEndDistance < fFarStartDistance * 2.00f)
		fFarEndDistance = fFarStartDistance * 2.00f;
	if(fFarEndDistance > fFarStartDistance * 4.00f)
		fFarEndDistance = fFarStartDistance * 4.00f;
}

void C_DoFFgBg::SetDepthOfField(const S32 iViewIndex, const F32 fPlaneInFocus, const F32 fAperture)
{
	F32 fNearStartDistance = 0.0f;
	F32 fNearEndDistance = 0.0f;
	F32 fFarStartDistance = 0.0f;
	F32 fFarEndDistance = 0.0f;
	ConvertDepthOfFieldParameters(fPlaneInFocus, fAperture, fNearStartDistance, fNearEndDistance, fFarStartDistance, fFarEndDistance);

	C_DoFFgBg::SetDepthOfField(iViewIndex, fNearStartDistance, fNearEndDistance, fFarStartDistance, fFarEndDistance, 1.0f);
	C_DoFBase::SetDepthOfField(iViewIndex, fPlaneInFocus, fAperture);
}

void C_DoFFgBg::SetDepthOfField(const S32 iViewIndex, const F32 fNearStartDistance, const F32 fNearEndDistance, const F32 fFarStartDistance, const F32 fFarEndDistance, const F32 fFarScale)
{
	if (iViewIndex < 0)
		return;
	if (iViewIndex >= m_DepthOfFieldSettings.Size())
		m_DepthOfFieldSettings.Resize(iViewIndex + 1);

	m_DepthOfFieldSettings[iViewIndex].m_fNearStartDistance	= fNearStartDistance;
	m_DepthOfFieldSettings[iViewIndex].m_fNearEndDistance	= fNearEndDistance;
	m_DepthOfFieldSettings[iViewIndex].m_fFarStartDistance	= fFarStartDistance;
	m_DepthOfFieldSettings[iViewIndex].m_fFarEndDistance	= fFarEndDistance;
	m_DepthOfFieldSettings[iViewIndex].m_fFarScale			= fFarScale;
}

void C_DoFFgBg::Display() const
{
	// [Blur2PC] Retail PC (Blur.exe 1.2, 0x00659be0) runs this pass with the
	// same enable checks; the snapshot's PC guard made the return unconditional.
	if(!m_Manager.Enabled() || !m_bEnabled)
		return;
	if(!g_debugDepthOfField)
		return;

	const S32 iRenderTarget = m_Manager.GetRenderTargetIndex();
	if(iRenderTarget < 0)
		return;		

	PERF_EVENT("DepthOfFieldFgBg");
	
	// calculate circles of confusion from plane in focus and apertue
	const F32 fNearClip = Display::Device()->GetViewSettings3D().GetNearClip();
	const F32 fFarClip = Display::Device()->GetViewSettings3D().GetFarClip() * g_debugDoFFgBgFarClipScale;

	const S32 iViewIndex = m_Manager.GetViewIndex();

	F32 fNearStartDistance	= g_debugDoFFgBgNearStartDistance;
	F32 fNearEndDistance	= g_debugDoFFgBgNearEndDistance;
	F32 fFarStartDistance	= g_debugDoFFgBgFarStartDistance;
	F32 fFarEndDistance		= g_debugDoFFgBgFarEndDistance;
	F32 fFarRatio			= g_debugDoFFgBgFarScale;
	if(g_debugDoFFgBgBaseOverride)
	{
		ConvertDepthOfFieldParameters(GetPlaneInFocus(iViewIndex), GetAperture(iViewIndex), fNearStartDistance, fNearEndDistance, fFarStartDistance, fFarEndDistance);
	}
	else if(!g_debugDoFFgBgOverride && iViewIndex >= 0 && iViewIndex < m_DepthOfFieldSettings.Size())
	{
		fNearStartDistance	= m_DepthOfFieldSettings[iViewIndex].m_fNearStartDistance;
		fNearEndDistance	= m_DepthOfFieldSettings[iViewIndex].m_fNearEndDistance;
		fFarStartDistance	= m_DepthOfFieldSettings[iViewIndex].m_fFarStartDistance;
		fFarEndDistance		= m_DepthOfFieldSettings[iViewIndex].m_fFarEndDistance;
		fFarRatio			= m_DepthOfFieldSettings[iViewIndex].m_fFarScale;
	}

	const F32 fNearScale = 1.0f / (fNearStartDistance - fNearEndDistance);
	const F32 fNearBias = -1.0f * fNearScale * fNearEndDistance;
	const F32 fFarScale = 1.0f / (fFarEndDistance - fFarStartDistance);
	const F32 fFarBias = -1.0f * fFarScale * fFarStartDistance;

	const U32 iWidth = m_Manager.GetView().GetWidth();
	const U32 iHeight = m_Manager.GetView().GetHeight();

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);


	// downsample the main colour buffer
	{
		Display::C_RenderTexture2D* pRenderTexture = m_SmallBlurRenderTextures[iRenderTarget];

		Display::Device()->SetColourDepthStencilSurface(&pRenderTexture->GetSurface(), 0);
		Display::C_Viewport viewport;
		viewport.m_W = pRenderTexture->GetSurface().GetWidth();
		viewport.m_H = pRenderTexture->GetSurface().GetHeight();
		Display::Device()->SetViewport(viewport);

		const Display::C_Float4 vSettings(fNearScale, fNearBias, 1.0f / (F32) iWidth, 1.0f / (F32) iHeight);
		const Display::C_Float4 vDepth = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
		Display::Device()->SetPixelShaderParameterF(m_DownsampleSettingsParameter, (F32*)&vSettings);
		Display::Device()->SetPixelShaderParameterF(m_DownsampleDepthParameter, (F32*)&vDepth);
		Display::Device()->SetPixelShader(&m_DownsampleShader);

		Display::Device()->SetTexture(0, m_Manager.GetMainTexture());
#if defined(PLATFORM_PS3)
		Display::Device()->SetTexture(1, Display::Device()->GetBackBuffer().GetDepthStencilTexture());
#else
		Display::Device()->SetTexture(1, Render::GetGlobalDepthStencilBuffer());
#endif
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);

		Display::Quad::DrawFullScreen();

		pRenderTexture->Resolve();
	}

	// gaussian blur downsampled texture
	{
		Display::C_RenderTexture2D* pDownsampleRenderTexture = m_DownsampleRenderTextures[iRenderTarget];
		Display::C_RenderTexture2D* pSmallBlurRenderTexture = m_SmallBlurRenderTextures[iRenderTarget];
		Display::C_RenderTexture2D* pLargeBlurRenderTexture = m_LargeBlurRenderTextures[iRenderTarget];
		m_GaussianBlurUtil.BilinearGaussBlurX(pDownsampleRenderTexture, pSmallBlurRenderTexture, Render::Texture::E_GaussianBlurShader_5);
		m_GaussianBlurUtil.BilinearGaussBlurY(pLargeBlurRenderTexture, pDownsampleRenderTexture, Render::Texture::E_GaussianBlurShader_5);
	}

	// near circle of confusion
	{
		Display::C_RenderTexture2D* pRenderTexture = m_DownsampleRenderTextures[iRenderTarget];

		Display::Device()->SetColourDepthStencilSurface(&pRenderTexture->GetSurface(), 0);
		Display::C_Viewport viewport;
		viewport.m_W = pRenderTexture->GetSurface().GetWidth();
		viewport.m_H = pRenderTexture->GetSurface().GetHeight();
		Display::Device()->SetViewport(viewport);
		Display::Device()->SetPixelShader(&m_NearShader);

		Display::Device()->SetTexture(0, &m_SmallBlurRenderTextures[iRenderTarget]->GetTexture());
		Display::Device()->SetTexture(1, &m_LargeBlurRenderTextures[iRenderTarget]->GetTexture());
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);

		Display::Quad::DrawFullScreen();

		pRenderTexture->Resolve();
	}

	// small blur
	{
		Display::C_RenderTexture2D* pRenderTexture = m_SmallBlurRenderTextures[iRenderTarget];

		Display::Device()->SetColourDepthStencilSurface(&pRenderTexture->GetSurface(), 0);
		Display::C_Viewport viewport;
		viewport.m_W = pRenderTexture->GetSurface().GetWidth();
		viewport.m_H = pRenderTexture->GetSurface().GetHeight();
		Display::Device()->SetViewport(viewport);

		const Display::C_Float4 vSettings(4.0f / (F32) iWidth, 4.0f / (F32) iHeight, 0.0f, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_BlurSettingsParameter, (F32*)&vSettings);
		Display::Device()->SetPixelShader(&m_BlurShader);

		Display::Device()->SetTexture(0, &m_DownsampleRenderTextures[iRenderTarget]->GetTexture());
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

		Display::Quad::DrawFullScreen();

		pRenderTexture->Resolve();
	}

	// display the final depth of field
	{
		m_Manager.SetViewport();

		const F32 d0 = g_debugDoFFgBgDistance0;
		const F32 d1 = g_debugDoFFgBgDistance1;
		const F32 d2 = 1.0f - d1 - d0;
		const Display::C_Float4 vSettings(1.0f / (F32) iWidth, 1.0f / (F32) iHeight, 0.0f, 0.0f);
		const Display::C_Float4 vScale(-1.0f / d0, -1.0f / d1, -1.0f / d2, 1.0f / d2);
		const Display::C_Float4 vBias(1.0f, (1.0f - d2) / d1, 1 / d2, (d2 - 1.0f) / d2);
		const Display::C_Float4 vFar(fFarScale, fFarBias, fFarRatio, 0.0f);
		const Display::C_Float4 vDepth = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
		Display::Device()->SetPixelShaderParameterF(m_CombineSettingsParameter, (F32*)&vSettings);
		Display::Device()->SetPixelShaderParameterF(m_CombineScaleParameter, (F32*)&vScale);
		Display::Device()->SetPixelShaderParameterF(m_CombineBiasParameter, (F32*)&vBias);
		Display::Device()->SetPixelShaderParameterF(m_CombineFarParameter, (F32*)&vFar);
		Display::Device()->SetPixelShaderParameterF(m_CombineDepthParameter, (F32*)&vDepth);
		Display::Device()->SetPixelShader(&m_CombineShader);

		Display::Device()->SetTexture(0, m_Manager.GetMainTexture());
		Display::Device()->SetTexture(1, &m_SmallBlurRenderTextures[iRenderTarget]->GetTexture());
		Display::Device()->SetTexture(2, &m_LargeBlurRenderTextures[iRenderTarget]->GetTexture());
#if defined(PLATFORM_PS3)
		Display::Device()->SetTexture(3, Display::Device()->GetBackBuffer().GetDepthStencilTexture());
#else
		Display::Device()->SetTexture(3, Render::GetGlobalDepthStencilBuffer());
#endif

		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureAddress(3, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureFilter(3, Display::E_TextureFilter_Point);

		Display::Device()->SetBlendMode(Display::E_BlendMode_AlphaPreMultiply);

		Display::Quad::DrawFullScreen();

		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	}

	// reset states
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Default);

	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
}

}; // namespace PostProcess
