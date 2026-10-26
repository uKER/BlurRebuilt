#include "StdAfx.h"
#include "TestSections/TestSectionSky.h"
#include "Render/Mesh/MeshLoader.h"
#include "Render/Utils/Helper.h"
#include "Render/View/ViewManager.h"
#include "System/Scheduler/jobqueue.h"
#include "Framework/Application/Application.h"
#include "Framework/Application/UserControls.h"
#include "System/Misc/StringUtil.h"

enum E_SkyDomeType
{
	E_SkyDomeType_Textured = 0,
	E_SkyDomeType_MultiTextured,
	E_SkyDomeType_HDR,
	E_SkyDomeType_Luminance,
	E_SkyDomeType_Preetham,
	E_SkyDomeType_Atmospheric,
	E_SkyDomeType_Count
};

enum E_CloudType
{
	E_CloudType_None,
	E_CloudType_TexturedPlane,
	E_CloudType_LitDome,
	E_CloudType_Count
};

DEBUG_ENUM(E_SkyDomeType)
	DE_OPTION(E_SkyDomeType_Textured,		"Textured")
	DE_OPTION(E_SkyDomeType_MultiTextured,	"MultiTextured")
	DE_OPTION(E_SkyDomeType_HDR,			"HDR")
	DE_OPTION(E_SkyDomeType_Luminance,		"Luminance")
	DE_OPTION(E_SkyDomeType_Preetham,		"Preetham")
	DE_OPTION(E_SkyDomeType_Atmospheric,	"Atmospheric")
END_DEBUG_ENUM

DEBUG_ENUM(E_CloudType)
	DE_OPTION(E_CloudType_None,				"None")
	DE_OPTION(E_CloudType_TexturedPlane,	"TexturedPlane")
	DE_OPTION(E_CloudType_LitDome,			"LitDome")
END_DEBUG_ENUM

Debug::Enum<E_SkyDomeType>	g_debugSkyType			("TestSection.Sky.Type",			E_SkyDomeType_Textured);
Debug::Enum<E_CloudType>	g_debugCloudType		("TestSection.Sky.CloudType",		E_CloudType_None);
Debug::Float				g_debugSkySunHeading	("TestSection.Sky.SunHeading",		0.00f,		Debug::Range(0.0f, 360.0f));
Debug::Float				g_debugSkySunElevation	("TestSection.Sky.SunElevation",	45.0f,		Debug::Range(0.0f, 90.0f));
Debug::Float				g_debugSkySunSize		("TestSection.Sky.SunSize",			0.50f,		Debug::Range(0.0f, 1.0f));
Debug::Float				g_debugSkySunOcclusion	("TestSection.Sky.SunOcclusion",	1.0f,		Debug::Range(0.0f, 1.0f));
Debug::Bool					g_debugSkySunBloom		("TestSection.Sky.SunBloom",		true);
Debug::Vector				g_debugSkyWindDirection	("TestSection.Sky.WindDirection",	1.0f, 0.5f, -0.5f);

Debug::String				g_debugSkyTexture		("TestSection.Sky.Texture",			"Textures/Sunny.dds");
Debug::String				g_debugSkyHdrTexture	("TestSection.Sky.HdrTexture",		"Textures/Sky.xet");
Debug::String				g_debugSkyBgTexture		("TestSection.Sky.BgTexture",		"Textures/SkyBg.dds");
Debug::String				g_debugSkyFgTexture		("TestSection.Sky.FgTexture",		"Textures/SkyFg.dds");
Debug::Bool					g_debugSkyReloadTextures("TestSection.Sky.ReloadTextures",	false);

#if defined(PLATFORM_PS3)
static Scheduler::Policy g_SkyPolicy[] = { 1, 1, 1, 1, 1, 1, 1, 1 };
#else
static Scheduler::Policy g_SkyPolicy[] = { 1, 1, 1, 1, 1, 1 };
#endif

C_TestSectionSky::C_TestSectionSky(const Util::C_String& name) : Framework::C_TestSection(name)
{
	OutputDebugString("C_TestSectionSky::Constructor Begin\n");

	m_pPostProcessMgr	= new PostProcess::C_Manager();
	m_pPostProcessHdr	= new PostProcess::C_HighDynamicRange();

	m_pPostProcessHdr->SetType(PostProcess::E_Type_Final);
	#ifdef PLATFORM_PS3
	m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
	#else
	m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_Shader);
	#endif

	SetupPostProcess();

	m_hPolicy = Scheduler::CreatePolicy(g_SkyPolicy);

	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(1);
	
	m_AtmosphericSkyDome.SetPolicy(m_hPolicy);

	m_TexturedSkyDome.SetTexture(g_debugSkyTexture.Value());
	
	m_MultiTexturedSkyDome.SetBgTexture(g_debugSkyBgTexture.Value());
	m_MultiTexturedSkyDome.SetFgTexture(g_debugSkyFgTexture.Value());

	OutputDebugString("C_TestSectionSky::Constructor End\n");
}

