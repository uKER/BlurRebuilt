#include "StdAfx.h"
#include "Samples/Section/SectionEmpty.h"

namespace Samples
{

C_SectionEmpty::C_SectionEmpty(const Util::C_String& name) : C_Section(name)
{
}

C_SectionEmpty::~C_SectionEmpty()
{
}

bool C_SectionEmpty::OnCreate()
{
	Display::CreateTexture2DFromFile("Textures/Test.dds", &m_Texture);

	return true;
}

bool C_SectionEmpty::OnCreateVolatile()
{
	return true;
}

void C_SectionEmpty::OnDestroyVolatile()
{
}

void C_SectionEmpty::OnDestroy()
{
	Display::Device()->Destroy(&m_Texture);
}

void C_SectionEmpty::OnUpdate(const C_Input& input, const C_Timer& timer)
{
}

void C_SectionEmpty::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionEmpty");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	if (m_Texture.IsCreated())
		Display::Quad::DrawFullScreen(&m_Texture);
	
	Util::C_String str;
	str.Printf("%.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 512.f, 384.f, 50.f, 50.f, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
}

}; // namespace Samples
