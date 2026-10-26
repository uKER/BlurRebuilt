#include "../../graphics/display/include/display/shaders/vsCommon.h"
#include "../../graphics/display/include/display/shaders/vsConstants.h"

float4x4			g_mViewProj	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);

struct VS_INPUT
{
	float3	m_Position	: POSITION;
	float2	m_TexCoord	: TEXCOORD0;
	float3	m_Normal	: NORMAL;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float2	m_TexCoord	: TEXCOORD0;
	float3	m_Colour	: TEXCOORD1;
	float4	m_ScreenPos	: TEXCOORD2;
};

#ifndef PLATFORM_PS3
struct C_SHNode
{
	float4	m_vRedCoeffs;
	float4	m_vGreenCoeffs;
	float4	m_vBlueCoeffs;
};
#endif

float4x4	g_FlagTransform		: register(c200);
float4		g_FlagColour		: register(c204);
float4		g_LodFade			: register(c205);
float4		g_LodDistanceScale	: register(c206);
#ifdef PLATFORM_PS3
float3x4	g_FlagSH			: register(c207);
#else
C_SHNode	g_FlagSH			: register(c207);
#endif

#ifdef PLATFORM_PS3
void RGBSH(in float3x4 shNode, in half3 vNormal, inout half4 vColour)
{
	vColour.r *= dot(half4(vNormal, 1.0h), half4(shNode[0][0], shNode[0][1], shNode[0][2], shNode[0][3]));
	vColour.g *= dot(half4(vNormal, 1.0h), half4(shNode[1][0], shNode[1][1], shNode[1][2], shNode[1][3]));
	vColour.b *= dot(half4(vNormal, 1.0h), half4(shNode[2][0], shNode[2][1], shNode[2][2], shNode[2][3]));
	
	/*vColour.r *= dot(half4(vNormal, 1.0h), shNode[0]);
	vColour.g *= dot(half4(vNormal, 1.0h), shNode[1]);
	vColour.b *= dot(half4(vNormal, 1.0h), shNode[2]);*/
}
#else
void RGBSH(in C_SHNode shNode, in half3 vNormal, inout half4 vColour)
{
	vColour.r *= dot(half4(vNormal, 1.0h), shNode.m_vRedCoeffs);
	vColour.g *= dot(half4(vNormal, 1.0h), shNode.m_vGreenCoeffs);
	vColour.b *= dot(half4(vNormal, 1.0h), shNode.m_vBlueCoeffs);
}
#endif

float EvaluateLodFade(in float fPositionW)
{
	float2 r = g_LodFade.xy * fPositionW * g_LodDistanceScale.x + g_LodFade.zw;
	return min(r.x, r.y);
}

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	//Calculate world position
	float4 vWorldPosition	= mul(float4(input.m_Position.xyz, 1.0f), g_FlagTransform);
	float3 vWorldNormal		= mul(input.m_Normal.xyz, (float3x3)g_FlagTransform);
	
	//Calculate position
	float4  vProjPos		= mul(vWorldPosition, g_mViewProj);
	output.m_Position		= vProjPos;
	output.m_TexCoord		= input.m_TexCoord;
	
	half4 vColour	= half4(g_FlagColour.rgb, 1.0h);
	RGBSH(g_FlagSH, vWorldNormal.xyz, vColour);
	
	output.m_Colour.rgb		= vColour.xyz;
	output.m_ScreenPos		= float4(output.m_Position.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f) * output.m_Position.w, output.m_Position.w, 0.0f);
	output.m_ScreenPos.w	= EvaluateLodFade(output.m_Position.w);
	
	return output;
}
