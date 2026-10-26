#include "stdafx.h"
#include "Display/Shader/GL/GLVertexShader.h"

namespace NewDisplay
{
namespace GL
{

C_VertexShader::C_VertexShader() : m_cgProgram(NULL)
{
	m_FileName = "";
	m_FileTime = 0;
}

C_VertexShader::~C_VertexShader()
{
	assert(!m_cgProgram && "The gl cg program has not been destroyed");
}

}; // namespace GL
}; // namespace NewDisplay
