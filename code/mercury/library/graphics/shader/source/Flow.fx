#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL

#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP
#define ENABLE_SPECULAR
#define ENABLE_TANGENTNORMAL
#define ENABLE_VIEWVECTOR

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// flow constants

float2		g_vFlowUVScale		< string Source = "FlowUVScale"; string Default = "2.0, 1.0"; string Type = "Float2"; >				= float2(2.0f, 1.0f);
float		g_fFlowSpeed		< string Source = "FlowSpeed"; string Default = "0.50"; string Type = "Float"; >					= 0.50f;
float		g_fFlowFoam			< string Source = "FlowFoam"; string Default = "0.40"; string Type = "Float"; >						= 0.40f;
float		g_fFlowRefract		< string Source = "FlowRefract"; string Default = "0.04"; string Type = "Float"; >					= 0.04f;
float3		g_vFlowSpecular		< string Source = "FlowSpecular"; string Default = "1.00, 0.80, 0.25"; string Type = "Float3"; >	= float3(1.0f, 0.8f, 0.25f);

texture2D	g_tFlow				< string Source = "Flow"; string Default = "Black"; >;

sampler FlowSampler = 
sampler_state
{
    Texture = <g_tFlow>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;	
};

// replace material

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;

	real	fFlowIntensity	= tex2D(AlbedoSampler, vtxData.m_vTexCoord.xy).a;
	real2	vFlowUV			= vtxData.m_vTexCoord.xy * g_vFlowUVScale;
	real2	vFlowTime		= float2(0.0f, g_fTime) + float2(fFlowIntensity, fFlowIntensity);

	real4	vFlowSample		= tex2D(FlowSampler, vFlowUV - vFlowTime * g_fFlowSpeed);

	real3	vFlowNormal		= vFlowSample.xyz * 2.0f - 1.0f;
	real	fFlowFoam		= fFlowIntensity * vFlowSample.w * g_fFlowFoam;

	real2	vTexCoord		= vtxData.m_vTexCoord.xy - vFlowNormal.xy * g_fFlowRefract * fFlowIntensity;

	// albedo
	real4 vAlbedo = tex2D(AlbedoSampler, vTexCoord);
	mtlData.m_vAlbedo			= saturate(vAlbedo.xyz + fFlowFoam);
	mtlData.m_fAlpha			= 1.0f;

	// tangent normal
#if defined(ENABLE_BUMPMAP)
	mtlData.m_vTangentNormal	= EvaluateTangentNormal(NormalSampler, vTexCoord);
#elif defined(ENABLE_TANGENTNORMAL)
	mtlData.m_vTangentNormal	= real3(0.0f, 0.0f, 1.0f);
#endif
	mtlData.m_vTangentNormal	= normalize(mtlData.m_vTangentNormal + vFlowNormal * fFlowIntensity);

	// specular
#if defined(ENABLE_SPECULAR)
	real3	vSpecularSample		= g_vFlowSpecular;
#if defined(ENABLE_SPECULARMAP)
			vSpecularSample		= lerp(tex2D(SpecularSampler, vTexCoord).rgb, vSpecularSample, fFlowIntensity);
#endif
	mtlData	= EvaluateSpecularParameters(mtlData, vSpecularSample);
#endif	

	// emissive
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
