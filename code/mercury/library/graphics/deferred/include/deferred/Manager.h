#ifndef DEFERRED_MANAGER_H
#define DEFERRED_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "btl/containers/map.h"

#if PLATFORM_PS3
#define USE_DEFERRED_RENDER_BUFFERS 1
#endif

namespace Deferred
{

#if USE_DEFERRED_RENDER_BUFFERS
	
struct C_RenderTargetSetup
{
	Display::E_Format		eColourFormat[4];
	Display::E_Format		eDepthStencilFormat;
	Display::E_Multisample	eMultisample;
	S32						iIndex;
#if defined(PLATFORM_PS3)
	Display::E_AllocHeap	eColourAllocHeap;
	Display::E_AllocHeap	eDepthAllocHeap;
	bool					bColourTiling;
	bool					bDepthTiling;
#endif
};

struct C_View : public Display2::C_RenderBuffer
{
	S32							iIndex;
	S32							iRenderTargetIndex;
};

class C_Manager : public Display::I_Renderer, public Ut::Singleton<Deferred::C_Manager, Ut::CreateExternal>
{
public:

	C_Manager();
	~C_Manager();

	bool	OnCreate();
	bool	OnCreateVolatile();
	void	OnDestroy();
	void	OnDestroyVolatile();

	bool	OnPreResize();
	bool	OnPostResize();

	bool	AddRenderTargetSetup(const C_RenderTargetSetup& setup);
	void	ClearRenderTargetSetups();

	bool	AddView(const U32 iWidth, const U32 iHeight, const S32 iIndex, const S32 iRenderTargetIndex);
	void	ClearViews();

	void	Begin(const S32 iIndex);
	void	End();

	const Display::C_Surface*		GetColourSurface(const S32 iViewIndex, const S32 iColourIndex);
	const Display::C_Surface*		GetDepthStencilSurface(const S32 iViewIndex);

	const Display::C_Texture2D*		GetColourTexture(const S32 iViewIndex, const S32 iColourIndex);
	const Display::C_Texture2D*		GetDepthStencilTexture(const S32 iViewIndex);

private:

	bool	CreateRenderTargets();

private:

	Btl::Vector<C_View>					m_Views;
	Btl::Vector<C_RenderTargetSetup>	m_Setups;
	Btl::Map<S32, S32>					m_ViewMapping;
	Btl::Map<S32, S32>					m_SetupMapping;

	bool								m_bActive;
	S32									m_iActiveIndex;

	U32									m_TextureBufferSize;
	U8*									m_pTextureBuffer;
};

#else // USE_DEFERRED_RENDER_BUFFERS

struct C_RenderTargetSetup
{
	Display::E_Format		eColourFormat[4];
	Display::E_Format		eDepthStencilFormat;
	Display::E_Multisample	eMultisample;
	S32						iIndex;
#if defined(PLATFORM_PS3)
	Display::E_AllocHeap	eColourAllocHeap;
	Display::E_AllocHeap	eDepthAllocHeap;
	bool					bColourTiling;
	bool					bDepthTiling;
#endif
};

struct C_View
{
	U32							iWidth;
	U32							iHeight;
	S32							iIndex;
	S32							iRenderTargetIndex;
	Btl::Vector<Display::C_RenderTexture2D*, Btl::VectorParameters_Array<4> >	renderTextures;
	Display::C_Surface*			pDepthStencilSurface;
	Display::C_Texture2D*		pDepthStencilTexture;
#if defined(PLATFORM_PS3)
	Btl::Vector<U8*, Btl::VectorParameters_Array<4> >	pColourAddress;
	Btl::Vector<U8, Btl::VectorParameters_Array<4> >	iColourTileIndex;
	Btl::Vector<U16, Btl::VectorParameters_Array<4> >	iColourTileBase;
	Btl::Vector<Display2::C_TileInfo, Btl::VectorParameters_Array<4> >	colourTileInfo;
	U8							iDepthTileIndex;
	U16							iDepthTileBase;
	U8*							pDepthAddress;
	Display2::C_TileInfo		depthTileInfo;
	U32							iZCullIndex;
	U32							iZCullStart;
#endif
};

class C_Manager : public Display::I_Renderer, public Ut::Singleton<Deferred::C_Manager, Ut::CreateExternal>
{
public:

	C_Manager();
	~C_Manager();

	bool	OnCreate();
	bool	OnCreateVolatile();
	void	OnDestroy();
	void	OnDestroyVolatile();

	bool	AddRenderTargetSetup(const C_RenderTargetSetup& setup);
	void	ClearRenderTargetSetups();

	bool	AddView(const U32 iWidth, const U32 iHeight, const S32 iIndex, const S32 iRenderTargetIndex);
	void	ClearViews();

	void	Begin(const S32 iIndex);
	void	End();

	const Display::C_Surface*		GetColourSurface(const S32 iViewIndex, const S32 iColourIndex);
	const Display::C_Surface*		GetDepthStencilSurface(const S32 iViewIndex);

	const Display::C_Texture2D*		GetColourTexture(const S32 iViewIndex, const S32 iColourIndex);
	const Display::C_Texture2D*		GetDepthStencilTexture(const S32 iViewIndex);

private:

	bool	CreateRenderTargets();

private:

	Btl::Vector<C_View>					m_Views;
	Btl::Vector<C_RenderTargetSetup>	m_Setups;
	Btl::Map<S32, S32>					m_ViewMapping;
	Btl::Map<S32, S32>					m_SetupMapping;

	bool								m_bActive;
	S32									m_iActiveIndex;
	
	U32									m_RenderTextureBufferSize;
	U32									m_DepthTextureBufferSize;
	U8*									m_pRenderTextureBuffer;
	U8*									m_pDepthTextureBuffer;
};

#endif // USE_DEFERRED_RENDER_BUFFERS

}; // namespace Deferred

#endif // DEFERRED_MANAGER_H
