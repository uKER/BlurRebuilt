#ifndef GENERIC_SHADER_OPERATIONS_H
#define GENERIC_SHADER_OPERATIONS_H

namespace SRD
{
	bool InitialiseGenericLightmapShader(ID3DXEffect* pShader);

	bool UpdateGenericLightmapShader(
		LightmapShaderProperties* pProperties,
		const D3DXMATRIX& view,
		const D3DXMATRIX& worldView,
		const D3DXMATRIX& worldObject);

	bool ExtractLightmapShaderPropertiesFromAftk(
		const Aftk::C_MaterialNode* pAftkMaterial, 
		LightmapShaderProperties* pShaderProperties);
}

#endif