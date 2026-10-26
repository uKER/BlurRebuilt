#define ALPHA_LUMINANCE

#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL
#define REPLACE_SURFACE

#define ENABLE_LIGHTMAP_OPTIMISATION

#define ENABLE_BUMPMAP
#define ENABLE_SPECULAR
#define ENABLE_SCREENCOORD
#define ENABLE_WORLD

#include <Common/Defines.h>

#if defined(ENABLE_TANGENTNORMAL)
#	undef ENABLE_TANGENTNORMAL
#endif

#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

#include <Special/Water.h>

float2		g_vNormalScale0		< string Source = "NormalScale0"; string Default = "1, 1"; string Type = "Float2"; >			= float2(1.0f, 1.0f);
float2		g_vNormalScale1		< string Source = "NormalScale1"; string Default = "1, 1"; string Type = "Float2"; >			= float2(1.0f, 1.0f);
float2		g_vNormalVelocity0	< string Source = "NormalVelocity0"; string Default = "0.01, 0.01"; string Type = "Float2"; >	= float2(0.01f, 0.01f);
float2		g_vNormalVelocity1	< string Source = "NormalVelocity1"; string Default = "0.01, 0.00"; string Type = "Float2"; >	= float2(0.01f, 0.00f);

texture2D	g_tBlendNormal		< string Source = "BlendNormal"; string Default = "Black"; >;

sampler BlendNormalSampler = 
sampler_state
{
    Texture = <g_tBlendNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tBlendNormal COMPRESSED_RGBA_S3TC_DXT1
#endif

real	g_fWaterSpecularIntensity	< string Source = "SpecularIntensity"; string Default = "1.0"; string Type = "Float"; >			= 1.00h;
real3	g_vWaterColour				< string Source = "WaterColour"; string Default = "0.30, 0.73, 0.63"; string Type = "Float3"; >	= real3(0.30h, 0.73h, 0.63h);

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;
	
	real2	vTexCoord0				= vtxData.m_vTexCoord.xy * g_vNormalScale0 + g_fTime * g_vNormalVelocity0;
	real2	vTexCoord1				= vtxData.m_vTexCoord.xy * g_vNormalScale1 + g_fTime * g_vNormalVelocity1;
	real3	vTangentNormal0			= tex2D(NormalSampler, vTexCoord0).xzy;
	real3	vTangentNormal1			= tex2D(BlendNormalSampler, vTexCoord1).xzy;
	mtlData.m_vTangentNormal		= normalize(vTangentNormal0 + vTangentNormal1 - 1.0f);

	mtlData.m_vAlbedo				= g_vWaterColour * mtlData.m_vTangentNormal.y;
	mtlData.m_fAlpha				= 1.0h;

	mtlData.m_fSpecularIntensity	= g_fWaterSpecularIntensity;
	mtlData.m_fHalfSmoothness		= g_fWaterSurfaceSmoothness;
	mtlData.m_fFresnelMidBrightness	= 0.0f;
	mtlData.m_vEmissive				= 0.0f;

	return mtlData;
}

C_Surface EvaluateSurface(in C_Material mtlData, in C_VertexOutput vtxData)
{
	C_Surface surfaceData	= (C_Surface) 0;
#if defined(ENABLE_WORLD)
	surfaceData.m_vPosition	= vtxData.m_vWorld.xyz;
#endif
#if defined(ENABLE_NORMAL)
	surfaceData.m_vNormal	= mtlData.m_vTangentNormal.xyz;
#endif
#if defined(ENABLE_VIEW)
	surfaceData.m_vView		= normalize(vtxData.m_vView.xyz);
#endif
	surfaceData.m_fDistance	= vtxData.m_vScreenPos.z;
#if defined(ENABLE_SPECULAR)
	surfaceData.m_fNDotV	= dot(surfaceData.m_vNormal, surfaceData.m_vView);
	surfaceData.m_vReflect	= surfaceData.m_vView - 2.0f * surfaceData.m_fNDotV * surfaceData.m_vNormal;
	surfaceData.m_fFresnel	= Schlick4(surfaceData.m_fNDotV) * mtlData.m_fSpecularIntensity;
#endif
	return surfaceData;
}

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

C_PixelOutput OceanPrepassPS(in C_VertexPrepassOutput vtxData)
{
    C_PixelOutput output = (C_PixelOutput) 0;

	real2	vTexCoord0				= vtxData.m_vTexCoord.xy * g_vNormalScale0 + g_fTime * g_vNormalVelocity0;
	real2	vTexCoord1				= vtxData.m_vTexCoord.xy * g_vNormalScale1 + g_fTime * g_vNormalVelocity1;
	real3	vTangentNormal0			= tex2D(NormalSampler, vTexCoord0).xzy;
	real3	vTangentNormal1			= tex2D(BlendNormalSampler, vTexCoord1).xzy;
	real3	vWorldNormal			= normalize(vTangentNormal0 + vTangentNormal1 - 1.0h);

#if defined(ENABLE_PREPASS_VIEWDEPTH)
	float	fDepth255		= vtxData.m_vTexCoord.z * 255.0f;
	real2	vViewNormal;
	vViewNormal.x			= dot(vWorldNormal, g_vViewMatrix0.xyz);
	vViewNormal.y			= dot(vWorldNormal, g_vViewMatrix1.xyz);
	output.m_vColour		= real4(vViewNormal * 0.5h + 0.5h, floor(fDepth255) / 255.0f, frac(fDepth255));
#elif defined(ENABLE_PREPASS_VIEWSPACE)
	real3	vViewNormal;
	vViewNormal.x			= dot(vWorldNormal, g_vViewMatrix0.xyz);
	vViewNormal.y			= dot(vWorldNormal, g_vViewMatrix1.xyz);
	vViewNormal.z			= dot(vWorldNormal, g_vViewMatrix2.xyz);
	output.m_vColour		= real4(vViewNormal * 0.5h + 0.5h, 1.0f);
#else
	output.m_vColour		= real4(vWorldNormal * 0.5h + 0.5h, 1.0f);
#endif

    return output;
}

// technique

technique Default
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

technique MaterialLSplitScreen <string Define = "SPLITSCREEN; ENABLE_LIGHTING_PREPASS";>
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
		PixelShader = compile ps_3_0 OceanPrepassPS();
	}
}
#endif
