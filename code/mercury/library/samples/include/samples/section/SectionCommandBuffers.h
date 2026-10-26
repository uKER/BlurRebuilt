#ifndef SAMPLES_SECTIONCOMMANDBUFFERS_H
#define SAMPLES_SECTIONCOMMANDBUFFERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include <Display/Buffer/CommandBuffer.h>

namespace Samples
{

class C_SectionCommandBuffers : public C_Section
{
public:

	C_SectionCommandBuffers(const Util::C_String& name);
	~C_SectionCommandBuffers();

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
	S32									m_CurrentCommandBuffer;
	Display::C_CommandBuffer			m_CommandBuffer[2];
};

}; // namespace Samples

#endif // SAMPLES_SECTIONCOMMANDBUFFERS_H
