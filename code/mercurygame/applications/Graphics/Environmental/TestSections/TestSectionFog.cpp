#include "StdAfx.h"
#include "Render/Mesh/MeshLoader.h"
#include "TestSections/TestSectionFog.h"
#include "Framework/Application/Application.h"

#include "Render/Utils/Globals.h"
#include "Render/Utils/Helper.h"
#include "Render/View/ViewManager.h"

#include "Dynamic/Node.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"
#include "Dynamic/Fog/Volume.h"
#include "Dynamic/Fog/Generic/BasicManager.h"

Debug::Colour g_debugFogVolumeColour("TestSection.Fog.Colour", 0.25f, 0.25f, 0.25f, 1.0f);
Debug::Float g_debugFogVolumeDensity("TestSection.Fog.Density", 4.0f, Debug::Range(0.0f, 4.0f));
Debug::Float g_debugFogVolumeHeightFalloff("TestSection.Fog.HeightFalloff", 1.0f, Debug::Range(0.0f, 4.0f));

C_TestSectionFog::C_TestSectionFog(const Util::C_String& name) : Framework::C_TestSectionHDR(name)
{
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
}

C_TestSectionFog::~C_TestSectionFog()
{
	SAFE_DELETE_PTR(m_pDebugRenderSys);
	SAFE_DELETE_PTR(m_pFogManager);
	SAFE_DELETE_PTR(m_pDynamicEffectRenderer);
}

bool C_TestSectionFog::OnCreate()
{
	if (!Render::Mesh::LoadMesh("Models/Test", &m_Mesh))
		return false;
	if (!m_Fog.Create())
		return false;

#ifdef PLATFORM_XENON
	const Display::C_View& view = Render::C_ViewManager::Instance().GetView(0)->m_View;
	if (!Display::Device()->CreateTexture2D(view.GetViewport().GetW(), view.GetViewport().GetH(), 1, 0, Display::E_Format_D24FS8, &m_DepthTexture))
		return false;
#endif

	m_Mesh.AddInstance();

	return true;
}

void C_TestSectionFog::OnDestroy()
{
#ifdef PLATFORM_XENON
	Display::Device()->Destroy(&m_DepthTexture);
#endif

	m_Fog.Destroy();
	m_Mesh.Destroy();
}

void C_TestSectionFog::OnUpdate()
{
	m_pFogVolume0->m_vSettings.x = g_debugFogVolumeDensity;
	m_pFogVolume0->m_vSettings.y = g_debugFogVolumeHeightFalloff;
	m_pFogVolume0->m_vColour = MathsStorage::Float4A16(g_debugFogVolumeColour.Value().r, g_debugFogVolumeColour.Value().g, g_debugFogVolumeColour.Value().b, g_debugFogVolumeColour.Value().a);
	m_pFogVolume1->m_vSettings.x = g_debugFogVolumeDensity;
	m_pFogVolume1->m_vSettings.y = g_debugFogVolumeHeightFalloff;
	m_pFogVolume1->m_vColour = MathsStorage::Float4A16(g_debugFogVolumeColour.Value().r, g_debugFogVolumeColour.Value().g, g_debugFogVolumeColour.Value().b, g_debugFogVolumeColour.Value().a);
	m_pDynamicEffectRenderer->Update(Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime(), Framework::C_Application::GetInstance()->GetTimer().GetFrameTime(), Framework::C_Application::GetInstance()->GetTimer().IsPaused());
}

void C_TestSectionFog::OnPreRender()
{
	m_pDynamicEffectRenderer->BeginFrame();
}

void C_TestSectionFog::OnRender3DViewWithAA(const S32 viewIndex)
{
	PERF_EVENT("RenderSectionPreethamSkyDome");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_pDynamicEffectRenderer->BeginView();

	m_SkyDome.Render();
	m_Mesh.Render();

	m_pDynamicEffectRenderer->Build(Display::Device()->GetView()->m_Index);
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_Opaque);
}

void C_TestSectionFog::OnRender3DViewWithoutAA(const S32 viewIndex)
{
#ifdef PLATFORM_XENON
	Render::g_pDepthStencilBuffer = &m_DepthTexture;
	D3DRECT rect;
	rect.x1 = 0;
	rect.y1 = 0;
	rect.x2 = Display::Device()->GetView()->GetViewport().GetW();
	rect.y2 = Display::Device()->GetView()->GetViewport().GetH();
	Display::Device()->GetD3DDevice()->Resolve(D3DRESOLVE_DEPTHSTENCIL, &rect, m_DepthTexture.m_pd3dTexture, NULL, 0, 0, NULL, 1.f, 0, NULL);
#endif

	m_Fog.Render();

	m_pDynamicEffectRenderer->BeginView();
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_TwoDimensional | DynamicEffect::E_RenderPass_Transparent | DynamicEffect::E_RenderPass_Debug);

	Util::C_String str;
	str.Printf("Fog Section : %.2f fps", Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}
