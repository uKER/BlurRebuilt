#ifndef SAMPLES_SECTIONGEOMCULLING_H
#define SAMPLES_SECTIONGEOMCULLING_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include <Display/SPU/GeomCullingParams.h>
#include <Display/SPU/Edge/Geom/edgegeom_structs.h>
#include <System/SPU/Scheduler.h>
#include <Render/Mesh/Mesh.h>

namespace Samples
{

class C_SectionGeomCulling : public C_Section
{
public:

	C_SectionGeomCulling(const Util::C_String& name);
	~C_SectionGeomCulling();

	bool OnCreate();
	void OnDestroy();

	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Display::C_VertexBufferPatched		m_VertexBuffer;
	Display::C_IndexBuffer				m_IndexBuffer;
	Display::C_VertexDeclaration *		m_pVertexDeclaration;
	U32									m_VertexStride;

	Render::Mesh::C_Mesh				m_Mesh;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONGEOMCULLING_H
