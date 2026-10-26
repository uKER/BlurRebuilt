#ifndef SAMPLES_SECTIONSCHEDULERTEST_H
#define SAMPLES_SECTIONSCHEDULERTEST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"

namespace Samples
{

class C_SectionSchedulerTest : public C_Section
{
public:

	C_SectionSchedulerTest(const Util::C_String& name);
	~C_SectionSchedulerTest();

	void OnRender(const C_Input& input, const C_Timer& timer);

private:
};

}; // namespace Samples

#endif // SAMPLES_SECTIONSKYDOME_H
