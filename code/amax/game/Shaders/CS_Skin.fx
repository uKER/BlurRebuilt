#include <Core.h>
#include <Globals.h>

#define FLIP_NORMAL_MAP_X
#define ENABLE_SHNODE_IN_PIXEL_SHADER

#define ENABLE_ALBEDOMAP
#define ENABLE_BUMPMAP
#define ENABLE_MATERIALMAP
#define ENABLE_SPECULAR
#define NO_LIGHTMAP

#define DISABLE_VERTEXLIGHTING
#define DISABLE_LODFADE

//#define ENABLE_BUMPMAP_OPTIMISATION
//#define ENABLE_MATERIALMAP_OPTIMISATION
//#define ENABLE_LIGHTMAP_OPTIMISATION
//#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#ifndef DAY_LIGHTING
#define REPLACE_LIGHTING_DIFFUSE
#define REPLACE_LIGHTING_SPECULAR
#define REPLACE_LIGHTING_STATIC
#endif

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

float	g_fHighlightPower < string Source = "HighlightPower"; string Default = "16.0"; string Range = "(1.0, 32.0)"; string Type = "Float"; > = 16.0f;
float	g_fReflectionBlur < string Source = "ReflectionBlur"; string Default = "6.0"; string Range = "(1.0, 7.0)"; string Type = "Float"; > = 6.0f;

float3	g_vBackLightColour < string Source = "BackLightColour"; string Default = "1.00, 0.20, 0.15"; >	= float3(1.00f, 0.20f, 0.15f);
float	g_fBacklightFactor < string Source = "BacklightFactor"; string Default = "0.3"; string Type = "Float"; string range = "0.0, 1.0"; > = 0.3f;

float3	g_vWrapSHColour	< string Source = "WrapSHColour"; string Default = "1.00, 0.60, 0.3"; >	= float3(1.00f, 0.60f, 0.3f);
float	g_vWrapSHFactor < string Source = "WrapSHFactor"; string Default = "0.3"; string Type = "Float"; string range = "0.0, 1.0"; > = 0.3f;

float3	g_vWrapDirectionalColour	< string Source = "WrapDirectionalColour"; string Default = "1.00, 0.60, 0.3"; >	= float3(1.00f, 0.60f, 0.3f);
float	g_vWrapDirectionalFactor < string Source = "WrapDirectionalFactor"; string Default = "0.1"; string Type = "Float"; string range = "0.0, 1.0"; > = 0.1f;

#ifdef REPLACE_LIGHTING_STATIC


void BiasedRGBSH(in C_SHNode shNode, in real3 vWorldNormal, in real3 bias, inout real3 vColour )
{
	vColour.r *= dot(float4(vWorldNormal * (1.0f - bias.r), 1.0f + bias.r), shNode.m_vRedCoeffs);
	vColour.g *= dot(float4(vWorldNormal * (1.0f - bias.g), 1.0f + bias.g), shNode.m_vGreenCoeffs);
	vColour.b *= dot(float4(vWorldNormal * (1.0f - bias.b), 1.0f + bias.b), shNode.m_vBlueCoeffs);
}


C_Lighting EvaluateLighting(in C_Material mtlData, in C_Surface surfaceData, in C_VertexOutput vtxData)
{
	C_Lighting lightData = (C_Lighting) 0;
	lightData.m_vLighting	= 1.0f;

#if !defined(CFG_CONVERTER)
#if defined(ENABLE_LIGHTMAP)
	RGBLightmapNormalisedGamma(LightmapSampler, vtxData.m_vTexCoord.zw, lightData.m_vLighting);
#endif
	BiasedRGBSH(g_SHNode, surfaceData.m_vNormal, g_vWrapSHColour * g_vWrapSHFactor, lightData.m_vLighting);
#endif

#if defined(ENABLE_VERTEXCOLOURS) && defined(ENABLE_VERTEXLIGHTING)
	lightData.m_vLighting *= vtxData.m_vColour.rgb*vtxData.m_vColour.rgb;
#endif


	return lightData;
}
#endif

#ifdef REPLACE_LIGHTING_DIFFUSE
#include <Lighting/KeyFillLighting.h>

real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3	vStaticLighting		= (lightData.m_vLighting) * g_vLightColour.rgb;
	real3	vDynamicLighting	= EvaluateDynamicLighting(vtxData);	
	real3	vKeyFillLighting	= EvaluateKeyFillLighting(mtlData, surfaceData, lightData, vtxData, g_vWrapDirectionalColour * g_vWrapDirectionalFactor) * g_vLightColour.rgb;
	
	real3	vDiffuse			= vStaticLighting + vDynamicLighting + vKeyFillLighting;
	
	#if !defined(CFG_CONVERTER)
	real3 avgLight = real3(g_SHNode.m_vRedCoeffs.w, g_SHNode.m_vGreenCoeffs.w, g_SHNode.m_vBlueCoeffs.w) * g_vLightColour.rgb;
	float	fNDotV			= 1.0f - saturate(-surfaceData.m_fNDotV);
	float   fBacklight		= fNDotV * g_fBacklightFactor;
	real3	vSkinLighting	= avgLight * (fBacklight * g_vBackLightColour);
	vDiffuse += vSkinLighting;
	#endif
	
	return vDiffuse * mtlData.m_vAlbedo.rgb;
}
#endif // REPLACE_LIGHTING_DIFFUSE

#ifdef REPLACE_LIGHTING_SPECULAR

real3 EvaluateSpecular(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
#if defined(ENABLE_SPECULAR)
	real3	vHighlight0			= Phong(g_vCutSceneLightDir0.xyz, surfaceData.m_vNormal, surfaceData.m_vView, g_fHighlightPower);
			vHighlight0			*= g_vCutSceneLightColour0.rgb;
	real3	vHighlight1			= Phong(g_vCutSceneLightDir1.xyz, surfaceData.m_vNormal, surfaceData.m_vView, g_fHighlightPower);
			vHighlight1			*= g_vCutSceneLightColour1.rgb;
	real3	vHighlight			= (vHighlight0 + vHighlight1) * mtlData.m_fSpecularIntensity * (2.0f + g_fHighlightPower) * 0.5f;
	#ifdef CFG_CONVERTER
	real3	vReflection			= 0.0f;
	#else
	real3	vReflection			= AdjustCubemapIntensity(texCUBEbias(EnvironmentSampler, float4(surfaceData.m_vReflect, g_fReflectionBlur)).rgb);
	#endif
	return vHighlight + vReflection;
#else
	return	0.0f;
#endif
}

#endif

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
#endif

#if PLATFORM_PS3

technique MaterialAlphaLuminance < string Define = "ALPHA_LUMINANCE"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique MaterialAlphaLuminanceLights < string Define = "ALPHA_LUMINANCE; ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#endif

#if !defined(CFG_CONVERTER)
technique MaterialPrepass < string Define = "ENABLE_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 PrepassVS();
		PixelShader = compile ps_3_0 PrepassPS();
	}
}
#endif

#if !defined(CFG_CONVERTER)
technique MaterialPrepassSolid < string Define = "OPAQUE_PASS; ENABLE_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 PrepassVS();
		PixelShader = compile ps_3_0 PrepassPS();
	}
}
#endif

