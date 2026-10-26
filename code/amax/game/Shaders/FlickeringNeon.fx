#include <Core.h>
#include <Globals.h>

#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_ALBEDOMAP
#define ENABLE_MATERIALMAP

#define REPLACE_MATERIAL

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// vertex shader

#include <Common/Vertex.h>

// replace material

real		g_fThresholdMax	< string Source = "ThresholdOn"; string Default = "1.0"; >	= 1.0h;
real		g_fThresholdMin	< string Source = "ThresholdOff"; string Default = "0.0"; >	= 0.0h;

texture2D	g_tNeon			< string Source = "Neon"; string Default = "Black"; >;

texture2D	g_tFlicker		< string Source = "Flicker"; string Default = "Black"; string Format = "A8";>;
real		g_fFlickerSpeed	< string Source = "FlickerSpeed"; string Default = "1.0"; >	= 1.0h;

sampler FlickerSampler = 
sampler_state
{
    Texture = <g_tFlicker>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};


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

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);

	real	fNeonScale		= h4tex2D(FlickerSampler, float2(g_fFlickerSpeed * g_fTime, 0.f)).a; //Alpha
			fNeonScale		= smoothstep(g_fThresholdMax, g_fThresholdMin, fNeonScale);
	mtlData.m_vEmissive		= h4tex2D(NeonSampler, vTexCoord).rgb * fNeonScale;

	return mtlData;
}

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

#if PLATFORM_PS3

technique MaterialAlphaLuminance < string Define = "ALPHA_LUMINANCE"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0  MainPS();
	}
}

technique MaterialAlphaLuminanceLOD < string Define = "ALPHA_LUMINANCE; LOD"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0  MainPS();
	}
}

technique MaterialAlphaLuminanceLights < string Define = "ALPHA_LUMINANCE; ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0  MainPS();
	}
}

#endif
