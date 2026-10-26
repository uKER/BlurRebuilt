#ifndef RENDER_GLOBALS_H
#define RENDER_GLOBALS_H

#ifdef _MSC_VER
#pragma once
#endif

extern Debug::Int g_debugRenderQuality;

namespace Render
{

const Display::C_Texture2D* GetGlobalColourBuffer(const bool useAAIfAvailable = false);
const Display::C_Texture2D* GetGlobalDepthStencilBuffer(const bool useAAIfAvailable = false);

void SetGlobalColourBuffer(const Display::C_Texture2D* pTexture);
void SetGlobalDepthStencilBuffer(const Display::C_Texture2D* pTexture);

#if defined(PLATFORM_PC)
// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x006a1940/0x006a1970, value 0x01152b50): the display window's mask
// texture (C_PCD3D9Device::RenderWindowMask), published with the colour and depth buffers
const Display::C_Texture2D* GetGlobalStencilBuffer();
void SetGlobalStencilBuffer(const Display::C_Texture2D* pTexture);
#endif

#if defined(PLATFORM_PS3) && !defined(USE_DISPLAY_WINDOWS)
extern Display::C_RenderTexture2D g_ColourBuffer;
extern Display::C_RenderTexture2D g_DepthStencilBuffer;

bool CreateGlobalGraphicsBuffers();
void DestroyGlobalGraphicsBuffers();
#endif

#if defined(PLATFORM_PC) && !defined(USE_DISPLAY_WINDOWS)	// [Blur2PC] retail Blur PC uses display windows (T089)
extern Display::C_RenderTexture2D g_ColourBuffer;
extern Display::C_RenderTexture2D g_DepthStencilBuffer;

bool CreateGlobalColourBuffer();
bool CreateGlobalDepthStencilBuffer();
void DestroyGlobalDepthStencilBuffer();
void DestroyGlobalColourBuffer();
#endif

}; // Render

#endif // RENDER_GLOBALS_H
