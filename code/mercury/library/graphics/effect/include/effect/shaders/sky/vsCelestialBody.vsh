#include "../vsCommon.h"
#include "Effect/Shaders/Constants.h"

float4x4	g_mView			: register(CCV_DISPLAY_VIEW_MATRIX);
float4x4	g_mProj			: register(CCV_DISPLAY_PROJ_MATRIX);
float3		g_vViewPosition	: register(CCV_DISPLAY_VIEW_POSITION);
float4		g_vColour		: register(CCV_RENDER_SETTINGS);
float4		g_vSettings		: register(CCV_RENDER_SETTINGS_1);

#define		g_vWorldOffset	g_vSettings.xyz
#define		g_fSize			g_vSettings.w

struct VS_INPUT
{
	float2	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 links stages by register: interpolants from zero, SV_Position last.

	float2	m_TexCoord	: TEXCOORD0;
	float4	m_Colour	: TEXCOORD1;
	float4	m_Position	: SV_Position;
	#else
	float4	m_Position	: POSITION;
	float2	m_TexCoord	: TEXCOORD0;
	float4	m_Colour	: TEXCOORD1;
	#endif
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	float3	vWorldPosition	= g_vWorldOffset + g_vViewPosition;
	float4	vViewPosition	= mul(float4(vWorldPosition, 1.0f), g_mView);
	float2	vViewOffset		= (input.m_Position * 2.0f - 1.0f) * g_fSize;
			vViewPosition	= vViewPosition + float4(vViewOffset, 0.0f, 0.0f);
		
	output.m_Position	= mul(vViewPosition, g_mProj);
	output.m_TexCoord	= input.m_Position;
	output.m_Colour		= g_vColour;
	
	return output;
}
