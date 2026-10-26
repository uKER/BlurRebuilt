#include "StdAfx.h"
#include "Samples/Section/SectionCommandBuffers.h"
#include "Render/Render/GridRender.h"
#include "Render/Mesh/MeshLoader.h"

namespace Samples
{

Debug::Bool g_debugCommandBuffersEnable("Samples.Seciton.CommandBuffers.Enable", false);
Debug::Bool g_debugCommandBuffersUpdate0("Samples.Seciton.CommandBuffers.Update0", true);
Debug::Bool g_debugCommandBuffersUpdate1("Samples.Seciton.CommandBuffers.Update1", true);

C_SectionCommandBuffers::C_SectionCommandBuffers(const Util::C_String& name) : C_Section(name)
{
	m_CurrentCommandBuffer = 0;
}

C_SectionCommandBuffers::~C_SectionCommandBuffers()
{
}

bool C_SectionCommandBuffers::OnCreate()
{
	if(Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		m_Mesh.AddInstance();

	U32 size = 1*1024*1024;
	if(!m_CommandBuffer[0].Create(size))
		return false;
	if(!m_CommandBuffer[1].Create(size))
		return false;
	
	return true;
}

bool C_SectionCommandBuffers::OnCreateVolatile()
{
	return true;
}

void C_SectionCommandBuffers::OnDestroyVolatile()
{
}

void C_SectionCommandBuffers::OnDestroy()
{
	m_Mesh.Destroy();
	m_CommandBuffer[0].Destroy();
	m_CommandBuffer[1].Destroy();
}

void C_SectionCommandBuffers::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("OnUpdate");
}

void C_SectionCommandBuffers::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionCommandBuffers");
}

void C_SectionCommandBuffers::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionCommandBuffers");

	if(g_debugCommandBuffersEnable)
	{
		if(g_debugCommandBuffersUpdate0 && m_CurrentCommandBuffer == 0)
		{
			m_CommandBuffer[m_CurrentCommandBuffer].Bind();
			Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
			m_CommandBuffer[m_CurrentCommandBuffer].Unbind();
			g_debugCommandBuffersUpdate0 = false;
		}

		if(g_debugCommandBuffersUpdate1 && m_CurrentCommandBuffer == 1)
		{
			m_CommandBuffer[m_CurrentCommandBuffer].Bind();
			Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
			m_CommandBuffer[m_CurrentCommandBuffer].Unbind();
			g_debugCommandBuffersUpdate1 = false;
		}
	}

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::GREY);

	m_SkyDome.Render();
	if(g_debugCommandBuffersEnable)
	{
		m_CommandBuffer[m_CurrentCommandBuffer].Set();
		m_CurrentCommandBuffer = m_CurrentCommandBuffer == 0 ? 1 : 0;
	}
	m_Mesh.Render();
	Display::Device()->SetFillMode(Display::E_FillMode_Solid);
	Render::DrawGrid();

	Util::C_String str;
	str.Printf("Mesh Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
