#ifndef DISPLAY_BMPTEXTURE2DUTILS_H
#define DISPLAY_BMPTEXTURE2DUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/Texture2D.h"

namespace NewDisplay
{
namespace BMP
{

bool LoadTexture2DFromFile(const Util::C_String& fileName, C_Texture2DParams* pParams);

}; // namespace BMP
}; // namespace NewDisplay

#endif // DISPLAY_BMPTEXTURE2DUTILS_H
