#include "stdafx.h"

#include "SceneGraph/RenderGroupSettings.h"

#include "Display/Device/Device.h"

#include "Display/Window/Window.h"	// [Blur2PC] PC too (T089)

namespace SceneGraph
{


void C_RenderGroupSettings::SetWindow(const S32 windowIndex, const bool aa)
{
#ifndef PLATFORM_PC
	const Display::C_Window* pWindow = Display::Device()->GetWindow(windowIndex);
	
	if(aa)
	{
		m_Multisample = pWindow->GetMultisample();
		m_WindowIndex = windowIndex;
	}
	else
	{
		m_WindowIndex = -1;
		m_Multisample = Display::E_Multisample_None;
	}
#else
	// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x0079f470): the window's multisample when drawing into it, and its
	// colour and depth textures; the surfaces are left as they are
	Display::C_Window* pWindow = Display::Device()->GetWindow(windowIndex);

	if(aa)
	{
		m_Multisample = pWindow->GetMultisample();
		m_WindowIndex = windowIndex;
	}
	else
	{
		m_WindowIndex = -1;
		m_Multisample = Display::E_Multisample_None;
	}

	m_pColourTexture = pWindow->GetColourTexture();
	m_pDepthStencilTexture = pWindow->GetDepthStencilTexture();
#endif
}

} // namespace SceneGraph

