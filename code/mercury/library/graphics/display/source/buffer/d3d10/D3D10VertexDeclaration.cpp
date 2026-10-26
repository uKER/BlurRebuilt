#include "stdafx.h"
#include "Display/Buffer/D3D10/D3D10VertexDeclaration.h"

namespace NewDisplay
{
namespace D3D10
{

C_VertexDeclaration::C_VertexDeclaration() : m_pd3dInputLayout(NULL)
{
}

C_VertexDeclaration::~C_VertexDeclaration()
{
	if (m_pd3dInputLayout)
	{
		const ULONG refCount = m_pd3dInputLayout->Release();
		assert(refCount == 0);
		m_pd3dInputLayout = NULL;
	}
}

}; // namespace D3D10
}; // namespace NewDisplay
