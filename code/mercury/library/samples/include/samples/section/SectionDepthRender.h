#ifndef SAMPLES_SECTIONDEPTHRENDER_H
#define SAMPLES_SECTIONDEPTHRENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Render/DepthRender.h"

namespace Samples
{

class C_SectionDepthRender : public C_Section
{
public:

	C_SectionDepthRender(const Util::C_String& name);
	~C_SectionDepthRender();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome	m_SkyDome;
	Render::Mesh::C_Mesh	m_Mesh;
	Render::C_DepthRender	m_DepthRender;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONDEPTHRENDER_H
