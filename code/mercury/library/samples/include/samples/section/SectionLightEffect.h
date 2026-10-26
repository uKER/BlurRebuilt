#ifndef SAMPLES_SECTIONLIGHTEFFECT_H
#define SAMPLES_SECTIONLIGHTEFFECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Dynamic/Handle.h"
#include "Dynamic/Node.h"
#include "Dynamic/Light/Enums.h"
#include "Dynamic/Light/Light.h"

namespace DynamicEffect
{
	class C_Renderer;

namespace Light
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

class C_TestLight
{
public:
	MathsStorage::Float3A16	m_vPosition;
	F32						m_fExpiryTime;
	MathsStorage::Float4A16	m_vRotation;
	MathsStorage::Float4A16	m_vColour;
	MathsStorage::Float4A16	m_vCentre;
	MathsStorage::Float4A16	m_vSize;
	DynamicEffect::Light::LightHandle m_hLight;
	U32						m_iVisibility;
};

class C_SectionLightEffect : public C_Section
{
public:

	C_SectionLightEffect(const Util::C_String& name);
	~C_SectionLightEffect();

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
	DynamicEffect::Light::C_Manager*	m_pLightManager;

	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;

//	Ut::Vector<C_TestLight>				m_TestLights;

	DynamicEffect::Light::LightHandle	m_hSunLight;
	DynamicEffect::C_Node				m_SunNode;

	Display::C_OcclusionQueryRender		m_OcclusionQueryRender;
};

} // namespace Samples

#endif // SAMPLES_SECTIONLIGHTEFFECT_H
