#include "StdAfx.h"
#include "Samples/Section/SectionOcclusionQuery.h"
#include "Render/Render/GridRender.h"

namespace Samples
{

C_SectionOcclusionQuery::C_SectionOcclusionQuery(const Util::C_String& name) : C_Section(name)
{
	m_Visible = 0.f;
}

C_SectionOcclusionQuery::~C_SectionOcclusionQuery()
{
}

bool C_SectionOcclusionQuery::OnCreate()
{
	if (!m_OcclusionTest.Create(2))
		return false;

	return true;
}

void C_SectionOcclusionQuery::OnDestroy()
{
	m_OcclusionTest.Destroy();
}

void C_SectionOcclusionQuery::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionOcclusionQuery");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	m_SkyDome.Render();

	Display::BasicRender()->DrawColoured3DBox(Maths::HVec3(0.f, 2.f, -6.f), Maths::HVec3(4.f, 2.f, 0.5f), Display::GREEN);

	Render::DrawGrid();

	// receive occlusion query
	m_Visible = m_OcclusionTest.Visibility();
	
	// send occlusion query
	OcclusionTests::Begin(true);
		Maths::HMat43 matrix;
		Maths::HVec3 scale(1.0f, 1.0f, 1.0f);
		matrix.SetIdentity();

		m_OcclusionTest.Ask(matrix, scale);
	OcclusionTests::End();

	Util::C_String str;
	str.Printf("Occlusion Query Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));

	str.Printf("Occlusion Query Result = %.3f %s", m_Visible, m_Visible > 0.f ? "Visible" : "NotVisible");
	Display::FontRender()->Draw(str.Chars(), 40.f, 80.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
