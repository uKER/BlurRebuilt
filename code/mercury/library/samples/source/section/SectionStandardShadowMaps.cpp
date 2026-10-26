#include "StdAfx.h"
#include "Samples/Section/SectionStandardShadowMaps.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"
#include "Render/Shaders/Constants.h"

namespace Samples
{

C_SectionStandardShadowMaps::C_SectionStandardShadowMaps(const Util::C_String& name) : C_Section(name)
{
	m_Light.Initialise(Maths::HVec3(-20.f, 20.f, -20.f), Maths::HVec3(1.f, -1.f, 1.f));
}

C_SectionStandardShadowMaps::~C_SectionStandardShadowMaps()
{
}

bool C_SectionStandardShadowMaps::OnCreate()
{
	if(!m_StandardShadowMap.Create())
		return false;

	if(Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh, Render::Mesh::E_VertexShader_StandardShadowMap, Render::Mesh::E_PixelShader_StandardShadowMap))
		m_Mesh.AddInstance();

	return true;
}

bool C_SectionStandardShadowMaps::OnCreateVolatile()
{
	if(!m_StandardShadowMap.CreateVolatile(1024, 1024))
		return false;

	return true;
}

void C_SectionStandardShadowMaps::OnDestroyVolatile()
{
	m_StandardShadowMap.DestroyVolatile();
}

void C_SectionStandardShadowMaps::OnDestroy()
{
	m_StandardShadowMap.Destroy();

	m_Mesh.Destroy();
}

void C_SectionStandardShadowMaps::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	m_Light.DebugUpdate(timer.GetFrameTime());

	Render::Shadow::C_ShadowMapSetup shadowMapSetup;
	shadowMapSetup.m_NearClip = 10.f;
	shadowMapSetup.m_FarClip = 50.f;
	shadowMapSetup.m_ViewSize = 15.f;
	shadowMapSetup.m_Position = m_Light.m_Position;
	shadowMapSetup.m_Direction = m_Light.m_Direction;
	m_StandardShadowMap.SetupView(shadowMapSetup);
}

void C_SectionStandardShadowMaps::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	m_StandardShadowMap.Bind();
	m_Mesh.Render(Render::Mesh::E_Pass_Depth);
	m_StandardShadowMap.Unbind();
}

void C_SectionStandardShadowMaps::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionStandardShadowMaps");

	m_StandardShadowMap.SetTexture(2, CV_DISPLAY_LIGHT_MATRIX, CV_DISPLAY_SHADOW_MATRIX);

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	m_SkyDome.Render();
	m_Mesh.Render();
	//Render::DrawGrid();

	m_StandardShadowMap.DebugRender();
	m_Light.DebugRender();
	
	Util::C_String str;
	str.Printf("Standard Shadow Maps : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
