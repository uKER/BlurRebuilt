#ifndef TEST_SECTION_SKY_H
#define TEST_SECTION_SKY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "Render/Mesh/Mesh.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Effect/Sky/AtmosphericSkyDome.h"
#include "Effect/Sky/HDRSkyDome.h"
#include "Effect/Sky/LitCloudDome.h"
#include "Effect/Sky/LuminanceSkyDome.h"
#include "Effect/Sky/MultiTexturedSkyDome.h"
#include "Effect/Sky/PreethamSkyDome.h"
#include "Effect/Sky/TexturedSkyDome.h"
#include "Effect/Sky/TexturedCloudPlane.h"

class C_TestSectionSky : public Framework::C_TestSection
{
public:

	C_TestSectionSky(const Util::C_String& name);
	~C_TestSectionSky();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroy();
	void OnDestroyVolatile();
	void OnUpdate();
	void OnPreRender();
	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender3DViewWithoutAA(const S32 viewIndex);

	void OnBeginPostEffects();
	void OnEndPostEffects();
	void OnResize();

private:

	void SetupPostProcess();

private:

	PostProcess::C_Manager*				m_pPostProcessMgr;
	PostProcess::C_HighDynamicRange*	m_pPostProcessHdr;

	Render::Mesh::C_Mesh				m_Mesh;

	Effect::Sky::C_AtmosphericSkyDome	m_AtmosphericSkyDome;
	Effect::Sky::C_HDRSkyDome			m_HDRSkyDome;
	Effect::Sky::C_LuminanceSkyDome		m_LuminanceSkyDome;
	Effect::Sky::C_PreethamSkyDome		m_PreethamSkyDome;
	Effect::Sky::C_TexturedSkyDome		m_TexturedSkyDome;
	Effect::Sky::C_MultiTexturedSkyDome	m_MultiTexturedSkyDome;

	Effect::Sky::C_LitCloudDome			m_LitCloudDome;
	Effect::Sky::C_TexturedCloudPlane	m_TexturedCloudPlane;

	Scheduler::PolicyHandle				m_hPolicy;

};

#endif // TEST_SECTION_SKY_H
