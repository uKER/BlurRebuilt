#include "..\\vsConstants.h"
#include "..\\vsCommon.h"

float4x4 g_WorldMatrix		: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4 g_ViewProjMatrix	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);

struct VS_INPUT
{
	float3 Position			: POSITION;
	float4 Colour0			: COLOR0;
	float2 Texcoord0		: TEXCOORD0;
};

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 links stages by register: interpolants from zero, SV_Position last.
	float4 Colour0			: COLOR0;
	float4 Texcoord0		: TEXCOORD0;
	float4 Position			: SV_Position;
	#else
	float4 Position			: POSITION;
	float4 Colour0			: COLOR0;
	float4 Texcoord0		: TEXCOORD0;
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
	
	OUT.Texcoord0.xy = IN.Texcoord0;
	
	return OUT;
}