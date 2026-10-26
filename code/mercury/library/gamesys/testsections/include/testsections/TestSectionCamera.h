#ifndef TEST_SECTIONS_TEST_SECTION_CAMERA_H
#define TEST_SECTIONS_TEST_SECTION_CAMERA_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"
namespace Cam2 { class C_DebugTools; }
namespace Render { namespace Sky { class C_SkyDome; } }

namespace Framework
{

class C_TestSectionCamera : public C_TestSection
{
public:

	C_TestSectionCamera(const Util::C_String& name);
	~C_TestSectionCamera();

	void OnUpdate();
	void CycleCameras();
	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnSetupUserControls(const S32) { };

protected:
	Render::Sky::C_SkyDome* m_pSkyDome;
	Cam2::C_DebugTools* m_pDebugTools;
	S32 m_camGroup, m_camSel;
	Ut::Vector< Ut::Vector< S32 > > m_camList;
};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_CAMERA_H
