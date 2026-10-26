#ifndef AMAX_SHADERS_COMMON_LIGHTING_H
#define AMAX_SHADERS_COMMON_LIGHTING_H

#include <Shader/Common/Fog.h>
#include <Shader/Common/Hdr.h>
#include <Common/LodFade.h>

// structures

struct C_LightingInfo annotation(export)
{
	real4	m_vSunColour;
	real4	m_vSkyColour;
	real4	m_vLightColour;
	real4	m_vEmissiveColour;
	real4	m_vIntensities;
};

struct C_SHNode
{
	real4	m_vRedCoeffs;
	real4	m_vGreenCoeffs;
	real4	m_vBlueCoeffs;
#ifndef SH_NO_CHANNEL_COEFFS
	real4	m_vChannelCoeffs;
#endif
};

// constants

#ifdef SHNODE_IN_VERTEX_CONSTANT
#if !defined(CFG_CONVERTER)
shared C_SHNode			g_SHNode		: register(CCV_LIGHTING_SH_NODE)	< string Source = "<LuxCustom>"; string InstanceMode = "InstanceOnly"; string Default = "0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1"; >;
#else
static C_SHNode			g_SHNode	= { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.f, 0.f, 0.f, 0.f };
#endif
#else
#if !defined(CFG_CONVERTER)
shared C_SHNode			g_SHNode		: register(CCP_LIGHTING_SH_NODE)	< string Source = "<LuxCustom>"; string InstanceMode = "InstanceOnly"; string Default = "0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1"; >;
#else
static C_SHNode			g_SHNode	= { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f };
#endif
#endif

shared C_LightingInfo	g_LightingInfo	: register(CCP_LIGHTING_INFO) =
	{
		{ 4.00h,	4.00h,		4.00h,		1.00h },
		{ 1.00h,	1.00h,		1.00h,		1.00h },
		{ 1.00h,	1.00h,		1.00h,		1.00h },
		{ 1.00h,	1.00h,		1.00h,		1.00h },
		{ 1.00h,	1.00h,		1.00h,		1.00h },
	};

shared real4		g_vSunDirection	: register(CCP_LIGHTING_SUN_DIRECTION) = { 0.57735h, 0.57735h, -0.57735h, 0.00h };

shared real			g_fDynamicScale	: register(CCP_LIGHTING_DYNAMIC_SCALE) = 1.0f;

#define	g_vSunColour			g_LightingInfo.m_vSunColour
#define	g_vSkyColour			g_LightingInfo.m_vSkyColour
#define	g_vLightColour			g_LightingInfo.m_vLightColour
#define	g_vEmissiveColour		g_LightingInfo.m_vEmissiveColour

// textures

#if !defined(CFG_CONVERTER)
shared textureCUBE g_tEnvironment < string Source = "<LocalisedCubemap>"; string InstanceMode = "InstanceOnly"; >;

sampler EnvironmentSampler : register(SSP_SHADER_ENVIRONMENTMAP) = 
sampler_state
{
    Texture = <g_tEnvironment>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};
#endif

#if !defined(CFG_CONVERTER)
shared texture2D g_tLightmap < string Source = "<Lightmap>"; string UVSet = "luxlightmap"; string InstanceMode = "Instance"; string Default = "White"; int NumMipMaps = 2; string MipFilter = "BOX"; int Stream = 1; LIGHTMAP_DEFINE_STRING >;

