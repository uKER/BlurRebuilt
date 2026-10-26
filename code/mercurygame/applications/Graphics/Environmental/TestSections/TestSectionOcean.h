#ifndef TEST_SECTION_OCEAN_H
#define TEST_SECTION_OCEAN_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Effect/Ocean/Ocean.h"
#include <TestSections/TestSection.h>
#include "Render/Sky/SkyDome.h"

class C_TestSectionOcean : public Framework::C_TestSection
{
public:

	C_TestSectionOcean(const Util::C_String& name);
	~C_TestSectionOcean();

	virtual void OnUpdate();
	virtual void OnPreRender();
	virtual void OnRender3DViewWithAA(const S32 viewIndex);

private:

	Render::Sky::C_SkyDome	m_SkyDome;

	Effect::Ocean::C_Ocean	m_Ocean;

};

#endif // TEST_SECTION_OCEAN_H
