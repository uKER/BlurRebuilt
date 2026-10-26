#ifndef SAMPLES_SECTIONSKY_H
#define SAMPLES_SECTIONSKY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Mesh/Mesh.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Effect/Sky/AtmosphericSkyDome.h"
#include "Effect/Sky/LitCloudDome.h"
#include "Effect/Sky/LuminanceSkyDome.h"
#include "Effect/Sky/MultiTexturedSkyDome.h"
#include "Effect/Sky/PreethamSkyDome.h"
#include "Effect/Sky/TexturedSkyDome.h"
#include "Effect/Sky/TexturedCloudPlane.h"

namespace Samples
{

class C_SectionSky : public C_Section
{
public:

	C_SectionSky(const Util::C_String& name);
	~C_SectionSky();

	bool OnCreate();
	void OnDestroy();
	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer);
	void OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer);

	void OnBeginRenderSplitscreenWithoutAA();
	void OnEndRenderSplitscreenWithoutAA();
	void OnResize();

private:

	void SetupPostProcess();

private:

	PostProcess::C_Manager*				m_pPostProcessMgr;
	PostProcess::C_HighDynamicRange*	m_pPostProcessHdr;

	Render::Mesh::C_Mesh				m_Mesh;

	Effect::Sky::C_AtmosphericSkyDome	m_AtmosphericSkyDome;
	Effect::Sky::C_LuminanceSkyDome		m_LuminanceSkyDome;
	Effect::Sky::C_PreethamSkyDome		m_PreethamSkyDome;
	Effect::Sky::C_TexturedSkyDome		m_TexturedSkyDome;
	Effect::Sky::C_MultiTexturedSkyDome	m_MultiTexturedSkyDome;

	Effect::Sky::C_LitCloudDome			m_LitCloudDome;
	Effect::Sky::C_TexturedCloudPlane	m_TexturedCloudPlane;

	Scheduler::PolicyHandle				m_hPolicy;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONSKY_H
