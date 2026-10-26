#ifndef SAMPLES_SECTIONMESH_H
#define SAMPLES_SECTIONMESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"

namespace Samples
{

class C_SectionMesh : public C_Section
{
public:

	C_SectionMesh(const Util::C_String& name);
	~C_SectionMesh();

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
};

}; // namespace Samples

#endif // SAMPLES_SECTIONMESH_H
