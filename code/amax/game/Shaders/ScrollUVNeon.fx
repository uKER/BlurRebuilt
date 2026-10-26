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

float	g_vScrollTextureX	< string Source = "ScrollTextureX"; string Default = "0.0"; string Type = "Float"; >	= 0.0f;
float	g_vScrollTextureY	< string Source = "ScrollTextureY"; string Default = "0.0"; string Type = "Float"; >	= 0.0f;
float	g_vScrollSpeedX		< string Source = "ScrollSpeedX"; string Default = "0.0"; string Type = "Float"; >		= 0.0f;
float	g_vScrollSpeedY		< string Source = "ScrollSpeedY"; string Default = "0.0"; string Type = "Float"; >		= 0.0f;

texture2D	g_tNeon			< string Source = "Neon"; string Default = "Black"; >;


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

	float2	vTexCoord		= vtxData.m_vTexCoord.xy  + float2(g_vScrollTextureX,g_vScrollTextureY) + float2(g_vScrollSpeedX,g_vScrollSpeedY) * g_fTime;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);

	mtlData.m_vEmissive		= h4tex2D(NeonSampler, vTexCoord).rgb;

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