C_TestSectionSky::~C_TestSectionSky()
{
	Scheduler::Shutdown();

	SAFE_DELETE_PTR(m_pPostProcessMgr);
}

bool C_TestSectionSky::OnCreate()
{
	if (!Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		return false;

	m_Mesh.AddInstance();

	if (!m_AtmosphericSkyDome.Create())
		return false;
	if (!m_HDRSkyDome.Create())
		return false;
	if (!m_LuminanceSkyDome.Create())
		return false;
	if (!m_PreethamSkyDome.Create())
		return false;
	if (!m_TexturedSkyDome.Create())
		return false;
	if (!m_MultiTexturedSkyDome.Create())
		return false;
	if (!m_LitCloudDome.Create())
		return false;
	if (!m_TexturedCloudPlane.Create())
		return false;

	return true;
}

bool C_TestSectionSky::OnCreateVolatile()
{
	if (!m_AtmosphericSkyDome.CreateVolatile())
		return false;
	if (!m_HDRSkyDome.CreateVolatile())
		return false;
	if (!m_LuminanceSkyDome.CreateVolatile())
		return false;
	if (!m_PreethamSkyDome.CreateVolatile())
		return false;
	if (!m_TexturedSkyDome.CreateVolatile())
		return false;
	if (!m_MultiTexturedSkyDome.CreateVolatile())
		return false;
	if (!m_LitCloudDome.CreateVolatile())
		return false;
	if (!m_TexturedCloudPlane.CreateVolatile())
		return false;

	return true;
}

void C_TestSectionSky::OnDestroy()
{
	m_Mesh.Destroy();

	m_AtmosphericSkyDome.Destroy();
	m_HDRSkyDome.Destroy();
	m_LuminanceSkyDome.Destroy();
	m_PreethamSkyDome.Destroy();
	m_TexturedSkyDome.Destroy();
	m_MultiTexturedSkyDome.Destroy();
	m_LitCloudDome.Destroy();
	m_TexturedCloudPlane.Destroy();
}

void C_TestSectionSky::OnDestroyVolatile()
{
	m_AtmosphericSkyDome.DestroyVolatile();
	m_HDRSkyDome.DestroyVolatile();
	m_LuminanceSkyDome.DestroyVolatile();
	m_PreethamSkyDome.DestroyVolatile();
	m_TexturedSkyDome.DestroyVolatile();
	m_MultiTexturedSkyDome.DestroyVolatile();
	m_LitCloudDome.DestroyVolatile();
	m_TexturedCloudPlane.DestroyVolatile();
}

void C_TestSectionSky::OnUpdate()
{
	const F32 fCosHeading	= cosf(g_debugSkySunHeading * Maths::Pi / 180.0f);
	const F32 fCosElevation	= cosf(g_debugSkySunElevation * Maths::Pi / 180.0f);
	const F32 fSinHeading	= sinf(g_debugSkySunHeading * Maths::Pi / 180.0f);
	const F32 fSinElevation	= sinf(g_debugSkySunElevation * Maths::Pi / 180.0f);
	const Display::C_Float4 vSunDirection(fSinHeading * fSinElevation, fCosElevation, fCosHeading * fSinElevation, 0.0f);

	m_PreethamSkyDome.SetSunHeading(g_debugSkySunHeading);
	m_PreethamSkyDome.SetSunElevation(g_debugSkySunElevation);
	m_AtmosphericSkyDome.SetSunDirection(vSunDirection);

	m_AtmosphericSkyDome.Update();
	
	Maths::HVec3 vWindDirection = Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&g_debugSkyWindDirection.Value());
	vWindDirection.Normalise();

	m_TexturedCloudPlane.SetWindDirection(vWindDirection);
	m_TexturedCloudPlane.SetSunDirection(vSunDirection);
	m_TexturedCloudPlane.Update(Framework::C_Application::GetInstance()->GetTimer().GetFrameTime());

	m_LitCloudDome.SetSunDirection(vSunDirection);

	if (g_debugSkyReloadTextures)
	{
		m_TexturedSkyDome.SetTexture(g_debugSkyTexture.Value());
		m_TexturedSkyDome.ReloadTexture();

		m_MultiTexturedSkyDome.SetBgTexture(g_debugSkyBgTexture.Value());
		m_MultiTexturedSkyDome.SetFgTexture(g_debugSkyFgTexture.Value());
		m_MultiTexturedSkyDome.ReloadTextures();

		const Util::C_String hdrTextureName = g_debugSkyHdrTexture.Value();
		m_HDRSkyDome.SetTextureLDR(Util::RemoveFileExtension(hdrTextureName) + "_LDR." + Util::ExtractFileExtension(hdrTextureName));
		m_HDRSkyDome.SetTextureHDR(Util::RemoveFileExtension(hdrTextureName) + "_HDR." + Util::ExtractFileExtension(hdrTextureName));
		m_HDRSkyDome.ReloadTextures();

		g_debugSkyReloadTextures = false;
	}
}

