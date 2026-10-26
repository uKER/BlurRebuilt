#include "stdafx.h"
#include "TestSections/TestSectionBloom.h"

#include <Render/Mesh/MeshLoader.h>
#include <Render/PostProcess/Bloom/Bloom.h>
#include <Render/PostProcess/Bloom/BloomSun.h>
#include <Render/PostProcess/Bloom/BloomFeedback.h>
#include <Render/PostProcess/HDR/HDR.h>

enum E_TestSectionBloomType
{
	E_TestSectionBloomType_Default,
	E_TestSectionBloomType_Sun,
	E_TestSectionBloomType_Feedback,
	E_TestSectionBloomType_Count
};

DEBUG_ENUM(E_TestSectionBloomType)
	DE_OPTION(E_TestSectionBloomType_Default,	"Default")
	DE_OPTION(E_TestSectionBloomType_Sun,		"Sun")
	DE_OPTION(E_TestSectionBloomType_Feedback,	"Feedback")
END_DEBUG_ENUM

Debug::Enum<E_TestSectionBloomType>	g_debugTestSectionBloomType("TestSection.Bloom.Type", E_TestSectionBloomType_Default);

Debug::Float g_debugTestSectionBloomSunHeading("TestSection.Bloom.SunHeading", 0.00f, Debug::Range(0.0f, 360.0f));
Debug::Float g_debugTestSectionBloomSunElevation("TestSection.Bloom.SunElevation", 45.0f, Debug::Range(0.0f, 90.0f));
Debug::Float g_debugTestSectionBloomSunSize("TestSection.Bloom.SunSize", 0.5f, Debug::Range(0.0f, 1.0f));

C_TestSectionBloom::C_TestSectionBloom(const Util::C_String& name) : Framework::C_TestSectionHDR(name)
{
	m_pBloom = new PostProcess::C_Bloom();
	m_pBloomSun = new PostProcess::C_BloomSun();
	m_pBloomFeedback = new PostProcess::C_BloomFeedback();
}

C_TestSectionBloom::~C_TestSectionBloom()
{
}

bool C_TestSectionBloom::OnCreate()
{
	if (!Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		return false;

	m_Mesh.AddInstance();

	return true;
}

void C_TestSectionBloom::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_TestSectionBloom::OnUpdate()
{
	if (g_debugTestSectionBloomType == E_TestSectionBloomType_Default)
	{
		m_pPostProcessHdr->SetBloom(m_pBloom);
	}
	else if (g_debugTestSectionBloomType == E_TestSectionBloomType_Sun)
	{
		m_pPostProcessHdr->SetBloom(m_pBloomSun);
	}
	else if (g_debugTestSectionBloomType == E_TestSectionBloomType_Feedback)
	{
		m_pPostProcessHdr->SetBloom(m_pBloomFeedback);
	}
	m_pPostProcessHdr->SetBloomGamma(true);
}

void C_TestSectionBloom::OnRenderHdr3DViewWithAA(const S32 viewIndex)
{
	// update the postprocess sun parameters
	const F32 fCosHeading = cosf(g_debugTestSectionBloomSunHeading * Maths::Pi / 180.0f);
	const F32 fCosElevation = cosf(g_debugTestSectionBloomSunElevation * Maths::Pi / 180.0f);
	const F32 fSinHeading = sinf(g_debugTestSectionBloomSunHeading * Maths::Pi / 180.0f);
	const F32 fSinElevation = sinf(g_debugTestSectionBloomSunElevation * Maths::Pi / 180.0f);
	const Maths::HVec3 vWorldSunDirection(fSinHeading * fSinElevation, fCosElevation, fCosHeading * fSinElevation);

	// screen position
	const F32 fFarClip = Display::Device()->GetViewSettings3D().GetFarClip();
	Maths::HVec3	vViewPosition = Display::Device()->GetViewSettings3D().GetCameraPosition();
	Maths::HVec4	vPositionOne = Maths::HVec4(vViewPosition + vWorldSunDirection * fFarClip);
					vPositionOne.W(Maths::Constants<Maths::HF32>::One());
	Maths::HMat44	mViewProj = Display::Device()->GetViewSettings3D().GetWorldToDevice();
	Maths::HVec4	vScreenPosition = vPositionOne * mViewProj;
					vScreenPosition *= vScreenPosition.W().Rcp();
	
	if (vScreenPosition.W() < 0.0f)
		m_pBloomSun->SetBloomSun(false);
	else
		m_pBloomSun->SetBloomSun(true);

	const F32 fAspectRatio = 1.0f / Display::Device()->GetViewSettings3D().GetAspectRatio();
	const Display::C_Float2 vTexturePosition(vScreenPosition.X() * 0.5f + 0.5f, vScreenPosition.Y() * -0.5f + 0.5f);

	m_pBloomSun->SetBloomSunIntensity(1.0f);
	m_pBloomSun->SetBloomSunSize(Display::C_Float2(g_debugTestSectionBloomSunSize * fAspectRatio, g_debugTestSectionBloomSunSize));
	m_pBloomSun->SetBloomSunPosition(vTexturePosition);

	// render the scene
	m_Mesh.Render();
	m_SkyDome.Render();
}

void C_TestSectionBloom::OnRenderHdr3DViewWithoutAA(const S32 viewIndex)
{
}
