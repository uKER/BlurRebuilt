#ifndef TEST_SECTION_SPLASH_H
#define TEST_SECTION_SPLASH_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "Render/Sky/SkyDome.h"
#include "Render/Texture/RenderUtils.h"
#include "Effect/Fluids/Heightfield.h"
#include "System/Misc/Random.h"

class C_TestSectionSplash : public Framework::C_TestSection
{
public:

	C_TestSectionSplash(const Util::C_String& name);
	~C_TestSectionSplash();
	
	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnUpdate();
	void OnPreRender();
	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender3DViewWithoutAA(const S32 viewIndex);

private:

	Render::Sky::C_SkyDome			m_SkyDome;
	Render::Texture::C_RenderUtils	m_TextureRender;
	
	Effect::C_HeightfieldSurface	m_HeightfieldSurface;
	Effect::C_HeightfieldGpuSolver	m_HeightfieldGpuSolver;
	Effect::C_HeightfieldCpuSolver	m_HeightfieldCpuSolver;
	Display::C_Texture2D			m_SplashTexture;
	Util::C_Random					m_SplashRand;

	U32								m_TextureFence;

};

#endif // TEST_SECTION_SPLASH_H
