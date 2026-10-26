#ifndef GAME_WORLD_VIEW_REFLECTION_H
#define GAME_WORLD_VIEW_REFLECTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/Texture/GaussianBlur.h>
#include <Game/GameWorld/GameWorldTasks.h>

#include <SceneGraph/RenderGroupSettings.h>

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

namespace Game
{

class C_View;
class C_ViewManager;

class C_ReflectionView : public Display::I_Renderer
{
public:
	C_ReflectionView(C_ViewManager* pViewManager);
	~C_ReflectionView() {}

	bool	OnCreate();
	bool	OnCreateVolatile();
	void	OnDestroyVolatile();
	void	OnDestroy();

	void	Gather(C_View& gameView);
	void	Schedule(C_View& gameView);
	void	Render(C_View& gameView);
	void	SetDynamicDetail(F32 detail);
#if PLATFORM_PC
	void	SetGraphicalDetail(const U32 graphicalDetail);	// [Blur2PC] retail 0x020bffa8
	bool	SizeFromDisplayWindow();						// [Blur2PC] retail 0x00427750
	bool	IsGraphicalDetailEnabled() const { return m_GraphicalDetailEnabled; }
#endif

	const Display::C_RenderTexture2D*	GetRenderTexture() const;
	const Display::C_RenderTexture2D*	GetBlurRenderTexture() const;
	const Display::C_Matrix&			GetMatrix() const;
	const Display::C_Float4&			GetPlane() const { return m_ReflectionPlane; }

	void								EnableBlackBorderReflection(const bool blackBorderReflection);
	void								UpdateFakeSunLight(const Maths::C_RGBA_F32& colour, const Maths::HVec3& position, const F32& minSize, const F32& maxSize, const bool& depthTest);

private:
	C_ViewManager*						m_pViewManager;

	SceneGraph::C_SceneGraph*			m_pSceneGraph;
	SceneGraph::C_View*					m_pView;
	SceneGraph::C_RenderGroupSettings	m_RenderGroupSettings;

	GameView::C_ReflectionView*			m_pReflection;

	Render::Texture::C_GaussianBlur		m_GaussianBlur;
	Display::C_RenderTexture2D			m_BlurReflectionTexture;
	Display::C_RenderTexture2D			m_BlurAuxReflectionTexture;
	Display::C_Float4					m_ReflectionPlane;
	bool								m_BlackBorderReflection;

	//Frame
	Display::C_VertexBuffer				m_FrameVertexBuffer;

	void CreateBlackFrame();
	void DestroyBlackFrame();
	void RenderBackFrame();

	F32									m_CameraInc;

	void RenderFakeLight();

	Maths::C_RGBA_F32					m_FakeSunLightColour;
	Maths::HVec3						m_FakeSunLightPosition;
	F32									m_FakeSunLightMinSize;
	F32									m_FakeSunLightMaxSize;
	bool								m_FakeSunLightDepthTest;

	F32									m_DetailFactor;
#if PLATFORM_PC
	bool								m_GraphicalDetailEnabled;	// [Blur2PC] retail +0x15d: only detail 4 renders the reflection
	S32									m_ReflectionResolution;		// [Blur2PC] retail +0x164: the city's ReflectionResolution
	U32									m_TargetWidth;				// [Blur2PC] retail +0x168/+0x16c/+0x170: the reflection target's
	U32									m_TargetHeight;				// size and the display window multisample it was sized for
	Display::E_Multisample				m_TargetMultisample;
#endif
};

} // namespace Game

#endif // GAME_WORLD_VIEW_TYPE_H
