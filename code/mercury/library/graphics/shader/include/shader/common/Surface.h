#ifndef SHADER_SURFACE_H
#define SHADER_SURFACE_H

#include <Shader/Lighting/Equations.h>

// structures

struct C_Surface
{
	real3	m_vNormal;
#if defined(ENABLE_VIEWVECTOR)
	real3	m_vView;
#endif
#if defined(ENABLE_TANGENTNORMAL)
	real3	m_vTangent;
	real3	m_vBinormal;
#endif
#if defined(ENABLE_WORLDPOS)
	float3	m_vPosition;
#endif
#if defined(ENABLE_FOG)
	float	m_fDistance;
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
#if defined(ENABLE_TANGENTNORMAL)
	real3	vWorldNormal	= normalize(vtxData.m_vNormal.xyz);
	real3	vWorldTangent	= normalize(vtxData.m_vTangent.xyz);
	real3	vWorldBinormal	= normalize(vtxData.m_vBinormal.xyz);
	surfaceData.m_vNormal	= EvaluateWorldNormal(mtlData.m_vTangentNormal, vWorldNormal, vWorldTangent, vWorldBinormal);
	surfaceData.m_vTangent	= vWorldTangent;
	surfaceData.m_vBinormal	= vWorldBinormal;
#else
	surfaceData.m_vNormal	= normalize(vtxData.m_vNormal.xyz);
#endif
#if defined(ENABLE_WORLDPOS)
	surfaceData.m_vPosition	= vtxData.m_vWorldPos.xyz;
#endif
#if defined(ENABLE_FOG)
	surfaceData.m_fDistance	= vtxData.m_vProjPos.w;
#endif
#if defined(ENABLE_VIEWVECTOR)
	surfaceData.m_vView		= normalize(vtxData.m_vView.xyz);
#endif
#if defined(ENABLE_SPECULAR)
	surfaceData.m_fNDotV	= dot(surfaceData.m_vNormal, surfaceData.m_vView);
	surfaceData.m_vReflect	= surfaceData.m_vView - 2.0f * surfaceData.m_fNDotV * surfaceData.m_vNormal;
	surfaceData.m_fFresnel	= Fresnel(surfaceData.m_fNDotV, mtlData.m_fFresnelMidBrightness);
#endif
	return surfaceData;
}
#endif // REPLACE_SURFACE

#if !defined(DISABLE_LODFADE)
shared float4 g_lodFade : register(CCP_LOD_FADE) < string Source = "<LodFade>"; bool Uninitialised = true; string InstanceMode = "ResetMtl"; >;

float EvaluateLodFade(in C_Surface surfaceData)
{
	float2 r = saturate(g_lodFade.xy * surfaceData.m_fDistance + g_lodFade.zw);
	return min(r.x, r.y);
}

float EvaluateLodFadeNoSaturate(in float fPositionW)
{
	float2 r = g_lodFade.xy * fPositionW + g_lodFade.zw;
	return min(r.x, r.y);
}
#else

float EvaluateLodFade(in C_Surface surfaceData) {return 1.f;}

#endif

#endif // SHADER_SURFACE_H
