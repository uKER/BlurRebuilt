#ifndef AMAX_GRAPHICS_LIGHTING_PREPASS_H
#define AMAX_GRAPHICS_LIGHTING_PREPASS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "SceneGraph/SceneGraph.h"

namespace Deferred
{
	class C_DynamicLights;
	class C_Light;
	class C_Manager;
	class C_Mask;
	class C_TiledVolumeSpotlights;
};

class C_PointLights;
class C_SpotLights;

namespace Prepass
{
namespace JobQueue
{
	class C_Prepass;
};
};

class C_LightingPrepass
{
public:

	C_LightingPrepass();
	~C_LightingPrepass();

	bool Create();
	void Destroy();

	void Begin() const;
	void End() const;

	void RenderLights1(const SceneGraph::C_VisiblePointLightList& visiblePointLights, const SceneGraph::C_VisibleSpotLightList& visibleSpotLights) const;
	void RenderLights2(const SceneGraph::C_VisiblePointLightList& visiblePointLights, const SceneGraph::C_VisibleSpotLightList& visibleSpotLights) const;

	const Display::C_Texture2D* GetNormalTexture(const S32 iView) const;
 
	const Display::C_Texture2D* GetDepthTexture(const S32 iView) const;

	// [Blur2PC] Recovered retail PC getter, Blur.exe 1.2 0x004a2290 (T232).
	const Display::C_Surface* GetDepthStencilSurface(const S32 iView) const;
 
	const Display::C_Texture2D* GetLightingTexture(const S32 iView) const;

	void SetupRenderTargets();
	void SetupViews();
	void Resize();

	void Show(const S32 iView);

	void SetFarClip(const F32 fFarClip);

	Deferred::C_Manager* GetDeferredManager() { return m_pDeferredManager; }

protected:

	void RenderPointLights(const SceneGraph::C_VisiblePointLightList& visiblePointLights) const;
	void RenderSpotLights(const SceneGraph::C_VisibleSpotLightList& visibleSpotLights) const;

protected:

	Deferred::C_Manager*			m_pDeferredManager;

#if defined( PLATFORM_PC )
	Deferred::C_DynamicLights*		m_pDeferredPointLights;
	Deferred::C_Light*				m_pDeferredSpotLights;
#else
	C_PointLights*					m_pDeferredPointLights;
	C_SpotLights*					m_pDeferredSpotLights;
#endif

	Deferred::C_Mask*				m_pDeferredMask;

#if defined(PLATFORM_PS3)
	Prepass::JobQueue::C_Prepass*	m_pDeferredSPULighting;
#endif

	F32								m_fFarClip;

};

#endif // AMAX_GRAPHICS_LIGHTING_PREPASS_H
