#ifndef TEST_SECTIONS_TEST_SECTION_FS_H
#define TEST_SECTIONS_TEST_SECTION_FS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace Framework
{

class C_TestSectionFs : public C_TestSection
{
public:

	C_TestSectionFs(const Util::C_String& name);
	~C_TestSectionFs();

protected:
};

}; // namespace Framework

#endif
