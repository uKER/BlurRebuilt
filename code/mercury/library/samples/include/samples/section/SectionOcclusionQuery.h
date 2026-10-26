#ifndef SAMPLES_SECTIONOCCLUSIONQUERY_H
#define SAMPLES_SECTIONOCCLUSIONQUERY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Display/Occlusion/OcclusionQuery.h"
#include "Render/Occlusion/OcclusionTests.h"
#include "Render/Sky/SkyDome.h"

namespace Samples
{

class C_SectionOcclusionQuery : public C_Section
{
public:

	C_SectionOcclusionQuery(const Util::C_String& name);
	~C_SectionOcclusionQuery();

	bool OnCreate();
	void OnDestroy();

	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	F32									m_Visible;
	Display::C_OcclusionQueryRender	m_OcclusionQueryRender;
	OcclusionTests::C_PercentageVisible	m_OcclusionTest;
	Render::Sky::C_SkyDome				m_SkyDome;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONOCCLUSIONQUERY_H
