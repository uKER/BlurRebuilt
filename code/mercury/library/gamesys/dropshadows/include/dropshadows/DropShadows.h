#ifndef DROPSHADOWS_DROP_SHADOWS_H
#define DROPSHADOWS_DROP_SHADOWS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include <SceneGraph/RenderNode.h>
#include <SceneGraph/GatherNode.h>

namespace Deferred
{
	class C_Manager;
};

namespace Display
{
	class C_Context;
};

namespace SceneGraph
{
	class C_Context;
}

namespace DropShadows
{

struct C_DropShadow
{
	MathsStorage::Float3	m_vPosition;
	F32						m_fRadius;
	F32						m_fIntensity;
};

typedef Btl::Vector<C_DropShadow> DropShadows;

struct C_VisibleDropShadow
{
	Maths::C_Rect			m_rectangle;
	C_DropShadow*			m_pShadow;
};

typedef Btl::Vector<C_VisibleDropShadow> VisibleDropShadows;

enum E_DropShadowShader
{
	E_DropShadowShader_1 = 0,
	E_DropShadowShader_2,
	E_DropShadowShader_3,
	E_DropShadowShader_4,
	E_DropShadowShader_5,
	E_DropShadowShader_6,
	E_DropShadowShader_7,
	E_DropShadowShader_8,
	E_DropShadowShader_Count
};

class C_DropShadows : public Display::I_Renderer, public SceneGraph::I_RenderNode, public SceneGraph::I_GatherNode
{
public:

	C_DropShadows(Deferred::C_Manager* pDeferredManager, U32 visibilityFlags);
	~C_DropShadows();

	bool	OnCreate();
	bool	OnCreateVolatile();
	void	OnDestroy();
	void	OnDestroyVolatile();

	void	Render() const;

	//Scenegraph
	void Render(Display::C_Context* pContext) const;

	void OnRender(SceneGraph::C_Context& context) const;
	void OnGather(const SceneGraph::C_View& view) const;
	void Accept(SceneGraph::I_Visitor& visitor) const;
	void SetRenderPass(const U32 renderPass);

	void	AddShadow(const U32 viewIndex, const MathsStorage::Float3& vPosition, const F32 fRadius, const F32 fIntensity);
	
	void	SetTesselation(const U32 x, const U32 y)											{ m_HorizontalTiles = x; m_VerticalTiles = y; }

	void	SetFarClip(const F32 fFarClip)														{ m_FarClip = fFarClip; }

private:

	void	GatherVisibleShadows(const U32 viewIndex, VisibleDropShadows& visibleShadows, const Display::C_ViewSettings3D& viewSettings3D) const;
	//return the last visible shadow processed, false if it has more shadows
	bool	GatherVisibleParameters(const VisibleDropShadows& visibleShadows, const Maths::C_Rect& rectangle, U32& firstProcessedShadow, U32& shadowCount, U32* pShadowsIndexes) const;

private:

	mutable DropShadows m_DropShadows[16];

	U32	m_HorizontalTiles;
	U32	m_VerticalTiles;

private:

	Display::C_RenderTexture2D	m_RenderTexture;

	Deferred::C_Manager*		m_pDeferredManager;

	Display::C_VertexBuffer		m_VertexBuffer;

	Display::C_VertexShader		m_VertexShader;
	Display::C_PixelShader		m_PixelShader[E_DropShadowShader_Count];

	Display::C_PixelShaderParameterArray	m_PositionParameter[E_DropShadowShader_Count];
	Display::C_PixelShaderParameterArray	m_IntensityParameter[E_DropShadowShader_Count];
	Display::C_PixelShaderParameter			m_ViewUpVectorParameter[E_DropShadowShader_Count];
	Display::C_PixelShaderParameter			m_DepthParameter[E_DropShadowShader_Count];

	F32					m_FarClip;

	U32					m_RenderPass;
	U32					m_VisibilityFlags;

#ifdef PLATFORM_PS3
	struct ParamsCache
	{
		bool	m_Patched;
		U32		m_ShadowsIndex[E_DropShadowShader_Count];
	};
	mutable ParamsCache m_ParamsCache[E_DropShadowShader_Count];
	mutable U32	m_CachedShader;
#endif

};

}; // namespace DropShadows

#endif // RENDER_DEFERRED_DROP_SHADOWS_H
