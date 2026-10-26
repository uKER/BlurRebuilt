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
#define REPLACE_LIGHTING_HIGHLIGHT
#endif

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

texture2D	g_tTangent	< string Source = "Tangent"; int Default = 0x0000ff00; >;

sampler TangentSampler = 
sampler_state
{
    Texture = <g_tTangent>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

float	g_fSpecularPower		< string Source = "SpecularPower"; string Default = "20.0"; string Type = "Float"; string range = "0.0, 50.0"; > = 20.0f;
float	g_fSpecularIntensity	< string Source = "SpecularIntensity"; string Default = "2.0"; string Type = "Float"; string range = "1.0, 100.0"; > = 2.0f;
float	g_fRootSpecularShift		< string Source = "RootSpecularShift"; string Default = "2.0"; string Type = "Float"; string range = "-1.0, 1.0"; > = 1.0f;


#ifdef REPLACE_LIGHTING_DIFFUSE

#include <Lighting/KeyFillLighting.h>

real EvaluateHairDiffuse(real3 vLight, real3 vTangent, real3 vOutNormal)
{
	real	fTDotL = dot(vTangent, vLight);
	real	fTangentFactor		= sqrt(1.0f - fTDotL * fTDotL);
	real	fNDotL = saturate(dot(vOutNormal, vLight) * 0.75f + 0.25f);
	return fTangentFactor * fNDotL;
}

real3 EvaluateHairKeyFillLighting(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
#if defined(CFG_CONVERTER)
	return real3(0.0f, 0.0f, 0.0f);
#else

	real3 vTangentStrandDir = normalize(tex2D(TangentSampler, vtxData.m_vTexCoord).rgb - 0.5f);
	real3 vVertexNormal = normalize(vtxData.m_vNormal.xyz);
	real3 vWorldStrandDir = EvaluateWorldNormal(vTangentStrandDir, vVertexNormal, surfaceData.m_vTangent, surfaceData.m_vBinormal);
	
	real3	vStrandTemp			= cross(surfaceData.m_vNormal, vWorldStrandDir);
	real3	vStrandDir			= cross(vStrandTemp, surfaceData.m_vNormal);
	
	vStrandDir = normalize(vStrandDir);

	float3 vOutNormal = vVertexNormal;
	float3 vNormal = surfaceData.m_vNormal;	
	
	real3	vLightColour = 0.0h;
	vLightColour += g_vCutSceneLightColour0.rgb * EvaluateHairDiffuse(g_vCutSceneLightDir0.xyz, vStrandDir, vOutNormal);
	vLightColour += g_vCutSceneLightColour1.rgb * EvaluateHairDiffuse(g_vCutSceneLightDir1.xyz, vStrandDir, vOutNormal);
	vLightColour += g_vCutSceneLightColour2.rgb * EvaluateHairDiffuse(g_vCutSceneLightDir2.xyz, vStrandDir, vOutNormal);
	vLightColour += g_vCutSceneLightColour3.rgb * EvaluateHairDiffuse(g_vCutSceneLightDir3.xyz, vStrandDir, vOutNormal);

	/*
	real4	vLightFactors;
	vLightFactors.x	= saturate(dot(g_vCutSceneLightDir0.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour0.w + (1.0f - g_vCutSceneLightColour0.w));
	vLightFactors.y	= saturate(dot(g_vCutSceneLightDir1.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour1.w + (1.0f - g_vCutSceneLightColour1.w));
	vLightFactors.z	= saturate(dot(g_vCutSceneLightDir2.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour2.w + (1.0f - g_vCutSceneLightColour2.w));
	vLightFactors.w	= saturate(dot(g_vCutSceneLightDir3.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour3.w + (1.0f - g_vCutSceneLightColour3.w));

	real3	vLightColour = 0.0h;
	vLightColour += g_vCutSceneLightColour0.rgb * vLightFactors.x;
	vLightColour += g_vCutSceneLightColour1.rgb * vLightFactors.y;
	vLightColour += g_vCutSceneLightColour2.rgb * vLightFactors.z;
	vLightColour += g_vCutSceneLightColour3.rgb * vLightFactors.w;
	*/

	return vLightColour * g_vLightColour.rgb;
#endif
}

real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3	vStaticLighting		= lightData.m_vLighting * g_vLightColour.rgb;
	real3	vDynamicLighting	= EvaluateDynamicLighting(vtxData);	
	real3	vKeyFillLighting	= EvaluateHairKeyFillLighting(mtlData, surfaceData, lightData, vtxData) * g_vLightColour.rgb;
	return (vStaticLighting + vDynamicLighting + vKeyFillLighting) * mtlData.m_vAlbedo.rgb;
}
#endif // REPLACE_LIGHTING_DIFFUSE

