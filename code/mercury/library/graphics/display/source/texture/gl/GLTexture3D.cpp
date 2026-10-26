#include "stdafx.h"
#include "Display/Texture/GL/GLTexture3D.h"

namespace NewDisplay
{
namespace GL
{

C_Texture3D::C_Texture3D()
{
}

C_Texture3D::~C_Texture3D()
{
	assert(!IsCreated() && "The gl 3d texture has not been destroyed");
}

bool C_Texture3D::Upload(C_TextureParams* pParams)
{
	assert(pParams);

	return false;
}

}; // namespace GL
}; // namespace NewDisplay
