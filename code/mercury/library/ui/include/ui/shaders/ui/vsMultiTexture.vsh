#include "..\vsCommon.h"

float4x4	g_UiTransform		: register( CCV_UI_TRANSFORM );

struct VS_INPUT
{
	float4 Position				: POSITION;
	float4 Colour				: COLOR;
	float2 TexCoord0			: TEXCOORD0;
	
#if defined( SAMPLERS_2 ) || defined( SAMPLERS_3 )
	float2 TexCoord1			: TEXCOORD1;
#endif
#if defined( SAMPLERS_3 )
	float2 TexCoord2			: TEXCOORD2;
#endif
};

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 links stages by register: interpolants from zero, SV_Position last.
	float4 Colour				: COLOR0;
	float4 MyWorldPosition		: TEXCOORD3;
	float2 TexCoord0			: TEXCOORD0;

#if defined( SAMPLERS_2 ) || defined( SAMPLERS_3 )
	float2 TexCoord1			: TEXCOORD1;
#endif
#if defined( SAMPLERS_3 )
	float2 TexCoord2			: TEXCOORD2;
#endif
	float4 Position				: SV_Position;
	#else
	float4 Position				: POSITION;
	float4 Colour				: COLOR0;
	float4 MyWorldPosition		: TEXCOORD3;
	float2 TexCoord0			: TEXCOORD0;

#if defined( SAMPLERS_2 ) || defined( SAMPLERS_3 )
	float2 TexCoord1			: TEXCOORD1;
#endif
#if defined( SAMPLERS_3 )
	float2 TexCoord2			: TEXCOORD2;
#endif
	#endif
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	OUT.MyWorldPosition = IN.Position;
	OUT.Position = mul( float4( IN.Position.xyz, 1.0f ), g_UiTransform );

#if defined(PS3)
    OUT.Colour = IN.Colour.yzwx;
#else
    OUT.Colour = IN.Colour.xyzw;
#endif
    
    OUT.TexCoord0 = IN.TexCoord0;
    
#if defined( SAMPLERS_2 ) || defined( SAMPLERS_3 )
    OUT.TexCoord1 = IN.TexCoord1;
#endif

#if defined( SAMPLERS_3 )
    OUT.TexCoord2 = IN.TexCoord2;
#endif
	
	return(OUT);
}
