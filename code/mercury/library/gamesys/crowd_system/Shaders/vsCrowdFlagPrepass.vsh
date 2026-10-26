#include "../../graphics/display/include/display/shaders/vsCommon.h"
#include "../../graphics/display/include/display/shaders/vsConstants.h"

float4x4			g_mViewProj	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4			g_mView		: register(CCV_DISPLAY_VIEW_MATRIX);

struct VS_INPUT
{
	float3	m_Position	: POSITION;
	float2	m_TexCoord	: TEXCOORD0;
	float3	m_Normal	: NORMAL;
};

struct VS_OUTPUT_PREPASS
{
	float4	m_Position	: POSITION;
	float3	m_Normal	: TEXCOORD0;
};

float4x4	g_FlagTransform		: register(c200);
float4		g_NearFar			: register(c204);

VS_OUTPUT_PREPASS main(VS_INPUT input)
{
	VS_OUTPUT_PREPASS output = (VS_OUTPUT_PREPASS) 0;
	
	//Calculate world position
	float4 vWorldPosition = mul(float4(input.m_Position.xyz, 1.0f), g_FlagTransform);
	float3 vWorldNormal = mul(input.m_Normal.xyz, (float3x3)g_FlagTransform);
	
	//Calculate position
	float4  vProjPos = mul(vWorldPosition, g_mViewProj);
	output.m_Position = vProjPos;
	
	output.m_Normal.xy = mul(vWorldNormal, (float3x3)g_mView).xy;
	output.m_Normal.z = output.m_Position.w / g_NearFar.w;
	
	return output;
}
