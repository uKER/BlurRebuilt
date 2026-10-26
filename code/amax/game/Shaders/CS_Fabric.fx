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
#endif

#define ENABLE_DETAIL_NORMAL

#ifdef ENABLE_DETAIL_NORMAL
#define REPLACE_MATERIAL
#define REPLACE_MATERIAL_PREPASS
#endif

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// we want the normal scale to appear next to the normal parameter in the HLSL window
float		g_fNormalScale			< string Source = "NormalScale"; string Default = "1"; string Type = "Float"; string range = "0.0, 1.0"; >		= 1.0f;
texture2D	g_tBlendNormal			< string Source = "BlendNormal"; string uScale = "UVScale"; string vScale = "UVScale"; string Default = "Normal"; bool invertNormalMapX = true; > ;
float		g_fBlendNormalScale		< string Source = "BlendNormalScale"; string Default = "0.5"; string Type = "Float"; string range = "0.0, 1.0"; >		= 0.5f;

float2		g_vTextureScale			< string Source = "UVScale"; string Default = "30, 30"; string Type = "Float2"; >		= float2(30.0f, 30.0f);
float2		g_vTextureOffset		< string Source = "UVOffset"; string Default = "0, 0"; string Type = "Float2"; >	= float2(0.0f, 0.0f);

float		g_fMicroTextureShade	< string Source = "MicroTextureShade"; string Default = "0"; string Type = "Float"; string range = "0.0, 1.0"; >		= 0.0f;

float		g_fSheen			< string Source = "Sheen"; string Default = "0.25"; string Type = "Float"; string range = "0.0, 1.0"; >		= 0.25f;

sampler BlendNormalSampler = 
sampler_state
{
    Texture = <g_tBlendNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

real3 AdjustAlbedoForFabricSheen( real3 vAlbedo, float fNDotV )
{
	real fSheenFalloff = 1.0f - saturate(-fNDotV);
	fSheenFalloff = fSheenFalloff * fSheenFalloff;
//	fSheenFalloff = fSheenFalloff * fSheenFalloff;
	return lerp(vAlbedo, float3(1.0f, 1.0f, 1.0f), fSheenFalloff * g_fSheen);
}

// we are adding heightfield derivatives so we can't do standard vector addition
// see "Care and Feeding of Normal Vectors" http://www.coretechniques.info/	
float3 AddNormalMapValues(float3 normalA, float3 normalB, float alpha, float beta)
{	
	float3 result;		
	result.xy = alpha * normalA.xy * normalB.z + beta * normalB.xy * normalA.z;
	result.z = normalA.z * normalB.z;
	return normalize(result);
}

#ifdef REPLACE_LIGHTING_DIFFUSE

#include <Lighting/KeyFillLighting.h>

real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	// we are adjusting the albedo here as we only have fNdotV at this point in the shader

	mtlData.m_vAlbedo.rgb = AdjustAlbedoForFabricSheen( mtlData.m_vAlbedo.rgb, surfaceData.m_fNDotV );

	real3	vStaticLighting		= lightData.m_vLighting * g_vLightColour.rgb;
	real3	vDynamicLighting	= EvaluateDynamicLighting(vtxData);	
	real3	vKeyFillLighting	= EvaluateKeyFillLighting(mtlData, surfaceData, lightData, vtxData) * g_vLightColour.rgb;
	return (vStaticLighting + vDynamicLighting + vKeyFillLighting) * mtlData.m_vAlbedo.rgb;
}
#endif // REPLACE_LIGHTING_DIFFUSE

#if defined(REPLACE_MATERIAL)
C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);
	//mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);

	#ifdef ENABLE_DETAIL_NORMAL
	real4	vDetailNormalSample		= tex2D(BlendNormalSampler, vtxData.m_vTexCoord.xy * g_vTextureScale + g_vTextureOffset);
	real3	vDetailNormal			= normalize(vDetailNormalSample.xyz - 0.5f);
#if defined(ENABLE_BUMPMAP)
	real3	vTangentNormal			= EvaluateTangentNormal(NormalSampler, vtxData.m_vTexCoord.xy);
		
	mtlData.m_vTangentNormal = AddNormalMapValues(vTangentNormal.xyz, vDetailNormal.xyz, g_fNormalScale, g_fBlendNormalScale);
	
#else
	mtlData.m_vTangentNormal		= vDetailNormal.xyz;
#endif
	mtlData.m_vAlbedo				= mtlData.m_vAlbedo * lerp(1.0f, vDetailNormalSample.a * 2.0f, g_fMicroTextureShade);
	#endif // ENABLE_DETAIL_NORMAL

	return mtlData;
}
#endif

#if defined(REPLACE_MATERIAL_PREPASS)
C_Material EvaluateMaterialPrepass(in C_VertexPrepassOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	//mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);

	
	#ifdef ENABLE_DETAIL_NORMAL	
	real4	vDetailNormalSample		= tex2D(BlendNormalSampler, vtxData.m_vTexCoord.xy * g_vTextureScale + g_vTextureOffset);
	real3	vDetailNormal			= normalize(vDetailNormalSample.xyz - 0.5f);
#if defined(ENABLE_BUMPMAP)
	real3	vTangentNormal			= EvaluateTangentNormal(NormalSampler, vtxData.m_vTexCoord.xy);
		
	mtlData.m_vTangentNormal = AddNormalMapValues(vTangentNormal.xyz, vDetailNormal.xyz, g_fNormalScale, g_fBlendNormalScale);
	
#else
	mtlData.m_vTangentNormal		= vDetailNormal.xyz;
#endif
	mtlData.m_vAlbedo				= mtlData.m_vAlbedo * lerp(1.0f, vDetailNormalSample.a * 2.0f, g_fMicroTextureShade);
	#endif // ENABLE_DETAIL_NORMAL	

	return mtlData;
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

