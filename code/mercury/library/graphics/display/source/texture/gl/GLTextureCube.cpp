#include "stdafx.h"
#include "Display/Texture/GL/GLTextureCube.h"

namespace NewDisplay
{
namespace GL
{

C_TextureCube::C_TextureCube()
{
}

C_TextureCube::~C_TextureCube()
{
	assert(!IsCreated() && "The gl cube texture has not been destroyed");
}

void* C_TextureCube::Lock(const E_CubeTextureFace face, const U32 level)
{
	return NULL;
}

void C_TextureCube::Unlock(const E_CubeTextureFace face, const U32 level)
{
}

bool C_TextureCube::Upload(C_TextureParams* pParams)
{
	assert(pParams);

	return false;
}

}; // namespace GL
}; // namespace NewDisplay
