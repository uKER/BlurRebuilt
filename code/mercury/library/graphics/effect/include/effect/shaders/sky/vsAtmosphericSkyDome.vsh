#include "Effect/Shaders/Constants.h"
#include "../vsCommon.h"

float4x4	g_mViewProj		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4		g_vViewPosition	: register(CCV_DISPLAY_VIEW_POSITION);
float4		g_vSettings		: register(CCV_RENDER_SETTINGS);

#define		g_fSize			g_vSettings.x
#define		g_fHeight		g_vSettings.y
#define		g_fOffset		g_vSettings.z

struct VS_INPUT
{
	float3	m_Position		: POSITION;
	float2	m_TexCoord		: TEXCOORD0;
};

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 links stages by register: interpolants from zero, SV_Position last.

	float2	m_TexCoord		: TEXCOORD0;
	float3	m_ViewVector	: TEXCOORD1;
	float4	m_Position		: SV_Position;
	#else
	float4	m_Position		: POSITION;
	float2	m_TexCoord		: TEXCOORD0;
	float3	m_ViewVector	: TEXCOORD1;
	#endif
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	float3	vObjectPosition	= input.m_Position;
	vObjectPosition.xyz	*= g_fSize;
	vObjectPosition.y	*= g_fHeight;
	vObjectPosition.y	+= g_fSize * g_fOffset;

	float4	vWorldPosition = float4(vObjectPosition.xyz + g_vViewPosition.xyz, 1.0f);
	output.m_Position	= mul(vWorldPosition, g_mViewProj);
	output.m_TexCoord	= input.m_TexCoord;
	output.m_ViewVector	= normalize(g_vViewPosition.xyz - vWorldPosition.xyz);
	
	return output;
}
