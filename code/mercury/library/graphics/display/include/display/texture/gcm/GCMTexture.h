#ifndef DISPLAY_TEXTURE_GCM_GCM_TEXTURE_H
#define DISPLAY_TEXTURE_GCM_GCM_TEXTURE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"
#include "Display/Resource/Resource.h"

namespace Display
{

typedef Display2::C_Texture C_Texture;
typedef Display2::C_Texture C_Texture2D;
typedef Display2::C_Texture C_Texture3D;
typedef Display2::C_Texture C_TextureCube;

}; // namespace Display

#include "Display/Texture/GCM/GCMTexture.inl"

#endif // DISPLAY_TEXTURE_GCM_GCM_TEXTURE_H
