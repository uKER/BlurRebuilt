#include "stdafx.h"
#include "AftkRender.h"

#include "Shaders/vsConstants.h"
//#include "Shaders/psConstants.h"

#include "Shaders/PCD3D9/vsAftkRenderBasic.shader"
#include "Shaders/PCD3D9/vsAftkRenderBillboard.shader"

#include "Shaders/PCD3D9/psAftkRenderDiffuse.shader"
#include "Shaders/PCD3D9/psAftkRenderDiffuseAlpha.shader"

using namespace AftkRender;

I_RenderShader::I_RenderShader()
:
	m_VertexShader(),
	m_PixelShader()
{
}

I_RenderShader::~I_RenderShader()
{
}

Aftk::C_Status I_RenderShader::OnCreate()
{
	return Aftk::C_Status::E_Status_Success;
}

void I_RenderShader::OnEnable()
{
	NewDisplay::Device()->SetVertexShader(&m_VertexShader);
	NewDisplay::Device()->SetPixelShader(&m_PixelShader);
}

void I_RenderShader::OnDisable()
{
	NewDisplay::Device()->SetVertexShader(0);
	NewDisplay::Device()->SetVertexShader(0);
}


void I_RenderShader::SetGlobalConstants()
{
	const NewDisplay::C_ViewSettings3D& viewSettings3D = NewDisplay::Device()->GetViewSettings3D();
	const Maths::HVec3 cameraPosition = viewSettings3D.GetCameraPosition();
	const F32 farClip = viewSettings3D.GetFarClip();

	// Set view projection matrix (assuming it is common to all shaders!)
	{
		// load this into HMat for processing
		Maths::HMat43 view(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrix()));
		Maths::HMat44 proj(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix()));

		Maths::HMat44 viewProj = Maths::HMat44(view) * proj;

		NewDisplay::C_Matrix viewProjMatrix;
		Maths::Store(viewProj, &viewProjMatrix);
		NewDisplay::Device()->SetVertexShaderConstantF(CV_DISPLAY_VIEWPROJ_MATRIX, (F32*)&viewProjMatrix, 4);
	}
}


C_RenderShader_DiffuseTexturedAlpha::C_RenderShader_DiffuseTexturedAlpha()
:
	I_RenderShader()
{
}

C_RenderShader_DiffuseTexturedAlpha::~C_RenderShader_DiffuseTexturedAlpha()
{
	NewDisplay::Device()->Destroy(&m_VertexShader);
	NewDisplay::Device()->Destroy(&m_PixelShader);
}

Aftk::C_Status C_RenderShader_DiffuseTexturedAlpha::OnCreate()
{
	if (!NewDisplay::CreateVertexShaderFromFunc("Shaders/vsAftkRenderBasic", 0, &m_VertexShader, sizeof(vsAftkRenderBasic), vsAftkRenderBasic))
	{
		LOGWARN( g_Log, "Failed to create vertex shader vsAftkRenderDiffuseAlpha\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	if (!NewDisplay::CreatePixelShaderFromFunc("Shaders/psAftkRenderDiffuseAlpha", 0, &m_PixelShader, sizeof(psAftkRenderDiffuseAlpha), psAftkRenderDiffuseAlpha))
	{
		LOGWARN( g_Log, "Failed to create pixel shader psAftkRenderDiffuseAlpha\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	return Aftk::C_Status::E_Status_Success;
}

void C_RenderShader_DiffuseTexturedAlpha::OnEnable()
{
	I_RenderShader::OnEnable();
}

void C_RenderShader_DiffuseTexturedAlpha::OnDisable()
{
	I_RenderShader::OnDisable();
}





C_RenderShader_Billboard::C_RenderShader_Billboard()
:
	I_RenderShader()
{
}

C_RenderShader_Billboard::~C_RenderShader_Billboard()
{
	NewDisplay::Device()->Destroy(&m_VertexShader);
	NewDisplay::Device()->Destroy(&m_PixelShader);
}

Aftk::C_Status C_RenderShader_Billboard::OnCreate()
{
	if (!NewDisplay::CreateVertexShaderFromFunc("Shaders/vsAftkRenderBillboard", 0, &m_VertexShader, sizeof(vsAftkRenderBillboard), vsAftkRenderBillboard))
	{
		LOGWARN( g_Log, "Failed to create vertex shader vsAftkRenderDiffuseAlpha\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	if (!NewDisplay::CreatePixelShaderFromFunc("Shaders/psAftkRenderDiffuseAlpha", 0, &m_PixelShader, sizeof(psAftkRenderDiffuseAlpha), psAftkRenderDiffuseAlpha))
	{
		LOGWARN( g_Log, "Failed to create pixel shader psAftkRenderDiffuseAlpha\n");
		return Aftk::C_Status::E_Status_Failure;
	}

	return Aftk::C_Status::E_Status_Success;
}

void C_RenderShader_Billboard::OnEnable()
{
	I_RenderShader::OnEnable();
}

void C_RenderShader_Billboard::OnDisable()
{
	I_RenderShader::OnDisable();
}