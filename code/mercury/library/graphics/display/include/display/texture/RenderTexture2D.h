#ifndef DISPLAY_TEXTURE_RENDER_TEXTURE_2D_H
#define DISPLAY_TEXTURE_RENDER_TEXTURE_2D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"

#if defined(DEVICE_PCD3D11)
#include "Display/Device/D3D11/D3D11Resources.h" // [Blur1PCDX11] Native resources.
#elif defined(DEVICE_PS3GCM)
#include "Display/Texture/GCM/GCMRenderTexture2D.h"
#elif defined(DEVICE_PCD3D10)
#include "Display/Texture/D3D10/D3D10RenderTexture2D.h"
#elif defined(DEVICE_PCGL) || defined(DEVICE_PSGL)
#include "Display/Texture/GL/GLRenderTexture2D.h"
#else
#pragma message("Unsupported device define when compiling RenderTexture2D.h")
#endif

#endif // DISPLAY_TEXTURE_RENDER_TEXTURE_2D_H
