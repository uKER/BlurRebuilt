#include "StdAfx.h"
#include "TestSections/TestSectionOcean.h"
#include "Framework/Application/Application.h"
#include "Render/Utils/Helper.h"

C_TestSectionOcean::C_TestSectionOcean(const Util::C_String& name) : Framework::C_TestSection(name)
{
}

C_TestSectionOcean::~C_TestSectionOcean()
{
}

void C_TestSectionOcean::OnUpdate()
{
	m_Ocean.Update(Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime(), Framework::C_Application::GetInstance()->GetTimer().GetFrameTime());
}

void C_TestSectionOcean::OnPreRender()
{
	PERF_EVENT("PreRenderTestSectionOcean");

	m_Ocean.PreRender();
}

void C_TestSectionOcean::OnRender3DViewWithAA(const S32 viewIndex)
{
	PERF_EVENT("RenderTestSectionOcean");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_SkyDome.Render();

	m_Ocean.Render();

	Util::C_String str;
	str.Printf("Ocean Test Section : %.2f fps", Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}
