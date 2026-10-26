#ifndef DISPLAY_TEXTURE_TEXTURE_2D_UTILS_H
#define DISPLAY_TEXTURE_TEXTURE_2D_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/Texture2D.h"
#include "Display/Texture/TextureUtils.h"

#if defined(DEVICE_PCD3D11) // [Blur1PCDX11] Native PC texture stream loader.
#include "Display/Texture/D3D11/D3D11TextureLoad.h"
#elif defined(DEVICE_PS3GCM)
#include "Display/Texture/GCM/GCMTextureLoad.h"
#else
#pragma message("Unsupported device define when compiling Texture2DUtils.h")
#endif

namespace Display
{

bool UploadTexture2D(C_Texture2D* pTexture, C_TextureParams* pTextureParams);

U32 GetTexture2DSize(const U32 width, const U32 height, const U32 flags, const E_Format format);
U32 GetTexture2DSize(U32 width, U32 height, const U32 levels, const U32 flags, const E_Format format);

bool CreateTexture2DFromFile(Util::C_String fileName, C_Texture2D* pTexture);

U32 GetRenderTexture2DSize(U32 width, U32 height, const U32 levels, const U32 flags, const E_Format format, const E_Multisample multisample);

// Size of actual texture data ignoring tiling and alignment.
U32 GetActiveDataSize(const C_Texture2D* pTexture);

bool IsFloatFormat(const C_Texture2D* pTexture);

}; // namespace Display

#endif // DISPLAY_TEXTURE_TEXTURE_2D_UTILS_H
