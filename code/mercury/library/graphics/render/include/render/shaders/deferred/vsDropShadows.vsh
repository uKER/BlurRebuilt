#include "../vsCommon.h"
#include "../vsConstants.h"

float4	g_vFar0			: register(CCV_RENDER_SETTINGS);
float4	g_vFar1			: register(CCV_RENDER_SETTINGS_1);
float4	g_vFar2			: register(CCV_RENDER_SETTINGS_2);
float4	g_vFar3			: register(CCV_RENDER_SETTINGS_3);
float4	g_vUVScale		: register(CCV_RENDER_SETTINGS_4);

struct VS_INPUT
{
	float2	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float2	m_TexCoord	: TEXCOORD0;
	float3	m_Scale		: TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	output.m_Position = float4(input.m_Position, 0.0f, 1.0f);
	output.m_TexCoord	= input.m_Position * g_vUVScale.xy + g_vUVScale.zw;
	output.m_Scale		= lerp(lerp(g_vFar1, g_vFar2, output.m_TexCoord.x), lerp(g_vFar0, g_vFar3, output.m_TexCoord.x), output.m_TexCoord.y);
	
	return output;
}
