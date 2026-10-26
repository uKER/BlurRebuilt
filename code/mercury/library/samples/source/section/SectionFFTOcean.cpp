#include "StdAfx.h"
#include "Samples/Section/SectionFFTOcean.h"

namespace Samples
{

C_SectionFFTOcean::C_SectionFFTOcean(const Util::C_String& name) : C_Section(name), m_Ocean()
{
}

C_SectionFFTOcean::~C_SectionFFTOcean()
{
}

void C_SectionFFTOcean::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	m_Ocean.Update(timer.GetElapsedTime(), timer.GetFrameTime());
}

void C_SectionFFTOcean::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionFFTOcean");

	m_Ocean.PreRender();
}

void C_SectionFFTOcean::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionFFTOcean");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_SkyDome.Render();

	m_Ocean.Render();

	Util::C_String str;
	str.Printf("FFT Ocean Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