void C_TestSectionSky::OnPreRender()
{
}

void C_TestSectionSky::OnRender3DViewWithAA(const S32 viewIndex)
{
	PERF_EVENT("RenderSectionSky");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_pPostProcessMgr->SetView(Display::Device()->GetView()->GetIndex());
	Render::BeginPostEffectStates();

	m_Mesh.Render();

	if (g_debugSkyType == E_SkyDomeType_Textured)
	{
		m_TexturedSkyDome.Render();
	}
	else if (g_debugSkyType == E_SkyDomeType_MultiTextured)
	{
		m_MultiTexturedSkyDome.Render();
	}
	else if (g_debugSkyType == E_SkyDomeType_HDR)
	{
		m_HDRSkyDome.Render();
	}
	else if (g_debugSkyType == E_SkyDomeType_Luminance)
	{
		m_LuminanceSkyDome.Render();
	}
	else if (g_debugSkyType == E_SkyDomeType_Preetham)
	{
		m_PreethamSkyDome.Render();
	}
	else if (g_debugSkyType == E_SkyDomeType_Atmospheric)
	{
		m_AtmosphericSkyDome.Render();
	}

	if (g_debugCloudType == E_CloudType_TexturedPlane)
	{
		m_TexturedCloudPlane.Render();
	}
	else if (g_debugCloudType == E_CloudType_LitDome)
	{
		m_LitCloudDome.Render();
	}

	Render::EndPostEffectStates();
}

void C_TestSectionSky::OnRender3DViewWithoutAA(const S32 viewIndex)
{
	Util::C_String str;
	str.Printf("Sky Section : %.2f fps", Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

void C_TestSectionSky::OnBeginPostEffects()
{
	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());
}

void C_TestSectionSky::OnEndPostEffects()
{
	// update the postprocess sun parameters
	const F32 fCosHeading	= cosf(g_debugSkySunHeading * Maths::Pi / 180.0f);
	const F32 fCosElevation	= cosf(g_debugSkySunElevation * Maths::Pi / 180.0f);
	const F32 fSinHeading	= sinf(g_debugSkySunHeading * Maths::Pi / 180.0f);
	const F32 fSinElevation	= sinf(g_debugSkySunElevation * Maths::Pi / 180.0f);
	const Maths::HVec3	vWorldSunDirection(fSinHeading * fSinElevation, fCosElevation, fCosHeading * fSinElevation);

	// screen position
	F32 fFarClip = Display::Device()->GetViewSettings3D().GetFarClip();
	Maths::HVec3 vViewPosition = Display::Device()->GetViewSettings3D().GetCameraPosition();
	Maths::HVec4 vPositionOne = Maths::HVec4(vViewPosition + vWorldSunDirection * fFarClip);
	vPositionOne.W(Maths::Constants<Maths::HF32>::One());
	Maths::HMat44 mViewProj = Display::Device()->GetViewSettings3D().GetWorldToDevice();
	Maths::HVec4 vScreenPosition = vPositionOne * mViewProj;
	vScreenPosition *= vScreenPosition.W().Rcp();
	Maths::HF32 vHalf(Maths::Constants<Maths::HF32>::Half());
	
	if (vScreenPosition.W() < 0.0f)
		m_pPostProcessHdr->SetBloomSun(false);
	else
		m_pPostProcessHdr->SetBloomSun(g_debugSkySunBloom);

	const F32 fAspectRatio = 1.0f / Display::Device()->GetViewSettings3D().GetAspectRatio();

	const Display::C_Float2 vTexturePosition(vScreenPosition.X() * 0.5f + 0.5f, vScreenPosition.Y() * -0.5f + 0.5f);

	m_pPostProcessHdr->SetBloomSunIntensity(g_debugSkySunOcclusion);
	m_pPostProcessHdr->SetBloomSunSize(Display::C_Float2(g_debugSkySunSize * fAspectRatio, g_debugSkySunSize));
	m_pPostProcessHdr->SetBloomSunPosition(vTexturePosition);

	m_pPostProcessHdr->Display();
	m_pPostProcessMgr->End();
}

void C_TestSectionSky::OnResize()
{
	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();
	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();
}

void C_TestSectionSky::SetupPostProcess()
{
	const S32 viewCount = Render::C_ViewManager::Instance().GetViewCount();
	for (U32 viewNo = 0; viewNo < viewCount; ++viewNo)
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
