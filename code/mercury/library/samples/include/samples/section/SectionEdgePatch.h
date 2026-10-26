#ifndef SAMPLES_SECTIONEDGEPATCH_H
#define SAMPLES_SECTIONEDGEPATCH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"

namespace Samples
{

class C_SectionEdgePatch : public C_Section
{
public:

	C_SectionEdgePatch(const Util::C_String& name);
	~C_SectionEdgePatch();

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

};

}; // namespace Samples

#endif // SAMPLES_SECTIONEDGEPATCH_H
