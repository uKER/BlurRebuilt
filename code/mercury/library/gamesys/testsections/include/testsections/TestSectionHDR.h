#ifndef TEST_SECTIONS_TEST_SECTION_HDR_H
#define TEST_SECTIONS_TEST_SECTION_HDR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace PostProcess
{
	class C_Bloom;
	class C_HighDynamicRange;
	class C_Manager;
};

namespace Framework
{

class C_TestSectionHDR : public Framework::C_TestSection
{
public:

	C_TestSectionHDR(const Util::C_String& name);
	~C_TestSectionHDR();

	bool OnCreate() { return true; }
	void OnDestroy() {}

	void OnUpdate() {}
	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender3DViewWithoutAA(const S32 viewIndex);

	virtual void OnRenderHdr3DViewWithAA(const S32 viewIndex) {};
	virtual void OnRenderHdr3DViewWithoutAA(const S32 viewIndex) {};
	
	void OnBeginRenderSplitscreenWithoutAA();
	void OnEndRenderSplitscreenWithoutAA();
	void OnResize();

protected:

	void SetupPostProcess();

protected:

	PostProcess::C_Manager*				m_pPostProcessMgr;
	PostProcess::C_HighDynamicRange*	m_pPostProcessHdr;
	PostProcess::C_Bloom*				m_pPostProcessBloom;
};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_HDR_H
