#include "stdafx.h"
#include "Display/Shader/D3D10/D3D10PixelShader.h"

namespace NewDisplay
{
namespace D3D10
{

C_PixelShader::C_PixelShader() : m_pd3dPixelShader(NULL)
{
#ifdef DEVICE_DEBUG
	m_pHeader = NULL;
#endif
}

C_PixelShader::~C_PixelShader()
{
	assert(!IsCreated() && "The d3d pixel shader has not been destroyed");

#ifdef DEVICE_DEBUG
	SAFE_DELETE_PTR(m_pHeader);
#endif
}

}; // namespace D3D10
}; // namespace NewDisplay
