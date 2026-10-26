#ifndef TEST_SECTION_DYNAMIC_EFFECT_H
#define TEST_SECTION_DYNAMIC_EFFECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>

#include "Render/Sky/SkyDome.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"

namespace DynamicEffect { class C_Renderer; }
namespace DynamicEffect { class C_Minimal; }
namespace Render { namespace Debug { class C_DebugRenderSys; } }

class C_TestSectionDynamicEffect : public Framework::C_TestSection
{
public:

	C_TestSectionDynamicEffect(const Util::C_String& name);
	~C_TestSectionDynamicEffect();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();
	
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

	Render::Sky::C_SkyDome				m_SkyDome;
	PostProcess::C_Manager*				m_pPostProcessMgr;
	PostProcess::C_HighDynamicRange*	m_pPostProcessHdr;

	DynamicEffect::C_Renderer*			m_pDynamicEffectRenderer;
	DynamicEffect::C_Minimal*			m_pMinimalDynamicEffect;

	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;
};

#endif // TEST_SECTION_DYNAMIC_EFFECT_H
