#include "dynamic\shaders\vsCommon.h"
#include "dynamic\shaders\constants.h"

float4x4	g_mWorld		: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4	g_mViewProj		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4		g_vViewPosition	: register(CCV_DISPLAY_VIEW_POSITION);
float4x4	g_mInvWorld		: register(CCV_RENDER_SETTINGS);
float4		g_vFogSettings	: register(CCV_RENDER_SETTINGS_4);
float4		g_vFogColour	: register(CCV_RENDER_SETTINGS_5);

struct VS_INPUT
{
	float4	m_Position			: POSITION;
};

struct VS_OUTPUT
{
	float4	m_Position			: POSITION;
	float4	m_FogColour			: COLOR0;
	float4	m_ProjPosition		: TEXCOORD0;
	float3	m_EntryPosition		: TEXCOORD1;
	float3	m_ViewVector		: TEXCOORD2;
	float3	m_FogSettings		: TEXCOORD3;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	float3	vObjectPosition	= input.m_Position.xyz;
	float4	vWorldPosition	= mul(input.m_Position, g_mWorld);
#if defined(FRONT_FACES)
	float3	vWorldViewVec	= vWorldPosition.xyz - g_vViewPosition.xyz;
#elif defined(BACK_FACES)
	float3	vWorldViewVec	= g_vViewPosition.xyz - vWorldPosition.xyz;
#endif	
	float3	vObjectViewVec	= mul(vWorldViewVec, (float3x3) g_mInvWorld);

	float	fCDotM			= dot(vObjectPosition, vObjectViewVec);
	float	fM2				= dot(vObjectViewVec, vObjectViewVec);
	float	fLambda			= max(0.0f, -2.0f * fCDotM / fM2);

	output.m_Position		= mul(vWorldPosition, g_mViewProj);
	output.m_FogColour		= g_vFogColour;
	output.m_ProjPosition	= output.m_Position;
#if defined(FRONT_FACES)
	output.m_EntryPosition	= vWorldPosition.xyz;
	output.m_ViewVector		= fLambda * vWorldViewVec;
#elif defined(BACK_FACES)
	output.m_EntryPosition	= (g_vFogSettings.z < 0.0f) ? vWorldPosition.xyz + fLambda * vWorldViewVec : g_vViewPosition.xyz;
	output.m_ViewVector		= vWorldPosition.xyz - output.m_EntryPosition;
#endif
	output.m_FogSettings	= float3(g_vFogSettings.xy, exp(-g_vFogSettings.y * output.m_EntryPosition.y));

	return output;
}
