#ifndef DISPLAY_GLRENDERTEXTURE2D_H
#define DISPLAY_GLRENDERTEXTURE2D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/GL/GLTexture2D.h"
#include "Display/Surface/GL/GLSurface.h"

namespace NewDisplay
{
namespace GL
{

class C_RenderTexture2D
{
public:

	C_RenderTexture2D();
	~C_RenderTexture2D();

	const bool IsCreated() const;

public:

	C_Texture2D	m_Texture;
	C_Surface	m_Surface;
};

#include "GLRenderTexture2D.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLRENDERTEXTURE2D_H