sampler LightmapSampler : register(SSP_SHADER_LIGHTMAP) = 
sampler_state
{
    Texture = <g_tLightmap>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if !defined(CFG_CONVERTER)
shared texture2D g_tDynamicLights < string Default = "Black"; >;

sampler DynamicLightsSampler : register(SSP_SHADER_DYNAMICLIGHTS) = 
sampler_state
{
    Texture = <g_tDynamicLights>;
    MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};
#endif

#if defined(PLATFORM_PS3) && (__PIXEL__)
#pragma texformat g_tEnvironment COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_tLightmap COMPRESSED_RGBA_S3TC_DXT5
#pragma texformat g_tDynamicLights RGBA8
#endif

// functions

#if defined(REDUCE_DYNAMIC_LIGHTING)
real g_fDynamicLightFactor : register(CCP_LIGHTING_FACTOR) < string Source = "DynamicLightFactor"; string Type = "Float"; string Default = "0.5"; string Range = "(0, 1)"; > = 0.5f;
#endif

real3 EvaluateDynamicLighting(in C_VertexOutput vtxData)
{
#if defined(ENABLE_LIGHTING_PREPASS)
	float2	vScreenCoord	= vtxData.m_vScreenPos.xy / vtxData.m_vScreenPos.z;
#if !defined(PLATFORM_PS3)
			vScreenCoord	= vScreenCoord + g_vHalfPixelOffset.xy;
#endif
	real3	vColour			= h4tex2D(DynamicLightsSampler, vScreenCoord).rgb;
#if defined(REDUCE_DYNAMIC_LIGHTING)
			vColour			= vColour * g_fDynamicLightFactor;
#endif
	return vColour;
#else
	return 0.0h;
#endif
}

real3 AdjustCubemapIntensity(in real3 vReflectionSample)
{
	return vReflectionSample * vReflectionSample;
}

// lighting

#if defined(NIGHT_LIGHTING)
#include <Lighting/NightLighting.h>
#elif defined(DAY_LIGHTING)
#include <Lighting/DayLighting.h>
#else
#include <Lighting/DefaultLighting.h>
#endif

real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData);
real3 EvaluateReflection(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData);

#if !defined(REPLACE_LIGHTING)

#if !defined(REPLACE_LIGHTING_SPECULAR)
real3 EvaluateSpecular(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3 vSpecular = 0.0h;
	vSpecular += EvaluateHighlight(mtlData, surfaceData, lightData, vtxData);
	vSpecular += EvaluateReflection(mtlData, surfaceData, lightData, vtxData);
	return vSpecular;
}
#endif // REPLACE_LIGHTING_SPECULAR

// final

real3 EvaluateColour(in real3 vDiffuse, in real3 vSpecular, in C_Material mtlData, in C_Surface surfaceData)
{
#if defined(ENABLE_SPECULAR)
	real3 vColour = 0.0h;
	vColour += vDiffuse * (1.0h - surfaceData.m_fFresnel * mtlData.m_fFresnelMidBrightness);
	vColour += vSpecular * surfaceData.m_fFresnel;
#else
	real3 vColour = vDiffuse;
#endif
#if defined(ENABLE_EMISSIVE)
	vColour += mtlData.m_vEmissive * g_vEmissiveColour.rgb;
#endif
#if defined(SHOW_DIFFUSE)
	vColour = vDiffuse;
#elif defined(SHOW_SPECULAR) && defined(ENABLE_SPECULAR)
	vColour = vSpecular;
#elif defined(SHOW_EMISSIVE) && defined(ENABLE_EMISSIVE)
	vColour = mtlData.m_vEmissive * g_vEmissiveColour.rgb;
#endif
	return vColour;
}

real EvaluateAlpha(in C_Material mtlData)
{
#if defined(ADDITIVE)
	return (1.0h / 255.0h) * sign(mtlData.m_fAlpha);
#else
	return mtlData.m_fAlpha;
#endif
}

real4 EvaluateFinalColour(real4 vColour)
{
	return GetExposure(vColour);
}

#if !defined(DISABLE_LODFADE)
real EvaluateLodFadeNoSaturate(in float fPositionW)
{
	real2 r = g_lodFade.xy * fPositionW * g_lodDistanceScale.x + g_lodFade.zw;
	return min(r.x, r.y);
}
#endif

#if defined(SHNODE_IN_VERTEX_CONSTANT)
C_VertexOutput EvaluateColour(in C_VertexOutput output, in C_VertexInput input, in C_VertexData vtxData)
{
#if defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_LIGHTMAP)
	// default version of the shader used to detect permutations
	output.m_vColour = input.m_vColour.rgba;
#elif defined(ENABLE_LIGHTMAP)
	// lightmap
	// do nothing
#elif defined(ENABLE_VERTEXCOLOURS)
	// vertex colours
	output.m_vColour = input.m_vColour.rgba;
#else
	// SH lighting
	real3 vColour = 1.0h;
	RGBSH(g_SHNode, vtxData.m_vNormal.xyz, vColour);
	output.m_vColour = real4(vColour, 1.0h);
#endif

#if !defined(DISABLE_LODFADE)
	output.m_vNormal.a = EvaluateLodFadeNoSaturate(output.m_vPosition.w);
#endif

	return output;
}
#endif

#endif // REPLACE_LIGHTING

#endif // AMAX_SHADERS_COMMON_LIGHTING_H
