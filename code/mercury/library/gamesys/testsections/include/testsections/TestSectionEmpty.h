#ifndef TEST_SECTIONS_TEST_SECTION_EMPTY_H
#define TEST_SECTIONS_TEST_SECTION_EMPTY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace Framework
{

class C_TestSectionEmpty : public C_TestSection
{
public:

	C_TestSectionEmpty(const Util::C_String& name);
	~C_TestSectionEmpty();
};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_EMPTY_H
