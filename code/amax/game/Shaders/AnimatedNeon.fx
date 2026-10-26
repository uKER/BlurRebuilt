#include <Core.h>
#include <Globals.h>

#define ENABLE_VERTEXCOLOURS_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION

#define ENABLE_ALBEDOMAP
#define ENABLE_MATERIALMAP

#define REPLACE_MATERIAL
#define REPLACE_VERTEXSHADER_INPUT
#define REPLACE_VERTEXSHADER_OUTPUT

#include <Common/Defines.h>
#include <Common/Parameters.h>

struct C_VertexInput
{
	VS_INPUT_BASE
	float2	m_vNeonCoord	: TEXCOORD3 annotation(xy(uv:1) | format(F16_2));
};

struct C_VertexOutput
{
	VS_OUTPUT_BASE
	float2	m_vNeonCoord	: TEXCOORD7;
};

#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// vertex shader

#include <Common/Vertex.h>

float	g_fAnimatedNeonX	< string Source = "AnimateNeonX"; string Default = "0.0"; >	= 0.0f;
float	g_fAnimatedNeonY	< string Source = "AnimateNeonY"; string Default = "0.0"; >	= 0.0f;
float	g_fScrollSpeedX	< string Source = "ScrollSpeedX"; string Default = "0.0"; >	= 0.0f;
float	g_fScrollSpeedY	< string Source = "ScrollSpeedY"; string Default = "0.0"; >	= 0.0f;

C_VertexOutput AnimatedNeonVS(in C_VertexInput input)
{
    C_VertexOutput output = MainVS(input);
	output.m_vNeonCoord = input.m_vNeonCoord + float2(g_fAnimatedNeonX, g_fAnimatedNeonY) + float2(g_fScrollSpeedX,g_fScrollSpeedY) * g_fTime;
    return output;    
}

// replace material

texture2D	g_tNeon	< string Source = "Neon"; string Default = "Black"; >;

sampler NeonSampler = 
sampler_state
{
    Texture = <g_tNeon>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tNeon COMPRESSED_RGBA_S3TC_DXT1
#endif

real3	g_vAnimatedNeonColour	< string Source = "AnimateNeonColour"; string Default = "1.0, 1.0, 1.0"; > = real3(1.0h, 1.0h, 1.0h);

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);

	mtlData.m_vEmissive	= h4tex2D(NeonSampler, vtxData.m_vNeonCoord.xy).rgb * g_vAnimatedNeonColour;

	return mtlData;
}

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material
{
	pass p0
	{
		VertexShader = compile vs_3_0 AnimatedNeonVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 AnimatedNeonVS();
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

technique MaterialAlphaLuminanceLOD < string Define = "ALPHA_LUMINANCE; LOD"; >
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
