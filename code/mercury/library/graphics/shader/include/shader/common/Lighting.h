#ifndef SHADER_LIGHTING_H
#define SHADER_LIGHTING_H

#include <Shader/Common/Fog.h>
#include <Shader/Common/Hdr.h>
#include <Shader/Misc/ColourOp.h>

// structures

struct C_LightingInfo annotation(export)
{
	float4	m_vSunColour;
	float4	m_vSkyColour;
	float4	m_vBakedColour;
	float4	m_vEmissiveColour;
	float4	m_vIntensities;
};

struct C_SHNode
{
	float4	m_vRedCoeffs;
	float4	m_vGreenCoeffs;
	float4	m_vBlueCoeffs;
	float4	m_vChannelCoeffs;
};

struct C_Lighting
{
	real3	m_vLighting;
	real3	m_vHighlight;
	real3	m_vDirection;
	real3	m_vAverage;
	real	m_fSunOcclusion;
	real	m_fSkyOcclusion;
};

// constants

#if !defined(CFG_CONVERTER)
shared C_SHNode			g_SHNode		: register(CCP_LIGHTING_SH_NODE)	< string Source = "<LuxCustom>"; string InstanceMode = "InstanceOnly"; string Default = "0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0"; >;
#else
static C_SHNode			g_SHNode	= { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f };
#endif
shared C_LightingInfo	g_LightingInfo	: register(CCP_LIGHTING_INFO) =
	{
		{ 1.00f,	1.00f,		1.00f,		1.00f },
		{ 0.25f,	0.25f,		0.25f,		1.00f },
		{ 1.00f,	1.00f,		1.00f,		1.00f },
		{ 0.00f,	0.00f,		0.00f,		1.00f },
		{ 1.00f,	10.00f,		1.00f,		1.00f },
	};

shared float4			g_vSunDirection	: register(CCP_LIGHTING_SUN_DIRECTION) = { 0.57735f, 0.57735f, -0.57735f, 0.00f };

#define	g_vSunColour			g_LightingInfo.m_vSunColour
#define	g_vSkyColour			g_LightingInfo.m_vSkyColour
#define	g_vBakedColour			g_LightingInfo.m_vBakedColour
#define	g_vEmissiveColour		g_LightingInfo.m_vEmissiveColour

#define	g_fDiffuseIntensity		g_LightingInfo.m_vIntensities.x
#define	g_fHighlightIntensity	g_LightingInfo.m_vIntensities.y
#define	g_fReflectionIntensity	g_LightingInfo.m_vIntensities.z
#define	g_fGlobalIntensity		g_LightingInfo.m_vIntensities.w

// textures

#if !defined(CFG_CONVERTER)
shared textureCUBE g_tEnvironment < string Source = "<LocalisedCubemap>"; bool Uninitialised = true; int Stream = 1; string InstanceMode = "InstanceOnly"; >;

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

shared texture2D g_tLightmap0 < string Source = "<Lightmap+0>"; bool Uninitialised = true; string UVSet = "luxlightmap"; string InstanceMode = "Instance"; string Default = "White"; int NumMipMaps = 2; int Stream = 1; string MipFilter = "BOX"; LIGHTMAP_DEFINE_STRING >;
shared texture2D g_tLightmap1 < string Source = "<Lightmap+1>"; bool Uninitialised = true; string UVSet = "luxlightmap"; string InstanceMode = "Instance"; string Default = "White"; int NumMipMaps = 2; int Stream = 1; string MipFilter = "BOX"; >;
shared texture2D g_tLightmap2 < string Source = "<Lightmap+2>"; bool Uninitialised = true; string UVSet = "luxlightmap"; string InstanceMode = "Instance"; string Default = "White"; int NumMipMaps = 2; int Stream = 1; string MipFilter = "BOX"; >;

