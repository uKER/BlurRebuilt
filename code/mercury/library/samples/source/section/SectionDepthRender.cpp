#include "StdAfx.h"
#include "Samples/Section/SectionDepthRender.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"

namespace Samples
{

C_SectionDepthRender::C_SectionDepthRender(const Util::C_String& name) : C_Section(name)
{
}

C_SectionDepthRender::~C_SectionDepthRender()
{
}

bool C_SectionDepthRender::OnCreate()
{
	if (Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		m_Mesh.AddInstance();
	
	return true;
}

bool C_SectionDepthRender::OnCreateVolatile()
{
	return true;
}

void C_SectionDepthRender::OnDestroyVolatile()
{
}

void C_SectionDepthRender::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_SectionDepthRender::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionDepthRender");

	m_DepthRender.Bind();

	m_Mesh.Render(Render::Mesh::E_Pass_Depth);
	m_DepthRender.Unbind();
}

void C_SectionDepthRender::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionDepthRender");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::GREY);

	m_SkyDome.Render();
	m_Mesh.Render();
	//Render::DrawGrid();
	
	Display::BasicRender()->DrawTextured2DRect(100.f, 100.f, 200.f, 200.f, Display::WHITE, &m_DepthRender.m_RenderTexture.m_Texture);
	
	Util::C_String str;
	str.Printf("Render Texture 2D Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
