#ifndef DISPLAY_GLTEXTURE_H
#define DISPLAY_GLTEXTURE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GL/GL.h"
#include "Display/Resource/Resource.h"

namespace NewDisplay
{
namespace GL
{

class C_Texture : public C_Resource
{
public:

	C_Texture();
	~C_Texture();

public:

	GLuint m_glID;
};

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLTEXTURE_H
