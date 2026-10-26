#ifndef SHADER_SCENE_BASIS_LIGHTING_H
#define SHADER_SCENE_BASIS_LIGHTING_H

#include <Shader/Lighting/Lightmap.h>
#include <Shader/Lighting/SphericalHarmonics.h>

C_Lighting EvaluateLighting(in C_Material mtlData, in C_Surface surfaceData, in C_VertexOutput vtxData)
{
	C_Lighting lightData = (C_Lighting) 0;
	lightData.m_fSunOcclusion		= 1.0f;
	lightData.m_fSkyOcclusion		= 1.0f;
	lightData.m_vLighting			= 1.0f;

#if !defined(MAYA)
#if defined(ENABLE_LIGHTMAP)
#if defined(ENABLE_TANGENTNORMAL)
	BasisLightmap(Lightmap0Sampler, vtxData.m_vTexCoord.zw, mtlData.m_vTangentNormal, lightData.m_vLighting, lightData.m_fSunOcclusion, lightData.m_fSkyOcclusion);
#else
	BasisLightmapNoNormal(Lightmap0Sampler, vtxData.m_vTexCoord.zw, lightData.m_vLighting, lightData.m_fSunOcclusion, lightData.m_fSkyOcclusion);
#endif
#endif
#if !defined(ENABLE_LIGHTMAP) && !(defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING))
	BasisSH(g_SHNode, surfaceData.m_vNormal, lightData.m_vLighting, lightData.m_fSunOcclusion, lightData.m_fSkyOcclusion);
#endif
#endif

#if defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING)
	lightData.m_fSunOcclusion *= vtxData.m_vColour.b;
	lightData.m_fSkyOcclusion *= vtxData.m_vColour.g;
#endif

	lightData.m_vAverage			= lightData.m_vLighting;
	lightData.m_vHighlight			= g_vSunColour.xyz * lightData.m_fSunOcclusion;
	lightData.m_vDirection			= g_vSunDirection.xyz;

	return lightData;
}

#endif // SHADER_SCENE_BASIS_LIGHTING_H
