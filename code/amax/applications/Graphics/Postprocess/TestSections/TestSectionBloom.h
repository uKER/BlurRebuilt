#ifndef TEST_SECTION_BLOOM_H
#define TEST_SECTION_BLOOM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSectionHDR.h>
#include <Render/Mesh/Mesh.h>
#include <Render/Sky/SkyDome.h>

namespace PostProcess
{
	class C_Bloom;
	class C_BloomSun;
	class C_BloomFeedback;
};

class C_TestSectionBloom : public Framework::C_TestSectionHDR
{
public:

	C_TestSectionBloom(const Util::C_String& name);
	~C_TestSectionBloom();

	bool OnCreate();
	void OnDestroy();

	void OnUpdate();
	void OnRenderHdr3DViewWithAA(const S32 viewIndex);
	void OnRenderHdr3DViewWithoutAA(const S32 viewIndex);

private:
	
	Render::Mesh::C_Mesh					m_Mesh;
	Render::Sky::C_SkyDome					m_SkyDome;
	PostProcess::C_Bloom*					m_pBloom;
	PostProcess::C_BloomSun*				m_pBloomSun;
	PostProcess::C_BloomFeedback*			m_pBloomFeedback;

};

#endif // TEST_SECTION_BLOOM_H
