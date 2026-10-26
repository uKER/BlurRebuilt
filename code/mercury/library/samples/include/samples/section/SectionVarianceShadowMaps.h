#ifndef SAMPLES_SECTIONVARIANCESHADOWMAPS_H
#define SAMPLES_SECTIONVARIANCESHADOWMAPS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Light/Light.h"
#include "Render/Texture/TextureUtils.h"
#include "Render/Shadow/ShadowMapVariance.h"

namespace Samples
{

class C_SectionVarianceShadowMaps : public C_Section
{
public:

	C_SectionVarianceShadowMaps(const Util::C_String& name);
	~C_SectionVarianceShadowMaps();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome				m_SkyDome;
	Render::Mesh::C_Mesh				m_Mesh;
	Render::Light::C_Light				m_Light;
	Render::Texture::C_TextureUtils		m_TextureUtils;
	Render::Shadow::C_VarianceMap		m_VarianceShadowMap;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONVARIANCESHADOWMAPS_H
