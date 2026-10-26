#ifndef TEST_SECTIONS_TEST_SECTION_LAYER_H
#define TEST_SECTIONS_TEST_SECTION_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Framework/Layer/Layer.h"
#include "TestSections/TestSectionManager.h"
#include "TestSections/TestSectionLauncher.h"

namespace Framework
{

//! C_TestSectionLayer

class C_TestSectionLayer : public C_Layer
{
public:

	C_TestSectionLayer();
	~C_TestSectionLayer();

	template <class T>
	void AddTestSection(const Util::C_String& sectionName)
	{
		assert(m_pTestSectionManager);
		m_pTestSectionManager->AddSection(new C_TestSectionLauncher<T>(sectionName));
	}

	void StraightToTestSection(const Util::C_String& testSectionName);

	HRESULT ProcessMessage(const E_LayerMessage message);

private:

	HRESULT Create();
	void Destroy();

	void Update();
	void PreRender();
	void RenderSplitscreenWithAA();
	void RenderSplitscreenWithoutAA();
	void RenderFullscreen();
	void PostRender();
	void BeginRenderSplitscreenWithoutAA();
	void EndRenderSplitscreenWithoutAA();

private:
	
	bool						m_StraightToTestSection;
	C_TestSectionManager*		m_pTestSectionManager;
};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_LAYER_H
