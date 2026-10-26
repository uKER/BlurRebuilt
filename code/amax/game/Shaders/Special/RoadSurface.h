#ifndef AMAX_SHADERS_SPECIAL_ROAD_SURFACE_H
#define AMAX_SHADERS_SPECIAL_ROAD_SURFACE_H

#include <Common/Reflection.h>

real	g_fRoadReflectionScale	< string Source = "ReflectionScale"; string Default = "0.5"; string Type = "Float"; >	= 0.5f;

shared real	g_fRoadReflectionBrightness : register(CCV_SHADER_USER_4)	= 0.0h;
shared real	g_fRoadReflectionContrast : register(CCV_SHADER_USER_5)	= 1.0h;
shared real	g_fRoadSurfaceSmoothness : register(CCV_SHADER_USER_6)	= 1.0h;
shared real	g_fRoadDistortionScale : register(CCV_SHADER_USER_7)		= 0.5h;
shared real	g_fRoadDistortionOffset : register(CCV_SHADER_USER_8)		= 0.2h;

real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
#if defined(DAY_LIGHTING) && defined(ENABLE_SPECULAR)
	real	fNDotL			= saturate(dot(surfaceData.m_vNormal.xyz, g_vSunDirection.xyz));
	real	fRDotL			= saturate(dot(surfaceData.m_vNormal.xyz, g_vSunDirection.xyz));
	real3	vHighlight		= fNDotL * fRDotL * fRDotL;
			vHighlight		*= g_vSunColour.rgb * lightData.m_fSunOcclusion;
			vHighlight		*= mtlData.m_fSpecularIntensity * (1.0h + 6.0h * mtlData.m_fHalfSmoothness);
	return	vHighlight;
#else
	return	0.0h;
#endif
}

#ifdef CFG_DEBUG
	float g_fRoadDistortionNormal < string DebugVariable = "DistortionNormal"; string Default = "0.03"; string Type = "Float"; > = 0.03f;
#else
	const real g_fRoadDistortionNormal = 0.03h;
#endif

real3 EvaluateReflection(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
#if !defined(CFG_CONVERTER) && !defined(LOD) && !defined(SPLITSCREEN)
	float2	vScreenCoords;
			vScreenCoords.xy = surfaceData.m_vPosition.xy / surfaceData.m_vPosition.z;
			vScreenCoords.xy += surfaceData.m_vNormal.xz * g_fRoadDistortionNormal * g_fRoadSurfaceSmoothness;
			vScreenCoords.x	+= min(mtlData.m_fSpecularIntensity - g_fRoadDistortionOffset, g_fRoadDistortionOffset) * g_fRoadDistortionScale;
	real3	vPlanar			= InverseExposure(tex2D(BlurScreenReflectionSampler, vScreenCoords).rgb);

	return vPlanar * g_fRoadReflectionContrast;
#else
	#if defined(ENABLE_SPECULAR)
		return AdjustCubemapIntensity(texCUBElod(EnvironmentSampler, float4(surfaceData.m_vReflect, 9.0h - 6.0h * g_fRoadSurfaceSmoothness)));
	#else
		return 0.0h;
	#endif
#endif
}

#if !defined(NO_WORLDNORMAL)
texture2D	g_tWorldNormal	< string Source = "WorldNormal"; string Default = "Normal"; string Define = "NW"; >;

sampler WorldNormalSampler : register(SSP_SHADER_NORMAL) = 
sampler_state
{
    Texture = <g_tWorldNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tWorldNormal COMPRESSED_RGBA_S3TC_DXT1
#endif

#ifdef ENABLE_TANGENTNORMAL2D
real3 TangentToWorld(in C_VertexPrepassOutput vtxData, real3 vTS)
{
	real4 vTemp = vTS.xxzz * vtxData.m_vTangentBinormal2d;
	real2 vWS = vTemp.xy + vTemp.zw;
	return (real3(vWS.x, vTS.y, vWS.y));
}
#else
real3 TangentToWorld(in C_VertexPrepassOutput vtxData, real3 vTS)
{
	return vTS;
}
#endif

#if !defined(NO_WORLDNORMAL)
real3 EvaluateWorldNormal(in C_VertexPrepassOutput vtxData, real2 vTexCoord)
{
	real3	vTangentNormal	= h4tex2D(WorldNormalSampler, vTexCoord.xy).xzy * 2.0h - 1.0h;
	real3	vWorldNormal	= TangentToWorld(vtxData, vTangentNormal);
#if defined(BLUR_DX11)
	// [Blur2PC][Blur1PCDX11][retail-recovered] PC prepass shader 0x10de23c0
	// (race trace draw 7047278) uses mov_sat on this blend interpolant.
	// Its w component also carries clip w for the separate depth output.
			vWorldNormal	= normalize(lerp(real3(0.0h, 1.0h, 0.0h), vWorldNormal, saturate(vtxData.m_vTexCoord.w)));
#else
			vWorldNormal	= normalize(lerp(real3(0.0h, 1.0h, 0.0h), vWorldNormal, vtxData.m_vTexCoord.w));
#endif
	return vWorldNormal;
}
#endif

#endif // AMAX_SHADERS_SPECIAL_ROAD_SURFACE_H
