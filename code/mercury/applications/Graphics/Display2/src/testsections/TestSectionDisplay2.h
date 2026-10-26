#ifndef TEST_SECTION_DISPLAY2_H
#define TEST_SECTION_DISPLAY2_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>

namespace Render { namespace Sky { class C_SkyDome; }; };
namespace Render { namespace Mesh2 { class C_Mesh; }; };

class C_TestSectionDisplay2 : public Framework::C_TestSection
{
public:

	C_TestSectionDisplay2(const Util::C_String& name);
	~C_TestSectionDisplay2();

	bool OnCreate();
	void OnDestroy();
	void OnUpdate();
	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender2DView();

protected:

	Display2::C_VertexShaderParam	m_vsAlbedo_WorldMatrix;
	Display2::C_VertexShaderParam	m_vsAlbedo_ViewProjMatrix;
	Display2::C_VertexShader		m_vsAlbedo;

	Display2::C_PixelShaderParam	m_psAlbedo_AlbedoTexture;
	Display2::C_PixelShader			m_psAlbedo;
	
	Display2::C_Device*			m_pDevice;
	
	Render::Mesh2::C_Mesh*		m_pMesh;
	Render::Sky::C_SkyDome*		m_pSkyDome;
};

#endif // TEST_SECTION_DISPLAY2_H
