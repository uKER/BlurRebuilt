#ifndef TEST_SECTIONS_TEST_SECTION_CURVE_H
#define TEST_SECTIONS_TEST_SECTION_CURVE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace Render { namespace Sky { class C_SkyDome; }; };

namespace Framework
{

class C_TestSectionCurve : public C_TestSection
{
public:

	C_TestSectionCurve(const Util::C_String& name);
	~C_TestSectionCurve();

	virtual void OnUpdate();

	void OnRender3DViewWithAA(const S32 viewIndex);

protected:
	Render::Sky::C_SkyDome*		m_pSkyDome;
	F32							m_param;
};

}; 

#endif 
