#ifndef DISPLAY_TEXTURE_TEXTURE_UTILS_H
#define DISPLAY_TEXTURE_TEXTURE_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/Texture.h"

#include <System/Serialise.h>

namespace Display
{

bool LoadTexture(C_Texture* pTexture, Sys::I_File* pFile, U32& flags);

void RefreshTextures(const Util::C_String& dir);

}; // namespace Display

#endif // DISPLAY_TEXTURE_TEXTURE_UTILS_H
