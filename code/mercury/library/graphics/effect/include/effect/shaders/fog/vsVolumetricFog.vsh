#include "../vsCommon.h"
#include "Effect/Shaders/Constants.h"

float4	g_ViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);
float4	g_PositionUvVector	: register(CCV_DISPLAY_POSITION_UV_VECTOR);

float4	g_vViewPlane00		: register(CCV_RENDER_SETTINGS);
float4	g_vViewPlane10		: register(CCV_RENDER_SETTINGS_1);
float4	g_vViewPlane11		: register(CCV_RENDER_SETTINGS_2);
float4	g_vViewPlane01		: register(CCV_RENDER_SETTINGS_3);

struct VS_INPUT
{
	float2	m_Position		: POSITION;
#ifndef XB2
	float2	m_TexCoord		: TEXCOORD0;
#endif
};

struct VS_OUTPUT
{
	float4	m_Position		: POSITION0;
	float2	m_TexCoord		: TEXCOORD0;
	float3	m_ViewVector	: TEXCOORD1;
	float4	m_ViewPosition	: TEXCOORD2;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	output.m_Position = float4(input.m_Position, 0.0f, 1.0f);
	#ifndef XB2
	output.m_TexCoord = input.m_TexCoord;
	#else
	output.m_TexCoord = input.m_Position * float2(g_PositionUvVector.x, g_PositionUvVector.y) + float2(0.5f, 0.5f);
	#endif

	float3 vViewPlane0 = lerp(g_vViewPlane00, g_vViewPlane10, output.m_TexCoord.x);
	float3 vViewPlane1 = lerp(g_vViewPlane01, g_vViewPlane11, output.m_TexCoord.x);
	output.m_ViewVector = lerp(vViewPlane0, vViewPlane1, output.m_TexCoord.y);
	output.m_ViewPosition = g_ViewPosition;

	return output;
}
