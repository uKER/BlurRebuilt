#include "StdAfx.h"
#include "Samples/Section/SectionSkyDome.h"
#include "Render/Render/GridRender.h"

namespace Samples
{

C_SectionSkyDome::C_SectionSkyDome(const Util::C_String& name) : C_Section(name)
{
}

C_SectionSkyDome::~C_SectionSkyDome()
{
}

void C_SectionSkyDome::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionSkyDome");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_SkyDome.Render();

	Render::DrawGrid();

	Util::C_String str;
	str.Printf("Sky Dome Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
