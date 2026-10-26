#include "..\\vsConstants.h"
#include "..\\vsCommon.h"

#if defined(BLUR_DX11) // [Blur1PCDX11] SM5 parameters live in reflected constant buffers.
cbuffer LegacyVertexConstants : register(b0)
{
	float4 g_ClipVector : packoffset(CCV_DISPLAY_CLIP_VECTOR);
}
#else
float4 g_ClipVector			: register(CCV_DISPLAY_CLIP_VECTOR);
#endif

struct VS_INPUT
{
	float2 Position			: POSITION;
	float4 Colour0			: COLOR0;
};

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 position semantic.
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
	
	OUT.Position = TransformPosition2D(IN.Position, g_ClipVector);
	OUT.Colour0 = IN.Colour0;
	
	return OUT;
}
