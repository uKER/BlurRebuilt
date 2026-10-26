#ifndef AMAX_SHADERS_COMMON_MATERIAL_H
#define AMAX_SHADERS_COMMON_MATERIAL_H

#include <Shader/Material/Normal.h>

// structures

struct C_Material
{
	real3	m_vAlbedo;
	real	m_fAlpha;
	real3	m_vTangentNormal;
	real	m_fHalfSmoothness;
	real	m_fSpecularIntensity;
	real	m_fFresnelMidBrightness;
	real3	m_vEmissive;
};

// textures

#if defined(ENABLE_ALBEDOMAP)
texture2D	g_tAlbedo	< string Source = ALBEDO_NAME_STRING; bool Uninitialised = true; string Default = "White"; bool AlphaDetect = true; MIPMAP_ALPHA_ENHANCE >;

sampler AlbedoSampler : register(SSP_SHADER_ALBEDO) = 
sampler_state
{
    Texture = <g_tAlbedo>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};
#endif

#if defined(ENABLE_MATERIALMAP)
texture2D	g_tMaterial	< string Source = MATERIAL_NAME_STRING; bool Uninitialised = true; string Default = "Black"; bool INR0 = true; MATERIAL_DEFINE_STRING >;

sampler MaterialSampler : register(SSP_SHADER_MATERIAL) = 
sampler_state
{
    Texture = <g_tMaterial>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};
#endif

#if defined(ENABLE_BUMPMAP)
#ifdef FLIP_NORMAL_MAP_X
texture2D	g_tNormal	< string Source = BUMP_NAME_STRING; bool Uninitialised = true; bool invertNormalMapX = true; string MipFilter = "BOX"; string Default = "Normal"; BUMP_DEFINE_STRING >;
#else
texture2D	g_tNormal	< string Source = BUMP_NAME_STRING; bool Uninitialised = true; string MipFilter = "BOX"; string Default = "Normal"; BUMP_DEFINE_STRING >;
#endif

sampler NormalSampler : register(SSP_SHADER_NORMAL) = 
sampler_state
{
    Texture = <g_tNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tAlbedo COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_tMaterial COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_tNormal COMPRESSED_RGBA_S3TC_DXT1
#endif

// functions

real3 EvaluateMaterialTangentNormal(in real3 vSample)
{
	return normalize(vSample - 0.5h);
}

C_Material EvaluateAlbedo(in C_Material mtlData, in float2 vTexCoord)
{
#if defined(ENABLE_ALBEDOMAP)
	real4	vAlbedo					= h4tex2D(AlbedoSampler, vTexCoord).xyzw;
	mtlData.m_vAlbedo				= vAlbedo.rgb;
	mtlData.m_fAlpha				= vAlbedo.a;
#else
	mtlData.m_vAlbedo				= real3(1.0h, 1.0h, 1.0h);
	mtlData.m_fAlpha				= 1.0h;
#endif
	return mtlData;
}

C_Material EvaluateTangentNormal(in C_Material mtlData, in float2 vTexCoord)
{
#if defined(ENABLE_BUMPMAP)
	real3	vTangentNormal			= h4tex2D(NormalSampler, vTexCoord).xyz;
	mtlData.m_vTangentNormal		= EvaluateMaterialTangentNormal(vTangentNormal);
#ifdef FLIP_NORMAL_MAP_X
#ifdef CFG_CONVERTER
	mtlData.m_vTangentNormal.x = -mtlData.m_vTangentNormal.x;
#endif
#endif
#else
	mtlData.m_vTangentNormal		= real3(0.0h, 0.0h, 1.0h);
#endif
	return mtlData;
}

C_Material EvaluateMaterialParameters(in C_Material mtlData, in real4 vSample)
{
#if defined(DAY_LIGHTING)
	mtlData.m_fSpecularIntensity	= vSample.r;
	mtlData.m_fHalfSmoothness		= 0.5h * vSample.g;
	mtlData.m_fFresnelMidBrightness	= vSample.b;
	mtlData.m_vEmissive				= (1.0h - vSample.a) * mtlData.m_vAlbedo.rgb;
#else
	mtlData.m_fSpecularIntensity	= vSample.r;
	mtlData.m_fFresnelMidBrightness	= vSample.b;
	mtlData.m_fHalfSmoothness		= vSample.b;
	mtlData.m_vEmissive				= vSample.g * mtlData.m_vAlbedo.rgb;
#endif
	return mtlData;
}

C_Material EvaluateMaterialParameters(in C_Material mtlData, in C_VertexOutput vtxData, in float2 vTexCoord)
{
#if defined(ENABLE_MATERIALMAP)
#if defined(ENABLE_MATERIALBIAS) && defined(ENABLE_SPECULAR)
	real	fNDotV					= dot(vtxData.m_vNormal.xyz, normalize(vtxData.m_vView.xyz));
	real	fMaterialBias			= 4.0h * (abs(fNDotV) - 1.0h);
	real4	vMaterial				= tex2Dbias(MaterialSampler, float4(vTexCoord, 0.0f, fMaterialBias));
#else
	real4	vMaterial				= h4tex2D(MaterialSampler, vTexCoord);
#endif
	mtlData							= EvaluateMaterialParameters(mtlData, vMaterial);
#endif
	return mtlData;
}

#if !defined(REPLACE_MATERIAL)
C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);

	return mtlData;
}
#endif

#if !defined(REPLACE_MATERIAL_PREPASS)
C_Material EvaluateMaterialPrepass(in C_VertexPrepassOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	mtlData = EvaluateAlbedo(mtlData, vtxData.m_vTexCoord.xy);
	mtlData	= EvaluateTangentNormal(mtlData, vtxData.m_vTexCoord.xy);

	return mtlData;
}
#endif

#endif // AMAX_SHADERS_COMMON_MATERIAL_H
