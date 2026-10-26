#ifndef AMAX_SHADERS_LIGHTING_DAY_H
#define AMAX_SHADERS_LIGHTING_DAY_H

#include <Shader/Lighting/Lightmap.h>
#include <Shader/Lighting/SphericalHarmonics.h>

struct C_Lighting
{
	real3	m_vLighting;
};

#if !defined(REPLACE_LIGHTING)

#if !defined(REPLACE_LIGHTING_STATIC)
#ifndef SHNODE_IN_VERTEX_CONSTANT
C_Lighting EvaluateLighting(in C_Material mtlData, in C_Surface surfaceData, in C_VertexOutput vtxData)
{
	C_Lighting lightData = (C_Lighting) 0;
	lightData.m_vLighting	= 1.0h;

#if !defined(CFG_CONVERTER)
#if defined(ENABLE_LIGHTMAP)
	RGBLightmapNormalisedGamma(LightmapSampler, vtxData.m_vTexCoord.zw, lightData.m_vLighting);
#else
	RGBSH(g_SHNode, surfaceData.m_vNormal, lightData.m_vLighting);
#endif
#endif

#if defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING)
	lightData.m_vLighting *= vtxData.m_vColour.rgb * vtxData.m_vColour.rgb;
#endif

	return lightData;
}
#else
C_Lighting EvaluateLighting(in C_Material mtlData, in C_Surface surfaceData, in C_VertexOutput vtxData)
{
	C_Lighting lightData = (C_Lighting) 0;
	lightData.m_vLighting	= 1.0h;

#if !defined(CFG_CONVERTER)
#if defined(ENABLE_LIGHTMAP)
	RGBLightmapNormalisedGamma(LightmapSampler, vtxData.m_vTexCoord.zw, lightData.m_vLighting);
#elif defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING)
	lightData.m_vLighting *= vtxData.m_vColour.rgb * vtxData.m_vColour.rgb;
#else
	lightData.m_vLighting = vtxData.m_vColour.rgb;
#endif
#endif

	return lightData;
}
#endif
#endif // REPLACE_LIGHTING_STATIC

#if !defined(REPLACE_LIGHTING_DIFFUSE)
real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3	vStaticLighting		= lightData.m_vLighting;
	real3	vDynamicLighting	= EvaluateDynamicLighting(vtxData);
	return (vStaticLighting + vDynamicLighting) * mtlData.m_vAlbedo.rgb;
}
#endif // REPLACE_LIGHTING_DIFFUSE

#if !defined(REPLACE_LIGHTING_HIGHLIGHT)
real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	return 0.0h;
}
#endif // REPLACE_LIGHTING_HIGHLIGHT

#if !defined(REPLACE_LIGHTING_REFLECTION)
real3 EvaluateReflection(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
#if defined(ENABLE_SPECULAR)
	real	fMipLevel			= 12.0h * (0.5h - mtlData.m_fHalfSmoothness);
	real3	vReflection			= AdjustCubemapIntensity(texCUBEbias(EnvironmentSampler, float4(surfaceData.m_vReflect, fMipLevel)).rgb);
	return	vReflection;
#else
	return	0.0h;
#endif
}
#endif // REPLACE_LIGHTING_REFLECTION

#endif // REPLACE_LIGHTING

#endif // AMAX_SHADERS_LIGHTING_DAY_H
