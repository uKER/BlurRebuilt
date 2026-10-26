#ifndef AMAX_SHADERS_SPECIAL_WATER_H
#define AMAX_SHADERS_SPECIAL_WATER_H

#include <Common/Reflection.h>
#include <Shader/Misc/Resolve.h>

shared texture2D	g_tColourBuffer;

sampler ColourBufferSampler : register(SSP_SHADER_COLOUR_BUFFER) = 
sampler_state
{
	Texture = <g_tColourBuffer>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

shared texture2D	g_tDepthBuffer;

sampler DepthBufferSampler : register(SSP_SHADER_DEPTH_BUFFER)  = 
sampler_state
{
	Texture = <g_tDepthBuffer>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tColourBuffer RGBA8
#pragma texformat g_tDepthBuffer RGBA8
#endif

shared real	g_fWaterSurfaceSmoothness	= 1.0h;
shared real	g_fWaterDistortionScale		= 0.2h;
shared real g_fReflectAmount			= 500.0h;

#if defined(REPLACE_LIGHTING_REFLECTION)
real3 EvaluateReflection(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
#if !defined(CFG_CONVERTER) && !defined(LOD) && !defined(SPLITSCREEN)
#ifdef FAKE_REFLECTION
	float3 normal = surfaceData.m_vNormal.xyz;
	normal.xz *= g_fWaterDistortionScale;

	float3 vReflection = reflect(vtxData.m_vView.xyz, normalize(normal));

	float3 vStartPos = surfaceData.m_vPosition;

	float fHeight = (g_vViewPosition.y - surfaceData.m_vPosition.y);
	float fHeightCompensation = g_fReflectAmount / (fHeight * fHeight);
	float3 vIntersection = vStartPos + vReflection * fHeightCompensation;

	float4 vProjPos = mul( float4x4(g_vViewProjMatrix0, g_vViewProjMatrix1, g_vViewProjMatrix2, g_vViewProjMatrix3), float4( vIntersection, 1.0f ));
	real2 vReflectCoords = (vProjPos.xy / vProjPos.w) * float2(0.5f, -0.5f) + 0.5f;

	float3 vHackReflection = SampleColourBuffer(ColourBufferSampler, vReflectCoords);

	float2 vTemp = saturate( abs(vReflectCoords - 0.5f) * 4.0f - 3.0f);

	float fBlend = 1.0f - max(vTemp.x,vTemp.y);
	fBlend *= saturate( vReflectCoords.y * 10.0f);

	// fade out hack with height
	fBlend *= 1.0f - saturate( (fHeight - 10.0f) * 0.1f );

	return vHackReflection * fBlend;
#else
	// planar reflections
	float2	vScreenCoords;
			vScreenCoords.xy = surfaceData.m_vPosition.xy / surfaceData.m_vPosition.z;
			vScreenCoords.xy += surfaceData.m_vNormal.xz * g_fWaterDistortionScale;
	real3	vPlanar			= InverseExposure(tex2D(ScreenReflectionSampler, vScreenCoords).rgb);

	return vPlanar;
#endif
#else
#if defined(ENABLE_SPECULAR)
	return AdjustCubemapIntensity(texCUBE(EnvironmentSampler, surfaceData.m_vReflect));
#else
	return 0.0h;
#endif
#endif
}
#endif

// some code to do raytraced reflections if you wish

//	float3 vReflection = reflect(vtxData.m_vView.xyz, surfaceData.m_vNormal.xyz);

//	float3 vStartPos = surfaceData.m_vPosition.xyz;

//	float fHeight = (g_vViewPositionPS.y - surfaceData.m_vPosition.y);
//	float fHeightCompensation = 128.0f / (fHeight * fHeight);
//	float3 vIntersection = vStartPos + vReflection * fHeightCompensation;

//	float fProjX = dot(float4(vIntersection, 1.0f), g_vViewProjMatrix0);
//	float fProjY = dot(float4(vIntersection, 1.0f), g_vViewProjMatrix1);
//	float fProjW = dot(float4(vIntersection, 1.0f), g_vViewProjMatrix3);
//	float2 vReflectCoords = (float2(fProjX, fProjY) / fProjW) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

//	float3 vHackReflection = SampleColourBuffer(ColourBufferSampler, vReflectCoords);

//	float2 vTemp = saturate(abs(vReflectCoords - 0.5f) * 4.0f - 3.0f);

//	float fBlend = 1.0f - max(vTemp.x, vTemp.y);
//	fBlend *= saturate(vReflectCoords.y * 10.0f);
//	fBlend *= 1.0f - saturate((fHeight - 10.0f) * 0.1f);

//	return vHackReflection;// * fBlend;
#endif // AMAX_SHADERS_SPECIAL_WATER_H
