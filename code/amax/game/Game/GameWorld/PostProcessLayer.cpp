#include "stdafx.h"

#include <Game/Game.h>

#include <Framework/Application/ApplicationBase.h>

#include <Game/GameWorld/PostProcessLayer.h>
#include <Game/Messaging\GameFlowMessages.h>

#include <Graphics/HDR.h>

#include <Render/PostProcess/Manager.h>
#include <Render/PostProcess/Bloom/BloomFeedbackGhost.h>
#include <Effect/PostProcess/MotionBlur/MotionBlurReduced.h>
#include <Effect/PostProcess/DoFFgBg.h>
#include <Render/Utils/Helper.h>
#include <Render/View/ViewManager.h>

namespace HDR
{
	extern Display::C_Float2	g_vFlareScales[4];
	extern Display::C_Float4	g_vFlareColours[4];
};


namespace HDR
{
	Debug::Float g_Gamma("Graphics.Game.Gamma", 2.2f, Debug::Range(0.0f, 3.0f));
};

namespace Game
{

#if PLATFORM_PC
extern Debug::Bool g_MainPrepassEnable;	// [Blur2PC] T107: View/Main.cpp
#endif

C_GameWorldPostProcess::C_GameWorldPostProcess( )
	: m_pPostProcessMgr(NULL)
	, m_pPostProcessHdr(NULL)
	, m_pPostProcessMotionBlur(NULL)
	, m_pPostProcessBloom(NULL)
	, m_CanRender(false)
	, m_fFocusPlane(0.0f)
	, m_fAperture(0.0f)
	, m_bMotionBlur(true)
	, m_bDepthOfField(false)
	, m_pTextureBuffer(0)
	, m_iTextureBufferSize(0)
	, m_pDownSampleTextureBuffer(0)
	, m_iDownSampleTextureBufferSize(0)
{
	m_pPostProcessMgr = new PostProcess::C_Manager();
	m_pPostProcessMgr->SetName("Game");

	m_pPostProcessMotionBlur = new PostProcess::C_MotionBlurReduced(*m_pPostProcessMgr);
	m_pPostProcessMotionBlur->SetSamples(PostProcess::E_MotionBlurSamples_7);
	m_pPostProcessMotionBlur->SetComposite(false);

	m_pPostProcessDoF = new PostProcess::C_DoFFgBg(*m_pPostProcessMgr);

	m_pPostProcessHdr = new C_HighDynamicRange(*m_pPostProcessMgr);
	m_pPostProcessHdr->SetType(PostProcess::E_Type_Final);
	#ifdef PLATFORM_PS3
	m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
	#else
	m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_Shader);
	#endif

	m_pPostProcessBloom = new PostProcess::C_BloomFeedbackGhost(*m_pPostProcessMgr);
}

C_GameWorldPostProcess::~C_GameWorldPostProcess()
{
	SAFE_DELETE_PTR(m_pPostProcessMgr);
}

void C_GameWorldPostProcess::RenderSplitScreenWithAA()
{
	m_pPostProcessMgr->SetView(Display::Device()->GetView()->GetIndex());
}

bool C_GameWorldPostProcess::Create()
{
	SetupPostProcess();

	if(!CreateMemoryBuffer())
		return false;


	if(!m_pPostProcessMgr->Create())
		return false;

	if(!m_pPostProcessMgr->CreateVolatile())
		return false;

	m_pPostProcessHdr->SetBloom(m_pPostProcessBloom);
	m_pPostProcessHdr->SetBloomGamma(true);

	return true;
}

void C_GameWorldPostProcess::Destroy()
{
	m_pPostProcessMgr->Destroy();
	m_pPostProcessMgr->DestroyVolatile();

	DestroyMemoryBuffer();
}

