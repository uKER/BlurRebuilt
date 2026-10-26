#include "stdafx.h"
#include "TestSections/TestSectionLUT.h"

#include <Render/Mesh/MeshLoader.h>
#include <Render/PostProcess/Bloom/Bloom.h>
#include <Render/PostProcess/Colour/LookupTable.h>
#include <Render/PostProcess/HDR/HDR.h>
#include <Render/PostProcess/Manager.h>

Debug::Colour	g_debugTestSectionLUTAdd		("TestSection.LUT.Add",			0.0f, 0.0f, 0.0f, 1.0f);
Debug::Colour	g_debugTestSectionLUTModulate	("TestSection.LUT.Modulate",	1.0f, 1.0f, 1.0f, 1.0f);
Debug::Colour	g_debugTestSectionLUTDesaturate	("TestSection.LUT.Desaturate",	1.0f, 1.0f, 1.0f, 1.0f);
Debug::Float	g_debugTestSectionLUTSepia		("TestSection.LUT.Sepia",		0.0f,	Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugTestSectionLUTBrightness	("TestSection.LUT.Brightness",	0.0f,	Debug::Range(-1.0f, 1.0f));
Debug::Float	g_debugTestSectionLUTContrast	("TestSection.LUT.Contrast",	1.0f,	Debug::Range(0.0f, 2.0f));
Debug::Float	g_debugTestSectionLUTGamma		("TestSection.LUT.Gamma",		1.0f,	Debug::Range(0.0f, 3.0f));

C_TestSectionLUT::C_TestSectionLUT(const Util::C_String& name) : Framework::C_TestSectionHDR(name)
{
	m_pBloom = new PostProcess::C_Bloom(*m_pPostProcessMgr);
	m_pLookupTable = new PostProcess::C_LookupTable(*m_pPostProcessMgr);
	m_pPostProcessHdr->SetType(PostProcess::E_Type_Intermediate);
	m_pLookupTable->SetType(PostProcess::E_Type_Final);
	m_pPostProcessHdr->SetBloom(m_pBloom);
	m_pPostProcessHdr->SetBloomGamma(true);

	for (U32 i = 0; i < 8; ++i)
		m_Policy[i] = 7;
	m_PolicyHandle = Scheduler::CreatePolicy(m_Policy);

	m_pLookupTable->SetJobQueuePolicyHandle(m_PolicyHandle);

	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(1);
}

C_TestSectionLUT::~C_TestSectionLUT()
{
	Scheduler::Shutdown();
}

bool C_TestSectionLUT::OnCreate()
{
	if (!Render::Mesh::LoadMesh("Models/Test", &m_Mesh))
		return false;

	m_Mesh.AddInstance();

	return true;
}

void C_TestSectionLUT::OnDestroy()
{
	m_Mesh.Destroy();
}

void C_TestSectionLUT::OnUpdate()
{
	Debug::ColourStruct vAdd		= g_debugTestSectionLUTAdd.Value();
	Debug::ColourStruct vModulate	= g_debugTestSectionLUTModulate.Value();
	Debug::ColourStruct vDesaturate	= g_debugTestSectionLUTDesaturate.Value();

	m_pLookupTable->SetBrightness(g_debugTestSectionLUTBrightness);
	m_pLookupTable->SetContrast(g_debugTestSectionLUTContrast);
	m_pLookupTable->SetGamma(1.0f / g_debugTestSectionLUTGamma);
	m_pLookupTable->SetSepia(g_debugTestSectionLUTSepia);
	m_pLookupTable->SetAdd(vAdd.r * vAdd.a, vAdd.g * vAdd.a, vAdd.b * vAdd.a);
	m_pLookupTable->SetModulate(vModulate.r * vModulate.a, vModulate.g * vModulate.a, vModulate.b * vModulate.a);
	m_pLookupTable->SetDesaturate(vDesaturate.r * vDesaturate.a, vDesaturate.g * vDesaturate.a, vDesaturate.b * vDesaturate.a);

	m_pLookupTable->BeginUpdate();
}

void C_TestSectionLUT::OnRenderHdr3DViewWithAA(const S32 viewIndex)
{
	m_Mesh.Render();
	m_SkyDome.Render();
}

void C_TestSectionLUT::OnRenderHdr3DViewWithoutAA(const S32 viewIndex)
{
}

void C_TestSectionLUT::OnEndPostEffects()
{
	m_pPostProcessHdr->Display();
	m_pLookupTable->EndUpdate();
	m_pLookupTable->Display();
	m_pPostProcessMgr->End();
}
