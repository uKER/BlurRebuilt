#include "..\\vsCommon.h"

//float4 g_ClipVector			: register(CCV_CLIP_VECTOR);

struct VS_INPUT
{
	float2 Position			: POSITION;
};

struct VS_OUTPUT
{
	float4 Position			: POSITION;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	//OUT.Position = TransformPosition2D(IN.Position, g_ClipVector);
	OUT.Position = float4((IN.Position * float2(0.002, 0.002)) + float2(0, 0), 0.f, 1.f);

	return OUT;
}