bool C_GameWorldPostProcess::CreateMemoryBuffer()
{
#if defined(PLATFORM_PC)
	// [Blur2PC] the PC device params carry the back buffer size as m_Width/m_Height
	U32 iWidth = Display::Device()->m_DeviceParams.m_Width;
	U32 iHeight = Display::Device()->m_DeviceParams.m_Height;
#else
	U32 iWidth = Display::Device()->m_DeviceParams.m_BackBufferWidth;
	U32 iHeight = Display::Device()->m_DeviceParams.m_BackBufferHeight;
#endif

	m_pPostProcessMgr->ReserveView(iWidth, iHeight);

	// allocate shared memory buffers for effects
	const U32 iMotionBlurBufferSize		= m_pPostProcessMotionBlur->GetTextureBufferRequiredSize();
	const U32 iDepthOfFieldBufferSize	= m_pPostProcessDoF->GetTextureBufferRequiredSize();
		
	m_iTextureBufferSize = Maths::Max(iMotionBlurBufferSize, iDepthOfFieldBufferSize);

#if defined(PLATFORM_PS3)
	if (!Display::Device()->AllocMemory(128, m_iTextureBufferSize, &m_pTextureBuffer, Display::E_AllocHeap_Video, Display::E_AllocType_Default, Display2::E_MemoryType_Texture))
		return false;
#endif

	m_pPostProcessMotionBlur->SetTextureBuffer(m_pTextureBuffer, m_iTextureBufferSize);
	m_pPostProcessDoF->SetTextureBuffer(m_pTextureBuffer, m_iTextureBufferSize);

	
	
	U32 iSecondaryUsageFlags = Display::E_Usage_Colour;
	iSecondaryUsageFlags |= Display::E_Usage_ReadWrite;
	for (U32 iDownsample = 0, iScale = 1; iDownsample < 5; ++iDownsample)
	{
		iScale *= 2;
		m_iDownSampleTextureBufferSize += Display::GetTexture2DSize(iWidth / iScale, iHeight / iScale, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8);		
		if(iSecondaryUsageFlags & Display::E_Usage_ReadWrite)
			m_iDownSampleTextureBufferSize += Display::GetTexture2DSize(iWidth / iScale, iHeight / iScale, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8);		
	}
#if defined(PLATFORM_PS3)
	if (!Display::Device()->AllocMemory(128, m_iDownSampleTextureBufferSize, &m_pDownSampleTextureBuffer, Display::E_AllocHeap_Video, Display::E_AllocType_Default, Display2::E_MemoryType_Texture))
		return false;
#endif

	m_pPostProcessMgr->SetSecondaryTextureBuffer(m_pDownSampleTextureBuffer, m_iDownSampleTextureBufferSize);
	
	return true;
}

void C_GameWorldPostProcess::DestroyMemoryBuffer()
{	
	m_pPostProcessMotionBlur->SetTextureBuffer(0, 0);
	m_pPostProcessDoF->SetTextureBuffer(0, 0);
	
#if defined(PLATFORM_PS3)
	Display::Device()->FreeMemory(m_iTextureBufferSize, m_pTextureBuffer, Display::E_AllocHeap_Video, Display2::E_MemoryType_Texture);
#endif
	m_pTextureBuffer = 0;
	m_iTextureBufferSize = 0;

	m_pPostProcessMgr->SetSecondaryTextureBuffer(0, 0);

#if defined(PLATFORM_PS3)
	Display::Device()->FreeMemory(m_iDownSampleTextureBufferSize, m_pDownSampleTextureBuffer, Display::E_AllocHeap_Video, Display2::E_MemoryType_Texture);
#endif
	m_pDownSampleTextureBuffer = 0;
	m_iDownSampleTextureBufferSize = 0;
}

void C_GameWorldPostProcess::BeginRenderSplitscreenWithoutAA()
{
	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());
}

