#include "StdAfx.h"
#include "Samples/Section/SectionPerlin.h"


namespace Samples
{



C_SectionPerlin::C_SectionPerlin(const Util::C_String& name) : C_Section(name)
{
}



C_SectionPerlin::~C_SectionPerlin()
{
}



void C_SectionPerlin::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	m_Perlin.Update( timer.GetTime() );
}



void C_SectionPerlin::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionPerlin");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 0, 0, 255), Display::Device()->GetClearDepth(), Display::Device()->GetClearStencil());

	m_Perlin.Render();

	Display::BasicRender()->DrawTextured2DRect(0.0f, 0.0f, 512.0f, 512.0f, Display::WHITE, m_Perlin.Texture());

	Util::C_String str;
	str.Printf("Perlin Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.0f, 45.0f, 20.0f, 20.0f, Display::RGBA(255, 255, 255, 255));
}



}; // namespace Samples
