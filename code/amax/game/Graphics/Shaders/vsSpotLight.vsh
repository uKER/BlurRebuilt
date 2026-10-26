#include "deferred/shaders/vsCommon.h"
#include "deferred/shaders/vsConstants.h"

float4x4	g_mWorldView	: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4	g_mProj			: register(CCV_DISPLAY_PROJ_MATRIX);
float4		g_vSettings		: register(CCV_RENDER_SETTINGS);
float4		g_vInvProj		: register(CCV_RENDER_SETTINGS_1);
float4		g_vFarClip		: register(CCV_RENDER_SETTINGS_2);

struct VS_INPUT
{
	float3	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float4	m_Screen	: TEXCOORD0;
	float3	m_View		: TEXCOORD1;
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
	output.m_View.xy	= output.m_Screen.xy * g_vInvProj.xy + output.m_Screen.w * g_vInvProj.zw;
	output.m_View.z		= output.m_Screen.w * g_vFarClip.x;

	return output;
}
