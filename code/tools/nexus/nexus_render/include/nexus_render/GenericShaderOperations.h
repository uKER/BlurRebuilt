#ifndef GENERIC_SHADER_OPERATIONS_H
#define GENERIC_SHADER_OPERATIONS_H

namespace SRD
{
	bool InitialiseGenericLightmapShader(ID3DXEffect* pShader);

	bool UpdateGenericLightmapShader(
		LightmapShader* pProporties,
		const D3DXMATRIX& view,
		const D3DXMATRIX& worldView,
		const D3DXMATRIX& worldObject);
}

#endif