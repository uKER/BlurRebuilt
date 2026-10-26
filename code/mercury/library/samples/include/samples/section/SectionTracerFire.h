#ifndef SAMPLES_SECTIONTRACERFIRE_H
#define SAMPLES_SECTIONTRACERFIRE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Samples/GameFunctionality.h"
#include "Render/Sky/SkyDome.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Dynamic/Handle.h"
#include "Dynamic/Node.h"

namespace DynamicEffect
{
	class C_Renderer;

namespace Tracer
{
	class C_Manager;
};
};

namespace Render
{
namespace Debug
{
	class C_DebugRenderSys;
};
};

namespace Samples
{

class C_SectionTracerFire : public C_Section
{
public:

	C_SectionTracerFire(const Util::C_String& name);
	~C_SectionTracerFire();

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
	DynamicEffect::Tracer::C_Manager*	m_pTracerManager;

	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;

	ProjectileList						m_Projectiles;
	ProjectilePool						m_ProjectilePool;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONTRACERFIRE_H
