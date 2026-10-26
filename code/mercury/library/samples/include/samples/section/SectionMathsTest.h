#ifndef SAMPLES_SECTION_MATHS_TEST_H
#define SAMPLES_SECTION_MATHS_TEST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"

namespace Samples
{

class C_SectionMathsTest : public C_Section
{
public:

	C_SectionMathsTest( const Util::C_String& name );
	~C_SectionMathsTest();

	void OnRender( const C_Input& input, const C_Timer& timer );

private:

	
};

}; // namespace Samples

#endif // SAMPLES_SECTION_MATHS_TEST_H
