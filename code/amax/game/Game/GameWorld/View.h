#ifndef GAME_WORLD_VIEW_H
#define GAME_WORLD_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/GameWorld/View/Main.h>
#include <Render/Texture/GaussianBlur.h>

#include "gravity/narrowphase/CollisionFilter.h"
#include "gravity/narrowphase/ray.h"

#include <Game/GameWorld/GameWorldTasks.h>
#include <SceneGraph/MultiView.h>

namespace SceneGraph
{
	class C_SceneGraph;
	class C_View;
	class C_MultiView;
}

namespace PostProcess
{
	class C_Manager;
	class C_BloomFeedbackGhost;
}

namespace Framework
{
	class C_View;
}

namespace GameView
{
	class C_ReflectionView;
}

namespace FX
{
	class I_EnvironmentSettings;
}

namespace Game
{

class C_View;
class C_AmaxWorldLayer;
class C_GameWorld;
class C_ViewManager;

class C_MirrorView;
class C_ReflectionView;

class C_MultiView : public SceneGraph::C_MultiView
{
public:
	C_MultiView(SceneGraph::C_SceneGraph* pSceneGraph, const C_View* pView) : SceneGraph::C_MultiView(pSceneGraph), m_pView(pView) {}
	~C_MultiView() {}

	void				BeginGather() const;

private:
	const C_View*		m_pView;
};

class C_View
{
public:
	C_View(C_ViewManager* pViewManager, const S32 index);
	~C_View();

	void	Begin();
	void	Schedule();

	void	PreRenderSplitscreen();
	void	RenderWithAA();
	void	RenderWithoutAA();

	void	Create(const bool createMirror, const bool createReflection);
	void	Destroy();

	void	UpdateView();

	const Framework::C_View*			GetFrameworkView() const { return m_pFrameworkView; }
	const S32							GetIndex() const { return m_Index; }

	const SceneGraph::C_View&			GetMainView() const { return *m_MainView.m_pMain; }

	const C_MirrorView*					GetMirror() const { return m_pMirror; }
	C_MirrorView*						GetMirror() { return m_pMirror; }

	const C_ReflectionView*				GetReflection() const { return m_pReflection; }
	C_ReflectionView*					GetReflection() { return m_pReflection; }

	const Maths::HVec3					GetReferencePosition() const { return m_ReferencePosition;};

	FX::I_EnvironmentSettings			*GetEnvironmentSettings() { return m_pEnvironmentSettings;};
	void								ResetEnvironmentSettings(FX::I_EnvironmentSettings* sorceryEnvironmentSettings);
	void								SetDynamicDetail(const F32 detail);
#if PLATFORM_PC
	void								SetGraphicalDetail(const U32 graphicalDetail, const bool inUI);	// [Blur2PC] retail 0x00419620
#endif

	//Rim generatos
	C_RimGenerator ALIGN16						m_RimGeneratorVector[GameWorld_MaxRimGenerators];
	U32											m_NumRimGenerators;
	//Shadow casters
	C_ShadowCaster								m_ShadowCaster[GameWorld_MaxShadowCasters]; //Max number
	U32											m_NumShadowCasters;
	U32											m_MaxShadowCasters;

	friend class C_MultiView;
	friend class C_MainView;
	friend class C_MirrorView;
	friend class C_ReflectionView;

private:

	// general stuff
	const S32							m_Index;

	SceneGraph::C_SceneGraph*			m_pSceneGraph;
	C_ViewManager*						m_pViewManager;
	const Framework::C_View*			m_pFrameworkView;
	C_MultiView*						m_pMultiView;

	C_MainView							m_MainView;
	C_MirrorView*						m_pMirror;
	C_ReflectionView*					m_pReflection;

	//Ray to calculate reference position
	Collision::C_Ray					m_GroundRay;
	//View reference position
	Maths::HVec3						m_ReferencePosition;

	FX::I_EnvironmentSettings			*m_pEnvironmentSettings;
};

} // namespace Game

#endif // GAME_WORLD_VIEW_TYPE_H
