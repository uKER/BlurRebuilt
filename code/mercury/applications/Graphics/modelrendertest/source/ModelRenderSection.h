#ifndef TEST_SECTION_TEMPLATE_H
#define TEST_SECTION_TEMPLATE_H

#ifdef _MSC_VER
#pragma once
#endif

class ModelRendererApp;
struct Effect;
namespace Render { namespace Sky { class C_SkyDome; } }
namespace Cam2
{
	class C_DebugTools;
	class C_FlyCamera;
}

#include <TestSections/TestSectionHDR.h>
#include <Render/PostProcess/Manager.h>
#include <Render/PostProcess/HDR/HDR.h>

class C_ModelRenderSection : public Framework::C_TestSectionHDR
{
public:

	C_ModelRenderSection(const Util::C_String& name);
	~C_ModelRenderSection();

	void OnUpdate();
	void OnRenderHdr3DViewWithAA(const S32 iView);

protected:

	void SetupPostProcess();

protected:

	ModelRendererApp *m_pModelRendererApp;

	Effect*							m_pFX;
	Display::C_TextureCube*			m_pCubemap;
	Render::Sky::C_SkyDome*			m_pSkyDome;
	Cam2::C_DebugTools*				m_pDebugTools;
	Ut::Vector<Cam2::C_FlyCamera*>	m_FlyCameras;
};

#endif // TEST_SECTION_TEMPLATE_H
