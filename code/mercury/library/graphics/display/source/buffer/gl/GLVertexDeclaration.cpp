#include "stdafx.h"
#include "Display/Buffer/GL/GLVertexDeclaration.h"

namespace NewDisplay
{
namespace GL
{

C_VertexDeclaration::C_VertexDeclaration()
{
}

C_VertexDeclaration::~C_VertexDeclaration()
{
	m_Elements.Clear();
}

}; // namespace GL
}; // namespace NewDisplay
