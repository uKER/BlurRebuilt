#include "stdafx.h"
#include "Display/Shader/GL/GLPixelShader.h"

namespace NewDisplay
{
namespace GL
{

C_PixelShader::C_PixelShader() : m_cgProgram(NULL)
{
	m_FileName = "";
	m_FileTime = 0;
}

C_PixelShader::~C_PixelShader()
{
	assert(!m_cgProgram && "The gl cg program has not been destroyed");
}

}; // namespace GL
}; // namespace NewDisplay
