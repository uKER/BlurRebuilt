#include "stdafx.h"
#include <scene_renderer/shaderproperties.h>

using namespace SRD;

ShaderProperties::ShaderProperties()
{
	m_pShader = 0;
}

ShaderProperties::ShaderProperties(const ShaderProperties& rhs)
{
	*this = rhs;
}

ShaderProperties::~ShaderProperties()
{
}

ID3DXEffect* ShaderProperties::GetShaderPtr()
{
	return m_pShader;
}

void ShaderProperties::SetShaderPtr(ID3DXEffect* pShader)
{
	m_pShader = pShader;
}