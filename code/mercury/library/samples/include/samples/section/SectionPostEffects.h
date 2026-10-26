#ifndef SAMPLES_SECTIONPOSTEFFECTS_H
#define SAMPLES_SECTIONPOSTEFFECTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"

namespace PostProcess
{
	class C_Manager;
	class C_HighDynamicRange;
	class C_Greyscale;
}

namespace Samples
{

class C_SectionPostEffects : public C_Section
{
public:

	C_SectionPostEffects(const Util::C_String& name);
	~C_SectionPostEffects();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
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

	Render::Sky::C_SkyDome				m_SkyDome;
	PostProcess::C_Manager*				m_pPostProcessMgr;
	PostProcess::C_HighDynamicRange*	m_pPostProcessHdr;
	PostProcess::C_Greyscale*			m_pPostProcessGreyscale;
};

} // namespace Samples

#endif // SAMPLES_SECTIONPOSTEFFECTS_H
