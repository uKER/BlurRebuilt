#ifndef TEST_SECTIONS_TEST_SECTION_TEXTURE_H
#define TEST_SECTIONS_TEST_SECTION_TEXTURE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace Render { namespace Sky { class C_SkyDome; }; };

namespace Framework
{

class C_TestSectionTexture : public C_TestSection
{
public:

	C_TestSectionTexture(const Util::C_String& name);
	~C_TestSectionTexture();

	bool OnCreate();
	void OnDestroy();

	void OnUpdate();
	void OnRender3DViewWithAA(const S32 viewIndex);

protected:

	Display::C_Texture			m_Texture2D_L8;
	Display::C_Texture			m_Texture2D_A8R8G8B8;
	Display::C_Texture			m_Texture3D_A8R8G8B8;
	Display::C_Texture			m_TextureCube_DXT1;

	Render::Sky::C_SkyDome*		m_pSkyDome;
};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_TEXTURE_H
