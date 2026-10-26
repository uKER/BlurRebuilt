#include "stdafx.h"
#include "Render/Utils/Globals.h"
#include "Render/Render/RenderSettings.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

Debug::Int g_debugRenderQuality ("Graphics.Render.Quality", 6, Debug::Range(0, 6));

#if PLATFORM_PS3
Debug::Bool g_debugForceNoAAForGlobalBuffers("Graphics.Render.ForceNoAAForGlobalBuffers", true);
#endif

namespace Render
{

const Display::C_Texture2D* g_pColourBuffer = NULL;
const Display::C_Texture2D* g_pDepthStencilBuffer = NULL;

#if PLATFORM_PS3
const Display::C_Texture2D* g_pOverriddenColourBuffer = NULL;
const Display::C_Texture2D* g_pOverriddenDepthStencilBuffer = NULL;
#endif

const Display::C_Texture2D* GetGlobalColourBuffer(const bool useAAIfAvailable)
{
#if PLATFORM_PS3
	if(!useAAIfAvailable && g_debugForceNoAAForGlobalBuffers)
	{
		return g_pOverriddenColourBuffer;
	}
	else
#endif
	{
		return g_pColourBuffer;
	}
}

const Display::C_Texture2D* GetGlobalDepthStencilBuffer(const bool useAAIfAvailable)
{
#if PLATFORM_PS3
	if(!useAAIfAvailable && g_debugForceNoAAForGlobalBuffers)
	{
		return g_pOverriddenDepthStencilBuffer;
	}
	else
#endif
	{
		return g_pDepthStencilBuffer;
	}
}

void SetGlobalColourBuffer(const Display::C_Texture2D*pTexture)
{
#if PLATFORM_PS3
	g_pOverriddenColourBuffer = Display::Device()->GetLocalSharedRenderBuffer()->GetColourTexture(0);
#endif
	
	g_pColourBuffer = pTexture;
}

#if defined(PLATFORM_PC)
// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x006a1940/0x006a1970)
const Display::C_Texture2D* g_pStencilBuffer = NULL;

const Display::C_Texture2D* GetGlobalStencilBuffer()
{
	return g_pStencilBuffer;
}

void SetGlobalStencilBuffer(const Display::C_Texture2D* pTexture)
{
	g_pStencilBuffer = pTexture;
}
#endif

void SetGlobalDepthStencilBuffer(const Display::C_Texture2D* pTexture)
{
#if PLATFORM_PS3
	g_pOverriddenDepthStencilBuffer = Display::Device()->GetLocalSharedRenderBuffer()->GetDepthStencilTexture();
#endif

	g_pDepthStencilBuffer = pTexture;
}

#if defined(PLATFORM_PS3) && !defined(USE_DISPLAY_WINDOWS)
Display::C_RenderTexture2D g_ColourBuffer;
Display::C_RenderTexture2D g_DepthStencilBuffer;

bool CreateGlobalGraphicsBuffers()
{
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	if (!Display::Device()->CreateRenderTexture2D(backBuffer.GetWidth(), backBuffer.GetHeight(), 1, 0, backBuffer.GetColourFormat(), Display::E_Multisample_None, NULL, &g_ColourBuffer))
	{
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&g_ColourBuffer, "_GlobalColourBufferRT");
	#endif

#ifdef PLATFORM_PS3
	// for the PS3 we can't sample depth-stencil textures directly, so we must instead sample a A8R8G8B8 surface
	// however, this is assuming that the depth-stencil format of the surface is 32bit, need to change this
	if (!Display::Device()->CreateRenderTexture2D(backBuffer.GetWidth(), backBuffer.GetHeight(), 1, 0, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, NULL, &g_DepthStencilBuffer))
	{
		return false;
	}
#else
	if (!Display::Device()->CreateRenderTexture2D(backBuffer.GetWidth(), backBuffer.GetHeight(), 1, 0, backBuffer.GetDepthStencilFormat(), Display::E_Multisample_None, NULL, &g_DepthStencilBuffer))
	{
		return false;
	}
#endif

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&g_DepthStencilBuffer, "_GlobalDepthStencilBufferRT");
	#endif

	g_pColourBuffer = &g_ColourBuffer.GetTexture();
	g_pDepthStencilBuffer = &g_DepthStencilBuffer.GetTexture();

	return true;
}

void DestroyGlobalGraphicsBuffers()
{
	Display::Device()->Destroy(&g_ColourBuffer);
	Display::Device()->Destroy(&g_DepthStencilBuffer);

	g_pColourBuffer = NULL;
	g_pDepthStencilBuffer = NULL;
}

#endif // PLATFORM_PS3

#if defined(PLATFORM_PC) && !defined(USE_DISPLAY_WINDOWS)	// [Blur2PC] retail Blur PC uses display windows (T089)

Display::C_RenderTexture2D g_ColourBuffer;
Display::C_RenderTexture2D g_DepthStencilBuffer;

bool CreateGlobalColourBuffer()
{
	if(!Render::C_RenderSettings::ColourBufferSupported())
		return true;

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	if(!Display::Device()->CreateRenderTexture2D(backBuffer.GetWidth(), backBuffer.GetHeight(), 1, Display::E_Usage_Colour, backBuffer.GetColourFormat(), Display::E_Multisample_None, NULL, &g_ColourBuffer))
	{
		return false;
	}
 
	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&g_ColourBuffer, "_GlobalColourBufferRT");
	#endif

	g_pColourBuffer = &g_ColourBuffer.GetTexture();
	
	
	return true;
}

bool CreateGlobalDepthStencilBuffer()
{
	if(!Render::C_RenderSettings::DepthStencilBufferSupported())
		return true;

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	if(!Display::Device()->CreateRenderTexture2D(backBuffer.GetWidth(), backBuffer.GetHeight(), 1,
										Display::E_Usage_Colour, 
										Display::E_Format_R32F, // E_Format_D24S8,  
										Display::E_Multisample_None, NULL, &g_DepthStencilBuffer))
	{
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&g_DepthStencilBuffer, "_GlobalDepthStencilBufferRT");
	#endif

	g_pDepthStencilBuffer = &g_DepthStencilBuffer.GetTexture();

	return true;
}

void DestroyGlobalDepthStencilBuffer()
{
	Display::Device()->Destroy(&g_DepthStencilBuffer);

	g_pDepthStencilBuffer = NULL;
}

void DestroyGlobalColourBuffer()
{
	Display::Device()->Destroy(&g_ColourBuffer);

	g_pColourBuffer = NULL;
}

#endif // PLATFORM_PC

}; // namespace Render
