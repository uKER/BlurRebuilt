#ifndef SHADER_SCENE_DEFAULT_LIGHTING_H
#define SHADER_SCENE_DEFAULT_LIGHTING_H

C_Lighting EvaluateLighting(in C_Material mtlData, in C_Surface surfaceData, in C_VertexOutput vtxData)
{
	C_Lighting lightData = (C_Lighting) 0;
	lightData.m_fSunOcclusion	= 1.0f;
	lightData.m_fSkyOcclusion	= surfaceData.m_vNormal.y * 0.5f + 0.5f;
	lightData.m_vLighting		= 0.0f;
#if defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING)
	lightData.m_vLighting		= vtxData.m_vColour.rgb;
#endif
	lightData.m_vAverage		= lightData.m_vLighting;
	lightData.m_vHighlight		= g_vSunColour.xyz * lightData.m_fSunOcclusion;
	lightData.m_vDirection		= g_vSunDirection.xyz;
	return lightData;
}

#endif // SHADER_SCENE_DEFAULT_LIGHTING_H
