#include "StdAfx.h"
#include "TestSections/TestSectionDepthOfField.h"

#include "Render/Mesh/MeshLoader.h"
#include "Render/Utils/Globals.h"
#include "Render/Utils/Helper.h"

#include "Framework/Application/Application.h"

enum E_DepthOfFieldType
{
	E_DepthOfFieldType_Simple,
	E_DepthOfFieldType_FgBg,
	E_DepthOfFieldType_Classify,
	E_DepthOfFieldType_Count
};

DEBUG_ENUM(E_DepthOfFieldType)
	DE_OPTION(E_DepthOfFieldType_Simple,	"Simple")
	DE_OPTION(E_DepthOfFieldType_FgBg,		"FgBg")
	DE_OPTION(E_DepthOfFieldType_Classify,	"Classify")
END_DEBUG_ENUM

Debug::Enum<E_DepthOfFieldType>	g_debugDepthOfFieldType("TestSection.DepthOfField.Type", E_DepthOfFieldType_Simple);

C_TestSectionDepthOfField::C_TestSectionDepthOfField(const Util::C_String& name) : Framework::C_TestSection(name)
{
	Render::g_pColourBuffer = &m_ColourTexture;
	Render::g_pDepthStencilBuffer = &m_DepthTexture;
}

C_TestSectionDepthOfField::~C_TestSectionDepthOfField()
{
}

bool C_TestSectionDepthOfField::OnCreate()
{
	if (!Display::Device()->CreateTexture2D(1280, 720, 1, Display::E_Usage_System, Display::E_Format_A8R8G8B8, &m_ColourTexture))
		return false;
	if (!Display::Device()->CreateTexture2D(1280, 720, 1, Display::E_Usage_System, Display::E_Format_D24FS8, &m_DepthTexture))
		return false;

	if (!Render::Mesh::LoadMesh("Models/Test", &m_Mesh))
		return false;

	m_Mesh.AddInstance();

	return true;
}

void C_TestSectionDepthOfField::OnDestroy()
{
	m_Mesh.Destroy();

	Display::Device()->Destroy(&m_ColourTexture);
	Display::Device()->Destroy(&m_DepthTexture);
}

void C_TestSectionDepthOfField::OnRender3DViewWithAA(const S32 viewIndex)
{
	PERF_EVENT("RenderSectionDepthOfField");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_SkyDome.Render();
	m_Mesh.Render();
}

void C_TestSectionDepthOfField::OnRender3DViewWithoutAA(const S32 viewIndex)
{
	// resolve colour and depth buffers
#ifdef PLATFORM_XENON
	Display::Device()->Resolve(Display::E_Resolve_Colour, Render::g_pColourBuffer);
	Display::Device()->Resolve(Display::E_Resolve_DepthStencil, Render::g_pDepthStencilBuffer);
#endif

	if (g_debugDepthOfFieldType == E_DepthOfFieldType_Simple)
		m_DepthOfFieldSimple.Render();
	else if (g_debugDepthOfFieldType == E_DepthOfFieldType_FgBg)
		m_DepthOfFieldFgBg.Render();
	else if (g_debugDepthOfFieldType == E_DepthOfFieldType_Classify)
		m_DepthOfFieldClassify.Render();

	Util::C_String str;
	str.Printf("Depth Of Field Section : %.2f fps", Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}
