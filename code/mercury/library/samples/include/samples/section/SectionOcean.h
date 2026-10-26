#ifndef SAMPLES_SECTIONOCEAN_H
#define SAMPLES_SECTIONOCEAN_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Effects/Ocean/Ocean.h"

namespace Samples
{

class C_SectionOcean : public C_Section
{
public:

	C_SectionOcean(const Util::C_String& name);
	~C_SectionOcean();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome m_SkyDome;

	C_Ocean m_Ocean;

	Display::C_Texture2D		m_NormalMap0;
	Display::C_Texture2D		m_NormalMap1;
	Display::C_Texture2D		m_FoamMap;
	Display::C_TextureCube	m_CubeMap;

};

}; // namespace Samples

#endif // SAMPLES_SECTIONOCEAN_H
