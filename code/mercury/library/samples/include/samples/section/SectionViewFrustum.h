#ifndef SAMPLES_SECTIONVIEWFRUSTUM_H
#define SAMPLES_SECTIONVIEWFRUSTUM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"

namespace Samples
{

class C_SectionViewFrustum : public C_Section
{
public:

	C_SectionViewFrustum(const Util::C_String& name);
	~C_SectionViewFrustum();

	bool OnCreate();
	void OnDestroy();

	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome m_SkyDome;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONVIEWFRUSTUM_H
