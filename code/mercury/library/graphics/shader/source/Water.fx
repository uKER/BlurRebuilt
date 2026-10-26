#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL

#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_BUMPMAP
#define ENABLE_SPECULAR
#define ENABLE_PROJPOS

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

#include <Shader/Misc/Resolve.h>

float2		g_vNormalScale0		< string Source = "NormalScale0"; string Default = "1, 1"; string Type = "Float2"; >			= float2(1.0f, 1.0f);
float2		g_vNormalScale1		< string Source = "NormalScale1"; string Default = "1, 1"; string Type = "Float2"; >			= float2(1.0f, 1.0f);
float2		g_vNormalVelocity0	< string Source = "NormalVelocity0"; string Default = "0.01, 0.01"; string Type = "Float2"; >	= float2(0.01f, 0.01f);
float2		g_vNormalVelocity1	< string Source = "NormalVelocity1"; string Default = "-0.01, 0.01"; string Type = "Float2"; >	= float2(-0.01f, 0.01f);

texture2D	g_tBlendNormal		< string Source = "BlendNormal"; string Default = "Black"; >;

sampler BlendNormalSampler = 
sampler_state
{
    Texture = <g_tBlendNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

shared texture2D	g_tColourBuffer;

sampler ColourBufferSampler : register(SSP_SHADER_COLOUR_BUFFER) = 
sampler_state
{
    Texture = <g_tColourBuffer>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

shared texture2D	g_tDepthBuffer;

sampler DepthBufferSampler : register(SSP_SHADER_DEPTH_BUFFER)  = 
sampler_state
{
    Texture = <g_tDepthBuffer>;
    MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

shared float4		g_vDepthScale : register(CCP_SHADER_DEPTH_SCALE);

float3	g_vSpecularParams	< string Source = "SpecularParams"; string Default = "1.0, 0.7, 0.1"; string Type = "Float3"; >	= float3(1.00f, 0.70f, 0.10f);
float3	g_vWaterColour		< string Source = "WaterColour"; string Default = "0.30, 0.73, 0.63"; string Type = "Float3"; >	= float3(0.30f, 0.73f, 0.63f);
float	g_fRefractAmount	< string Source = "RefractAmount"; string Default = "0.1"; string Type = "Float"; >				= 0.1f;
float	g_fExtinctionScale	< string Source = "ExtinctionScale"; string Default = "1.0"; string Type = "Float"; >			= 1.0f;
float	g_fDiffuseScale		< string Source = "DiffuseScale"; string Default = "0.2"; string Type = "Float"; >				= 0.2f;

// vertex shader

#include <Common/Vertex.h>

// pixel shader

// we are adding heightfield derivatives so we can't do standard vector addition
// see "Care and Feeding of Normal Vectors" http://www.coretechniques.info/	
float3 AddNormalMapValues(float3 normalA, float3 normalB, float alpha, float beta)
{	
	float3 result;		
	result.xy = alpha * normalA.xy * normalB.z + beta * normalB.xy * normalA.z;
	result.z = normalA.z * normalB.z;
	return normalize(result);
}

C_PixelOutput MainPS(C_VertexOutput vtxData) 
{ 
    C_PixelOutput output = (C_PixelOutput) 0;

	C_Material mtlData = (C_Material) 0;
	mtlData.m_vAlbedo				= 1.0f;
	mtlData.m_fAlpha				= 1.0f;
#if defined(ENABLE_BUMPMAP)
	real2	vTexCoord0				= vtxData.m_vTexCoord.xy * g_vNormalScale0 + g_fTime * g_vNormalVelocity0;
	real2	vTexCoord1				= vtxData.m_vTexCoord.xy * g_vNormalScale1 + g_fTime * g_vNormalVelocity1;
	real3	vTangentNormal0			= EvaluateTangentNormal(NormalSampler, vTexCoord0);
	real3	vTangentNormal1			= EvaluateTangentNormal(BlendNormalSampler, vTexCoord1);
	mtlData.m_vTangentNormal		= AddNormalMapValues(vTangentNormal0, vTangentNormal1, 1.0f, 1.0f);
#elif defined(ENABLE_TANGENTNORMAL)
	mtlData.m_vTangentNormal		= real3(0.0f, 0.0f, 1.0f);
#endif
#if defined(ENABLE_SPECULAR)
	mtlData							= EvaluateSpecularParameters(mtlData, g_vSpecularParams);
#endif	
	
	C_Surface	surfaceData	= EvaluateSurface(mtlData, vtxData);
	C_Lighting	lightData	= EvaluateLighting(mtlData, surfaceData, vtxData);
	
	real3		vDiffuse	= EvaluateDiffuse(mtlData, surfaceData, lightData, vtxData);
	real3		vSpecular	= EvaluateSpecular(mtlData, surfaceData, lightData, vtxData);
	
	// perform refraction calculation 
#if !defined(CFG_CONVERTER)
	{
		real2	vScreenCoords	= vtxData.m_vProjPos.xy / vtxData.m_vProjPos.w * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);

		float	fSceneDepth		= SampleDepthBuffer(DepthBufferSampler, vScreenCoords, g_vDepthScale);
		float	fWaterDepth		= vtxData.m_vProjPos.w; 
		float	fDistUnderwater	= max(0.0f, fSceneDepth - fWaterDepth);

		real2	vRefractCoords	= vScreenCoords.xy + surfaceData.m_vNormal.xz * g_fRefractAmount * surfaceData.m_fNDotV;
		real3	vRefractColour	= SampleColourBuffer(ColourBufferSampler, vRefractCoords);

		real3	vExtinction		= exp(-g_fExtinctionScale * fDistUnderwater * g_vWaterColour);
		real3	vInScattering	= vDiffuse * g_fDiffuseScale;
	
		vDiffuse = vRefractColour * vExtinction + vInScattering;
	}
#endif
	
	output.m_vColour.xyz	= EvaluateColour(vDiffuse, vSpecular, mtlData, surfaceData, lightData);
	output.m_vColour.w		= mtlData.m_fAlpha;

	output.m_vColour		= EvaluateFog(output.m_vColour, surfaceData);
	output.m_vColour		= EvaluateFinalColour(output.m_vColour);

#ifndef CFG_SPEED
#if !defined(CFG_CONVERTER)
#if PLATFORM_XENON
	if (length(g_vDebugColour) > 0.0)
		output.m_vColour = g_vDebugColour;
#endif
#endif
#endif

    return output;
}

// technique

technique Material < string SortType = "postresolve"; bool TranslucentForPVS = true; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
