#ifndef TEST_SECTION_POSTPROCESS_H
#define TEST_SECTION_POSTPROCESS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSectionHDR.h>
#include "Render/Sky/SkyDome.h"

class C_TestSectionPostprocess : public Framework::C_TestSectionHDR
{
public:

	C_TestSectionPostprocess(const Util::C_String& name);
	~C_TestSectionPostprocess();

	void OnRenderHdr3DViewWithAA(const S32 viewIndex);
	void OnRenderHdr3DViewWithoutAA(const S32 viewIndex);

private:
	
	Render::Sky::C_SkyDome					m_SkyDome;

};

#endif // TEST_SECTION_POSTPROCESS_H
