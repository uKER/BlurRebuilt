#include "Effect/Shaders/Constants.h"
#include "../vsCommon.h"

float4x4	g_mViewProj		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4		g_vViewPosition	: register(CCV_DISPLAY_VIEW_POSITION);
float4		g_vSettings		: register(CCV_RENDER_SETTINGS);
float4		g_vSkySettings	: register(CCV_RENDER_SETTINGS_1);
float4		g_vFogSettings	: register(CCV_RENDER_SETTINGS_2);

#define		g_fSize			g_vSettings.x
#define		g_fHeight		g_vSettings.y
#define		g_fOffset		g_vSettings.z

#define		g_fRotation		g_vSkySettings.x
#define		g_fIntensity	g_vSkySettings.y
#define		g_fBlackHeight	g_vSkySettings.z
#define		g_fBlackSize	g_vSkySettings.w

#define		g_fFogAlpha		g_vFogSettings.x
#define		g_fFogTop		g_vFogSettings.y
#define		g_fFogPower		g_vFogSettings.z

struct VS_INPUT
{
	float3	m_Position	: POSITION;
	float2	m_TexCoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 links stages by register: interpolants from zero, SV_Position last.

#if defined(PLATFORM_PS3)
	float4  m_TexCoord  : TEXCOORD0;
#else
	float4	m_TexCoord	: COLOR0;
#endif
	float4	m_Position	: SV_Position;
	#else
	float4	m_Position	: POSITION;
#if defined(PLATFORM_PS3)
	float4  m_TexCoord  : TEXCOORD0;
#else
	float4	m_TexCoord	: COLOR0;
#endif
	#endif
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	float3	vObjectPosition	= input.m_Position;
	vObjectPosition.xyz		*= g_fSize;
	vObjectPosition.y		*= g_fHeight;
	vObjectPosition.y		+= g_fSize * g_fOffset;

	float4	vWorldPosition	= float4(vObjectPosition.xyz + g_vViewPosition.xyz, 1.0f);
	output.m_Position		= mul(vWorldPosition, g_mViewProj);
	output.m_TexCoord.xy	= input.m_TexCoord.xy + float2(g_fRotation, 0.0f);	
	float   fFogIntensity	= saturate((output.m_TexCoord.y + g_fFogTop - 1.0f) / g_fFogTop);
			fFogIntensity	= pow(fFogIntensity, g_fFogPower);
			fFogIntensity	= fFogIntensity * g_fFogAlpha;
	output.m_TexCoord.z		= fFogIntensity;
	output.m_TexCoord.w		= saturate((g_fBlackHeight - output.m_TexCoord.y) * g_fBlackSize);
	return output;
}
