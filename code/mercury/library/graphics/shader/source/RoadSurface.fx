#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL
#define REPLACE_LIGHTING_HIGHLIGHT
#define REPLACE_VERTEXSHADER_INPUT
#define REPLACE_VERTEXSHADER_OUTPUT

//#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP
#define DISABLE_VERTEXLIGHTING

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#include <Common/Defines.h>
#include <Common/Parameters.h>

struct C_VertexInput
{
	VS_INPUT_BASE
	float2	m_vDetailCoord	: TEXCOORD3 annotation(xy(uv:1) | format(F16_2));
};

struct C_VertexOutput
{
	VS_OUTPUT_BASE
	float2	m_vDetailCoord	: TEXCOORD6;
};

#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// two colour, two specular

float		g_fRoadSurfaceHighlightIntensity	< string Source = "HighlightIntensity"; string Default = "1.0"; >	= 1.0f;
float		g_fRoadSurfaceReflectionIntensity	< string Source = "ReflectionIntensity"; string Default = "1.0"; >	= 1.0f;
float		g_fRoadSurfaceSmoothness			< string Source = "Smoothness"; string Default = "0.5"; >			= 0.5f;

texture2D	g_tDetailAlbedo		< string Source = "DetailColour"; string Default = "Black"; >;

sampler DetailAlbedoSampler = 
sampler_state
{
    Texture = <g_tDetailAlbedo>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

texture2D	g_tDetailSpecular	< string Source = "DetailSpecular"; string Default = "Black"; >;

sampler DetailSpecularSampler = 
sampler_state
{
    Texture = <g_tDetailSpecular>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;
	float2	vDetailCoord	= vtxData.m_vDetailCoord.xy;

	real4	vAlbedo			= tex2D(AlbedoSampler, vTexCoord);
	real4	vDetailAlbedo	= tex2D(DetailAlbedoSampler, vDetailCoord);
	real	fBlend			= vDetailAlbedo.a;
#if defined(ENABLE_VERTEXCOLOURS)
			fBlend			*= vtxData.m_vColour.a;
#endif

	mtlData.m_vAlbedo				= lerp(vAlbedo.xyz, vDetailAlbedo.xyz, fBlend);
	mtlData.m_fAlpha				= vAlbedo.w;

	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);

#if defined(ENABLE_SPECULAR)
#if defined(ENABLE_SPECULARMAP)
	real	fBaseIntensity			= tex2D(SpecularSampler, vTexCoord).r;
	real	fDetailIntensity		= tex2D(DetailSpecularSampler, vDetailCoord).r;
	real	fIntensity				= lerp(fBaseIntensity, fDetailIntensity, fBlend);
#else
	real	fIntensity				= tex2D(DetailSpecularSampler, vTexCoord).r;
#endif
	mtlData.m_fHighlightIntensity	= fIntensity * g_fRoadSurfaceHighlightIntensity;
	mtlData.m_fReflectionIntensity	= fIntensity * g_fRoadSurfaceReflectionIntensity;
	mtlData.m_fReflectionPower		= EvaluateReflectionPower(g_fRoadSurfaceSmoothness);
	mtlData.m_fFresnelIntensity		= fIntensity;
	mtlData.m_fFresnelMidBrightness	= 0.0f;
#endif

	mtlData	= EvaluateEmissive(mtlData, vTexCoord);

	return mtlData;
}

real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3 vHighlight = 0.0f;
#if defined(ENABLE_SPECULAR) && !defined(CFG_CONVERTER)
	real fNDotL = saturate(dot(surfaceData.m_vNormal, lightData.m_vDirection));
	real fRDotL = saturate(dot(surfaceData.m_vReflect, lightData.m_vDirection));
	vHighlight += lightData.m_vHighlight * fNDotL * fRDotL * fRDotL * mtlData.m_fHighlightIntensity * g_fHighlightIntensity;
#endif
	return vHighlight;
}

// vertex shader

#include <Common/Vertex.h>

C_VertexOutput RoadSurfaceVS(C_VertexInput input)
{
    C_VertexOutput output = MainVS(input);

	output.m_vDetailCoord = input.m_vDetailCoord;

    return output;    
}

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material
{
	pass p0
	{
		VertexShader = compile vs_3_0 RoadSurfaceVS();
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