#ifdef REPLACE_LIGHTING_HIGHLIGHT
real EvaluateHairSpecular(real3 vLight, real3 vBias, real3 vTangent, real3 vView, real fSpecularPower)
{
	real3	vHalf		= normalize(vLight - vView);
	real	fFactor		= saturate(1.0f - abs(dot(vHalf, vTangent + vBias)));
	return safe_pow(fFactor, fSpecularPower);
}

real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{		
	real3	vHighlight				= 0.0f;
#if defined(ENABLE_SPECULAR)	
	real3 vTangentStrandDir = normalize(tex2D(TangentSampler, vtxData.m_vTexCoord).rgb - 0.5f);
	real3 vVertexNormal = normalize(vtxData.m_vNormal.xyz);
	real3 vWorldStrandDir = EvaluateWorldNormal(vTangentStrandDir, vVertexNormal, surfaceData.m_vTangent, surfaceData.m_vBinormal);
	
	real3	vStrandTemp			= cross(surfaceData.m_vNormal, vWorldStrandDir);
	real3	vStrandDir			= cross(vStrandTemp, surfaceData.m_vNormal);
	
	vStrandDir					= normalize(vStrandDir);
	
	real	fHighlightShift			= mtlData.m_vAlbedo.r;
	real3	vRootColour				= normalize(mtlData.m_vAlbedo);
	
	{		
		real3   vLightDirection = g_vCutSceneLightDir0;
		real3	vHighlightColour		= g_vCutSceneLightColour0;
			
		real	fSpecularBrightnessRoot	= EvaluateHairSpecular(vLightDirection, surfaceData.m_vNormal * g_fRootSpecularShift * fHighlightShift, vStrandDir, surfaceData.m_vView, g_fSpecularPower);
		real	fSpecularBrightnessTip	= EvaluateHairSpecular(vLightDirection, surfaceData.m_vNormal * 0.0f, vStrandDir, surfaceData.m_vView, g_fSpecularPower);
		
		real	fShadowFactor			= saturate(dot(vVertexNormal, vLightDirection));
		
		vHighlight += (fSpecularBrightnessRoot * vRootColour + fSpecularBrightnessTip) * vHighlightColour * fShadowFactor * mtlData.m_fSpecularIntensity * g_fSpecularIntensity;
	}
	
	{		
		real3   vLightDirection = g_vCutSceneLightDir1;
		real3	vHighlightColour		= g_vCutSceneLightColour1;
			
		real	fSpecularBrightnessRoot	= EvaluateHairSpecular(vLightDirection, surfaceData.m_vNormal * g_fRootSpecularShift * fHighlightShift, vStrandDir, surfaceData.m_vView, g_fSpecularPower);
		real	fSpecularBrightnessTip	= EvaluateHairSpecular(vLightDirection, surfaceData.m_vNormal * 0.0f, vStrandDir, surfaceData.m_vView, g_fSpecularPower);
		
		real	fShadowFactor			= saturate(dot(vVertexNormal, vLightDirection));
		
		vHighlight += (fSpecularBrightnessRoot * vRootColour + fSpecularBrightnessTip) * vHighlightColour * fShadowFactor * mtlData.m_fSpecularIntensity * g_fSpecularIntensity;
	}	
#endif	

	return vHighlight;
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
