#include "stdafx.h"
#include "Display/Surface/GL/GLSurface.h"

namespace NewDisplay
{
namespace GL
{

C_SurfaceParams::C_SurfaceParams()
{
}

C_SurfaceParams::~C_SurfaceParams()
{
}

C_Surface::C_Surface() : m_glID(NULL), m_pHeader(NULL)
{
}

C_Surface::~C_Surface()
{
	assert(!IsCreated() && "The gl surface has not been destroyed");

	SAFE_DELETE_PTR(m_pHeader);
}

}; // namespace GL
}; // namespace NewDisplay
