#include "../vsCommon.h"
#include "../vsConstants.h"

float4		g_vSettings			: register(CCV_RENDER_SETTINGS);
sampler2D	g_tTiles			: register(s0);

#define		g_vPixelSize		g_vSettings.xy
#define		g_fTileSize			g_vSettings.z
#define		g_fReferenceValue	g_vSettings.w


struct VS_INPUT
{
	float2	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float4	m_TexCoord	: TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

#ifndef VS_2_0

#if defined(PS3)
	float2 uv = input.m_Position * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
#else	
	float2 uv = input.m_Position * float2(0.5f, -0.5f) + float2(0.5f, 0.5f) + 0.5f * g_vPixelSize;
#endif
	
	float fDepth		= tex2Dlod(g_tTiles, float4(uv, 0.0f, 0.0f)).r;
	float fCompare		= saturate(abs(fDepth - g_fReferenceValue));
	
	output.m_Position	= float4(input.m_Position + 100.0f * fCompare, 0.0f, 1.0f);
	output.m_TexCoord	= float4(uv, g_vPixelSize * g_fTileSize);	

#endif
	
	return output;
}
