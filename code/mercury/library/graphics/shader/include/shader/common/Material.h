#ifndef SHADER_MATERIAL_H
#define SHADER_MATERIAL_H

#include <Shader/Material/Colour.h>
#include <Shader/Material/Normal.h>
#include <Shader/Material/Specular.h>

// structures
struct C_Material
{
	real3	m_vAlbedo;
	real	m_fAlpha;
#if defined(ENABLE_TANGENTNORMAL)
	real3	m_vTangentNormal;
#endif
#if defined(ENABLE_SPECULAR)
	real	m_fHighlightIntensity;
	real	m_fHighlightPower;
	real	m_fReflectionIntensity;
	real	m_fReflectionPower;
	real	m_fFresnelIntensity;
	real	m_fFresnelMidBrightness;
#endif
#if defined(ENABLE_EMISSIVE)
	real3	m_vEmissive;
#endif
};

// textures

texture2D	g_tAlbedo	< string Source = ALBEDO_NAME_STRING; bool Uninitialised = true; string Default = "White"; ALPHA_DETECT MIPMAP_ALPHA_ENHANCE >;

sampler AlbedoSampler : register(SSP_SHADER_ALBEDO) = 
sampler_state
{
    Texture = <g_tAlbedo>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

#if defined(ENABLE_SPECULARMAP)
texture2D	g_tSpecular	< string Source = SPECULAR_NAME_STRING; bool Uninitialised = true; string Default = "Black"; SPECULAR_DEFINE_STRING >;

sampler SpecularSampler : register(SSP_SHADER_SPECULAR) = 
sampler_state
{
    Texture = <g_tSpecular>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};
#endif

#if defined(ENABLE_BUMPMAP)
texture2D	g_tNormal	< string Source = BUMP_NAME_STRING; bool Uninitialised = true; string Default = "Normal"; BUMP_DEFINE_STRING >;

sampler NormalSampler : register(SSP_SHADER_NORMAL) = 
sampler_state
{
    Texture = <g_tNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if defined(ENABLE_EMISSIVE)
texture2D	g_tEmissive	< string Source = EMISSIVE_NAME_STRING; bool Uninitialised = true; string Default = "Black"; EMISSIVE_DEFINE_STRING >;

sampler EmissiveSampler : register(SSP_SHADER_EMISSIVE) = 
sampler_state
{
    Texture = <g_tEmissive>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};
#endif

// functions

C_Material EvaluateAlbedo(in C_Material mtlData, in float2 vTexCoord)
{
	real4 vAlbedo = tex2D(AlbedoSampler, vTexCoord);
	mtlData.m_vAlbedo				= vAlbedo.xyz;
	mtlData.m_fAlpha				= vAlbedo.w;
	return mtlData;
}

C_Material EvaluateTangentNormal(in C_Material mtlData, in float2 vTexCoord)
{
#if defined(ENABLE_BUMPMAP)
	mtlData.m_vTangentNormal		= EvaluateTangentNormal(NormalSampler, vTexCoord);
#elif defined(ENABLE_TANGENTNORMAL)
	mtlData.m_vTangentNormal		= real3(0.0f, 0.0f, 1.0f);
#endif
	return mtlData;
}

C_Material EvaluateSpecularParameters(in C_Material mtlData, in real3 vMaterialSample)
{
#if defined(ENABLE_SPECULAR)
	mtlData.m_fHighlightIntensity	= EvaluateHighlightIntensity(vMaterialSample);
	mtlData.m_fHighlightPower		= EvaluateHighlightPower(vMaterialSample);
	mtlData.m_fReflectionIntensity	= EvaluateReflectionIntensity(vMaterialSample);
	mtlData.m_fReflectionPower		= EvaluateReflectionPower(vMaterialSample);
	mtlData.m_fFresnelIntensity		= EvaluateFresnelIntensity(vMaterialSample);
	mtlData.m_fFresnelMidBrightness	= EvaluateFresnelMidBrightness(vMaterialSample);
#endif
	return mtlData;
}

C_Material EvaluateSpecularParameters(in C_Material mtlData, in float2 vTexCoord)
{
#if defined(ENABLE_SPECULARMAP) && defined(ENABLE_SPECULAR)
	real3 vSpecularSample = tex2D(SpecularSampler, vTexCoord).rgb;
	mtlData = EvaluateSpecularParameters(mtlData, vSpecularSample);
#endif
	return mtlData;
}

C_Material EvaluateEmissive(in C_Material mtlData, in float2 vTexCoord)
{
#if defined(ENABLE_EMISSIVE)
#if defined(MONOCHROME_EMISSIVE)
	mtlData.m_vEmissive = tex2D(EmissiveSampler, vTexCoord).rgb * mtlData.m_vAlbedo;
#else
	mtlData.m_vEmissive = tex2D(EmissiveSampler, vTexCoord).rgb;
#endif

#endif
	return mtlData;
}

#if !defined(REPLACE_MATERIAL)
C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;

	float2 vTexCoord = vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateSpecularParameters(mtlData, vTexCoord);
	mtlData	= EvaluateEmissive(mtlData, vTexCoord);
	return mtlData;
}
#endif // REPLACE_MATERIAL

#endif // SHADER_MATERIAL_H
