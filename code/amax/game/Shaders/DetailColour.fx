#define ALPHA_LUMINANCE

#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL
#define REPLACE_VERTEXSHADER_INPUT
#define REPLACE_VERTEXSHADER_OUTPUT

#define ENABLE_ALBEDOMAP
#define ENABLE_MATERIALMAP

#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION

#define DISABLE_VERTEXLIGHTING

#include <Common/Defines.h>
#include <Common/Parameters.h>

struct C_VertexInput
{
	VS_INPUT_BASE
#if !defined(CFG_CONVERTER)
	float2	m_vDetailCoord	: TEXCOORD3 annotation(xy(uv:1) | format(F16_2));
#else
	float2	m_vDetailCoord0	: TEXCOORD3;
#endif
};

struct C_VertexOutput
{
	VS_OUTPUT_BASE
	float2	m_vExtraCoord	: TEXCOORD7;
};

#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// replace albedo

texture2D	g_tDetailAlbedo		< string Source = "DetailColour"; string Default = "White"; >;

sampler DetailAlbedoSampler = 
sampler_state
{
    Texture = <g_tDetailAlbedo>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tDetailAlbedo COMPRESSED_RGBA_S3TC_DXT1
#endif

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;

	float2 vTexCoord = vtxData.m_vTexCoord.xy;
	float2 vDetailCoord = vtxData.m_vExtraCoord.xy;

	real4	vAlbedo0	= h4tex2D(AlbedoSampler, vTexCoord);
	real4	vAlbedo1	= h4tex2D(DetailAlbedoSampler, vDetailCoord);
#if defined(ENABLE_VERTEXCOLOURS)
	mtlData.m_vAlbedo	= lerp(vAlbedo0.rgb, vAlbedo1.rgb, vAlbedo1.a * vtxData.m_vColour.a);
#else
	mtlData.m_vAlbedo	= lerp(vAlbedo0.rgb, vAlbedo1.rgb, vAlbedo1.a);
#endif
	mtlData.m_fAlpha	= vAlbedo0.a;
	
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);
	
	return mtlData;
}

// vertex shader

#include <Common/Vertex.h>

C_VertexOutput DetailColourVS(C_VertexInput input)
{
    C_VertexOutput output = MainVS(input);

#if !defined(CFG_CONVERTER)
	output.m_vExtraCoord = input.m_vDetailCoord.xy;
#else
	output.m_vExtraCoord = input.m_vDetailCoord0;
#endif

    return output;    
}

// pixel shader

#include <Common/Pixel.h>

// technique

// technique

technique Material
{
	pass p0
	{
		VertexShader = compile vs_3_0 DetailColourVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 DetailColourVS();
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
