#ifndef DISPLAY_TEXTURE_TEXTURE_3D_UTILS_H
#define DISPLAY_TEXTURE_TEXTURE_3D_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/Texture3D.h"
#include "Display/Texture/TextureUtils.h"

namespace Display
{
	
bool UploadTexture3D(C_Texture3D* pTexture, C_TextureParams* pTextureParams);

U32 GetTexture3DSize(const U32 width, const U32 height, const U32 depth, const U32 flags, const E_Format format);
U32 GetTexture3DSize(U32 width, U32 height, const U32 depth, const U32 levels, const U32 flags, const E_Format format);

bool CreateTexture3DFromFile( Util::C_String fileName, C_Texture3D* pTexture, const U32 flags = 0);

}; // namespace Display

#endif // DISPLAY_TEXTURE_TEXTURE_3D_UTILS_H
