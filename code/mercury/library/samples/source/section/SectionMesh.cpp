#include "StdAfx.h"
#include "Samples/Section/SectionMesh.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"

namespace Samples
{

C_SectionMesh::C_SectionMesh(const Util::C_String& name) : C_Section(name)
{
}

C_SectionMesh::~C_SectionMesh()
{
}

bool C_SectionMesh::OnCreate()
{
	if(Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		m_Mesh.AddInstance();
	
	return true;
}

bool C_SectionMesh::OnCreateVolatile()
{
	return true;
}

void C_SectionMesh::OnDestroyVolatile()
{
}

void C_SectionMesh::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_SectionMesh::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("OnUpdate");
}

void C_SectionMesh::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionMesh");
}

void C_SectionMesh::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionMesh");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::GREY);

	m_SkyDome.Render();
	m_Mesh.Render();
	Render::DrawGrid();

	Util::C_String str;
	str.Printf("Mesh Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
