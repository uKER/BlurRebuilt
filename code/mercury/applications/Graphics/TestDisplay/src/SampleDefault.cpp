#include "StdAfx.h"
#include "SampleDefault.h"
#include <Mesh2/Mesh/Mesh2Utils.h>
#include <Mesh2/Mesh/Mesh2Render.h>
#include <Display2/Debug/Debug.h>

C_SampleDefault::C_SampleDefault(C_Application* pApplication) : C_Sample(pApplication)
{
	m_MeshFileRoot			= "Meshes/sphere/";
	m_MeshFilePath			= "Meshes/sphere/sphere.";
	#if PLATFORM_XENON
	m_MeshFilePath			+= "x2msh";
	#elif PLATFORM_PS3
	m_MeshFilePath			+= "p3msh";
	#else
	m_MeshFilePath			+= "pcmsh";
	#endif
}

C_SampleDefault::~C_SampleDefault()
{
}

bool C_SampleDefault::OnCreate()
{
	m_MeshShaders.SetDevice(Display2::Device());

	m_pVertexShader = m_MeshShaders.CreateVertexShader("Shaders/vsMesh");

	if(m_pVertexShader)
	{
		m_hWorldMatrix = m_pVertexShader->GetParamFromName("g_WorldMatrix");
		m_hViewProjMatrix = m_pVertexShader->GetParamFromName("g_ViewProjMatrix");
	}

	m_pPixelShader = m_MeshShaders.CreatePixelShader("Shaders/psMesh");

	Render::LoadMesh2(&m_Mesh, m_MeshFilePath, m_MeshFileRoot, Display2::Device());

	Display::CreateTexture2DFromFile("Textures/L8.dds", &m_Texture);

	return true;
}

bool C_SampleDefault::OnCreateVolatile()
{
	return true;
}

void C_SampleDefault::OnDestroyVolatile()
{
}

void C_SampleDefault::OnDestroy()
{
	Display::Device()->Destroy(&m_Texture);

	Render::DestroyMesh2(&m_Mesh, Display2::Device());

	m_MeshShaders.Destroy();
}

void C_SampleDefault::OnUpdate()
{
}


void C_SampleDefault::OnRender()
{
	GPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "Sample");

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);

	Display2::Device()->SetVertexShaderParamF(m_hWorldMatrix, (F32*)&worldMatrix);
	Display2::Device()->SetVertexShaderParamF(m_hViewProjMatrix, (F32*)&viewSettings3D.m_ViewProjMatrix);

	Display2::Device()->SetVertexShader(m_pVertexShader);
	Display2::Device()->SetPixelShader(m_pPixelShader);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	
	//Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);

	#if PLATFORM_PS3
	Display2::C_ContextNoCheckNoTrack context;
	#elif PLATFORM_XENON || PLATFORM_PC
	Display2::C_ContextNoCheckNoTrack context(Display::Device()->GetD3DDevice());
	#endif
	
	#if PLATFORM_PS3
	//U32* pHeadAddress = context.GetData()->current;
	#endif
	
	{
		GPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "Mesh");

		Render::RenderMesh2(&m_Mesh, &context);
	}

	#if PLATFORM_PS3
	//U32* pTailAddress = context.GetData()->current;
	//Display2::C_WalkCommandBufferInput input;
	//Display2::C_WalkCommandBufferOutput output;
	//Display2::WalkCommandBuffer(pHeadAddress, pTailAddress, input, output);
	#endif

	//Display::Device()->SetFillMode(Display::E_FillMode_Solid);

	/*static U32 frame = 0;

	if(frame == 0)
	{
		CellGcmControl* pGcmControl = cellGcmGetControlRegister();
		
		CORE_PRINT("put %.8x get %.8x ref %.8x\n", pGcmControl->put, pGcmControl->get, pGcmControl->ref);

		CORE_PRINT("begin %.8x end %.8x current %.8x (%.8x)\n", gCellGcmCurrentContext->begin, gCellGcmCurrentContext->end, gCellGcmCurrentContext->current, Display2::AddressToOffset(gCellGcmCurrentContext->current));

		cellGcmSetColorMask(0xAAAAAAAA);
		cellGcmSetColorMask(0xAAAAAAAA);
		cellGcmSetColorMask(0xAAAAAAAA);
		cellGcmSetColorMask(0xAAAAAAAA);
		cellGcmSetAlphaFunc(2, 0xAA);
		cellGcmSetAlphaFunc(0, -1);
	}

	frame++;*/

	Display::Device()->Reset();

	Display::BasicRender()->DrawTextured2DRect(50.0f, 50.0f, 200.0f, 200.0f, Maths::RGBA(255, 255, 255, 255), &m_Texture);
}
