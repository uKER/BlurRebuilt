#include "stdafx.h"
#include "Display/Shader/D3D10/D3D10VertexShader.h"

namespace NewDisplay
{
namespace D3D10
{

C_VertexShader::C_VertexShader() : m_pd3dVertexShader(NULL)
{
#ifdef DEVICE_DEBUG
	m_pHeader = NULL;
#endif
}

C_VertexShader::~C_VertexShader()
{
	assert(!IsCreated() && "The d3d vertex shader has not been destroyed");

#ifdef DEVICE_DEBUG
	SAFE_DELETE_PTR(m_pHeader);
#endif
}

}; // namespace D3D10
}; // namespace NewDisplay
