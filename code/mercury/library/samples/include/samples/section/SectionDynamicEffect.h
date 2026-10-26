#ifndef SAMPLES_SECTIONDYNAMICEFFECT_H
#define SAMPLES_SECTIONDYNAMICEFFECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"

namespace DynamicEffect
{
	class C_Renderer;
	class C_Minimal;
}

namespace Render
{
namespace Debug
{
	class C_DebugRenderSys;
}
}

namespace Samples
{

class C_SectionDynamicEffect : public C_Section
{
public:

	C_SectionDynamicEffect(const Util::C_String& name);
	~C_SectionDynamicEffect();

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

	DynamicEffect::C_Renderer*			m_pDynamicEffectRenderer;
	DynamicEffect::C_Minimal*			m_pMinimalDynamicEffect;

	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;
};

} // namespace Samples

#endif // SAMPLES_SECTIONDYNAMICEFFECT_H
