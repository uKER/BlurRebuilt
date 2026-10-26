#ifndef SAMPLES_SECTIONFFTOCEAN_H
#define SAMPLES_SECTIONFFTOCEAN_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Effect/Ocean/Ocean.h"
#include "Render/Sky/SkyDome.h"
#include "Samples/Section/Section.h"

namespace Samples
{

class C_SectionFFTOcean : public C_Section
{
public:

	C_SectionFFTOcean(const Util::C_String& name);
	~C_SectionFFTOcean();

	virtual void OnUpdate(const C_Input& input, const C_Timer& timer);
	virtual void OnPreRender(const C_Input& input, const C_Timer& timer);
	virtual void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome	m_SkyDome;

	Effect::Ocean::C_Ocean	m_Ocean;

};

}; // namespace Samples

#endif // SAMPLES_SECTIONFFTOCEAN_H
