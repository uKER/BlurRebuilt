#ifndef DISPLAY_TGATEXTURE2DUTILS_H
#define DISPLAY_TGATEXTURE2DUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/Texture2D.h"

namespace NewDisplay
{
namespace TGA
{

bool LoadTexture2DFromFile(const Util::C_String& fileName, C_Texture2DParams* pParams);

}; // namespace TGA
}; // namespace NewDisplay

#endif // DISPLAY_TGATEXTURE2DUTILS_H
