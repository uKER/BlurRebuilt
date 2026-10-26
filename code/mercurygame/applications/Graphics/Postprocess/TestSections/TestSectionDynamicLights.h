#ifndef TEST_SECTION_DYNAMIC_LIGHTS_H
#define TEST_SECTION_DYNAMIC_LIGHTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "Render/Deferred/DynamicLights.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Texture/Depth.h"

class C_TestSectionDynamicLights : public Framework::C_TestSection
{
public:

	C_TestSectionDynamicLights(const Util::C_String& name);
	~C_TestSectionDynamicLights();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroy();
	void OnDestroyVolatile();

	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender3DViewWithoutAA(const S32 viewIndex);

private:
	
	Render::Mesh::C_Mesh					m_Mesh;
	Render::Sky::C_SkyDome					m_SkyDome;
	Render::Texture::C_Depth				m_Depth;
	Render::Deferred::C_DynamicLights		m_DynamicLights;

	Display::C_Texture2D					m_ColourTexture;
	Display::C_Texture2D					m_DepthTexture;

	Display::C_RenderTexture2D				m_NormalRenderTexture;

};

#endif // TEST_SECTION_DYNAMIC_LIGHTS_H
