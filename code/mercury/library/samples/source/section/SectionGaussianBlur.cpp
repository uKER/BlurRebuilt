#include "StdAfx.h"
#include "Samples/Section/SectionGaussianBlur.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"

namespace Samples
{

C_SectionGaussianBlur::C_SectionGaussianBlur(const Util::C_String& name) : C_Section(name)
{
}

C_SectionGaussianBlur::~C_SectionGaussianBlur()
{
}

bool C_SectionGaussianBlur::OnCreate()
{
	if (Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		m_Mesh.AddInstance();
	
	return true;
}

bool C_SectionGaussianBlur::OnCreateVolatile()
{
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
	
	if (!Display::Device()->CreateRenderTexture2D(m_TextureRender.m_Width, m_TextureRender.m_Height, 1, Display::E_Usage_Colour, backBuffer.GetColourFormat(), Display::E_Multisample_None, NULL, &m_RenderTexture))
		return false;

	return true;
}

void C_SectionGaussianBlur::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_RenderTexture);
}

void C_SectionGaussianBlur::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_SectionGaussianBlur::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionRenderTexture2D");

	m_TextureRender.Bind();
	m_Mesh.Render();
	m_TextureRender.Unbind();

	m_TextureUtils.GaussBlur(&m_TextureRender.m_RenderTexture, &m_RenderTexture);
}

void C_SectionGaussianBlur::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionRenderTexture2D");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::GREY);

	m_SkyDome.Render();
	m_Mesh.Render();
	//Render::DrawGrid();

	m_TextureRender.DebugRender();
	
	Util::C_String str;
	str.Printf("GaussianBlur Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
