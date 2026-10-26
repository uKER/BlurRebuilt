#ifndef AMAX_SHADERS_LIGHTING_DEFAULT_H
#define AMAX_SHADERS_LIGHTING_DEFAULT_H

#include <Shader/Lighting/Lightmap.h>
#include <Shader/Lighting/SphericalHarmonics.h>

struct C_Lighting
{
	real3	m_vLighting;
	real	m_fSunOcclusion;
	real	m_fSkyOcclusion;
};

#ifdef CFG_CONVERTER
static const float3 gToolLightDirection = normalize( float3(0.5f, 1.0f, 0.5f));
#endif

#if !defined(REPLACE_LIGHTING)

#if !defined(REPLACE_LIGHTING_STATIC)
#ifndef SHNODE_IN_VERTEX_CONSTANT
C_Lighting EvaluateLighting(in C_Material mtlData, in C_Surface surfaceData, in C_VertexOutput vtxData)
{
	C_Lighting lightData = (C_Lighting) 0;
	lightData.m_vLighting		= 1.0h;
	lightData.m_fSunOcclusion	= 1.0h;
	lightData.m_fSkyOcclusion	= 1.0h;
#if !defined(CFG_CONVERTER)
#if defined(ENABLE_LIGHTMAP)
	SunSkyLightmap(LightmapSampler, vtxData.m_vTexCoord.zw, lightData.m_fSunOcclusion, lightData.m_fSkyOcclusion);
#else
	SunSkySH(g_SHNode, surfaceData.m_vNormal, lightData.m_fSunOcclusion, lightData.m_fSkyOcclusion);
#endif
#else // CFG_CONVERTER
	lightData.m_vLighting *= saturate(dot(surfaceData.m_vNormal, gToolLightDirection) * 0.5h + 0.5h);
#endif // CFG_CONVERTER

#if defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING)
	lightData.m_vLighting *= vtxData.m_vColour.rgb;
#endif

#if !defined(CFG_CONVERTER)
	lightData.m_vLighting *= 0.5h;
	lightData.m_vLighting *= lightData.m_fSunOcclusion + lightData.m_fSkyOcclusion;
#endif

	return lightData;
}
#else
C_Lighting EvaluateLighting(in C_Material mtlData, in C_Surface surfaceData, in C_VertexOutput vtxData)
{
	C_Lighting lightData = (C_Lighting) 0;
	lightData.m_vLighting		= 1.0h;
	lightData.m_fSunOcclusion	= 1.0h;
	lightData.m_fSkyOcclusion	= 1.0h;
#if !defined(CFG_CONVERTER)
#if defined(ENABLE_LIGHTMAP)
	SunSkyLightmap(LightmapSampler, vtxData.m_vTexCoord.zw, lightData.m_fSunOcclusion, lightData.m_fSkyOcclusion);
#elif defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING)
	lightData.m_vLighting *= vtxData.m_vColour.rgb * vtxData.m_vColour.rgb;
#else
	lightData.m_vLighting = vtxData.m_vColour.rgb;
#endif
#endif

#if !defined(CFG_CONVERTER)
	lightData.m_vLighting *= 0.5h;
	lightData.m_vLighting *= lightData.m_fSunOcclusion + lightData.m_fSkyOcclusion;
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
#ifndef CFG_CONVERTER
	real	fMipLevel			= 12.0h * (0.5h - mtlData.m_fHalfSmoothness);
	real3	vReflection			= AdjustCubemapIntensity(texCUBElod(EnvironmentSampler, float4(surfaceData.m_vReflect, fMipLevel)).rgb);
			vReflection			*= mtlData.m_fSpecularIntensity;
	return vReflection;
#else // CFG_CONVERTER
	float	fIntensity = saturate(dot(surfaceData.m_vReflect, gToolLightDirection));
	fIntensity = fIntensity * fIntensity;
	fIntensity = fIntensity * fIntensity;
	return fIntensity * mtlData.m_fSpecularIntensity * 5.0h;
#endif // CFG_CONVERTER
#else
	return 0.0h;
#endif
}
#endif // REPLACE_LIGHTING_REFLECTION

#endif // REPLACE_LIGHTING

#endif // AMAX_SHADERS_LIGHTING_DEFAULT_H
