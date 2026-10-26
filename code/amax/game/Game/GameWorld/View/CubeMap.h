#ifndef GAME_WORLD_VIEW_CUBEMAP_H
#define GAME_WORLD_VIEW_CUBEMAP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameView/CubeMapView.h>
#include <SceneGraph/RenderGroupSettings.h>

namespace SceneGraph
{
	class C_SceneGraph;
	class C_View;
	class C_MultiView;
}

namespace GameView
{
	class C_CubeMapView;
}

namespace Framework
{
	class C_View;
}

namespace Game
{

class C_View;
class C_ViewManager;

class C_CubeMapView : public GameView::C_CubeMapView
{
public:
	C_CubeMapView(C_ViewManager* pViewManager) : GameView::C_CubeMapView(), m_pViewManager(pViewManager), m_pMultiView(NULL), m_Enabled(true), m_DetailFactor(1.f), m_FarClip(500.f)
#if PLATFORM_PC
		, m_GraphicalDetailEnabled(true)	// [Blur2PC] retail ctor 0x0042b260
#endif
	{}
	~C_CubeMapView() {}

	void	Create();
	void	Destroy();

	void			Gather();
	virtual void	GatherFace(const U32 face);
	
	void			Schedule();
	virtual void	ScheduleFace(const U32 face);
	
	void			Render();
	virtual void	RenderFace(const U32 face);

	void			UpdateView(C_View& gameView);
	void			SetDynamicDetail(F32 detail);

	void			UpdateFakeLight(const U32& index, const Maths::C_RGBA_F32& colour, const Maths::HVec3& position, const F32& size);
	void			GetFakeLight(const U32& index, bool& enable, Maths::C_RGBA_F32& colour, Maths::HVec3& position, F32& size);
private:
	C_ViewManager*						m_pViewManager;
	SceneGraph::C_View*					m_pView[6];
	SceneGraph::C_MultiView*			m_pMultiView;
	SceneGraph::C_RenderGroupSettings	m_RenderGroupSettings;
	bool								m_Enabled;
#if PLATFORM_PC
public:
	// [Blur2PC] Retail PC +0x2aac: the cube map only renders at graphical detail 3 and 4 (set by
	// C_ViewManager::SetGraphicalDetail, Blur.exe 1.2 0x0063ec40).
	void								SetGraphicalDetailEnabled(const bool enabled) { m_GraphicalDetailEnabled = enabled; }
private:
	bool								m_GraphicalDetailEnabled;
#endif

	void			RenderFakeLighting();

	struct	C_FakeLight
	{
		C_FakeLight()
		{
			m_Enabled = false;
		}
		bool				m_Enabled;
		Maths::HVec3		m_Position;
		F32					m_Size;
		Maths::C_RGBA_F32	m_Colour;
	};

	C_FakeLight			m_FakeLightingVector[3];

	F32					m_DetailFactor;
	F32					m_FarClip;
};

} // namespace Game

#endif // GAME_WORLD_VIEW_CUBEMAP_H
