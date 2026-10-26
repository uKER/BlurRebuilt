#include "..\vsCommon.h"

float4x4	g_UiTransform		: register( CCV_UI_TRANSFORM );

struct VS_INPUT
{
	float3 Position				: POSITION;
	float4 Colour				: COLOR;
	float2 TexCoord0			: TEXCOORD0;
	float2 TexCoord1			: TEXCOORD1;
	float2 TexCoord2			: TEXCOORD2;
};

struct VS_OUTPUT
{
	float4 Position				: POSITION;
	float4 Colour				: COLOR;
	float2 TexCoord0			: TEXCOORD0;
	float2 TexCoord1			: TEXCOORD1;
	float2 TexCoord2			: TEXCOORD2;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	OUT.Position = mul( float4( IN.Position.xyz, 1.0f ), g_UiTransform );
  
    #if defined(PS3)
    OUT.Colour = IN.Colour.yzwx;
    #else
    OUT.Colour = IN.Colour.xyzw;
    #endif
    
    OUT.TexCoord0 = IN.TexCoord0;
    OUT.TexCoord1 = IN.TexCoord1;
	OUT.TexCoord2 = OUT.Position.xy;
 	
	return(OUT);
}
