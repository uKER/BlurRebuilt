#ifndef SAMPLES_SECTIONFRAGMENTPATCH_H
#define SAMPLES_SECTIONFRAGMENTPATCH_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
#include <Display/SPU/FPIntoDefines.h>
#include <Display/SPU/FPIntoParams.h>
#include <System/SPU/Scheduler.h>
#endif

#include "Samples/Section/Section.h"
#include "Render/Patch/Patch.h"
#include "Render/Patch/ShaderBuffer.h"

namespace Samples
{

class C_SectionFragmentPatch : public C_Section
{
public:

	C_SectionFragmentPatch(const Util::C_String& name);
	~C_SectionFragmentPatch();

	bool OnCreate();
	void OnDestroy();

	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShader;
	Display::C_PixelShaderParameter	m_psColour;

	Display::C_VertexBuffer			m_VertexBuffer;
	Display::C_IndexBuffer			m_IndexBuffer;
	Display::C_VertexDeclaration *	m_pVertexDeclaration;
	U32								m_VertexStride;

	Render::Patch::C_Patcher		m_Patcher;
	Render::Patch::C_DoubleBuffer	m_DoubleBuffer;

};

}; // namespace Samples

#endif // SAMPLES_SECTIONFRAGMENTPATCH_H