void C_GameWorldPostProcess::EndRenderSplitscreenWithoutAA()
{
	GPU_TIMING(5, Maths::RGBA(255, 255, 0, 255), "GPU.PostProcess");
	
	Display::Device()->InvalidateState();

	m_pPostProcessHdr->SetGamma(HDR::g_Gamma);

	m_pPostProcessHdr->ManualCalculateLuminance();

	m_pPostProcessBloom->SetScales(HDR::g_vFlareScales[0], HDR::g_vFlareScales[1], HDR::g_vFlareScales[2], HDR::g_vFlareScales[3]);
	m_pPostProcessBloom->SetColours(HDR::g_vFlareColours[0], HDR::g_vFlareColours[1], HDR::g_vFlareColours[2], HDR::g_vFlareColours[3]);

#if PLATFORM_PC
	// [Blur2PC] T107: retail PC (Blur.exe 1.2, 0x00419340) runs motion blur and depth of field only while
	// Amax.Views.Main.PrepassEnable is on (graphical detail 2-4) and clears the tone map's motion-blur
	// texture otherwise
	if(!g_MainPrepassEnable)
	{
		m_pPostProcessHdr->SetMotionBlurTexture(NULL);
	}
	else
#endif
	{
		m_pPostProcessHdr->SetMotionBlurTexture(m_pPostProcessMotionBlur->GetTexture());

		if(m_bMotionBlur)
		{
			m_pPostProcessMotionBlur->Display();
		}
		if(m_bDepthOfField)
		{
			m_pPostProcessDoF->Display();
		}
	}
	m_pPostProcessHdr->Display();
	m_pPostProcessMgr->End();
}

void C_GameWorldPostProcess::Resize()
{
	// destroy post process volatile and re-add the views
	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();
	SetupPostProcess();


	m_pPostProcessMgr->CreateVolatile();
}

void C_GameWorldPostProcess::SetupPostProcess()
{
	const S32 viewCount = Render::C_ViewManager::Instance().GetViewCount();
	for (S32 viewNo = 0; viewNo < viewCount; ++viewNo)
	{
		const Render::C_View* pView = Render::C_ViewManager::Instance().GetView(viewNo);
		if (pView && pView->m_ViewFlags & Render::E_ViewFlag_EnablePostProcess)
		{
			const Display::C_Viewport& viewport = pView->m_View.GetViewport();
			if(m_pPostProcessHdr->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
				m_pPostProcessMgr->AddView(viewport.GetW(), viewport.GetH(), pView->m_ViewIndex, PostProcess::E_RenderTargetFormat_RGBA_U8, Display::E_Multisample_None, 5);
			else
				m_pPostProcessMgr->AddView(viewport.GetW(), viewport.GetH(), pView->m_ViewIndex, PostProcess::E_RenderTargetFormat_RGBA_F10, Display::E_Multisample_None, 5);
		}
	}
}

void C_GameWorldPostProcess::SetDofInfo(const F32 fFocusPlane, const F32 fAperture)
{
	m_fFocusPlane	= fFocusPlane;
	m_fAperture		= fAperture;
}

void C_GameWorldPostProcess::SetMotionBlur()
{
	m_bMotionBlur = true;
	m_bDepthOfField = false;
	m_pPostProcessHdr->SetToneMapShader(E_ToneMapShader_MotionBlur);
}

void C_GameWorldPostProcess::SetDepthOfField()
{	
	m_bDepthOfField = true;
	m_bMotionBlur = false;
	m_pPostProcessHdr->SetToneMapShader(E_ToneMapShader_DepthOfField);
}

void C_GameWorldPostProcess::SetQuality(const F32 fQuality)
{
	if(fQuality < 0.25f)
		m_pPostProcessMotionBlur->SetSamples(PostProcess::E_MotionBlurSamples_5);
	else if(fQuality < 0.50f)
		m_pPostProcessMotionBlur->SetSamples(PostProcess::E_MotionBlurSamples_7);
	else if(fQuality < 0.75f)
		m_pPostProcessMotionBlur->SetSamples(PostProcess::E_MotionBlurSamples_9);
	else
		m_pPostProcessMotionBlur->SetSamples(PostProcess::E_MotionBlurSamples_11);
}

void C_GameWorldPostProcess::ClearExternalTextures()
{
	if (m_pPostProcessHdr)
	{
		m_pPostProcessHdr->SetMotionBlurTexture(NULL);
		m_pPostProcessHdr->SetLensEffectTexture(NULL);
	}
}

} // namespace Game
