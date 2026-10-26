#include "..\vsCommon.h"

float4 g_ClipTransform			: register(CCV_DISPLAY_CLIP_VECTOR);

struct VS_INPUT
{
	float3 Position				: POSITION;
	float4 Colour				: COLOR;
	float2 TexCoord0			: TEXCOORD0;
	float2 TexCoord1			: TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 Position				: POSITION;
	float4 Colour				: COLOR;
	float2 TexCoord0			: TEXCOORD0;
	float2 TexCoord1			: TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	OUT.Position = TransformPosition2D(IN.Position.xy, g_ClipTransform);
    
    #if defined(PS3)
    OUT.Colour = IN.Colour.yzwx;
    #else
    OUT.Colour = IN.Colour.xyzw;
    #endif
    
    OUT.TexCoord0 = IN.TexCoord0;
    OUT.TexCoord1 = IN.TexCoord1;
	
	return(OUT);
}