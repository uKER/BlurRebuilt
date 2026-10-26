#include "StdAfx.h"
#include "Samples/Section/SectionPostEffects.h"
#include "Render/Utils/Helper.h"
#include "Render/Render/GridRender.h"

#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Render/PostProcess/Colour/Greyscale.h"

Debug::Bool g_debugUsePostEffects("Section.PostEffects.UsePostEffects", true);
Debug::Bool g_debugUsePostEffectsShaderHDR("Section.PostEffects.UseShaderHDR", false);

Debug::Bool g_debugUsePostEffectsShaderHDRU32("Section.PostEffects.UseShaderHDRU32", true);

#define USE_POST_EFFECTS
#define USE_POST_EFFECTS_HDR
//#define USE_POST_EFFECTS_GREYSCALE

namespace Samples
{

C_SectionPostEffects::C_SectionPostEffects(const Util::C_String& name) : C_Section(name)
{
	#ifdef USE_POST_EFFECTS
	m_pPostProcessMgr	= new PostProcess::C_Manager();
	
	#ifdef USE_POST_EFFECTS_HDR
	m_pPostProcessHdr	= new PostProcess::C_HighDynamicRange();
	#else
	m_pPostProcessHdr	= NULL;
	#endif

	#ifdef USE_POST_EFFECTS_GREYSCALE
	m_pPostProcessGreyscale	= new PostProcess::C_Greyscale();
	#else
	m_pPostProcessGreyscale = NULL;
	#endif

	#else
	m_pPostProcessMgr		= NULL;
	m_pPostProcessHdr		= NULL;
	m_pPostProcessGreyscale = NULL;
	#endif

	if(m_pPostProcessHdr)
	{
		m_pPostProcessHdr->SetType(PostProcess::E_Type_Final);

		if(g_debugUsePostEffectsShaderHDR)
			m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_Shader);
		else if(g_debugUsePostEffectsShaderHDRU32)
			m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
	}

	SetupPostProcess();
}

C_SectionPostEffects::~C_SectionPostEffects()
{
	SAFE_DELETE_PTR(m_pPostProcessMgr);
}

void C_SectionPostEffects::OnResize()
{
	if(!m_pPostProcessMgr)
		return;

	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();

	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();
}

bool C_SectionPostEffects::OnCreate()
{
	return true;
}

bool C_SectionPostEffects::OnCreateVolatile()
{
	return true;
}

void C_SectionPostEffects::OnDestroyVolatile()
{
}

void C_SectionPostEffects::OnDestroy()
{
}

void C_SectionPostEffects::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	if(m_pPostProcessHdr)
	{
		/*if(g_debugUsePostEffects)
		{
			#ifdef PLATFORM_XENON
			m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_Shader);
			#else
			m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
			#endif
		}
		else
		{
			m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_PostProcess);			
		}*/
	}
}

void C_SectionPostEffects::OnPreRender(const C_Input& input, const C_Timer& timer)
{
}

void C_SectionPostEffects::OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	Render::BeginRenderSplitscreenWithoutAAtates();

	m_SkyDome.Render();

	Render::DrawGrid();

	Render::EndRenderSplitscreenWithoutAAtates();

	Util::C_String str;
	str.Printf("Post Effects Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

void C_SectionPostEffects::OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer)
{
}

void C_SectionPostEffects::OnBeginRenderSplitscreenWithoutAA()
{
	if(!g_debugUsePostEffects || !m_pPostProcessMgr)
		return;

	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 160, 0, 255));
}

void C_SectionPostEffects::OnEndRenderSplitscreenWithoutAA()
{
	if(!g_debugUsePostEffects || !m_pPostProcessMgr)
		return;

	if(m_pPostProcessGreyscale)
		m_pPostProcessGreyscale->Display();

	if(m_pPostProcessHdr)
		m_pPostProcessHdr->Display();

	m_pPostProcessMgr->End();
}

void C_SectionPostEffects::SetupPostProcess()
{
	if(!m_pPostProcessMgr)
		return;

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	const S32 viewCount = Display::Device()->GetViewCount();
	for(S32 viewIndex=0; viewIndex<viewCount; viewIndex++)
	{
		const Display::C_View* pView = Display::Device()->GetView(viewIndex);
		if(pView->GetUserIndex() != -1)
		{
			const Display::C_Viewport& viewport = pView->GetViewport();

			if(!m_pPostProcessHdr || m_pPostProcessHdr->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
			{
				m_pPostProcessMgr->AddView(
					viewport.GetW(),
					viewport.GetH(),
					pView->GetIndex(),
					PostProcess::E_RenderTargetFormat_RGBA_U8,
					Display::E_Multisample_None,
					5);
			}			
			else
			{
				m_pPostProcessMgr->AddView(
					viewport.GetW(),
					viewport.GetH(),
					pView->GetIndex(),
					PostProcess::E_RenderTargetFormat_RGBA_F10,
					Display::E_Multisample_None,
					5);
			}
		}
	}
}

} // namespace Samples
