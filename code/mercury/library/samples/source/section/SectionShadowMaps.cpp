#include "StdAfx.h"
#include "Samples/Section/SectionShadowMaps.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshBuilder.h"
#include "Render/Mesh/MeshSaver.h"
#include "Render/Mesh/MeshLoader.h"
#include "Render/Shaders/Constants.h"

//#define USE_TEXTURE_RENDER

namespace Samples
{

C_SectionShadowMaps::C_SectionShadowMaps(const Util::C_String& name) : C_Section(name)
{
}

C_SectionShadowMaps::~C_SectionShadowMaps()
{
}

bool C_SectionShadowMaps::OnCreate()
{
	if (Render::Mesh::LoadMesh("Meshes/Floor", &m_FloorMesh))
		m_FloorMesh.AddInstance();
	if (Render::Mesh::LoadMesh("Meshes/Object", &m_ObjectMesh))
		m_ObjectMesh.AddInstance();

	return true;
}

bool C_SectionShadowMaps::OnCreateVolatile()
{
	return true;
}

void C_SectionShadowMaps::OnDestroyVolatile()
{
}

void C_SectionShadowMaps::OnDestroy()
{
	m_FloorMesh.Destroy();
	m_ObjectMesh.Destroy();
}

void C_SectionShadowMaps::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	#ifdef USE_TEXTURE_RENDER
	m_TextureRender.Bind();
	m_ShadowMap.SetShaders();
	#else
	m_ShadowMap.Bind();
	#endif
	m_ObjectMesh.Render(Render::Mesh::E_Pass_Depth);
	#ifdef USE_TEXTURE_RENDER
	m_TextureRender.Unbind();
	#else
	m_ShadowMap.Unbind();
	#endif
}

void C_SectionShadowMaps::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionShadowMaps");

	m_ShadowMap.SetTexture(2);

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	m_SkyDome.Render();
	m_FloorMesh.Render();
	m_ObjectMesh.Render();
	Render::DrawGrid();
	
	#ifdef USE_TEXTURE_RENDER
	Display::BasicRender()->DrawTextured2DRect(1024.f - 300.f - 20.f, 20.f, 300.f, 300.f, Maths::RGBA(255, 255, 255, 255), &m_TextureRender.m_RenderTexture.m_Texture);
	#else
	Display::BasicRender()->DrawTextured2DRect(1024.f - 300.f - 20.f, 20.f, 300.f, 300.f, Maths::RGBA(255, 255, 255, 255), &m_ShadowMap.m_RenderTexture.m_Texture);
	#endif
	
	Util::C_String str;
	str.Printf("Shadow Maps : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
