#include "StdAfx.h"
#include "Samples/Section/SectionSaverTextureCube.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"

#if defined(CFG_ALLOW_OPTIMIZE_OFF) || defined(USER_LLOYDW)
#pragma optimize( "", off ) 
#endif

namespace Samples
{

C_SectionSaverTextureCube::C_SectionSaverTextureCube(const Util::C_String& name) : C_Section(name)
{
}

C_SectionSaverTextureCube::~C_SectionSaverTextureCube()
{
}

bool C_SectionSaverTextureCube::OnCreate()
{
	if(Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		m_Mesh.AddInstance();
	
	return true;
}

bool C_SectionSaverTextureCube::OnCreateVolatile()
{
	return true;
}

void C_SectionSaverTextureCube::OnDestroyVolatile()
{
}

void C_SectionSaverTextureCube::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_SectionSaverTextureCube::OnUpdate(const C_Input& input, const C_Timer& timer)
{
}

void C_SectionSaverTextureCube::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionSaverTextureCube");

	for(U32 i=0; i<Display::E_CubeTextureFace_Count; i++)
	{
		m_CubeTextureSaver.Bind((Display::E_CubeTextureFace)i);
		m_SkyDome.Render();
		m_Mesh.Render();
		m_CubeTextureSaver.Unbind((Display::E_CubeTextureFace)i);
	}

	#if 1
	static bool firstTime = true;
	if(firstTime)
	{
		m_CubeTextureSaver.Save("Textures/CubeMaps/Test");
		firstTime = false;
	}
	#endif
}

void C_SectionSaverTextureCube::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionSaverTextureCube");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::GREY);

	m_SkyDome.Render();
	m_Mesh.Render();
	Render::DrawGrid();

	m_CubeTextureSaver.DebugRender();

	Util::C_String str;
	str.Printf("Saver Texture Cube Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	m_CubeTextureSaver.m_Position = viewSettings3D.m_CameraToWorld.AW();
	m_CubeTextureSaver.m_ViewSetup.m_ViewSetup3D.m_NearClip = viewSettings3D.m_NearClip;
	m_CubeTextureSaver.m_ViewSetup.m_ViewSetup3D.m_FarClip = viewSettings3D.m_FarClip;
}

}; // namespace Samples
