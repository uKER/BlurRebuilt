#ifndef TEST_SECTION_TEMPLATE_H
#define TEST_SECTION_TEMPLATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include <System/Scheduler/JobQueue.h>

class C_TestSectionSort : public Framework::C_TestSection
{
public:

	C_TestSectionSort(const Util::C_String& name);
	~C_TestSectionSort();

	void OnUpdate();
	void OnRender2DView();

};

#endif // TEST_SECTION_TEMPLATE_H
