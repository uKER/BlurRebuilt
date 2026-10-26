#include "../../graphics/display/include/display/shaders/vsCommon.h"
#include "../../graphics/display/include/display/shaders/vsConstants.h"

float4x4			g_mViewProj	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4			g_mView		: register(CCV_DISPLAY_VIEW_MATRIX);
float4				g_vViewPosition	: register(CCV_DISPLAY_VIEW_POSITION);

struct VS_INPUT
{
	float3	m_Position	: POSITION;
	float4	m_Colour	: TEXCOORD0;
	float3	m_Normal	: NORMAL;
	float3	m_Direction	: TANGENT;
};

struct VS_OUTPUT
{
	float4	m_Position		: POSITION;
	float4	m_Colour		: TEXCOORD0;
	float3	m_Normal		: TEXCOORD1;
};


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	//Calculate world position
	float4 vCentrePosition = float4(input.m_Position, 1.0f);
	float4  vCentreProjPos = mul(vCentrePosition, g_mViewProj);
	
	float3 viewVector = normalize(g_vViewPosition.xyz - vCentrePosition.xyz);
	
	float alpha = input.m_Colour.a * saturate((abs(dot(viewVector, input.m_Direction)) - 0.98f)/0.02f);
	alpha *= (1.f - saturate( 5.f * vCentreProjPos.z/vCentreProjPos.w));
	
	float4 vWorldPosition = vCentrePosition;
	
	vWorldPosition.xyz = (alpha < 0.01f) ? vWorldPosition.xyz : vWorldPosition.xyz + input.m_Normal.xyz;
	float4  vProjPos = mul(vWorldPosition, g_mViewProj);
	
	output.m_Position = vProjPos;
	output.m_Colour.rgb = input.m_Colour.rgb;
	output.m_Colour.a = alpha;
	
	output.m_Normal.xyz = normalize(input.m_Normal.xyz) / sqrt(0.5f);
	
	return output;
}
