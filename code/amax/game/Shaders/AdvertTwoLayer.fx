#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL

//expose albedo texture
//#define ENABLE_LAMBERT_SHADER
#define ENABLE_TEXTURE_NAMING_CONVENTIONS

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_MATERIALMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_ALBEDOMAP
//#define ENABLE_BUMPMAP
//#define ENABLE_MATERIALMAP

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

texture2D	g_tAlbedoTwoLayer	< string Source = "AlbedoTwoLayer"; bool Uninitialised = true; string Default = "White"; bool AlphaDetect = true; MIPMAP_ALPHA_ENHANCE >;

sampler AlbedoTwoLayerSampler : register(s3) = 
sampler_state
{
    Texture = <g_tAlbedoTwoLayer>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

float	g_AlbedoTwoLayerAlpha	< string Source = "AlbedoTwoLayerAlpha"; string Default = "1.0"; > = 1.0;

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);

	//Read second layer
	real4	vAlbedo2	= h4tex2D(AlbedoTwoLayerSampler, vTexCoord).xyzw;
	real alpha2 = vAlbedo2.a * g_AlbedoTwoLayerAlpha;
	//Blend
	mtlData.m_vAlbedo.rgb = mtlData.m_vAlbedo.rgb * (1.h - alpha2) + vAlbedo2.rgb * alpha2;

	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);

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
