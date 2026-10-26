#ifndef TEST_SECTION_AUDIO_H
#define TEST_SECTION_AUDIO_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>

class C_TestSectionUI : public Framework::C_TestSection
{
public:

	C_TestSectionUI(const Util::C_String& name);
	~C_TestSectionUI();

	void OnRender2DView();
};

#endif // TEST_SECTION_AUDIO_H
