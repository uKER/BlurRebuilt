#ifndef TEST_SECTION_LUT_H
#define TEST_SECTION_LUT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSectionHDR.h>
#include <Render/Mesh/Mesh.h>
#include <Render/Sky/SkyDome.h>
#include <System/Scheduler/JobQueue.h>

namespace PostProcess
{
	class C_Bloom;
	class C_LookupTable;
};

class C_TestSectionLUT : public Framework::C_TestSectionHDR
{
public:

	C_TestSectionLUT(const Util::C_String& name);
	~C_TestSectionLUT();

	bool OnCreate();
	void OnDestroy();

	void OnUpdate();
	void OnRenderHdr3DViewWithAA(const S32 viewIndex);
	void OnRenderHdr3DViewWithoutAA(const S32 viewIndex);
	void OnEndPostEffects();

private:
	
	Render::Mesh::C_Mesh		m_Mesh;
	Render::Sky::C_SkyDome		m_SkyDome;
	PostProcess::C_Bloom*		m_pBloom;
	PostProcess::C_LookupTable*	m_pLookupTable;

	Scheduler::Policy			m_Policy[8];
	Scheduler::PolicyHandle		m_PolicyHandle;

};

#endif // TEST_SECTION_LUT_H
