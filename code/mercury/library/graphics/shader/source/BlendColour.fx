#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL

#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION

#define DISABLE_VERTEXLIGHTING

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// replace albedo

float2		g_vTextureScale		< string Source = "TextureScale"; string Default = "1, 1"; string Type = "Float2"; >	= float2(1.0f, 1.0f);
float2		g_vTextureOffset	< string Source = "TextureOffset"; string Default = "0, 0"; string Type = "Float2"; >	= float2(0.0f, 0.0f);

texture2D	g_tBlendAlbedo		< string Source = "BlendColour"; string uScale = "TextureScale"; string vScale = "TextureScale"; string Default = "Black"; bool AlphaDetect = true; >;

sampler BlendAlbedoSampler = 
sampler_state
{
    Texture = <g_tBlendAlbedo>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;

	float2 vTexCoord = vtxData.m_vTexCoord.xy;

	real4	vAlbedo0	= tex2D(AlbedoSampler, vtxData.m_vTexCoord.xy);
	real4	vAlbedo1	= tex2D(BlendAlbedoSampler, vtxData.m_vTexCoord.xy * g_vTextureScale + g_vTextureOffset);
#if defined(ENABLE_VERTEXCOLOURS)
	mtlData.m_vAlbedo	= lerp(vAlbedo0.rgb, vAlbedo1.rgb, vAlbedo1.a * vtxData.m_vColour.a);
#else
	mtlData.m_vAlbedo	= lerp(vAlbedo0.rgb, vAlbedo1.rgb, vAlbedo1.a);
#endif
	mtlData.m_fAlpha	= vAlbedo0.a;
	
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
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
