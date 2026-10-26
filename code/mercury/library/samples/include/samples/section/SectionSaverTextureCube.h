#ifndef SAMPLES_SECTIONSAVERTEXTURECUBE_H
#define SAMPLES_SECTIONSAVERTEXTURECUBE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Render/CubeTextureSaver.h"

namespace Samples
{

class C_SectionSaverTextureCube : public C_Section
{
public:

	C_SectionSaverTextureCube(const Util::C_String& name);
	~C_SectionSaverTextureCube();

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
	Render::C_CubeTextureSaver			m_CubeTextureSaver;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONSAVERTEXTURECUBE_H
