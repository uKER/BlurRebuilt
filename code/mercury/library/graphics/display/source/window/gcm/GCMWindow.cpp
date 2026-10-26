#include "stdafx.h"
#include "Display/Window/GCM/GCMWindow.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Display
{
namespace GCM
{

C_Window::C_Window(const Maths::C_Rect& coords)
{
	m_Coords					= coords;

	m_pColourSurface			= NULL;
	m_pDepthStencilSurface		= NULL;

	m_pColourTexture			= NULL;
	m_pDepthStencilTexture		= NULL;
}

C_Window::~C_Window()
{
}

void C_Window::OverlapBuffers(C_Surface* pColourSurface, C_Surface* pDepthStencilSurface, C_Texture* pColourTexture, C_Texture* pDepthStencilTexture)
{
	m_pColourSurface			= pColourSurface;
	m_pDepthStencilSurface		= pDepthStencilSurface;

	m_pColourTexture			= pColourTexture;
	m_pDepthStencilTexture		= pDepthStencilTexture;
}

void C_Window::Resize(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat, const E_Format depthStencilFormat)
{
	const F32 w1 = ((F32)(width))*m_Coords.Min().X();
	const F32 w2 = ((F32)(width))*m_Coords.Max().X();
	const F32 h1 = ((F32)(height))*m_Coords.Min().Y();
	const F32 h2 = ((F32)(height))*m_Coords.Max().Y();

	m_RenderBuffer.SetWidth((U32)(w2-w1));
	m_RenderBuffer.SetHeight((U32)(h2-h1));
	m_RenderBuffer.SetMultisample(GetMultisample2(multisample));
	m_RenderBuffer.SetColourFormat(0, GetTextureFormat2(colourFormat));
	m_RenderBuffer.SetDepthStencilFormat(GetTextureFormat2(depthStencilFormat));
}

}; // namespace GCM
}; // namespace Display
