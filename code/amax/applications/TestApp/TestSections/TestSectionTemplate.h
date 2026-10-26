#ifndef TEST_SECTION_TEMPLATE_H
#define TEST_SECTION_TEMPLATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>

class C_TestSectionTemplate : public Framework::C_TestSection
{
public:

	C_TestSectionTemplate(const Util::C_String& name);
	~C_TestSectionTemplate();

	void OnRender2DView();
};

#endif // TEST_SECTION_TEMPLATE_H
