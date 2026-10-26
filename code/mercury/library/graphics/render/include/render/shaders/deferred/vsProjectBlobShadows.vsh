#include "../vsCommon.h"
#include "../vsConstants.h"

float4x4	g_WorldMatrix		: register(CCV_DISPLAY_WORLD_MATRIX);

float4x4	g_ShadowPlaneSpaceMatrix : register(CCV_RENDER_SETTINGS);
float4		g_ShadowPlane : register(CCV_RENDER_SETTINGS_5);
float4		g_LightPosition : register(CCV_RENDER_SETTINGS_6); //w -> Z pos for the shadow

struct VS_INPUT
{
	float3	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float2	m_ProjectPos : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	

	float4 objectPosition = float4(input.m_Position, 1.f);
	float4 worldPosition = mul(objectPosition, g_WorldMatrix);

	//Calculate shadow plane intersection
	float u = (dot(float4(g_LightPosition.xyz,1.f) , g_ShadowPlane))/(dot(g_ShadowPlane.xyz,(g_LightPosition.xyz - worldPosition.xyz).xyz));
	float4 shadowPosition = float4(g_LightPosition.xyz + (worldPosition.xyz - g_LightPosition.xyz) * u, 1.f); 
	
	//Convert to shadow texture space (2D)
	float3 shadowSpacePosition = mul(shadowPosition, g_ShadowPlaneSpaceMatrix);
	
	output.m_Position = float4(shadowSpacePosition.x, shadowSpacePosition.y, g_LightPosition.w, 1.f);
	output.m_ProjectPos.xy = output.m_Position.xy;
	
	return output;
}
