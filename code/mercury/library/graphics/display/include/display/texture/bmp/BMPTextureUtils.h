#ifndef DISPLAY_BMP_TEXTURE_UTILS_H
#define DISPLAY_BMP_TEXTURE_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/TextureParams.h"

namespace Display
{
namespace BMP
{

bool LoadTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams);

}; // namespace BMP
}; // namespace Display

#endif // DISPLAY_BMP_TEXTURE_UTILS_H
