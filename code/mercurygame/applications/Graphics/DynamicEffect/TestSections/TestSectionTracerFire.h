#ifndef TEST_SECTION_TRACER_FIRE_H
#define TEST_SECTION_TRACER_FIRE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "TestDynamic/GameFunctionality.h"
#include "Render/Sky/SkyDome.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Dynamic/Handle.h"
#include "Dynamic/Node.h"

namespace DynamicEffect { class C_Renderer; }
namespace DynamicEffect { namespace Tracer { class C_Manager; } }
namespace Render { namespace Debug { class C_DebugRenderSys; } }

class C_TestSectionTracerFire : public Framework::C_TestSection
{
public:

	C_TestSectionTracerFire(const Util::C_String& name);
	~C_TestSectionTracerFire();

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
	DynamicEffect::Tracer::C_Manager*	m_pTracerManager;

	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;

	ProjectileList						m_Projectiles;
	ProjectilePool						m_ProjectilePool;
};

#endif // TEST_SECTION_TRACER_FIRE_H
