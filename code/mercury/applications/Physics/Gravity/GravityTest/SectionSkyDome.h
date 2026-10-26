#ifndef SAMPLES_SECTIONSKYDOME_H
#define SAMPLES_SECTIONSKYDOME_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"

namespace Samples
{

class C_SectionSkyDome : public C_Section
{
public:

	C_SectionSkyDome(const Util::C_String& name);
	~C_SectionSkyDome();

	void OnRender(const C_Input& input, const C_Timer& timer);
	void OnUpdate(const Samples::C_Input &input, const Samples::C_Timer &timer);

private:

	Render::Sky::C_SkyDome m_SkyDome;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONSKYDOME_H