sampler Lightmap0Sampler : register(SSP_SHADER_LIGHTMAP_0) = 
sampler_state
{
    Texture = <g_tLightmap0>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler Lightmap1Sampler : register(SSP_SHADER_LIGHTMAP_1) = 
sampler_state
{
    Texture = <g_tLightmap1>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler Lightmap2Sampler : register(SSP_SHADER_LIGHTMAP_2) = 
sampler_state
{
    Texture = <g_tLightmap2>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

#endif

real3 AdjustCubemapIntensity( in real3 vReflectionSample )
{
	return InverseExposure(vReflectionSample);
}

// functions

#if !defined(REPLACE_LIGHTING_FUNCTIONS)

C_Lighting EvaluateLighting(in C_Material mtlData, in C_Surface surfaceData, in C_VertexOutput vtxData);
real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData);
real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData);
real3 EvaluateReflection(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData);
real3 EvaluateSpecular(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData);
real3 EvaluateColour(in real3 vDiffuse, in real3 vSpecular, in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData);
real4 EvaluateFog(in real4 vColour, in C_Surface surfaceData);
real4 EvaluateFinalColour(in real4 vColour);

// lightmaps

#if defined(BASIS_LIGHTING)
#	include <Shader/Scene/BasisLighting.h>
#elif defined(RGB_LIGHTING)
#	include <Shader/Scene/RGBLighting.h>
#elif defined(DIRECTIONAL_LIGHTING)
#	include <Shader/Scene/DirectionalLighting.h>
#elif defined(SUNSKY_LIGHTING)
#	include <Shader/Scene/SunSkyLighting.h>
#else
#	include <Shader/Scene/DefaultLighting.h>
#endif

// diffuse

#if !defined(REPLACE_LIGHTING_DIFFUSE)
real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3 vDiffuse = 0.0f;
#if !defined(CFG_CONVERTER)
	vDiffuse += lightData.m_vLighting * g_vBakedColour.rgb;
	vDiffuse += g_vSkyColour.rgb * lightData.m_fSkyOcclusion * saturate(surfaceData.m_vNormal.y * 0.5f + 0.5f);
	vDiffuse += g_vSunColour.rgb * saturate(dot(surfaceData.m_vNormal.xyz, g_vSunDirection.xyz)) * lightData.m_fSunOcclusion;
#else
	vDiffuse += float3(1.0f, 1.0f, 1.0f);
#endif
	vDiffuse *= mtlData.m_vAlbedo * g_fDiffuseIntensity;
	return vDiffuse;
}
#endif

// specular

#if !defined(REPLACE_LIGHTING_HIGHLIGHT)
real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3 vHighlight = 0.0f;
#if !defined(CFG_CONVERTER)
#if defined(ENABLE_SPECULAR)
#if defined(BLINN)
	vHighlight += lightData.m_vHighlight * Blinn(lightData.m_vDirection, surfaceData.m_vNormal, surfaceData.m_vView, mtlData.m_fHighlightPower) * mtlData.m_fHighlightIntensity * g_fHighlightIntensity;
#else
	vHighlight += lightData.m_vHighlight * Phong(lightData.m_vDirection, surfaceData.m_vNormal, surfaceData.m_vView, mtlData.m_fHighlightPower) * mtlData.m_fHighlightIntensity * g_fHighlightIntensity;
#endif
#endif
#endif
	return vHighlight;
}
#endif

#if !defined(REPLACE_LIGHTING_REFLECTION)
real3 EvaluateReflection(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3 vReflection = 0.0f;
#if !defined(CFG_CONVERTER)
#if defined(ENABLE_SPECULAR)
	real3	vReflectionSample	= texCUBElod(EnvironmentSampler, float4(surfaceData.m_vReflect, mtlData.m_fReflectionPower)).rgb;
			vReflectionSample	= AdjustCubemapIntensity(vReflectionSample);
	vReflection += vReflectionSample * mtlData.m_fReflectionIntensity * g_fReflectionIntensity;
#endif
#endif
	return vReflection;
}
#endif

#if !defined(REPLACE_LIGHTING_SPECULAR)
real3 EvaluateSpecular(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3 vSpecular = 0.0f;
	vSpecular += EvaluateHighlight(mtlData, surfaceData, lightData, vtxData);
	vSpecular += EvaluateReflection(mtlData, surfaceData, lightData, vtxData);
	return vSpecular;
}
#endif

// fog

#if !defined(REPLACE_FOG)
real4 EvaluateFog(in real4 vColour, in C_Surface surfaceData)
{
#if defined(ENABLE_FOG)
	return EvaluateDepthFog(vColour, surfaceData.m_fDistance);
#endif
	return vColour;
}
#endif

// final

real3 EvaluateColour(in real3 vDiffuse, in real3 vSpecular, in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData)
{
#if defined(ENABLE_SPECULAR)
	real3 vColour = lerp(vDiffuse, vSpecular, surfaceData.m_fFresnel * mtlData.m_fFresnelIntensity);
#else
	real3 vColour = vDiffuse;
#endif
#if defined(ENABLE_EMISSIVE)
	vColour += mtlData.m_vEmissive * g_LightingInfo.m_vEmissiveColour.rgb;
#endif
#if defined(SHOW_DIFFUSE)
	vColour = vDiffuse;
#elif defined(SHOW_SPECULAR) && defined(ENABLE_SPECULAR)
	vColour = vSpecular;
#elif defined(SHOW_EMISSIVE) && defined(ENABLE_EMISSIVE)
	vColour = mtlData.m_vEmissive * g_LightingInfo.m_vEmissiveColour.rgb;
#endif
	vColour *= g_fGlobalIntensity;
	return vColour;
}

real EvaluateAlpha(in C_Material mtlData)
{
#if defined(ADDITIVE)
	return (1.0f / 255.0f) * sign(mtlData.m_fAlpha);
#else
	return mtlData.m_fAlpha;
#endif
}

real4 EvaluateFinalColour(real4 vColour)
{
	return GetExposure(vColour);
}

#endif // REPLACE_LIGHTING_FUNCTIONS

#endif // SHADER_LIGHTING_H
