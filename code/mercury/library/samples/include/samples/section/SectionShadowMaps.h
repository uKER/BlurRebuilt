#ifndef SAMPLES_SECTIONSHADOWMAPS_H
#define SAMPLES_SECTIONSHADOWMAPS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Shadow/ShadowMap.h"
#include "Render/Render/TextureRender.h"

namespace Samples
{

class C_SectionShadowMaps : public C_Section
{
public:

	C_SectionShadowMaps(const Util::C_String& name);
	~C_SectionShadowMaps();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Shadow::C_ShadowMap		m_ShadowMap;
	Render::Sky::C_SkyDome			m_SkyDome;
	Render::Mesh::C_Mesh			m_FloorMesh;
	Render::Mesh::C_Mesh			m_ObjectMesh;
	Render::C_TextureRender			m_TextureRender;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONSHADOWMAPS_H
