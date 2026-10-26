#ifndef TEST_SECTIONS_TEST_SECTIONSTENCIL_H
#define TEST_SECTIONS_TEST_SECTIONSTENCIL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace Render { namespace Sky { class C_SkyDome; }; };

namespace Framework
{

class C_TestSectionStencil : public C_TestSection
{
public:

	C_TestSectionStencil(const Util::C_String& name);
	~C_TestSectionStencil();

	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender2DView();

protected:

	Render::Sky::C_SkyDome*		m_pSkyDome;
};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTIONSTENCIL_H
