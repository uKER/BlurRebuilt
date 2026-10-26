#ifndef DISPLAY_TGA_TEXTURE_UTILS_H
#define DISPLAY_TGA_TEXTURE_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/TextureParams.h"

namespace Display
{
namespace TGA
{

bool LoadTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams);

}; // namespace TGA
}; // namespace Display

#endif // DISPLAY_TGA_TEXTURE_UTILS_H
