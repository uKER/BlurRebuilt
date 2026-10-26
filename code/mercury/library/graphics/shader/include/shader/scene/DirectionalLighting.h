#ifndef SHADER_SCENE_DIRECTIONAL_LIGHTING_H
#define SHADER_SCENE_DIRECTIONAL_LIGHTING_H

#include <Shader/Lighting/Lightmap.h>
#include <Shader/Lighting/SphericalHarmonics.h>
#include <Shader/Misc/ColourOp.h>

C_Lighting EvaluateLighting(in C_Material mtlData, in C_Surface surfaceData, in C_VertexOutput vtxData)
{
	C_Lighting lightData = (C_Lighting) 0;
	lightData.m_fSkyOcclusion		= 1.0f;
	lightData.m_fSunOcclusion		= 1.0f;
	lightData.m_vLighting			= 1.0f;

#if !defined(MAYA)
#if defined(ENABLE_LIGHTMAP)
	RGBLightmapNormalisedGamma(Lightmap0Sampler, vtxData.m_vTexCoord.zw, lightData.m_vLighting);
	SunSkyLightmapGamma(Lightmap1Sampler, vtxData.m_vTexCoord.zw, lightData.m_fSunOcclusion, lightData.m_fSkyOcclusion);
	DirectionalLightmap(Lightmap2Sampler, vtxData.m_vTexCoord.zw, lightData.m_vDirection);
#endif
#if !defined(ENABLE_LIGHTMAP) && !(defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING))
	BasisSH(g_SHNode, surfaceData.m_vNormal, lightData.m_vLighting, lightData.m_fSunOcclusion, lightData.m_fSkyOcclusion);
	DirectionalSH(g_SHNode, lightData.m_vDirection);
#endif
#endif

#if defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING)
	lightData.m_vLighting *= vtxData.m_vColour.rgb;
#endif

	real fLerpFactor = saturate(lightData.m_fSunOcclusion * Greyscale(g_vSunColour.xyz));
	lightData.m_vAverage	= lightData.m_vLighting;
	lightData.m_vHighlight	= lerp(lightData.m_vLighting * g_vBakedColour.rgb, g_vSunColour.xyz, fLerpFactor);
	lightData.m_vLighting	= lightData.m_vLighting * saturate(dot(surfaceData.m_vNormal, lightData.m_vDirection));
	lightData.m_vDirection	= normalize(lerp(lightData.m_vDirection, g_vSunDirection.xyz, fLerpFactor));

	return lightData;
}

#endif // SHADER_SCENE_DIRECTIONAL_LIGHTING_H
