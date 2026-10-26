#include "stdafx.h"
#include "TestSections/TestSectionMotionBlur.h"

#include <Effect/PostProcess/MotionBlur/MotionBlurId.h>
#include <Effect/PostProcess/MotionBlurDoFReduced.h>
#include <Render/Mesh/MeshLoader.h>
#include <Render/PostProcess/HDR/HDR.h>

const F32 kMotionBlurBoxBoundary = 20.0f;

C_TestSectionMotionBlur::C_TestSectionMotionBlur(const Util::C_String& name) : Framework::C_TestSectionHDR(name)
{
	m_pMotionBlurId = new PostProcess::C_MotionBlurId(*m_pPostProcessMgr);
	m_pMotionBlurDoFReduced = new PostProcess::C_MotionBlurDoFReduced(*m_pPostProcessMgr);

	m_Boxes[0].m_vPosition = Maths::HVec3(0.0f, 10.0f, 10.0f);
	m_Boxes[1].m_vPosition = Maths::HVec3(5.0f, 0.0f, -15.0f);
	m_Boxes[2].m_vPosition = Maths::HVec3(-10.0f, 5.0f, 0.0f);

	m_Boxes[0].m_vVelocity = Maths::HVec3(50.0f, 0.0f, 0.0f);
	m_Boxes[1].m_vVelocity = Maths::HVec3(0.0f, 30.0f, 0.0f);
	m_Boxes[2].m_vVelocity = Maths::HVec3(0.0f, 0.0f, -60.0f);
}

C_TestSectionMotionBlur::~C_TestSectionMotionBlur()
{
}

bool C_TestSectionMotionBlur::OnCreate()
{
//	if (!Render::Mesh::LoadMesh("Models/Test", &m_Mesh))
//		return false;

//	m_Mesh.AddInstance();

	return true;
}

void C_TestSectionMotionBlur::OnDestroy()
{
//	m_Mesh.Destroy();
}

void C_TestSectionMotionBlur::OnUpdate()
{
	for (U32 i = 0; i < 3; ++i)
	{
		m_Boxes[i].m_vLastPosition	= m_Boxes[i].m_vPosition;
		m_Boxes[i].m_vPosition		= m_Boxes[i].m_vPosition + (1.0f / 60.0f) * m_Boxes[i].m_vVelocity;
		if (m_Boxes[i].m_vPosition.X() > kMotionBlurBoxBoundary || m_Boxes[i].m_vPosition.Y() > kMotionBlurBoxBoundary || m_Boxes[i].m_vPosition.Z() > kMotionBlurBoxBoundary)
			m_Boxes[i].m_vVelocity	*= -1.0f;
		if (m_Boxes[i].m_vPosition.X() < -kMotionBlurBoxBoundary || m_Boxes[i].m_vPosition.Y() < 0.0f || m_Boxes[i].m_vPosition.Z() < -kMotionBlurBoxBoundary)
			m_Boxes[i].m_vVelocity	*= -1.0f;
	}
}

void C_TestSectionMotionBlur::OnRenderHdr3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil, Maths::RGBA(0x00000000), 0.0f, 0);

//	m_Mesh.Render();

	Display::C_StencilState stencilState;
	stencilState.m_StencilEnable		= Display::E_Bool_True;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
	stencilState.m_StencilRef			= 0x00;
	stencilState.m_StencilMask			= 0xFFFFFFFF;
	stencilState.m_StencilWriteMask		= 0xFFFFFFFF;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail		= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass		= Display::E_StencilOp_Replace;

	stencilState.m_StencilRef = 0x01;
	Display::Device()->SetStencilState(stencilState);

	Display::BasicRender()->DrawColoured3DBox(m_Boxes[0].m_vPosition, Maths::HVec3(3.00f, 1.50f, 2.00f), Maths::RGBA(0xFFFF00FF));

	stencilState.m_StencilRef = 0x02;
	Display::Device()->SetStencilState(stencilState);

	Display::BasicRender()->DrawColoured3DBox(m_Boxes[1].m_vPosition, Maths::HVec3(2.75f, 1.50f, 3.50f), Maths::RGBA(0xFFFFFF00));

	stencilState.m_StencilRef = 0x03;
	Display::Device()->SetStencilState(stencilState);

	Display::BasicRender()->DrawColoured3DBox(m_Boxes[2].m_vPosition, Maths::HVec3(2.00f, 4.50f, 3.00f), Maths::RGBA(0xFF00FFFF));

	stencilState.m_StencilEnable = Display::E_Bool_False;
	Display::Device()->SetStencilState(stencilState);

	m_SkyDome.Render();
}

void C_TestSectionMotionBlur::OnRenderHdr3DViewWithoutAA(const S32 viewIndex)
{
}

void C_TestSectionMotionBlur::OnBeginPostEffects()
{
	Maths::HVec3 vMovement1(m_Boxes[0].m_vLastPosition - m_Boxes[0].m_vPosition);
	Maths::HVec3 vMovement2(m_Boxes[1].m_vLastPosition - m_Boxes[1].m_vPosition);
	Maths::HVec3 vMovement3(m_Boxes[2].m_vLastPosition - m_Boxes[2].m_vPosition);

	Maths::HMat44 mMovement1, mMovement2, mMovement3;
	mMovement1.AW(Maths::HVec4(vMovement1.X(), vMovement1.Y(), vMovement1.Z(), 1.0f));
	mMovement2.AW(Maths::HVec4(vMovement2.X(), vMovement2.Y(), vMovement2.Z(), 1.0f));
	mMovement3.AW(Maths::HVec4(vMovement3.X(), vMovement3.Y(), vMovement3.Z(), 1.0f));
	
	m_pMotionBlurId->ClearMatrixPalette();
	m_pMotionBlurId->SetMatrix(1, mMovement1);
	m_pMotionBlurId->SetMatrix(2, mMovement2);
	m_pMotionBlurId->SetMatrix(3, mMovement3);

	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());

	m_pMotionBlurDoFReduced->SetViewportAperture(pView->GetIndex(), 1.0f);
	m_pMotionBlurDoFReduced->SetViewportPlaneInFocus(pView->GetIndex(), 10.0f);
}

void C_TestSectionMotionBlur::OnEndPostEffects()
{
	//m_pMotionBlurId->Display();
	m_pMotionBlurDoFReduced->Display();
	m_pPostProcessHdr->Display();
	m_pPostProcessMgr->End();
}