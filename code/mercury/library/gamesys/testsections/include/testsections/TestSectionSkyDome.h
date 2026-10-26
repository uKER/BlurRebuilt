#ifndef TEST_SECTIONS_TEST_SECTION_SKY_DOME_H
#define TEST_SECTIONS_TEST_SECTION_SKY_DOME_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace Render { namespace Sky { class C_SkyDome; }; };

namespace Framework
{

class C_TestSectionSkyDome : public C_TestSection
{
public:

	C_TestSectionSkyDome(const Util::C_String& name);
	~C_TestSectionSkyDome();

	void OnRender3DViewWithAA(const S32 viewIndex);

protected:

	Render::Sky::C_SkyDome*		m_pSkyDome;
};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_SKY_DOME_H
