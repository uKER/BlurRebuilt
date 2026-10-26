#include "stdafx.h"

#include "Shaders/vsConstants.h"
//#include "Shaders/psConstants.h"

#include "Shaders/PCD3D9/vsAftkRenderBasic.shader"
#include "Shaders/PCD3D9/vsAftkRenderSPTBillboardLeaf.shader"

#include "Shaders/PCD3D9/psAftkRenderDiffuse.shader"
#include "Shaders/PCD3D9/psAftkRenderDiffuseAlpha.shader"
#include "Shaders/PCD3D9/psAftkRenderDiffuseVertexLit.shader"
#include "Shaders/PCD3D9/psAftkRenderDiffuseAlphaVertexLit.shader"

using namespace AftkRender;

I_RenderShaderEffect::I_RenderShaderEffect()
:
	m_VertexShader(),
	m_PixelShader()
{
}

I_RenderShaderEffect::~I_RenderShaderEffect()
{
	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);
}

Aftk::C_Status I_RenderShaderEffect::OnCreate()
{
	return Aftk::C_Status::E_Status_Success;
}

void I_RenderShaderEffect::OnEnable()
{
	Display::Device()->SetVertexShader(&m_VertexShader);
	Display::Device()->SetPixelShader(&m_PixelShader);
}

void I_RenderShaderEffect::OnDisable()
{
	Display::Device()->SetVertexShader(0);
	Display::Device()->SetVertexShader(0);
}


void I_RenderShaderEffect::SetGlobalConstants()
{
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	const Maths::HVec3 cameraPosition = viewSettings3D.GetCameraPosition();
	const F32 farClip = viewSettings3D.GetFarClip();

	// Set view projection matrix (assuming it is common to all shaders!)
	{
		// load this into HMat for processing
		Maths::HMat43 view(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrix()));
		Maths::HMat44 proj(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix()));

		Maths::HMat44 viewProj = Maths::HMat44(view) * proj;

		Display::C_Matrix viewProjMatrix;
		Maths::Store(viewProj, &viewProjMatrix);
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_VIEWPROJ_MATRIX, (F32*)&viewProjMatrix, 4);
	}
}


C_RenderShaderEffect_DiffuseTexturedAlpha::C_RenderShaderEffect_DiffuseTexturedAlpha()
:
	I_RenderShaderEffect()
{
}

C_RenderShaderEffect_DiffuseTexturedAlpha::~C_RenderShaderEffect_DiffuseTexturedAlpha()
{
}

Aftk::C_Status C_RenderShaderEffect_DiffuseTexturedAlpha::OnCreate()
{
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsAftkRenderBasic", 0, &m_VertexShader, sizeof(vsAftkRenderBasic), vsAftkRenderBasic))
	{
		LOGWARN( g_Log, "Failed to create vertex shader vsAftkRenderBasic\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	if (!Display::CreatePixelShaderFromFunc("Shaders/psAftkRenderDiffuseAlpha", 0, &m_PixelShader, sizeof(psAftkRenderDiffuseAlpha), psAftkRenderDiffuseAlpha))
	{
		LOGWARN( g_Log, "Failed to create pixel shader psAftkRenderDiffuseAlpha\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	return Aftk::C_Status::E_Status_Success;
}

void C_RenderShaderEffect_DiffuseTexturedAlpha::OnEnable()
{
	I_RenderShaderEffect::OnEnable();
}

void C_RenderShaderEffect_DiffuseTexturedAlpha::OnDisable()
{
	I_RenderShaderEffect::OnDisable();
}

C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit::C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit()
:
	I_RenderShaderEffect()
{
}

C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit::~C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit()
{
}

Aftk::C_Status C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit::OnCreate()
{
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsAftkRenderBasic", 0, &m_VertexShader, sizeof(vsAftkRenderBasic), vsAftkRenderBasic))
	{
		LOGWARN( g_Log, "Failed to create vertex shader vsAftkRenderBasic\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	if (!Display::CreatePixelShaderFromFunc("Shaders/psAftkRenderDiffuseAlphaVertexLit", 0, &m_PixelShader, sizeof(psAftkRenderDiffuseAlphaVertexLit), psAftkRenderDiffuseAlphaVertexLit))
	{
		LOGWARN( g_Log, "Failed to create pixel shader psAftkRenderDiffuseAlphaVertexLit\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	return Aftk::C_Status::E_Status_Success;
}

void C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit::OnEnable()
{
	I_RenderShaderEffect::OnEnable();
}

void C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit::OnDisable()
{
	I_RenderShaderEffect::OnDisable();
}



C_RenderShaderEffect_SPTBillboardLeaf::C_RenderShaderEffect_SPTBillboardLeaf()
:
	I_RenderShaderEffect()
{
}

C_RenderShaderEffect_SPTBillboardLeaf::~C_RenderShaderEffect_SPTBillboardLeaf()
{
}

Aftk::C_Status C_RenderShaderEffect_SPTBillboardLeaf::OnCreate()
{
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsAftkRenderSPTBillboardLeaf", 0, &m_VertexShader, sizeof(vsAftkRenderSPTBillboardLeaf), vsAftkRenderSPTBillboardLeaf))
	{
		LOGWARN( g_Log, "Failed to create vertex shader vsAftkRenderSPTBillboardLeaf\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	if (!Display::CreatePixelShaderFromFunc("Shaders/psAftkRenderDiffuseAlpha", 0, &m_PixelShader, sizeof(psAftkRenderDiffuseAlpha), psAftkRenderDiffuseAlpha))
	{
		LOGWARN( g_Log, "Failed to create pixel shader psAftkRenderDiffuseAlpha\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	return Aftk::C_Status::E_Status_Success;
}

void C_RenderShaderEffect_SPTBillboardLeaf::OnEnable()
{
	I_RenderShaderEffect::OnEnable();
}

void C_RenderShaderEffect_SPTBillboardLeaf::OnDisable()
{
	I_RenderShaderEffect::OnDisable();
}

C_RenderShaderEffect_SPTBillboardLeafVertexLit::C_RenderShaderEffect_SPTBillboardLeafVertexLit()
:
	I_RenderShaderEffect()
{
}

C_RenderShaderEffect_SPTBillboardLeafVertexLit::~C_RenderShaderEffect_SPTBillboardLeafVertexLit()
{
}

Aftk::C_Status C_RenderShaderEffect_SPTBillboardLeafVertexLit::OnCreate()
{
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsAftkRenderSPTBillboardLeaf", 0, &m_VertexShader, sizeof(vsAftkRenderSPTBillboardLeaf), vsAftkRenderSPTBillboardLeaf))
	{
		LOGWARN( g_Log, "Failed to create vertex shader vsAftkRenderSPTBillboardLeaf\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	if (!Display::CreatePixelShaderFromFunc("Shaders/psAftkRenderDiffuseAlphaVertexLit", 0, &m_PixelShader, sizeof(psAftkRenderDiffuseAlphaVertexLit), psAftkRenderDiffuseAlphaVertexLit))
	{
		LOGWARN( g_Log, "Failed to create pixel shader psAftkRenderDiffuseAlphaVertexLit\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	return Aftk::C_Status::E_Status_Success;
}

void C_RenderShaderEffect_SPTBillboardLeafVertexLit::OnEnable()
{
	I_RenderShaderEffect::OnEnable();
}

void C_RenderShaderEffect_SPTBillboardLeafVertexLit::OnDisable()
{
	I_RenderShaderEffect::OnDisable();
}