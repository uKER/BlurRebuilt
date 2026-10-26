#include "StdAfx.h"
#include "Samples/Section/SectionSky.h"
#include "Render/Mesh/MeshLoader.h"
#include "System/Scheduler/jobqueue.h"

enum E_SkyDomeType
{
	E_SkyDomeType_Textured = 0,
	E_SkyDomeType_MultiTextured,
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
	DE_OPTION(E_SkyDomeType_Luminance,		"Luminance")
	DE_OPTION(E_SkyDomeType_Preetham,		"Preetham")
	DE_OPTION(E_SkyDomeType_Atmospheric,	"Atmospheric")
END_DEBUG_ENUM

DEBUG_ENUM(E_CloudType)
	DE_OPTION(E_CloudType_None,				"None")
	DE_OPTION(E_CloudType_TexturedPlane,	"TexturedPlane")
	DE_OPTION(E_CloudType_LitDome,			"LitDome")
END_DEBUG_ENUM

namespace Samples
{

Debug::Enum<E_SkyDomeType>	g_debugSkyType			("Samples.Section.Sky.Type",			E_SkyDomeType_Atmospheric);
Debug::Enum<E_CloudType>	g_debugCloudType		("Samples.Section.Sky.CloudType",		E_CloudType_None);
Debug::Float				g_debugSkySunHeading	("Samples.Section.Sky.SunHeading",		0.00f,		Debug::Range(0.0f, 360.0f));
Debug::Float				g_debugSkySunElevation	("Samples.Section.Sky.SunElevation",	45.0f,		Debug::Range(0.0f, 90.0f));
Debug::Float				g_debugSkySunSize		("Samples.Section.Sky.SunSize",			0.50f,		Debug::Range(0.0f, 1.0f));
Debug::Float				g_debugSkySunOcclusion	("Samples.Section.Sky.SunOcclusion",	1.0f,		Debug::Range(0.0f, 1.0f));
Debug::Bool					g_debugSkySunBloom		("Samples.Section.Sky.SunBloom",		true);
Debug::Vector				g_debugSkyWindDirection	("Samples.Section.Sky.WindDirection",	1.0f, 0.5f, -0.5f);

Debug::String				g_debugSkyTexture		("Samples.Section.Sky.Texture",			"Textures/Sunny.dds");
Debug::String				g_debugSkyBgTexture		("Samples.Section.Sky.BgTexture",		"Textures/SkyBg.dds");
Debug::String				g_debugSkyFgTexture		("Samples.Section.Sky.FgTexture",		"Textures/SkyFg.dds");
Debug::Bool					g_debugSkyReloadTextures("Samples.Section.Sky.ReloadTextures",	false);

#if defined(PLATFORM_PS3)
static Scheduler::Policy g_SkyPolicy[] = { 1, 1, 1, 1, 1, 1, 1, 1 };
#else
static Scheduler::Policy g_SkyPolicy[] = { 1, 1, 1, 1, 1, 1 };
#endif

C_SectionSky::C_SectionSky(const Util::C_String& name) : C_Section(name)
{
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
	m_AtmosphericSkyDome.Initialise();

	m_TexturedSkyDome.SetTexture(g_debugSkyTexture.Value());
	
	m_MultiTexturedSkyDome.SetBgTexture(g_debugSkyBgTexture.Value());
	m_MultiTexturedSkyDome.SetFgTexture(g_debugSkyFgTexture.Value());
}

C_SectionSky::~C_SectionSky()
{
	m_AtmosphericSkyDome.Shutdown();
	
	Scheduler::Shutdown();

	SAFE_DELETE_PTR(m_pPostProcessMgr);
}

bool C_SectionSky::OnCreate()
{
	if (!Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		return false;

	m_Mesh.AddInstance();

	return true;
}

void C_SectionSky::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_SectionSky::OnUpdate(const C_Input& input, const C_Timer& timer)
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
	m_TexturedCloudPlane.Update(timer.GetFrameTime());

	m_LitCloudDome.SetSunDirection(vSunDirection);

	if (g_debugSkyReloadTextures)
	{
		m_TexturedSkyDome.SetTexture(g_debugSkyTexture.Value());
		m_TexturedSkyDome.ReloadTexture();

		m_MultiTexturedSkyDome.SetBgTexture(g_debugSkyBgTexture.Value());
		m_MultiTexturedSkyDome.SetFgTexture(g_debugSkyFgTexture.Value());
		m_MultiTexturedSkyDome.ReloadTextures();

		g_debugSkyReloadTextures = false;
	}
}

void C_SectionSky::OnPreRender(const C_Input& input, const C_Timer& timer)
{
}

void C_SectionSky::OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionSky");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	Render::BeginRenderSplitscreenWithoutAAtates();

	m_Mesh.Render();

	if (g_debugSkyType == E_SkyDomeType_Textured)
	{
		m_TexturedSkyDome.Render();
	}
	else if (g_debugSkyType == E_SkyDomeType_MultiTextured)
	{
		m_MultiTexturedSkyDome.Render();
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

	Render::EndRenderSplitscreenWithoutAAtates();
}

void C_SectionSky::OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer)
{
	Util::C_String str;
	str.Printf("Sky Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

void C_SectionSky::OnBeginRenderSplitscreenWithoutAA()
{
	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 160, 0, 255));
}

void C_SectionSky::OnEndRenderSplitscreenWithoutAA()
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

void C_SectionSky::OnResize()
{
	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();
	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();
}

void C_SectionSky::SetupPostProcess()
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
