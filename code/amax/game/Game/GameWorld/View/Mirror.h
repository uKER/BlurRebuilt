#ifndef GAME_WORLD_VIEW_MIRROR_H
#define GAME_WORLD_VIEW_MIRROR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/RenderStates.h>
#include <SceneGraph/RenderGroupSettings.h>

namespace SceneGraph
{
	class C_SceneGraph;
	class C_View;
}

namespace GameView
{
	class C_OffscreenView;
}

namespace Framework
{
	class C_View;
}

namespace Game
{

class C_View;
class C_ViewManager;

class C_MirrorView
{
public:
#if PLATFORM_PC
	C_MirrorView(C_ViewManager* pViewManager) : m_pViewManager(pViewManager), m_pView(NULL), m_pMirror(NULL), m_TargetWidth(0), m_TargetHeight(0), m_TargetMultisample(Display::E_Multisample_None) {}
#else
	C_MirrorView(C_ViewManager* pViewManager) : m_pViewManager(pViewManager), m_pView(NULL), m_pMirror(NULL) {}
#endif
	~C_MirrorView() {}

	void	Create();
	void	Destroy();

	void	Gather(C_View& gameView);
	void	Schedule(C_View& gameView);
	void	Render(C_View& gameView);

	void	SetupMirror(const Maths::HVec3 position, const Maths::HVec3 view, const Maths::HVec3 up, const F32 fieldOfView);
	void	SetDynamicDetail(F32 detail);
#if PLATFORM_PC
	void	SetGraphicalDetail(const U32 graphicalDetail);	// [Blur2PC] retail 0x004295f0
	void	SizeFromDisplayWindow();						// [Blur2PC] retail 0x00427660
#endif

	const Display::C_RenderTexture2D*	GetRenderTexture() const;

private:
	SceneGraph::C_View*				m_pView;
	GameView::C_OffscreenView*		m_pMirror;			
	SceneGraph::RenderStates		m_renderStates;
	SceneGraph::RenderStates		m_MirrorRefractAlphaRenderStates;
	C_ViewManager*						m_pViewManager;
	SceneGraph::C_RenderGroupSettings	m_RenderGroupSettings;
	F32									m_DetailFactor;
#if PLATFORM_PC
	// [Blur2PC] retail +0x10c/+0x110/+0x114: the display window size and multisample the target was sized for
	U32									m_TargetWidth;
	U32									m_TargetHeight;
	Display::E_Multisample				m_TargetMultisample;
#endif
};

} // namespace Game

#endif // GAME_WORLD_VIEW_MAIN_H
