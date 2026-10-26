#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// replace material

float		g_fRippleIntensity	< string Source = "RippleIntensity"; string Default = "1.0"; string Type = "Float"; >			= 1.0f;

float2		g_vNormalScale0		< string Source = "NormalScale0"; string Default = "1, 1"; string Type = "Float2"; >			= float2(1.0f, 1.0f);
float2		g_vNormalScale1		< string Source = "NormalScale1"; string Default = "1, 1"; string Type = "Float2"; >			= float2(1.0f, 1.0f);
float2		g_vNormalVelocity0	< string Source = "NormalVelocity0"; string Default = "0.01, 0.01"; string Type = "Float2"; >	= float2(0.01f, 0.01f);
float2		g_vNormalVelocity1	< string Source = "NormalVelocity1"; string Default = "-0.01, 0.01"; string Type = "Float2"; >	= float2(0.01f, 0.01f);

texture2D	g_tBlendNormal		< string Source = "BlendNormal"; string Default = "Black"; >;

sampler BlendNormalSampler = 
sampler_state
{
    Texture = <g_tBlendNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;

	float2 vTexCoord = vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
#if defined(ENABLE_BUMPMAP)
	real2	vTexCoord0			= vTexCoord * g_vNormalScale0 + g_fTime * g_vNormalVelocity0;
	real2	vTexCoord1			= vTexCoord * g_vNormalScale1 + g_fTime * g_vNormalVelocity1;
	real3	vTangentNormal0		= EvaluateTangentNormal(NormalSampler, vTexCoord0);
	real3	vTangentNormal1		= EvaluateTangentNormal(BlendNormalSampler, vTexCoord1);
	real3	vTangentNormal		= normalize(vTangentNormal0 + vTangentNormal1);
	mtlData.m_vTangentNormal	= normalize(lerp(real3(0.0f, 0.0f, 1.0f), vTangentNormal, g_fRippleIntensity));
#endif
	mtlData	= EvaluateSpecularParameters(mtlData, vTexCoord);
	mtlData	= EvaluateEmissive(mtlData, vTexCoord);

	return mtlData;
}

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material < bool TranslucentForPVS = true; >
{
	pass p0
	{
		AlphaTestEnable = true;
		AlphaBlendEnable = true;
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
