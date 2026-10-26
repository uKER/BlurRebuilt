#include "..\vsConstants.h"
#include "..\vsCommon.h"

float4 g_ClipVector			: register(CCV_DISPLAY_CLIP_VECTOR);

struct VS_INPUT
{
	float2 Position			: POSITION;
	float4 Color0			: COLOR0;
};

struct VS_OUTPUT
{
	float4 Position			: POSITION;
	float4 Color0			: COLOR0;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	OUT.Position = TransformPosition2D(IN.Position, g_ClipVector);	
	OUT.Color0 = IN.Color0;
	
	return OUT;
}