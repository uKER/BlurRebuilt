#include "stdafx.h"
#include "Display/Texture/D3D10/D3D10Texture2D.h"

namespace NewDisplay
{
namespace D3D10
{

C_Texture2D::C_Texture2D() : m_pd3dTexture2D(NULL)
{
#ifdef DEVICE_DEBUG
	m_pHeader	= NULL;
#endif
}

C_Texture2D::~C_Texture2D()
{
	assert(!IsCreated() && "The d3d texture 2d has not been destroyed");

#ifdef DEVICE_DEBUG
	SAFE_DELETE_PTR(m_pHeader);
#endif
}

bool C_Texture2D::Upload(C_Texture2DParams* pParams)
{
	assert(pParams);

	return false;
}

}; // namespace D3D10
}; // namespace NewDisplay
