#ifndef SAMPLES_SECTIONCELESTIAL_H
#define SAMPLES_SECTIONCELESTIAL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Effects/Celestial/CelestialBody.h"

namespace Samples
{

class C_SectionCelestial : public C_Section
{
public:

	C_SectionCelestial(const Util::C_String& name);
	~C_SectionCelestial();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome m_SkyDome;

	Display::C_OcclusionQueryRender	m_OcclusionQueryRender;

	C_CelestialBody * m_pCelestialBody;

};

}; // namespace Samples

#endif // SAMPLES_SECTIONCELESTIAL_H
