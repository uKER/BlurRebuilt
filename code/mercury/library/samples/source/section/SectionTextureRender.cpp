#include "StdAfx.h"
#include "Samples/Section/SectionTextureRender.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"

namespace Samples
{

C_SectionTextureRender::C_SectionTextureRender(const Util::C_String& name) : C_Section(name)
{
}

C_SectionTextureRender::~C_SectionTextureRender()
{
}

bool C_SectionTextureRender::OnCreate()
{
	if (Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		m_Mesh.AddInstance();
	
	return true;
}

bool C_SectionTextureRender::OnCreateVolatile()
{
	return true;
}

void C_SectionTextureRender::OnDestroyVolatile()
{
}

void C_SectionTextureRender::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_SectionTextureRender::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionRenderTexture2D");

	m_TextureRender.Bind();
	m_Mesh.Render();
	m_TextureRender.Unbind();
}

void C_SectionTextureRender::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionRenderTexture2D");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::GREY);

	m_SkyDome.Render();
	m_Mesh.Render();
	//Render::DrawGrid();

	m_TextureRender.DebugRender();
	
	Util::C_String str;
	str.Printf("Render Texture 2D Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
