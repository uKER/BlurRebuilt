#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL

#define ENABLE_TANGENTNORMAL
#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP

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

// replace normal

float2		g_vTextureScale		< string Source = "UVScale"; string Default = "1, 1"; string Type = "Float2"; >		= float2(1.0f, 1.0f);
float2		g_vTextureOffset	< string Source = "UVOffset"; string Default = "0, 0"; string Type = "Float2"; >	= float2(0.0f, 0.0f);
float		g_fNormalScale		< string Source = "NormalScale"; string Default = "1"; string Type = "Float"; >		= 1.0f;
float		g_fAlbedoShade		< string Source = "AlbedoShade"; string Default = "0"; string Type = "Float"; >		= 0.0f;

texture2D	g_tBlendNormal		< string Source = "BlendNormal"; string Default = "Black"; >;

sampler BlendNormalSampler = 
sampler_state
{
    Texture = <g_tBlendNormal>;
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

	real4	vDetailNormal			= tex2D(BlendNormalSampler, vtxData.m_vTexCoord.xy * g_vTextureScale + g_vTextureOffset) * 2.0f - 1.0f;
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
