#include "StdAfx.h"
#include "Samples/Section/SectionRenderTextureCube.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"

namespace Samples
{

C_SectionRenderTextureCube::C_SectionRenderTextureCube(const Util::C_String& name) : C_Section(name)
{
}

C_SectionRenderTextureCube::~C_SectionRenderTextureCube()
{
}

bool C_SectionRenderTextureCube::OnCreate()
{
	if(Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		m_Mesh.AddInstance();
	
	return true;
}

bool C_SectionRenderTextureCube::OnCreateVolatile()
{
	return true;
}

void C_SectionRenderTextureCube::OnDestroyVolatile()
{
}

void C_SectionRenderTextureCube::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_SectionRenderTextureCube::OnUpdate(const C_Input& input, const C_Timer& timer)
{
}

void C_SectionRenderTextureCube::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionRenderTextureCube");

	for(U32 i=0; i<Display::E_CubeTextureFace_Count; i++)
	{
		m_CubeTextureRender.Bind((Display::E_CubeTextureFace)i);
		m_SkyDome.Render();
		m_Mesh.Render();
		m_CubeTextureRender.Unbind((Display::E_CubeTextureFace)i);
	}
}

void C_SectionRenderTextureCube::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionRenderTextureCube");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::GREY);

	m_SkyDome.Render();
	m_Mesh.Render();
	Render::DrawGrid();

	m_CubeTextureRender.DebugRender();

	Util::C_String str;
	str.Printf("Render Texture Cube Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	m_CubeTextureRender.m_Position = viewSettings3D.m_CameraToWorld.AW();
	m_CubeTextureRender.m_ViewSetup.m_ViewSetup3D.m_NearClip = viewSettings3D.m_NearClip;
	m_CubeTextureRender.m_ViewSetup.m_ViewSetup3D.m_FarClip = viewSettings3D.m_FarClip;
}

}; // namespace Samples
