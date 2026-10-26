#ifndef RENDER_REDUCEDRENDER_MANAGER_H
#define RENDER_REDUCEDRENDER_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/ReducedRender/Enums.h>

namespace Render
{
namespace Reduced
{

class C_MSAA;

enum E_BlendMode
{
	E_BlendMode_Additive,
	E_BlendMode_Alpha
};

struct View
{
	U32	iWidth;
	U32 iHeight;
	S32 iIndex;
};

typedef Btl::Vector<View> Views;

struct RenderTarget
{
	Display::C_RenderTexture2D* pColourRenderTexture;
	Display::C_Surface* pDepthStencilSurface;
};

typedef Btl::Vector<RenderTarget> RenderTargets;

class C_Manager : public Display::I_Renderer
{
public:

	C_Manager(const bool bFullPrecision = false);
	~C_Manager();

	bool		OnCreate();
	bool		OnCreateVolatile();
	void		OnDestroy();
	void		OnDestroyVolatile();

	void		SetDepth(const S32 iIndex, const E_Downsample eDownsample, const Display::C_Texture2D* pDepthTexture) const;
	
	Display::C_Surface*	GetColourSurface(const S32 iIndex, const E_Downsample eDownsample) const;
	Display::C_Surface*	GetDepthStencilSurface(const S32 iIndex, const E_Downsample eDownsample) const;

	void		Begin(const S32 iIndex, const E_Downsample eDownsample) const;
	void		End() const;

	const Display::C_Texture2D* GetTexture(const S32 iIndex, const E_Downsample eDownsample) const;
	void		Render(const S32 iIndex, const E_Downsample eDownsample, const E_BlendMode eBlendMode) const;

	void		AddView(const S32 iIndex, const U32 iWidth, const U32 iHeight);

private:

	const bool			m_bFullPrecision;

	Views				m_Views;
	Btl::Vector<S32>	m_ViewMapping;

	RenderTargets		m_RenderTargets4;
	RenderTargets		m_RenderTargets16;
	Btl::Vector<S32>	m_RenderTargetMapping;

	mutable E_Downsample		m_eDownsample;
	mutable bool				m_bBegun;
	mutable U32					m_iIndex;

	Display::C_PixelShader				m_CompositeShader;
	Display::C_PixelShader				m_Downsample4Shader;
	Display::C_PixelShader				m_Downsample16Shader;
	Display::C_PixelShaderParameter		m_Downsample4Parameter;
	Display::C_PixelShaderParameter		m_Downsample16Parameter;

	mutable Display::C_Viewport			m_SavedViewport;
	mutable const Display::C_Surface*	m_pColourSurface;
	mutable const Display::C_Surface*	m_pDepthStencilSurface;

	mutable C_MSAA*						m_pMSAA;
};

} // namespace Reduced
} // namespace Render

#endif // RENDER_REDUCEDRENDER_MANAGER_H
