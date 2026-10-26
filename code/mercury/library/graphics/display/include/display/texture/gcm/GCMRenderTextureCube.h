#ifndef DISPLAY_TEXTURE_GCM_GCM_RENDER_TEXTURE_CUBE_H
#define DISPLAY_TEXTURE_GCM_GCM_RENDER_TEXTURE_CUBE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/GCM/GCMTextureCube.h"
#include "Display/Surface/GCM/GCMSurface.h"

namespace Display
{
namespace GCM
{

//! Render texture cube wrapper for gcm.

class C_RenderTextureCube : public Display2::C_RenderTextureCube
{
public:

	C_RenderTextureCube();
	~C_RenderTextureCube();
};

}; // namespace GCM
}; // namespace Display

#include "Display/Texture/GCM/GCMRenderTextureCube.inl"

#endif // DISPLAY_TEXTURE_GCM_GCM_RENDER_TEXTURE_CUBE_H
