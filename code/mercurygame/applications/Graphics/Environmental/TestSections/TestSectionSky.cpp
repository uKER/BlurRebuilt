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

DEBUG_ENUM(E_SkyDomeType)
	DE_OPTION(E_SkyDomeType_Textured,		"Textured")
	DE_OPTION(E_SkyDomeType_MultiTextured,	"MultiTextured")
	DE_OPTION(E_SkyDomeType_HDR,			"HDR")
	DE_OPTION(E_SkyDomeType_Luminance,		"Luminance")
	DE_OPTION(E_SkyDomeType_Preetham,		"Preetham")
	DE_OPTION(E_SkyDomeType_Atmospheric,	"Atmospheric")
END_DEBUG_ENUM

Debug::Enum<E_SkyDomeType>	g_debugSkyType			("TestSection.Sky.Type",			E_SkyDomeType_Textured);
Debug::Float				g_debugSkySunHeading	("TestSection.Sky.SunHeading",		0.00f,		Debug::Range(0.0f, 360.0f));
Debug::Float				g_debugSkySunElevation	("TestSection.Sky.SunElevation",	45.0f,		Debug::Range(0.0f, 90.0f));
Debug::Float				g_debugSkySunSize		("TestSection.Sky.SunSize",			0.50f,		Debug::Range(0.0f, 1.0f));
Debug::Float				g_debugSkySunOcclusion	("TestSection.Sky.SunOcclusion",	1.0f,		Debug::Range(0.0f, 1.0f));
Debug::Bool					g_debugSkySunBloom		("TestSection.Sky.SunBloom",		true);

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

C_TestSectionSky::C_TestSectionSky(const Util::C_String& name) : Framework::C_TestSectionHDR(name)
{
	m_hPolicy = Scheduler::CreatePolicy(g_SkyPolicy);

	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(1);
	
	m_AtmosphericSkyDome.SetPolicy(m_hPolicy);

	m_TexturedSkyDome.SetTexture(g_debugSkyTexture.Value());
	
	m_MultiTexturedSkyDome.SetBgTexture(g_debugSkyBgTexture.Value());
	m_MultiTexturedSkyDome.SetFgTexture(g_debugSkyFgTexture.Value());
}

C_TestSectionSky::~C_TestSectionSky()
{
	Scheduler::Shutdown();
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
}

void C_TestSectionSky::OnDestroyVolatile()
{
	m_AtmosphericSkyDome.DestroyVolatile();
	m_HDRSkyDome.DestroyVolatile();
	m_LuminanceSkyDome.DestroyVolatile();
	m_PreethamSkyDome.DestroyVolatile();
	m_TexturedSkyDome.DestroyVolatile();
	m_MultiTexturedSkyDome.DestroyVolatile();
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
}

void C_TestSectionSky::OnRender3DViewWithoutAA(const S32 viewIndex)
{
	Util::C_String str;
	str.Printf("Sky Section : %.2f fps", Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}
