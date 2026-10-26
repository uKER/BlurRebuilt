#include "../vsCommon.h"
#include "../vsConstants.h"

float4x4	g_mWorldView	: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4	g_mProj			: register(CCV_DISPLAY_PROJ_MATRIX);
float4		g_vSettings		: register(CCV_RENDER_SETTINGS);

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

	float4	vViewPosition	= mul(float4(input.m_Position, 1.0f), g_mWorldView);
	float4	vProjPosition	= mul(vViewPosition, g_mProj);
	
	output.m_Position	= vProjPosition;	
	output.m_Screen.xy	= 0.5f * float2(vProjPosition.w + vProjPosition.x, vProjPosition.w - vProjPosition.y);
	output.m_Screen.zw	= vProjPosition.zw;
#if defined(XB2) || defined(PC)
	output.m_Screen.xy	+= g_vSettings.xy * vProjPosition.w;
#endif

	return output;
}
