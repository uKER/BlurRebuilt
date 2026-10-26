#include "stdafx.h"

#include "ui/Graphics/PostProcess.h"

#ifndef PLATFORM_PS3

namespace UI
{

C_PostProcess gPostProcess;

#if defined( _GENESIS_ )
void C_PostProcess::Init( Display::C_Device* displayDevicePtr, Resource::C_Manager* resManPtr )
{
	m_DisplayDevicePtr = displayDevicePtr;
	m_ResManPtr = resManPtr;
	m_RenderTargetPtr = NULL;
	m_Active = false;
	m_DoUpdate = true;
}
#endif

void C_PostProcess::Create(const S32 layer)
{
	m_Layer = layer;
#ifndef PLATFORM_PS3
#if defined( _GENESIS_ )
	const Display::C_BackBuffer& backBuffer = m_DisplayDevicePtr->GetBackBuffer();
	m_RenderTargetPtr = Resource::CreateAcquireTexture( m_ResManPtr, Resource::C_ResourceId("UIPostProcess"), Display::C_SurfaceCreate( backBuffer.GetWidth(), backBuffer.GetHeight(), Display::E_SurfaceUsage_Colour, true ), 1, false);
	if( !m_RenderTargetPtr )
	{
		LOGWARN( g_Log,  "UI PostProcess failed to create render target texture\n");
		return;
	}
	// Copy back buffer to surface (this needs to be cleared first)
	if(m_RenderTargetPtr->Acquire())
	{
		Resource::C_Texture* colourTexturePtr;
		m_RenderTargetPtr->GetResource( colourTexturePtr );                          

		colourTexturePtr->CopyBackBuffer();
	}
	m_RenderTargetPtr->Release();
#endif // _GENESIS_
#endif // PLATFORM_PS3

	m_Active = true;
}

const bool C_PostProcess::Update()
{
	if (!m_Active)
		return true;

	if (!m_DoUpdate)
		return true;

#if defined( _GENESIS_ )
	if(!m_RenderTargetPtr->Acquire())
		return false;

	Resource::C_Texture* colourTexturePtr;
	m_RenderTargetPtr->GetResource( colourTexturePtr );

#ifndef PLATFORM_PS3
	colourTexturePtr->CopyBackBuffer();
#endif // PLATFORM_PS3

	m_RenderTargetPtr->Release();
#endif // _GENESIS_

	return true;
}
void C_PostProcess::Destroy()
{
	if(!m_Active)
		return;

#if defined( _GENESIS_ )
	m_RenderTargetPtr = NULL;
#endif

	m_Active = false;
}

void C_PostProcess::Exit()
{
#if defined( _GENESIS_ )

	assert(m_DisplayDevicePtr);

	m_DisplayDevicePtr->Unbind();

	m_RenderTargetPtr = NULL;
	m_DisplayDevicePtr = NULL;
	m_ResManPtr = NULL;

#endif // _GENESIS_
	m_Active = false;
}

}; // namespace UI

#endif // PLATFORM_PS3
