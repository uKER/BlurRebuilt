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
	
	output.m_Position = mul(vWorld, g_mViewProj);
	output.m_Screen = output.m_Position;
		
	return output;
}
