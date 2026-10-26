#ifndef DISPLAY_TEXTURE_GCM_GCM_RENDER_TEXTURE_2D_H
#define DISPLAY_TEXTURE_GCM_GCM_RENDER_TEXTURE_2D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/GCM/GCMTexture2D.h"
#include "Display/Surface/GCM/GCMSurface.h"

namespace Display
{
namespace GCM
{

//! Render texture 2d wrapper for gcm.

class C_RenderTexture2D : public Display2::C_RenderTexture2D
{
public:

	C_RenderTexture2D();
	~C_RenderTexture2D();

	void				Resolve() const;
};

}; // namespace GCM
}; // namespace Display

#include "Display/Texture/GCM/GCMRenderTexture2D.inl"

#endif // DISPLAY_TEXTURE_GCM_GCM_RENDER_TEXTURE_2D_H
