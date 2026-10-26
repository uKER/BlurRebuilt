#include "../vsCommon.h"
#include "../vsConstants.h"

float4x4	g_mWorld	: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4	g_mViewProj	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);

struct VS_INPUT
{
	float3	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float4	m_Screen	: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	float4	vWorld	= mul(float4(input.m_Position, 1.0f), g_mWorld);
	float4  vProjPos = mul(vWorld, g_mViewProj);
	output.m_Position = vProjPos;
	output.m_Screen = vProjPos * float4(0.5f, - 0.5, 1.f, 1.f) + float4(0.5f * vProjPos.w, 0.5f * vProjPos.w, 0.f, 0.f);
	
	return output;
}
