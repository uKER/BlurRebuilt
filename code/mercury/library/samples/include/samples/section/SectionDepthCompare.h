#ifndef SAMPLES_SECTIONDEPTHCOMPARE_H
#define SAMPLES_SECTIONDEPTHCOMPARE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Render/DepthRender.h"

namespace Samples
{

class C_SectionDepthCompare : public C_Section
{
public:

	C_SectionDepthCompare(const Util::C_String& name);
	~C_SectionDepthCompare();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome	m_SkyDome;

	Display::C_VertexDeclaration *	m_pVertexDeclaration;
	U32								m_VertexStride;
	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShader;

};

}; // namespace Samples

#endif // SAMPLES_SECTIONDEPTHCOMPARE_H
