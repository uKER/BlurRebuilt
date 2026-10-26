#include "StdAfx.h"
#include "Samples/Section/SectionViewFrustum.h"
#include "Render/Render/GridRender.h"

namespace Samples
{

C_SectionViewFrustum::C_SectionViewFrustum(const Util::C_String& name) : C_Section(name)
{
}

C_SectionViewFrustum::~C_SectionViewFrustum()
{
}

bool C_SectionViewFrustum::OnCreate()
{
	return true;
}

void C_SectionViewFrustum::OnDestroy()
{
}

void C_SectionViewFrustum::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionViewFrustum");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	m_SkyDome.Render();

	const Display::C_ViewFrustum& viewFrustum = Display::Device()->GetViewFrustum();

	F32 x = 10.f, y = 0.f, z = 10.f;
	Maths::HVec3 dim(5.f, 5.f, 5.f);

	const Display::C_CullResult cullResult = viewFrustum.CullAxisBox(Maths::HVec4(x, y, z, 1.f), dim);

	if (cullResult.Visible())
	{
		Display::BasicRender()->DrawColoured3DBox(Maths::HVec3(x, y, z), dim, Display::GREEN);
	}

	Render::DrawGrid();
	
	Util::C_String str;
	str.Printf("View Frustum Test : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));

	str.Printf("View Frustum Result = %s", cullResult.Visible() ? "Visible" : "NotVisible");
	Display::FontRender()->Draw(str.Chars(), 40.f, 80.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
