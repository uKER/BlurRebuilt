#include "stdafx.h"
#include "TestSections/TestSectionDisplay2.h"
#include <Render/Render/GridRender.h>
#include <Render/Sky/SkyDome.h>
#include <Display2/Mesh/Mesh2.h>
#include <Display2/Context/Context.h>

C_TestSectionDisplay2::C_TestSectionDisplay2(const Util::C_String& name) : Framework::C_TestSection(name)
{
	m_pDevice = new Display2::C_Device();
	m_pSkyDome = new Render::Sky::C_SkyDome();
	m_pMesh = new Render::Mesh2::C_Mesh(m_pDevice);
}

C_TestSectionDisplay2::~C_TestSectionDisplay2()
{
	SAFE_DELETE_PTR(m_pMesh);
	SAFE_DELETE_PTR(m_pSkyDome);
	SAFE_DELETE_PTR(m_pDevice);
}

bool C_TestSectionDisplay2::OnCreate()
{
	HRESULT hr;

	Display2::C_DeviceParams deviceParams;
	
	#if PLATFORM_PS3
	deviceParams.m_MainMemorySize		= 1024*1024*2;
	deviceParams.m_pMainMemory			= NULL;
	deviceParams.m_LocalMemorySize		= 1024*1024*20;
	deviceParams.m_pLocalMemory			= NULL;
	if(!Display::Device()->AllocMemory(128, deviceParams.m_LocalMemorySize, &deviceParams.m_pLocalMemory, Display::E_AllocHeap_Video, Display::E_AllocType_Start))
	{
		LOGERROR(Display2::g_Log, "Failed to allocate local memory %i\n", deviceParams.m_LocalMemorySize);
		return false;
	}
	cellGcmGetConfiguration(&Display2::g_gcmConfig);
	#endif
	
	m_pDevice->SetParams(deviceParams);
	
	hr = m_pDevice->Create();
	if(FAILURE(hr))
		return false;
	hr = m_pDevice->CreateVolatile();
	if(FAILURE(hr))
		return false;

	Display2::g_pDevice = m_pDevice;

	hr = Display2::CreateVertexShaderFromFile("Shaders/vsMeshAlb", 0, &m_vsAlbedo);
	if(FAILURE(hr))
		return false;

	m_vsAlbedo_WorldMatrix = m_vsAlbedo.GetParamFromName("g_WorldMatrix");
	m_vsAlbedo_ViewProjMatrix = m_vsAlbedo.GetParamFromName("g_ViewProjMatrix");

	hr = Display2::CreatePixelShaderFromFile("Shaders/psMeshAlb", 0, &m_psAlbedo);
	if(FAILURE(hr))
		return false;

	m_psAlbedo_AlbedoTexture = m_psAlbedo.GetParamFromName("g_AlbedoTexture");

	if(!m_pMesh->Load("Meshes/Chess/ChessKing.p3msh"))
		return false;

	return true;
}

void C_TestSectionDisplay2::OnDestroy()
{
	m_pDevice->Destroy(&m_vsAlbedo);
	m_pDevice->Destroy(&m_psAlbedo);

	m_pMesh->Destroy();

	m_pDevice->DestroyVolatile();
	m_pDevice->Destroy();

	Display2::g_pDevice = NULL;
}

void C_TestSectionDisplay2::OnUpdate()
{
	Display2::RefreshShaders();
}

void C_TestSectionDisplay2::OnRender3DViewWithAA(const S32 viewIndex)
{
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	m_pSkyDome->Render();

	Display2::C_Context& context = m_pDevice->GetContext();
	
	context.m_pState->SetDefaultStates();
	context.m_pData = gCellGcmCurrentContext;

	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);

	Display2::Device()->SetVertexShaderParamF(m_vsAlbedo_WorldMatrix, (F32*)&worldMatrix);
	Display2::Device()->SetVertexShaderParamF(m_vsAlbedo_ViewProjMatrix, (F32*)&viewSettings3D.GetViewProjMatrix());
	Display2::Device()->SetVertexShader(&m_vsAlbedo);

	Display2::Device()->SetPixelShader(&m_psAlbedo);

	m_pMesh->Render();

	//Display2::C_ContextNoCheckNoTrack context(gCellGcmCurrentContext);
	//
	//Display2::C_AlphaTestState alphaTestState;
	//alphaTestState.SetDefaultStates();
	//alphaTestState.SetAlphaTestEnable(Display2::E_Bool_True);
	//alphaTestState.SetAlphaTestCmpFunc(Display2::E_CmpFunc_Greater);
	//alphaTestState.SetAlphaTestRef(0x00);
	//context.SetAlphaTestState(alphaTestState);

	//Display2::C_BlendState blendState;
	//blendState.SetDefaultStates();
	//blendState.SetBlendEnable(Display2::E_Bool_True);
	//blendState.SetBlendColourSrc(Display2::E_Blend_SrcAlpha);
	//blendState.SetBlendColourDst(Display2::E_Blend_InvSrcAlpha);
	//blendState.SetBlendColourOp(Display2::E_BlendOp_Add);
	//blendState.SetBlendAlphaSrc(Display2::E_Blend_One);
	//blendState.SetBlendAlphaDst(Display2::E_Blend_Zero);
	//blendState.SetBlendAlphaOp(Display2::E_BlendOp_Add);
	//context.SetBlendState(blendState);

	Render::DrawGrid();

	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
}

void C_TestSectionDisplay2::OnRender2DView()
{
	Display::FontRender()->Draw("Display2", 64.f, 704.f, 20.f, 24.f, Maths::RGBA(255, 255, 255));
}
