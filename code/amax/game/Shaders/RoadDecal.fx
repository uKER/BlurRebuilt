#include <Core.h>
#include <Globals.h>

#define REPLACE_LIGHTING_HIGHLIGHT
#define REPLACE_LIGHTING_REFLECTION
#define REPLACE_MATERIAL
#define REPLACE_TRANSFORM_POSITION

#define ENABLE_ALBEDOMAP
#define ENABLE_SPECULAR
#define ENABLE_WORLD
#define ENABLE_NORMAL

#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define DISABLE_LODFADE
#define DISABLE_VERTEXLIGHTING
#define NO_WORLDNORMAL

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// road surface shader
#include <Special/RoadSurface.h>

texture2D	g_tSpecular	< string Source = "<Ipr0+Colour>"; string MipFilter="BOX"; string Default = "Black"; string Define = "NS"; >;

sampler SpecularSampler = 
sampler_state
{
    Texture = <g_tSpecular>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tSpecular COMPRESSED_RGBA_S3TC_DXT1
#endif

// replace vertex transform

C_VertexOutput EvaluatePosition(in C_VertexOutput output, in C_VertexData vtxData)
{
	output.m_vPosition		= mul(vtxData.m_vPosition, g_mViewProj); 
#if defined(ENABLE_NORMAL)
	output.m_vNormal.xyz	= vtxData.m_vNormal.xyz;
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
#if defined(ENABLE_NORMAL)
	float	fNormalFactor	= 1.0f - saturate(g_vPlaneReflection.y * vtxData.m_vNormal.y);
#else
	float	fNormalFactor	= 0.0f;
#endif
	float	fDistanceFactor	= saturate(abs(g_vPlaneReflection.w + vtxData.m_vPosition.y));
	output.m_vWorld.w		= max(fNormalFactor, fDistanceFactor);
#endif
#if !defined(LOD)
	output.m_vScreenPos	= float4(output.m_vPosition.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f) * output.m_vPosition.w, output.m_vPosition.w, vtxData.m_vPosition.w);
#endif
	return output;
}

// replace material

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);

#if !defined(NO_SPECULAR)
	real	fSpecIntensity			= h4tex2D(SpecularSampler, vTexCoord).r;
#else
	real	fSpecIntensity			= 0.0h;
#endif
	mtlData.m_fSpecularIntensity	= max(0.0h, fSpecIntensity + g_fRoadReflectionBrightness);
	mtlData.m_fHalfSmoothness		= g_fRoadSurfaceSmoothness;
	mtlData.m_fFresnelMidBrightness	= 0.0h;
	mtlData.m_vEmissive				= 0.0h;

#if defined(ENABLE_VERTEXCOLOURS)
	mtlData.m_fAlpha *= vtxData.m_vColour.a;
#endif

	return mtlData;
}

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

// technique

#define FLAG_EXCLUDE_FROM_CUBEMAP_MIRROR_REFLECTION 11

technique Material < int Flags = FLAG_EXCLUDE_FROM_CUBEMAP_MIRROR_REFLECTION; >
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

technique MaterialLOD < string Define = "LOD"; >
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

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
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
