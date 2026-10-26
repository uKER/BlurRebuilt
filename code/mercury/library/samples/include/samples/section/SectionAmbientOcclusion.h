#ifndef SAMPLES_SECTIONAMBIENTOCCLUSION_H
#define SAMPLES_SECTIONAMBIENTOCCLUSION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Sky/SkyDome.h"


namespace Samples
{

class C_SectionAmbientOcclusion : public C_Section
{
public:

	C_SectionAmbientOcclusion(const Util::C_String& name);
	~C_SectionAmbientOcclusion();

	bool OnCreate();
	void OnDestroy();

	void OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer);
	void OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome		m_SkyDome;
	Render::Mesh::C_Mesh		m_Mesh;

	Display::C_Texture3D		m_Texture3D;

	Display::C_PixelShader		m_PixelShader;

	MathsStorage::Float4		m_f4SamplePositions[8];

};

}; // namespace Samples

#endif // SAMPLES_SECTIONAMBIENTOCCLUSION_H
