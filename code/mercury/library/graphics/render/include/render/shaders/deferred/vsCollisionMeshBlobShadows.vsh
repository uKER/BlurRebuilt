#include "../vsCommon.h"
#include "../vsConstants.h"

float4x4	g_mViewProj					: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4	g_mView						: register(CCV_DISPLAY_VIEW_MATRIX);
float4x4	g_ShadowPlaneSpaceMatrix	: register(CCV_RENDER_SETTINGS);
float4		g_vViewPosition				: register(CCV_DISPLAY_VIEW_POSITION);

struct VS_INPUT
{
	float3	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	float4	position			: POSITION;
	float3	m_ShadowTexCoords	: TEXCOORD0; //z -> factor
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	float4	vWorld	= float4(input.m_Position, 1.0f);
	
	float3		vForward	= normalize(input.m_Position - g_vViewPosition.xyz);
	vWorld.xyz -= vForward * 0.5f; //small bias
	
	float4  vProjPos = mul(vWorld, g_mViewProj);
	
	output.position = vProjPos;
	
	
	//Calculate shadow texcoords
	float3 shadowSpacePosition = mul(float4(vWorld.xyz,1.f), g_ShadowPlaneSpaceMatrix);
	
	output.m_ShadowTexCoords.xy = shadowSpacePosition.xy * 0.5f + 0.5f;
	output.m_ShadowTexCoords.z = shadowSpacePosition.z;
	
	
	return output;
}
