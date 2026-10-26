#ifndef SAMPLES_SECTIONSTANDARDSHADOWMAPS_H
#define SAMPLES_SECTIONSTANDARDSHADOWMAPS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Light/Light.h"
#include "Render/Shadow/ShadowMapStandard.h"

namespace Samples
{

class C_SectionStandardShadowMaps : public C_Section
{
public:

	C_SectionStandardShadowMaps(const Util::C_String& name);
	~C_SectionStandardShadowMaps();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome			m_SkyDome;
	Render::Mesh::C_Mesh			m_Mesh;
	Render::Light::C_Light			m_Light;
	Render::Shadow::C_StandardMap	m_StandardShadowMap;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONSTANDARDSHADOWMAPS_H
