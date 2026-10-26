#include "..\\vsConstants.h"
#include "..\\vsCommon.h"

float4 g_ClipVector			: register(CCV_DISPLAY_CLIP_VECTOR);

struct VS_INPUT
{
	float2 Position			: POSITION;
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
	
	OUT.Position = TransformPosition2D(IN.Position, g_ClipVector);
	OUT.Colour0 = IN.Colour0;
	OUT.Texcoord0.xy = IN.Texcoord0;
	
	return OUT;
}