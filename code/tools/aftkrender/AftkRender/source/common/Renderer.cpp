#include "AftkRenderPch.h"

#include "common/ShaderManager.h"
#include "common/TextureManager.h"
#include "common/ShaderConstant.h"
#include "common/RenderContext.h"
#include "common/Renderer.h"
#include "common/Scene.h"
#include "common/SceneGraphNode.h"
#include "common/GeometryNode.h"

namespace AftkRender
{
	extern Debug::Logger& g_Log;
}

using namespace AftkRender;

C_ShaderManager		C_Renderer::m_ShaderManager;
C_TextureManager	C_Renderer::m_TextureManager;

C_Renderer::C_Renderer(const char* pName)
: Display::I_Renderer(pName)
, m_pOcclusionQuery(0)
, m_InVisibilityTest(false)
{
}

C_Renderer::~C_Renderer()
{
}

void C_Renderer::BeginVisibilityTest(const RECT & region)
{
	if (m_pOcclusionQuery)
	{

		//will need to set vertex shader before calling...
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
		Display::Device()->SetColourWriteEnable(0x00000000);
		Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasic.shader"));

		Display::Device()->SetScissorRect(region.left, region.top, region.right-region.left, region.bottom-region.top);
		Display::Device()->SetScissorTest(Display::E_Bool_True);

		m_pOcclusionQuery->Issue(D3DISSUE_BEGIN);
		m_InVisibilityTest = true;
	}
	else
		LOGERROR(g_Log, "Occlusion query not yet created...");
}

bool C_Renderer::EndVisibilityTest()
{
	if (m_InVisibilityTest)
	{
		m_pOcclusionQuery->Issue(D3DISSUE_END);

		//stall until we get back the result
		DWORD pixelCount = 0;
		HRESULT hr = S_FALSE;
		while (hr == S_FALSE)
			hr = m_pOcclusionQuery->GetData(&pixelCount, sizeof(DWORD), D3DGETDATA_FLUSH);

		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
		Display::Device()->SetColourWriteEnable(0xffffffff);
		Display::Device()->SetScissorTest(Display::E_Bool_False);

		m_InVisibilityTest = false;
		return (pixelCount>0);
	}

	return true;
}


bool C_Renderer::OnCreate()
{
	bool success = true;
	success &= m_ShaderManager.Initialise();
	success &= m_TextureManager.Initialise();
	return success;
}

bool C_Renderer::OnCreateVolatile()
{
	HRESULT hr = Display::Device()->GetD3DDevice()->CreateQuery(D3DQUERYTYPE_OCCLUSION, &m_pOcclusionQuery);
	if (FAILED(hr))
		LOGERROR(g_Log, "Failed to create D3D occlusion query");

	return true;
}

void C_Renderer::OnDestroyVolatile()
{
	if (m_pOcclusionQuery)
	{
		ULONG refCount = m_pOcclusionQuery->Release();
		assert(refCount == 0);

		m_pOcclusionQuery = 0;
		m_InVisibilityTest = false;
	}
}

void C_Renderer::OnDestroy()
{
	m_ShaderManager.Clear();
	m_TextureManager.FlushUnusedTextures();
}

bool C_Renderer::OnInitialise()
{
	return true;
}

void C_Renderer::OnShutdown()
{
}

void C_Renderer::OnReset()
{
}