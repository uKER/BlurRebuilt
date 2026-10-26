#include "StdAfx.h"
#include "Samples/Section/SectionDynamicEffect.h"
#include "Render/Utils/Helper.h"
#include "Render/Render/GridRender.h"

#include "Dynamic/Renderer.h"
#include "Dynamic/Minimal/Minimal.h"

// debug
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"

#include <System/Misc/Random.h>

#define USE_POST_EFFECTS
#define USE_POST_EFFECTS_HDR

namespace Samples
{

C_SectionDynamicEffect::C_SectionDynamicEffect(const Util::C_String& name) : C_Section(name)
{
	Util::C_Random random;

	#ifdef USE_POST_EFFECTS
	m_pPostProcessMgr	= new PostProcess::C_Manager();
	#ifdef USE_POST_EFFECTS_HDR
	m_pPostProcessHdr	= new PostProcess::C_HighDynamicRange();
	#else
	m_pPostProcessHdr	= NULL;
	#endif
	#else
	m_pPostProcessMgr	= NULL;
	m_pPostProcessHdr	= NULL;
	#endif

	if(m_pPostProcessHdr)
	{
		m_pPostProcessHdr->SetType(PostProcess::E_Type_Final);

			m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
	}

	m_pDynamicEffectRenderer = new DynamicEffect::C_Renderer;
	
	// add minimal dynamic effect example
	{
		m_pMinimalDynamicEffect = new DynamicEffect::C_Minimal(m_pDynamicEffectRenderer, 0, 1234);

		S32 randomObjects = 20;

		for(S32 i = 0; i < randomObjects; ++i)
		{
			Maths::C_MatrixRotateRPH rph(random.FRand(-Maths::Pi, Maths::Pi),
											random.FRand(-Maths::Pi, Maths::Pi),
											random.FRand(-Maths::Pi, Maths::Pi));
			Maths::HMat33 rotation(rph);
			Maths::HMat43 transform;
			transform.Translation(Maths::HVec3(random.FRand(-20.0f, 0.0f), random.FRand(10.0f, 0.0f), random.FRand(-20.0f, 0.0f)));
			transform.Rotation(rotation);
			Maths::C_RGBA_F32 colour(random.FRand(1.0f), random.FRand(1.0f), random.FRand(1.0f), 1.0f);

			m_pMinimalDynamicEffect->AddObject(colour, transform, false);
		}
	}

	if(!Render::Debug::g_pDebugRenderSys)
	{
		m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
		DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	}
	else
		m_pDebugRenderSys = NULL;

	SetupPostProcess();
}

C_SectionDynamicEffect::~C_SectionDynamicEffect()
{
	SAFE_DELETE_PTR(m_pPostProcessMgr);
	SAFE_DELETE_PTR(m_pDebugRenderSys);

	SAFE_DELETE_PTR(m_pMinimalDynamicEffect);
	SAFE_DELETE_PTR(m_pDynamicEffectRenderer);
}

void C_SectionDynamicEffect::OnResize()
{
	if(!m_pPostProcessMgr)
		return;

	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();

	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();
}

bool C_SectionDynamicEffect::OnCreate()
{
	return true;
}

bool C_SectionDynamicEffect::OnCreateVolatile()
{
	return true;
}

void C_SectionDynamicEffect::OnDestroyVolatile()
{
}

void C_SectionDynamicEffect::OnDestroy()
{
}

void C_SectionDynamicEffect::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	m_pDynamicEffectRenderer->Update(timer.GetTime(), timer.GetFrameTime(), timer.IsPaused());
}

void C_SectionDynamicEffect::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	m_pDynamicEffectRenderer->BeginFrame();
}

void C_SectionDynamicEffect::OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	m_pDynamicEffectRenderer->BeginView();

	Render::BeginRenderSplitscreenWithoutAAtates();

	m_SkyDome.Render();

	Render::DrawGrid();

	m_pDynamicEffectRenderer->Build(Display::Device()->GetView()->m_Index);
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_Opaque);

	Render::EndRenderSplitscreenWithoutAAtates();

	Util::C_String str;
	str.Printf("Post Effects Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

void C_SectionDynamicEffect::OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer)
{
	m_pDynamicEffectRenderer->BeginView();
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_TwoDimensional | DynamicEffect::E_RenderPass_Transparent | DynamicEffect::E_RenderPass_Debug);
}

void C_SectionDynamicEffect::OnBeginRenderSplitscreenWithoutAA()
{
	if(!m_pPostProcessMgr)
		return;

	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 160, 0, 255));
}

void C_SectionDynamicEffect::OnEndRenderSplitscreenWithoutAA()
{
	if(!m_pPostProcessMgr)
		return;

	if(m_pPostProcessHdr)
		m_pPostProcessHdr->Display();

	m_pPostProcessMgr->End();

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
}

void C_SectionDynamicEffect::SetupPostProcess()
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
