#ifndef DISPLAY_TEXTURE_TEXTURE_CUBE_UTILS_H
#define DISPLAY_TEXTURE_TEXTURE_CUBE_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/TextureCube.h"
#include "Display/Texture/TextureUtils.h"

namespace Display
{

bool UploadTextureCube(C_TextureCube* pTexture, C_TextureParams* pTextureParams);

U32 GetTextureCubeSize(const U32 width, const U32 flags, const E_Format format);
U32 GetTextureCubeSize(U32 width, const U32 levels, const U32 flags, const E_Format format);

bool CreateTextureCubeFromFile(Util::C_String fileName, C_TextureCube* pTexture);

}; // namespace Display

#endif // DISPLAY_TEXTURE_TEXTURE_CUBE_UTILS_H
