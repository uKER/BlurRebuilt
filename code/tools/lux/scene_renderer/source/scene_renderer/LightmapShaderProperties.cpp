#include "stdafx.h"
#include <scene_renderer/ShaderProperties.h>
#include <scene_renderer/LightmapShaderProperties.h>

using namespace SRD;

LightmapShaderProperties::LightmapShaderProperties() : ShaderProperties()
{
	m_pDiffuseTexture = 0;
	m_pLightmapTexture = 0;
}

LightmapShaderProperties::LightmapShaderProperties(const LightmapShaderProperties& rhs)
{
	*this = rhs;
}

LightmapShaderProperties::~LightmapShaderProperties()
{
}

ShaderPropertiesTypeEnum LightmapShaderProperties::GetType() const
{
	return SHADER_PROPORTIES_TYPE_LIGHTMAP;
}

IDirect3DTexture9* LightmapShaderProperties::GetDiffuseTexturePtr()
{
	return m_pDiffuseTexture;
}

IDirect3DTexture9* LightmapShaderProperties::GetLightmapTexturePtr()
{
	return m_pLightmapTexture;
}

void LightmapShaderProperties::SetDiffuseTexturePtr(IDirect3DTexture9* pTexture)
{
	m_pDiffuseTexture = pTexture;
}

void LightmapShaderProperties::SetLightmapTexturePtr(IDirect3DTexture9* pTexture)
{
	m_pLightmapTexture = pTexture;
}

