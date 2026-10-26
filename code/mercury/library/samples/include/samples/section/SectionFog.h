#ifndef SAMPLES_SECTIONFOG_H
#define SAMPLES_SECTIONFOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Effect/Fog/Fog.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Render/Mesh/Mesh.h"
#include "Effect/Sky/PreethamSkyDome.h"

namespace Render
{
namespace Debug
{
	class C_DebugRenderSys;
};
};

namespace DynamicEffect
{
	class C_Renderer;
};

namespace DynamicEffect
{
namespace Fog
{
	class C_BasicManager;
	class Volume;
};
};

namespace Samples
{

class C_SectionFog : public C_Section
{
public:

	C_SectionFog(const Util::C_String& name);
	~C_SectionFog();

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
	Effect::Sky::C_PreethamSkyDome		m_SkyDome;
	Effect::Fog::C_Fog					m_Fog;
	Render::Mesh::C_Mesh				m_Mesh;

	DynamicEffect::C_Renderer*			m_pDynamicEffectRenderer;
	DynamicEffect::Fog::C_BasicManager* m_pFogManager;
	DynamicEffect::Fog::Volume*		m_pFogVolume0;
	DynamicEffect::Fog::Volume*		m_pFogVolume1;

	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;

	Display::C_Texture2D				m_DepthTexture;

};

}; // namespace Samples

#endif // SAMPLES_SECTIONFOG_H
