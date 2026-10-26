#include "StdAfx.h"
#include "Render/Mesh/MeshLoader.h"
#include "Samples/Section/SectionFog.h"

#include "Dynamic/Node.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"
#include "Dynamic/Fog/Volume.h"
#include "Dynamic/Fog/Generic/BasicManager.h"

namespace Samples
{

Debug::Colour g_debugFogVolumeColour("Samples.Section.FogVolume.Colour", 0.25f, 0.25f, 0.25f, 1.0f);
Debug::Float g_debugFogVolumeDensity("Samples.Section.FogVolume.Density", 4.0f, Debug::Range(0.0f, 4.0f));
Debug::Float g_debugFogVolumeHeightFalloff("Samples.Section.FogVolume.HeightFalloff", 1.0f, Debug::Range(0.0f, 4.0f));

C_SectionFog::C_SectionFog(const Util::C_String& name) : C_Section(name)
{
	m_pPostProcessMgr	= new PostProcess::C_Manager();
	m_pPostProcessHdr	= new PostProcess::C_HighDynamicRange();

	m_pPostProcessHdr->SetType(PostProcess::E_Type_Final);
	#ifdef PLATFORM_PS3
	m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
	#else
	m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_Shader);
	#endif

	m_pDynamicEffectRenderer = &DynamicEffect::C_Renderer::Instance();
	m_pFogManager = new DynamicEffect::Fog::C_BasicManager(m_pDynamicEffectRenderer);

	MathsStorage::Float4A16 vColour(0.25f, 0.25f, 0.25f, 1.0f);
	MathsStorage::Float2A16 vSettings(4.0f, 1.0f);
	MathsStorage::Float4A16 vRotation(0.0f, 0.0f, 0.0f, 1.0f);

	m_pFogVolume0 = m_pFogManager->CreateVolume(DynamicEffect::E_EffectTypeIndex_FogCuboid, DynamicEffect::C_Node(MathsStorage::Float4A16(3.0f, 4.0f, -10.0f, 1.0f), vRotation), 0, MathsStorage::Float4A16(2.0f, 4.0f, 2.0f, 1000.0f), vColour, vSettings);
	m_pFogVolume1 = m_pFogManager->CreateVolume(DynamicEffect::E_EffectTypeIndex_FogCylinder, DynamicEffect::C_Node(MathsStorage::Float4A16(-11.0f, 3.5f, -10.0f, 1.0f), vRotation), 0, MathsStorage::Float4A16(3.0f, 4.0f, 3.0f, 1000.0f), vColour, vSettings);

	if(!Render::Debug::g_pDebugRenderSys)
	{
		m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
		DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	}
	else
		m_pDebugRenderSys = NULL;

	SetupPostProcess();
}

C_SectionFog::~C_SectionFog()
{
	SAFE_DELETE_PTR(m_pPostProcessMgr);
	SAFE_DELETE_PTR(m_pDebugRenderSys);
	SAFE_DELETE_PTR(m_pFogManager);
	SAFE_DELETE_PTR(m_pDynamicEffectRenderer);
}

void C_SectionFog::OnResize()
{
	// destroy post process volatile and re-add the views
	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();
	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();
}

bool C_SectionFog::OnCreate()
{
	if (!Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		return false;
	if (!m_Fog.Create())
		return false;


	m_Mesh.AddInstance();

	return true;
}

void C_SectionFog::OnDestroy()
{

	m_Fog.Destroy();
	m_Mesh.Destroy();
}

void C_SectionFog::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	m_pFogVolume0->m_vSettings.x = g_debugFogVolumeDensity;
	m_pFogVolume0->m_vSettings.y = g_debugFogVolumeHeightFalloff;
	m_pFogVolume0->m_vColour = MathsStorage::Float4A16(g_debugFogVolumeColour.Value().r, g_debugFogVolumeColour.Value().g, g_debugFogVolumeColour.Value().b, g_debugFogVolumeColour.Value().a);
	m_pFogVolume1->m_vSettings.x = g_debugFogVolumeDensity;
	m_pFogVolume1->m_vSettings.y = g_debugFogVolumeHeightFalloff;
	m_pFogVolume1->m_vColour = MathsStorage::Float4A16(g_debugFogVolumeColour.Value().r, g_debugFogVolumeColour.Value().g, g_debugFogVolumeColour.Value().b, g_debugFogVolumeColour.Value().a);
	m_pDynamicEffectRenderer->Update(timer.GetTime(), timer.GetFrameTime(), timer.IsPaused());
}

void C_SectionFog::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	m_pDynamicEffectRenderer->BeginFrame();
}

void C_SectionFog::OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionPreethamSkyDome");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_pDynamicEffectRenderer->BeginView();

	Render::BeginRenderSplitscreenWithoutAAtates();

	m_SkyDome.Render();
	m_Mesh.Render();

	m_pDynamicEffectRenderer->Build(Display::Device()->GetView()->m_Index);
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_Opaque);

	Render::EndRenderSplitscreenWithoutAAtates();
}

void C_SectionFog::OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer)
{

	m_Fog.Render();

	m_pDynamicEffectRenderer->BeginView();
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_TwoDimensional | DynamicEffect::E_RenderPass_Transparent | DynamicEffect::E_RenderPass_Debug);

	Util::C_String str;
	str.Printf("Fog Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

void C_SectionFog::OnBeginRenderSplitscreenWithoutAA()
{
	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 160, 0, 255));
}

void C_SectionFog::OnEndRenderSplitscreenWithoutAA()
{
	m_pPostProcessHdr->Display();
	m_pPostProcessMgr->End();
}

void C_SectionFog::SetupPostProcess()
{
	const S32 viewCount = Display::Device()->GetViewCount();
	for(S32 viewIndex=0; viewIndex<viewCount; viewIndex++)
	{
		const Display::C_View* pView = Display::Device()->GetView(viewIndex);
		if(pView->GetUserIndex() != -1)
		{
			const Display::C_Viewport& viewport = pView->GetViewport();

			if(m_pPostProcessHdr->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
				m_pPostProcessMgr->AddView(viewport.GetW(), viewport.GetH(), pView->GetIndex(), PostProcess::E_RenderTargetFormat_RGBA_U8, Display::E_Multisample_None, 5);
			else
				m_pPostProcessMgr->AddView(viewport.GetW(), viewport.GetH(), pView->GetIndex(), PostProcess::E_RenderTargetFormat_RGBA_F10, Display::E_Multisample_None, 5);
		}
	}
}

}; // namespace Samples
