#include "stdafx.h"
#include "Effect/Shadow/ShadowMap.h"
#include "Effect/Shadow/ShadowMapSurface.h"

namespace Effect
{

C_ShadowMap::C_ShadowMap()
{
	m_iWidth	= 1024;
	m_iHeight	= 1024;
}

C_ShadowMap::~C_ShadowMap()
{
}

bool C_ShadowMap::Create()
{
	const Display::E_Format eColourFormat = Display::E_Format_R32F;
	const Display::E_Format eDepthStencilFormat = Display::Device()->GetBackBuffer().GetDepthStencilFormat();

	if (!Display::Device()->CreateDepthStencilSurface(m_iWidth, m_iHeight, 0, eDepthStencilFormat, Display::E_Multisample_None, 0, &m_DepthStencilSurface))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetSurfaceName(&m_DepthStencilSurface, "_ShadowMapDS");
	#endif

	#if PLATFORM_PS3
	PS3CreateRenderTextureFromSurface(&m_DepthStencilSurface, &m_ColourRenderTexture);
	#else
	if (!Display::Device()->CreateRenderTexture2D(m_iWidth, m_iHeight, 1, 0, eColourFormat, Display::E_Multisample_None, 0, &m_ColourRenderTexture))
		return false;
	#endif

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_ColourRenderTexture, "_ShadowMapColourRT");
	#endif

	return true;
}

void C_ShadowMap::Destroy()
{
	Display::Device()->Destroy(&m_ColourRenderTexture);
	Display::Device()->Destroy(&m_DepthStencilSurface);
}

void C_ShadowMap::Bind()
{
	m_pSavedColourSurface = Display::Device()->GetRenderTarget().GetColourSurface();
	m_pSavedDepthStencilSurface = Display::Device()->GetRenderTarget().GetDepthStencilSurface();
	m_SavedViewport = Display::Device()->GetViewport();

	#if PLATFORM_PS3
	Display::Device()->SetColourDepthStencilSurface(0, &m_DepthStencilSurface);
	Display::Device()->SetColourWriteEnable(0);
	Display::Device()->Clear(Display::E_Clear_Depth | Display::E_Clear_Stencil);
	#else
	Display::Device()->SetColourDepthStencilSurface(&m_ColourRenderTexture.GetSurface(), &m_DepthStencilSurface);
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil, Maths::RGBA(0xFFFFFFFF));
	#endif

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = m_iWidth;
	viewport.m_H = m_iHeight;
	viewport.m_MinZ = m_SavedViewport.GetMinZ();
	viewport.m_MaxZ = m_SavedViewport.GetMaxZ();
	Display::Device()->SetViewport(viewport);	
}

void C_ShadowMap::Unbind()
{

	#if PLATFORM_PS3
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	#endif

	Display::Device()->SetColourDepthStencilSurface(m_pSavedColourSurface, m_pSavedDepthStencilSurface);
	Display::Device()->SetViewport(m_SavedViewport);
}

}; // namespace Effect
