#include "stdafx.h"
#include "Effect/Shadow/VarianceShadowMap.h"
#include "Effect/Shadow/ShadowMapSurface.h"

namespace Effect
{

C_VarianceShadowMap::C_VarianceShadowMap()
{
	m_iWidth	= 1024;
	m_iHeight	= 1024;
}

C_VarianceShadowMap::~C_VarianceShadowMap()
{
}

bool C_VarianceShadowMap::Create()
{
	#if PLATFORM_PS3
	const Display::E_Format eColourFormat = Display::E_Format_A16B16G16R16F;
	#else
	const Display::E_Format eColourFormat = Display::E_Format_G16R16F;
	#endif
	const Display::E_Format eDepthStencilFormat = Display::Device()->GetBackBuffer().GetDepthStencilFormat();

	if (!Display::Device()->CreateDepthStencilSurface(m_iWidth, m_iHeight, 0, eDepthStencilFormat, Display::E_Multisample_None, 0, &m_DepthStencilSurface))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetSurfaceName(&m_DepthStencilSurface, "_VarianceShadowMapDS");
	#endif

	#if PLATFORM_PS3
	PS3CreateDepthTextureFromSurface(&m_DepthStencilSurface, &m_ColourRenderTexture);
	#else
	if (!Display::Device()->CreateRenderTexture2D(m_iWidth, m_iHeight, 1, 0, eColourFormat, Display::E_Multisample_None, 0, &m_ColourRenderTexture))
		return false;
	#endif
	if (!Display::Device()->CreateRenderTexture2D(m_iWidth, m_iHeight, 1, 0, eColourFormat, Display::E_Multisample_None, 0, &m_IntermediateRenderTexture))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(m_iWidth, m_iHeight, 1, 0, eColourFormat, Display::E_Multisample_None, 0, &m_VarianceRenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_ColourRenderTexture, "_VarianceShadowMapColourRT");
	Display2::Device()->SetRenderTexture2DName(&m_IntermediateRenderTexture, "_VarianceShadowMapIntermediateRT");
	Display2::Device()->SetRenderTexture2DName(&m_VarianceRenderTexture, "_VarianceShadowMapVarianceRT");
	#endif

	return true;
}

void C_VarianceShadowMap::Destroy()
{
	Display::Device()->Destroy(&m_ColourRenderTexture);
	Display::Device()->Destroy(&m_IntermediateRenderTexture);
	Display::Device()->Destroy(&m_VarianceRenderTexture);
	Display::Device()->Destroy(&m_DepthStencilSurface);
}

void C_VarianceShadowMap::Bind()
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

void C_VarianceShadowMap::Unbind()
{

	#if PLATFORM_PS3
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	#endif

	m_VarianceFilter.VarianceFilter5(&m_VarianceRenderTexture, &m_IntermediateRenderTexture, &m_ColourRenderTexture);

	Display::Device()->SetColourDepthStencilSurface(m_pSavedColourSurface, m_pSavedDepthStencilSurface);
	Display::Device()->SetViewport(m_SavedViewport);
}

}; // namespace Effect
