#ifndef GAME_WORLD_VIEW_MAIN_H
#define GAME_WORLD_VIEW_MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/RenderGroupSettings.h>
#include <fx/Fx.h>
#include <Render/ReducedRender/Enums.h>

namespace SceneGraph
{
	class C_SceneGraph;
	class C_View;
}

namespace Framework
{
	class C_View;
}

namespace Game
{

class C_View;
class C_ViewManager;

class C_MainView
{
public:
	C_MainView(C_ViewManager* pViewManager) : m_pViewManager(pViewManager), m_pMain(NULL), m_pMainPrepass(NULL), m_InstanceDrawDistanceScale(1.0f), m_ModelInstanceDrawDistanceScale(1.0f)
#if PLATFORM_PC
		, m_LightsSubtechniqueHandle(0), m_LowDetailShadows(false), m_PrepassVehicles(false)
#endif
	{}
	~C_MainView() {}

	void	Create(const C_View& gameView);
	void	Destroy();

	// main view stuff
	void	UpdateView(const C_View& gameView);
	
	void	Gather(C_View& gameView);
	void	Schedule(const C_View& gameView);
	
	void	RenderPrepass1(C_View& gameView);
	void	RenderPrepass2(C_View& gameView);
	void	RenderWithAA(C_View& gameView);
	void	RenderWithoutAA(C_View& gameView);
	void	SetDynamicDetail(F32 detail);
#if PLATFORM_PC
	void	SetGraphicalDetail(const U32 graphicalDetail, const bool inUI);	// [Blur2PC] retail 0x00425450
#endif

	friend class C_MultiView;
	friend class C_View;

private:
	void	SetupPrepassLightSpheres() const;

	C_ViewManager*						m_pViewManager;
	SceneGraph::C_View*					m_pMain;
	SceneGraph::C_View*					m_pMainPrepass;
	SceneGraph::C_RenderGroupSettings	m_PrepassSettings;
	SceneGraph::C_RenderGroupSettings	m_MainWithAASettings;
	SceneGraph::C_RenderGroupSettings	m_MainWithoutAASettings;

	Render::Reduced::E_Downsample		m_ReduceRenderAlpha;
	F32									m_InstanceDrawDistanceScale;
	F32									m_ModelInstanceDrawDistanceScale;
	FX::SamplerStateHandle				m_MirrorLinear, m_ClampPoint, m_ClampLinear;
	Display::C_Texture2D				m_BlackTexture;
#if PLATFORM_PC
	// [Blur2PC] Retail PC graphical detail state (names ours).
	U32									m_LightsSubtechniqueHandle;	// retail stores it in m_MainWithAASettings (+0x54): Lights at detail 4, else LSplitScreen
	bool								m_LowDetailShadows;			// retail +0xb4: detail 1, car collision-mesh shadows drawn in the main view
	bool								m_PrepassVehicles;			// retail +0xb5: detail 4 or in the UI, vehicles and ghosts go into the lighting prepass
#endif
};

} // namespace Game

#endif // GAME_WORLD_VIEW_MAIN_H
