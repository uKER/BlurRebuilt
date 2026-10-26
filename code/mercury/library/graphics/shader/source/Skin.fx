#include <Core.h>
#include <Globals.h>

#define REPLACE_LIGHTING_DIFFUSE
#define REPLACE_LIGHTING_HIGHLIGHT
#define REPLACE_MATERIAL

#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP
#define ENABLE_WORLDPOS

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// replace lighting diffuse

texture2D	g_tSubsurface	< string Source = "Subsurface"; string Default = "White"; >;

sampler SubsurfaceSampler = 
sampler_state
{
    Texture = <g_tSubsurface>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

texture2D	g_tScatter		< string Source = "Scatter"; string Default = "Black"; >;

sampler ScatterSampler = 
sampler_state
{
    Texture = <g_tScatter>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

float3	g_vSkinDiffuseWrap		< string Source = "DiffuseWrap"; string Default = "0.50, 0.52, 0.55"; string Type = "Float3"; >			= float3(0.50f, 0.52f, 0.55f);
float3	g_vSkinDiffuseColour	< string Source = "DiffuseColour"; string Default = "0.50, 0.52, 0.55"; string Type = "Float3"; >		= float3(0.50f, 0.52f, 0.55f);
float	g_fSkinDiffuseIntensity	< string Source = "DiffuseIntensity"; string Default = "0.50"; string Type = "Float"; >					= 0.50f;
float3	g_vSkinCentre			< string Source = "SkinCentre"; string Default = "0.00, 0.90, 0.00"; string Type = "Float3"; >			= float3(0.00f, 0.90f, 0.00f);
float	g_fBackScatterRadius	< string Source = "BackScatterRadius"; string Default = "0.5"; string Type = "Float"; >					= 0.5f;
float	g_fBackScatterDepth		< string Source = "BackScatterDepth"; string Default = "0.0"; string Type = "Float"; >					= 0.0f;
float3	g_vBackScatterColour	< string Source = "BackScatterColour"; string Default = "1.00, 0.20, 0.15"; string Type = "Float3"; >	= float3(1.00f, 0.20f, 0.15f);
float	g_fBackScatterIntensity	< string Source = "BackScatterIntensity"; string Default = "8.00"; string Type = "Float"; >				= 8.00f;

real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3	vDiffuse		= 0.0f;
#if !defined(CFG_CONVERTER)
	real	fNDotL			= dot(surfaceData.m_vNormal, g_vSunDirection.xyz);
	
	vDiffuse += lightData.m_vLighting * g_vBakedColour.rgb;
	vDiffuse += g_vSkyColour.rgb * lightData.m_fSkyOcclusion;
	vDiffuse += g_vSunColour.rgb * saturate(fNDotL * g_vSkinDiffuseWrap + (1.0f - g_vSkinDiffuseWrap));
	
	float	fLightDistance	= dot(surfaceData.m_vPosition - g_vSkinCentre, lightData.m_vDirection.xyz);
	float	fViewDistance	= dot(surfaceData.m_vPosition - g_vSkinCentre, surfaceData.m_vView.xyz);
	float	fLightFactor	= smoothstep(-g_fBackScatterRadius - g_fBackScatterDepth, g_fBackScatterRadius - g_fBackScatterDepth, fLightDistance);
	float	fViewFactor		= smoothstep(-g_fBackScatterRadius - g_fBackScatterDepth, g_fBackScatterRadius - g_fBackScatterDepth, fViewDistance);

	float	fLDotV			= saturate(dot(lightData.m_vDirection.xyz, surfaceData.m_vView.xyz) * 0.5f + 0.5f);
	real3	vBackScatter	= fLightFactor * fViewFactor * fLDotV * g_vSunColour.rgb;
	
	real3	vSubsurface		= tex2D(SubsurfaceSampler, vtxData.m_vTexCoord.xy).rgb;
	real3	vScatter		= tex2D(ScatterSampler, vtxData.m_vTexCoord.xy).rgb;

	vDiffuse *= mtlData.m_vAlbedo * g_vSkinDiffuseColour.rgb * g_fSkinDiffuseIntensity;
	vDiffuse += vBackScatter * vSubsurface * vScatter.g * g_vBackScatterColour * g_fBackScatterIntensity;
#else
	vDiffuse += mtlData.m_vAlbedo;
#endif
	return vDiffuse;
}

// replace lighting highlight

float3	g_vSpecularColour		< string Source = "SpecularColour"; string Default = "1.0, 1.0, 1.0"; string Type = "Float3"; >	= float3(1.0f, 1.0f, 1.0f);
float	g_fRimSpecularScale		< string Source = "RimSpecularScale"; string Default = "0.0"; string Type = "Float"; >			= 0.0f;
float	g_fRimSpecularOffset	< string Source = "RimSpecularOffset"; string Default = "0.5"; string Type = "Float"; >			= 0.5f;

real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3 vHighlight = 0.0f;
#if !defined(CFG_CONVERTER)
#if defined(ENABLE_SPECULAR)
	vHighlight += saturate(g_fRimSpecularOffset + surfaceData.m_fNDotV) * g_fRimSpecularScale;
#if defined(BLINN)
	vHighlight += lightData.m_vHighlight * Blinn(lightData.m_vDirection, surfaceData.m_vNormal, surfaceData.m_vView, mtlData.m_fHighlightPower);
#else
	vHighlight += lightData.m_vHighlight * Phong(lightData.m_vDirection, surfaceData.m_vNormal, surfaceData.m_vView, mtlData.m_fHighlightPower);
#endif
	vHighlight *= g_vSpecularColour.rgb * mtlData.m_fHighlightIntensity * g_fHighlightIntensity;
#endif
#endif
	return vHighlight;
}

