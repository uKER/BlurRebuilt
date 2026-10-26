#include <Core.h>
#include <Globals.h>

#include <Shader/Material/Normal.h>

struct C_VertexInput
{
	float3	m_vPosition	: POSITION;
	float3	m_vNormal	: NORMAL;
	float2	m_vTexCoord	: TEXCOORD0;
	float3	m_vTangent	: TEXCOORD1 annotation(xyz(tangent) | format(SHHD_3N));
	float3	m_vBinormal	: TEXCOORD2 annotation(xyz(binormal) | format(SHHD_3N));
};

struct C_VertexOutput
{
	float4	m_vPosition	: POSITION;
	float2	m_vTexCoord	: TEXCOORD0;
	float3	m_vNormal	: TEXCOORD1;
	float3	m_vTangent	: TEXCOORD2;
	float3	m_vBinormal	: TEXCOORD3;
};

struct C_PixelOutput
{
	float4	m_vColour	: COLOR0;
};

texture2D	g_tNormal	< string Default = "Normal"; >;

sampler NormalSampler = 
sampler_state
{
    Texture = <g_tNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

C_VertexOutput MainVS(C_VertexInput input)
{
    C_VertexOutput output = (C_VertexOutput) 0;

	float4	vObjectPosition	= float4(input.m_vPosition, 1.0f);
	float4	vWorldPosition	= mul(vObjectPosition, g_mWorld);
	float4	vPosition		= mul(vWorldPosition, g_mViewProj);
	float3	vNormal			= mul(input.m_vNormal, (float3x3) g_mWorld);
	float3	vTangent		= mul(input.m_vTangent.xyz, (float3x3) g_mWorld);
	float3	vBinormal		= mul(input.m_vBinormal.xyz, (float3x3) g_mWorld);

	output.m_vPosition	= vPosition;
	output.m_vTexCoord	= input.m_vTexCoord;
	output.m_vNormal	= vNormal;
	output.m_vTangent	= vTangent;
	output.m_vBinormal	= vBinormal;

    return output;    
}

C_PixelOutput MainPS(C_VertexOutput input)
{
	C_PixelOutput output = (C_PixelOutput) 0;
	
	real3	vTangentNormal	= EvaluateTangentNormal(NormalSampler, input.m_vTexCoord);
	real3	vWorldNormal	= EvaluateWorldNormal(vTangentNormal, normalize(input.m_vNormal), normalize(input.m_vTangent), normalize(input.m_vBinormal));

	output.m_vColour = float4(vWorldNormal * 0.5f + 0.5f, 1.0f);
	
	return output;
}

technique Debug
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
