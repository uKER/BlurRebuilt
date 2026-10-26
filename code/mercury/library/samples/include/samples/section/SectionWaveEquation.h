#ifndef SAMPLES_SECTIONWAVEEQUATION_H
#define SAMPLES_SECTIONWAVEEQUATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Texture/RenderUtils.h"
#include "Effect/Fluids/WaveEquation.h"
#include "System/Misc/Random.h"

namespace Samples
{

class C_SectionWaveEquation : public C_Section
{
public:

	C_SectionWaveEquation(const Util::C_String& name);
	~C_SectionWaveEquation();
	
	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome			m_SkyDome;
	Render::Texture::C_RenderUtils	m_TextureRender;
	
	Effect::C_WaveEquationSurface	m_WaveSurface;
	Effect::C_WaveEquationSolver	m_WaveSolver;
	Display::C_Texture2D			m_SplashTexture;
	Util::C_Random					m_SplashRand;

};

}; // namespace Samples

#endif // SAMPLES_SECTIONWAVEEQUATION_H
