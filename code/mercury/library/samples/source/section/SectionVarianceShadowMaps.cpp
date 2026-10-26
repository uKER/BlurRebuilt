#include "StdAfx.h"
#include "Samples/Section/SectionVarianceShadowMaps.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"
#include "Render/Shaders/Constants.h"

namespace Samples
{

Debug::Bool g_debugGaussBlur("Render.ShadowMap.GaussBlur", true);

C_SectionVarianceShadowMaps::C_SectionVarianceShadowMaps(const Util::C_String& name) : C_Section(name)
{
	m_Light.Initialise(Maths::HVec3(-10.f, 10.f, -10.f), Maths::HVec3(1.f, -1.f, 1.f));
}

C_SectionVarianceShadowMaps::~C_SectionVarianceShadowMaps()
{
}

bool C_SectionVarianceShadowMaps::OnCreate()
{
	if(!m_VarianceShadowMap.Create())
		return false;

	if(Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh, Render::Mesh::E_VertexShader_VarianceShadowMap, Render::Mesh::E_PixelShader_VarianceShadowMap))
		m_Mesh.AddInstance();

	return true;
}

bool C_SectionVarianceShadowMaps::OnCreateVolatile()
{
	if(!m_VarianceShadowMap.CreateVolatile(512, 512))
		return false;

	return true;
}

void C_SectionVarianceShadowMaps::OnDestroyVolatile()
{
	m_VarianceShadowMap.DestroyVolatile();
}

void C_SectionVarianceShadowMaps::OnDestroy()
{
	m_VarianceShadowMap.Destroy();

	m_Mesh.Destroy();
}

void C_SectionVarianceShadowMaps::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	m_Light.DebugUpdate(timer.GetFrameTime());

	Render::Shadow::C_ShadowMapSetup shadowMapSetup;
	shadowMapSetup.m_NearClip = 5.f;
	shadowMapSetup.m_FarClip = 25.f;
	shadowMapSetup.m_ViewSize = 5.f;
	shadowMapSetup.m_Position = m_Light.m_Position;
	shadowMapSetup.m_Direction = m_Light.m_Direction;
	m_VarianceShadowMap.SetupView(shadowMapSetup);
}

void C_SectionVarianceShadowMaps::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	m_VarianceShadowMap.Bind();
	m_Mesh.Render(Render::Mesh::E_Pass_Depth);
	m_VarianceShadowMap.Unbind();

	if(g_debugGaussBlur)
	{
		//m_TextureUtils.GaussBlur(&m_VarianceShadowMap.m_RenderTexture, &m_VarianceShadowMap.m_ColourTexture);
	}
	else
	{
	}
}

void C_SectionVarianceShadowMaps::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionVarianceShadowMaps");

	m_VarianceShadowMap.SetTexture(2, CV_DISPLAY_SHADOW_MATRIX);

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	m_SkyDome.Render();
	m_Mesh.Render();
	//Render::DrawGrid();

	m_VarianceShadowMap.DebugRender();
	m_Light.DebugRender();
	
	Util::C_String str;
	str.Printf("Variance Shadow Maps : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