// replace material

float	g_fSkinHighlightIntensity	< string Source = "SkinHighlightIntensity"; string Default = "4.00"; string Type = "Float"; >	= 2.0f;
float	g_fSkinHighlightSmoothness	< string Source = "SkinHighlightSmoothness"; string Default = "0.5f"; string Type = "Float"; >	= 0.5f;
float	g_fSkinReflectionIntensity	< string Source = "SkinReflectionIntensity"; string Default = "2.00"; string Type = "Float"; >	= 4.0f;
float	g_fSkinReflectionSmoothness	< string Source = "SkinReflectionSmoothness"; string Default = "0.5f"; string Type = "Float"; >	= 0.5f;
float	g_fSkinFresnelIntensity		< string Source = "SkinFresnelIntensity"; string Default = "0.175"; string Type = "Float"; >	= 1.0f;
float	g_fSkinFresnelMidBrightness	< string Source = "SkinFresnelMidBrightness"; string Default = "0.0f"; string Type = "Float"; >	= 0.0f;

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;

	float2 vTexCoord = vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);

#if defined(ENABLE_SPECULARMAP) && defined(ENABLE_SPECULAR)
	real3 vSpecularSample	= tex2D(SpecularSampler, vTexCoord).rgb;
	real3 vHighlightSample	= vSpecularSample * real3(g_fSkinHighlightIntensity, g_fSkinHighlightSmoothness, 1.0f);
	real3 vReflectionSample	= vSpecularSample * real3(g_fSkinReflectionIntensity, g_fSkinReflectionSmoothness, 1.0f);
	real3 vFresnelSample	= vSpecularSample * real3(1.0f, 1.0f, g_fSkinFresnelMidBrightness);
	mtlData.m_fHighlightPower		= EvaluateHighlightPower(vHighlightSample);
	mtlData.m_fHighlightIntensity	= EvaluateHighlightIntensity(vHighlightSample, mtlData.m_fHighlightPower);
	mtlData.m_fReflectionIntensity	= EvaluateReflectionIntensity(vReflectionSample);
	mtlData.m_fReflectionPower		= EvaluateReflectionPower(vReflectionSample);
	mtlData.m_fFresnelIntensity		= EvaluateFresnelIntensity(vFresnelSample);
	mtlData.m_fFresnelMidBrightness	= EvaluateFresnelMidBrightness(vFresnelSample);
#endif

	mtlData	= EvaluateEmissive(mtlData, vTexCoord);

	return mtlData;
}

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

#if PLATFORM_PS3

technique MaterialAlphaLuminance < string Define = "ALPHA_LUMINANCE"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#endif
