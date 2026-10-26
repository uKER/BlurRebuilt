#ifndef AMAX_SHADERS_COMMON_SURFACE_H
#define AMAX_SHADERS_COMMON_SURFACE_H

#include <Shader/Lighting/Equations.h>
#include <Common/LodFade.h>

// structures

struct C_Surface
{
#if defined(ENABLE_WORLD)
	real3	m_vPosition;
#endif
#if defined(ENABLE_NORMAL)
	real3	m_vNormal;
#endif
#if defined(ENABLE_VIEW)
	real3	m_vView;
#endif
	real	m_fDistance;
#if defined(ENABLE_TANGENTNORMAL)
	real3	m_vTangent;
	real3	m_vBinormal;
#endif
#if defined(ENABLE_SPECULAR)
	real	m_fNDotV;
	real3	m_vReflect;
	real	m_fFresnel;
#endif
};

// functions

#if !defined(REPLACE_SURFACE)
C_Surface EvaluateSurface(in C_Material mtlData, in C_VertexOutput vtxData)
{
	C_Surface surfaceData	= (C_Surface) 0;
#if defined(ENABLE_WORLD)
	surfaceData.m_vPosition	= vtxData.m_vWorld.xyz;
#endif
#if defined(ENABLE_TANGENTNORMAL)
	real3	vWorldNormal	= normalize(vtxData.m_vNormal.xyz);
	real3	vWorldTangent	= normalize(vtxData.m_vTangent.xyz);
	real3	vWorldBinormal	= normalize(vtxData.m_vBinormal.xyz);
	surfaceData.m_vNormal	= normalize(vWorldNormal * mtlData.m_vTangentNormal.z + vWorldTangent * mtlData.m_vTangentNormal.x + vWorldBinormal * mtlData.m_vTangentNormal.y);
	surfaceData.m_vTangent	= vWorldTangent;
	surfaceData.m_vBinormal	= vWorldBinormal;
#elif defined(ENABLE_NORMAL)
	surfaceData.m_vNormal	= normalize(vtxData.m_vNormal.xyz);
#endif
#if defined(ENABLE_VIEW)
	surfaceData.m_vView		= normalize(vtxData.m_vView.xyz);
#endif
#if !defined(LOD)
	surfaceData.m_fDistance	= vtxData.m_vScreenPos.z;
#endif
#if defined(ENABLE_SPECULAR)
	surfaceData.m_fNDotV	= dot(surfaceData.m_vNormal, surfaceData.m_vView);
	surfaceData.m_vReflect	= surfaceData.m_vView - 2.0h * surfaceData.m_fNDotV * surfaceData.m_vNormal;
	surfaceData.m_fFresnel	= Fresnel(surfaceData.m_fNDotV, mtlData.m_fFresnelMidBrightness) * mtlData.m_fSpecularIntensity;
#endif
	return surfaceData;
}
#endif

#if !defined(REPLACE_SURFACE_PREPASS)
C_Surface EvaluateSurfacePrepass(in C_Material mtlData, in C_VertexPrepassOutput vtxData)
{
	C_Surface surfaceData	= (C_Surface) 0;
#if defined(ENABLE_TANGENTNORMAL)
	real3	vWorldNormal	= normalize(vtxData.m_vNormal);
	real3	vWorldTangent	= normalize(vtxData.m_vTangent);
	real3	vWorldBinormal	= normalize(vtxData.m_vBinormal);
	surfaceData.m_vNormal	= normalize(vWorldNormal * mtlData.m_vTangentNormal.z + vWorldTangent * mtlData.m_vTangentNormal.x + vWorldBinormal * mtlData.m_vTangentNormal.y);
	surfaceData.m_vTangent	= vWorldTangent;
	surfaceData.m_vBinormal	= vWorldBinormal;
#elif defined(ENABLE_NORMAL)
	surfaceData.m_vNormal	= normalize(vtxData.m_vNormal.xyz);
#endif
	return surfaceData;
}
#endif

#if !defined(DISABLE_LODFADE)
float EvaluateLodFade(in C_Surface surfaceData)
{
	real2 r = saturate(g_lodFade.xy * surfaceData.m_fDistance * g_lodDistanceScale.x + g_lodFade.zw);
	return min(r.x, r.y);
}

#else

real EvaluateLodFade(in C_Surface surfaceData)
{
	return 1.0h;
}

#endif

#endif // AMAX_SHADERS_COMMON_SURFACE_H
