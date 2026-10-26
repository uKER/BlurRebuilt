#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL
#define REPLACE_SURFACE
#define REPLACE_LIGHTING_REFLECTION
#define REPLACE_TRANSFORM_POSITION

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_ALBEDOMAP
#define ENABLE_BUMPMAP
#define ENABLE_MATERIALMAP
#define ENABLE_SPECULAR
#define ENABLE_SCREENCOORD
#define ENABLE_WORLD
#define ENABLE_NORMAL

#include <Common/Defines.h>

#if defined(ENABLE_TANGENTNORMAL)
#	undef ENABLE_TANGENTNORMAL
#endif

#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// water shader
#include <Special/Water.h>

// replace material normal

real4		g_vWaterColour		< string Source = "WaterColour"; string Default = "0.30, 0.73, 0.63, 1.0"; string Type = "Float4"; >	= float4(0.30f, 0.73f, 0.63f, 1.0f);

real		g_fRippleIntensity	< string Source = "RippleIntensity"; string Default = "1.0"; string Type = "Float"; >					= 1.0f;

float2		g_vNormalScale0		< string Source = "NormalScale0"; string Default = "1.0, 1.0"; string Type = "Float2"; >				= float2(1.00f, 1.00f);
float2		g_vNormalScale1		< string Source = "NormalScale1"; string Default = "1.0, 1.0"; string Type = "Float2"; >				= float2(1.00f, 1.00f);
float2		g_vNormalVelocity0	< string Source = "NormalVelocity0"; string Default = "0.01, 0.01"; string Type = "Float2"; >			= float2(0.01f, 0.01f);
float2		g_vNormalVelocity1	< string Source = "NormalVelocity1"; string Default = "-0.01, 0.01"; string Type = "Float2"; >			= float2(-0.01f, 0.01f);

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

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData				= (C_Material) 0;

	float2	vTexCoord				= vtxData.m_vTexCoord.xy;

	mtlData.m_vAlbedo				= g_vWaterColour.rgb;
	mtlData.m_fAlpha				= g_vWaterColour.a * h4tex2D(AlbedoSampler, vTexCoord).r;

#if defined(ENABLE_BUMPMAP)
	float2	vTexCoord0				= vTexCoord * g_vNormalScale0 + g_fTime * g_vNormalVelocity0;
	float2	vTexCoord1				= vTexCoord * g_vNormalScale1 + g_fTime * g_vNormalVelocity1;
	real3	vTangentNormal0			= h4tex2D(NormalSampler, vTexCoord0).xzy;
	real3	vTangentNormal1			= h4tex2D(BlendNormalSampler, vTexCoord1).xzy;
	real3	vTangentNormal			= normalize(vTangentNormal0 + vTangentNormal1 - 1.0h);
	mtlData.m_vTangentNormal		= normalize(lerp(real3(0.0h, 1.0h, 0.0h), vTangentNormal, g_fRippleIntensity));
#else
	mtlData.m_vTangentNormal		= real3(0.0h, 1.0h, 0.0h);
#endif

	mtlData							= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);
	
	mtlData.m_fHalfSmoothness		= g_fWaterSurfaceSmoothness;

	return mtlData;
}

C_Surface EvaluateSurface(in C_Material mtlData, in C_VertexOutput vtxData)
{
	C_Surface surfaceData	= (C_Surface) 0;
#if defined(ENABLE_WORLD)
	surfaceData.m_vPosition	= vtxData.m_vWorld.xyz;
#endif
#if defined(ENABLE_BUMPMAP)
	surfaceData.m_vNormal	= mtlData.m_vTangentNormal.xyz;
#elif defined(ENABLE_NORMAL)
	surfaceData.m_vNormal	= normalize(vtxData.m_vNormal.xyz);
#endif
#if defined(ENABLE_VIEW)
	surfaceData.m_vView		= normalize(vtxData.m_vView.xyz);
#endif
#if !defined(LOD)
	surfaceData.m_fDistance	= vtxData.m_vScreenPos.z;
#endif
#if defined(ENABLE_SPECULAR)
	surfaceData.m_fNDotV	= dot(surfaceData.m_vNormal, surfaceData.m_vView);
	surfaceData.m_vReflect	= surfaceData.m_vView - 2.0h * surfaceData.m_fNDotV * surfaceData.m_vNormal;
	surfaceData.m_fFresnel	= Schlick4(surfaceData.m_fNDotV) * mtlData.m_fSpecularIntensity * saturate(2.0h * (mtlData.m_fAlpha - 0.3h));
#endif
	return surfaceData;
}

// replace vertex transform

C_VertexOutput EvaluatePosition(in C_VertexOutput output, in C_VertexData vtxData)
{
	output.m_vPosition		= mul(vtxData.m_vPosition, g_mViewProj); 
#if defined(ENABLE_NORMAL)
	output.m_vNormal.xyz	= vtxData.m_vNormal.xyz;
#endif
#if defined(ENABLE_TANGENTNORMAL)
	output.m_vTangent		= vtxData.m_vTangent;
	output.m_vBinormal		= vtxData.m_vBinormal;
#endif
#if defined(ENABLE_VIEW)
	output.m_vView.xyz		= vtxData.m_vPosition.xyz - g_vViewPosition.xyz;
#endif
#if defined(ENABLE_WORLD)
	float3	vWorldPos		= vtxData.m_vPosition.xyz + float3(vtxData.m_vNormal.x, 0.0f, vtxData.m_vNormal.z);
	output.m_vWorld.x		= dot(float4(vWorldPos, 1.0f), g_ReflectionViewMatrix0);
	output.m_vWorld.y		= dot(float4(vWorldPos, 1.0f), g_ReflectionViewMatrix1);
	output.m_vWorld.z		= dot(float4(vWorldPos, 1.0f), g_ReflectionViewMatrix3);
	output.m_vWorld.xy		= output.m_vWorld.xy * float2(0.5f, -0.5f) + output.m_vWorld.z * float2(0.5f, 0.5f);
#endif
#if !defined(LOD)
	output.m_vScreenPos	= float4(output.m_vPosition.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f) * output.m_vPosition.w, output.m_vPosition.w, vtxData.m_vPosition.y);
#endif
	return output;
}

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

// technique

technique Default
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = FALSE;
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique DefaultLOD < string Define = "LOD"; >
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = FALSE;
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#if !defined(CFG_CONVERTER)
technique DefaultLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = FALSE;
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique DefaultLSplitScreen < string Define = "ENABLE_LIGHTING_PREPASS; SPLITSCREEN"; >
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = FALSE;
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#endif
