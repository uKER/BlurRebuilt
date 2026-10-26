#include <Core.h>
#include <Globals.h>

#define REPLACE_LIGHTING_DIFFUSE
#define REPLACE_MATERIAL

#define ENABLE_TANGENTNORMAL
#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP
#define ENABLE_VIEWVECTOR

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// replace diffuse

float3	g_vRimLightDirection	< string Source = "RimLightDirection"; string Default = "0.2, -0.3, -1.0"; >;
float3	g_vRimLightColour		< string Source = "RimLightColour"; string Default = "1, 1, 1"; >;

real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3 vDiffuse = 0.0f;
	vDiffuse += RimLight(g_vRimLightDirection, surfaceData.m_vNormal, surfaceData.m_vView) * g_vRimLightColour;
#if !defined(CFG_CONVERTER)
	vDiffuse += lightData.m_vLighting * g_vBakedColour.rgb;
	vDiffuse += g_vSkyColour.rgb * lightData.m_fSkyOcclusion;
	vDiffuse += g_vSunColour.rgb * saturate(dot(surfaceData.m_vNormal.xyz, g_vSunDirection.xyz)) * lightData.m_fSunOcclusion;
#else
	vDiffuse += float3(1.0f, 1.0f, 1.0f);
#endif
	vDiffuse *= mtlData.m_vAlbedo;
	return vDiffuse;
}

// replace material

float2		g_vTextureScale		< string Source = "UVScale"; string Default = "1, 1"; >;
float2		g_vTextureOffset	< string Source = "UVOffset"; string Default = "0, 0"; >;
float		g_fNormalScale		< string Source = "NormalScale"; string Default = "1"; >;
float		g_fAlbedoShade		< string Source = "AlbedoShade"; string Default = "0"; >;

texture2D	g_tDetailNormal		< string Source = "BlendNormal"; string Default = "Black"; >;

sampler DetailNormalSampler = 
sampler_state
{
    Texture = <g_tDetailNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;

	float2 vTexCoord = vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);

	real4	vDetailNormal			= tex2D(DetailNormalSampler, vtxData.m_vTexCoord.xy * g_vTextureScale + g_vTextureOffset) * 2.0f - 1.0f;
#if defined(ENABLE_BUMPMAP)
	real3	vTangentNormal			= EvaluateTangentNormal(NormalSampler, vtxData.m_vTexCoord.xy);
	mtlData.m_vTangentNormal		= normalize(vTangentNormal + g_fNormalScale * vDetailNormal.xyz);
#else
	mtlData.m_vTangentNormal		= normalize(vDetailNormal.xyz);
#endif
	mtlData.m_vAlbedo				= mtlData.m_vAlbedo * lerp(1.0f, vDetailNormal.a, g_fAlbedoShade);

	mtlData	= EvaluateSpecularParameters(mtlData, vTexCoord);
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
