#include "StdAfx.h"
#include "Samples/Section/SectionRenderTexture2D.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"

namespace Samples
{

C_SectionRenderTexture2D::C_SectionRenderTexture2D(const Util::C_String& name) : C_Section(name)
{
}

C_SectionRenderTexture2D::~C_SectionRenderTexture2D()
{
}

bool C_SectionRenderTexture2D::OnCreate()
{
	if (Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		m_Mesh.AddInstance();
	
	return true;
}

bool C_SectionRenderTexture2D::OnCreateVolatile()
{
	return true;
}

void C_SectionRenderTexture2D::OnDestroyVolatile()
{
}

void C_SectionRenderTexture2D::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_SectionRenderTexture2D::OnUpdate(const C_Input& input, const C_Timer& timer)
{
}

void C_SectionRenderTexture2D::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionRenderTexture2D");

	m_TextureRender.Bind();
	m_Mesh.Render();
	m_TextureRender.Unbind();
}

void C_SectionRenderTexture2D::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionRenderTexture2D");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::GREY);

	m_SkyDome.Render();
	m_Mesh.Render();
	Render::DrawGrid();

	m_TextureRender.DebugRender();
	
	Util::C_String str;
	str.Printf("Render Texture 2D Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
