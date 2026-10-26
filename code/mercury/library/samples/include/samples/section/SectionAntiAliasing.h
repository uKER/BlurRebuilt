#ifndef SAMPLES_SECTIONANTIALIASING_H
#define SAMPLES_SECTIONANTIALIASING_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Sky/SkyDome.h"


namespace Samples
{

class C_SectionAntiAliasing : public C_Section
{
public:

	C_SectionAntiAliasing(const Util::C_String& name);
	~C_SectionAntiAliasing();

	bool OnCreate();
	void OnDestroy();

	void OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer);
	void OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome		m_SkyDome;
	Render::Mesh::C_Mesh		m_Mesh;

	Display::C_PixelShader		m_PixelShader;

};

}; // namespace Samples

#endif // SAMPLES_SECTIONANTIALIASING_H
