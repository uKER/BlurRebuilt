#include "..\\vsConstants.h"
#include "..\\vsCommon.h"

#if defined(BLUR_DX11) // [Blur1PCDX11] Native SM5 constant-buffer inputs.
cbuffer LegacyVertexConstants : register(b0)
{
	float4x4 g_WorldMatrix : packoffset(CCV_DISPLAY_WORLD_MATRIX);
	float4x4 g_ViewProjMatrix : packoffset(CCV_DISPLAY_VIEWPROJ_MATRIX);
}
#else
float4x4 g_WorldMatrix		: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4 g_ViewProjMatrix	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
#endif

struct VS_INPUT
{
	float3 Position			: POSITION;
	float4 Colour0			: COLOR0;
};

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] Match the SM5 pixel shader signature.
	float4 Colour0			: TEXCOORD0;
	float4 Position			: SV_Position;
	#else
	float4 Position			: POSITION;
	float4 Colour0			: COLOR0;
	#endif
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	float4 position = float4(IN.Position, 1.f);
	
	position = mul(position, g_WorldMatrix);
	position = mul(position, g_ViewProjMatrix);
	
	OUT.Position = position;
	
	OUT.Colour0 = IN.Colour0;
	
	return OUT;
}
