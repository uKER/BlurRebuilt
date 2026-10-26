#ifndef GAME_WORLD_VIEW_MANAGER_H
#define GAME_WORLD_VIEW_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/RenderStates.h>
#include <Lod/LodInterface.h>
#include <Render/ReducedRender/Enums.h>

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace BlobShadows
{
	class C_BlobShadows;
}

class C_HighDynamicRange;

namespace Render
{

namespace Reduced
{
	class C_Manager;
}
}

namespace DynamicEffect
{
	class C_Renderer;
}

namespace AuxFlares
{
	class C_AuxFlares;
}

namespace AnimatedCrowd
{
	class C_CrowdFlags;
}

namespace DropShadows
{
	class C_DropShadows;
}

namespace PostProcess
{
	class C_Manager;
	class C_BloomFeedbackGhost;
}

namespace SceneGraph
{
	class C_View;
};

class C_LightingPrepass;

namespace Game
{

class C_View;
class C_CubeMapView;
class C_GameWorld;
class C_AmaxWorldLayer;

class C_ViewManager : public Lod::I_LodInterface
{
public:
	C_ViewManager(SceneGraph::C_SceneGraph* pSceneGraph, C_GameWorld* pGameWorld);
	~C_ViewManager();

	void			Create(const S32 views, const bool mirrorEnabled, const bool reflectionEnabled, const bool cubeMapEnabled);
	void			Destroy();

	void			BeginFrame();

	void			SetReflectionBlur(F32 blurX, F32 blurY, F32 deviation) { m_ReflectionBlurX = blurX; m_ReflectionBlurY = blurY; m_ReflectionBlurDeviation = deviation; }
	F32				GetReflectionBlurX() const { return m_ReflectionBlurX; }
	F32				GetReflectionBlurY() const { return m_ReflectionBlurY; }
	F32				GetReflectionBlurDeviation() const { return m_ReflectionBlurDeviation; }

	void			SetLightFlareParameters();

	S32				GetNumViews() const {return m_Views.Size();}
	C_View*			GetView(const S32 index) { return m_Views[index]; }
	SceneGraph::C_SceneGraph*	GetSceneGraph() { return m_pSceneGraph; }

	C_CubeMapView*	GetCubeMap() { return m_pCubeMap; }

	void			UpdateViews();

	const U32		GetLODSubtechniqueHandle() const { return m_LODSubtechniqueHandle; }
	const U32		GetPrepassSubtechniqueHandle() const { return m_PrepassSubtechniqueHandle; }
	const U32		GetSolidSubtechniqueHandle() const { return m_SolidSubtechniqueHandle; }
	const U32		GetAlphaLuminanceSubtechniqueHandle() const { return m_AlphaLuminanceSubtechniqueHandle; }
	const U32		GetElectricSubtechniqueHandle() const { return m_ElectricSubtechniqueHandle; }
	const U32		GetLightsSubtechniqueHandle() const { return m_LightsSubtechniqueHandle; }
	const U32		GetLightsSplitScreenSubtechniqueHandle() const { return m_LightsSplitScreenSubtechniqueHandle; }
	const U32		GetEnvironmentSubtechniqueHandle() const { return m_EnvironmentSubtechniqueHandle; }
#if PLATFORM_PC
	// [Blur2PC] Retail PC subtechniques (Blur.exe 1.2 view manager ctor 0x00428ff0, +0x7c/+0x80).
	const U32		GetNoDamageSubtechniqueHandle() const { return m_NoDamageSubtechniqueHandle; }
	const U32		GetNoDepthSubtechniqueHandle() const { return m_NoDepthSubtechniqueHandle; }
#endif

	const SceneGraph::RenderStates&		GetGhostDepthStates() const { return m_GhostDepthStates; }
	const SceneGraph::RenderStates&		GetGhostColourStates() const { return m_GhostColourStates; }

	void			SetDynamicDetail(const F32 cpuDetail, const F32 gpuDetail, const bool immediate);
#if PLATFORM_PC
	void			SetGraphicalDetail(const U32 graphicalDetail, const bool inUI);	// [Blur2PC] retail 0x0063ec40
#endif

	const Render::Reduced::E_Downsample		GetReduceRenderAlpha() const { return m_ReduceRenderAlpha; }

	Display::C_VertexShader&			GetFakeLightingVertexShader() { return m_FakeLightingVertexShader;}
	Display::C_PixelShader&				GetFakeLightingPixelShader() { return m_FakeLightingPixelShader;}

	SceneGraph::C_View* GetLightTrailsView() { return m_pLightTrailsView; }

	friend class C_View;
	friend class C_MainView;
	friend class C_MirrorView;
	friend class C_CubeMapView;
	friend class C_ReflectionView;
	friend class C_GameWorld;

private:

	SceneGraph::C_SceneGraph*			m_pSceneGraph;
	C_GameWorld*						m_pGameWorld;
	// cached pointers
	PostProcess::C_Manager*				m_pPostProcessManager;
	PostProcess::C_BloomFeedbackGhost*	m_pPostProcessBloom;
	C_HighDynamicRange*					m_pPostProcessHdr;
	Game::C_AmaxWorldLayer*				m_pAmaxWorldLayer;
	BlobShadows::C_BlobShadows*			m_pBlobShadows;
	DropShadows::C_DropShadows*			m_pDropShadows;
	Render::Reduced::C_Manager*			m_pReducedRender;
	C_LightingPrepass*					m_pLightingPrepass;
	DynamicEffect::C_Renderer*			m_pDynamicEffectRenderer;
	AuxFlares::C_AuxFlares*				m_pAuxFlares;
	AnimatedCrowd::C_CrowdFlags*		m_pCrowdFlagRenderer;

	Btl::Vector<C_View*, Btl::VectorParameters_Static<4> >	m_Views;
	C_CubeMapView*						m_pCubeMap;

	F32									m_ReflectionBlurX;
	F32									m_ReflectionBlurY;
	F32									m_ReflectionBlurDeviation;

	U32									m_LODSubtechniqueHandle;
	U32									m_PrepassSubtechniqueHandle;
	U32									m_LightsSubtechniqueHandle;
	U32									m_ElectricSubtechniqueHandle;
	U32									m_SolidSubtechniqueHandle;
	U32									m_AlphaLuminanceSubtechniqueHandle;
	U32									m_LightsSplitScreenSubtechniqueHandle;
	U32									m_EnvironmentSubtechniqueHandle;
#if PLATFORM_PC
	U32									m_NoDamageSubtechniqueHandle;	// [Blur2PC] retail +0x7c
	U32									m_NoDepthSubtechniqueHandle;	// [Blur2PC] retail +0x80
#endif

	Render::Reduced::E_Downsample		m_ReduceRenderAlpha;
	F32									m_ReduceRenderAlphaCycle;
	F32									m_DisableAACycle;

	SceneGraph::RenderStates			m_GhostDepthStates;
	SceneGraph::RenderStates			m_GhostColourStates;

	Display::C_VertexShader				m_FakeLightingVertexShader;
	Display::C_PixelShader				m_FakeLightingPixelShader;

	SceneGraph::C_View*					m_pLightTrailsView;
};

} // namespace Game

#endif // GAME_WORLD_VIEW_TYPE_H
