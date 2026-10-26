#ifndef DISPLAY_DDS_TEXTURE_UTILS_H
#define DISPLAY_DDS_TEXTURE_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/TextureParams.h"

namespace Display
{
namespace DDS
{

bool LoadTextureFromMemory(U8* pData, const size_t size, C_TextureParams* pParams, const char* filename=NULL);

bool LoadTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams);

}; // namespace DDS
}; // namespace Display

#endif // DISPLAY_DDS_TEXTURE_UTILS_H
