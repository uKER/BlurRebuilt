#include "..\vsCommon.h"

float4x4	g_UiTransform						: register( CCV_UI_TRANSFORM );

float4x4	g_PolygonTransform					: register( CCV_POLYGON_TRANSFORM );
float4		g_PolygonColour						: register( CCV_POLYGON_COLOUR );
float4		g_TextureVector						: register( CCV_POLYGON_TEXTURE_VECTOR );

struct VS_INPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] Numerically convert PC SHORT2 fetches.
	int2 Position                : POSITION;
	#else
	float2 Position				: POSITION;
	#endif
	#if defined(BLUR_DX11) // [Blur1PCDX11] The PC declaration also packs UV as SHORT2.
	int2 TexCoord0               : TEXCOORD0;
	#else
	float2 TexCoord0			: TEXCOORD0;
	#endif
};

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 links stages by register: interpolants from zero, SV_Position last.
	float4 Colour				: COLOR0;
	float4 MyWorldPosition		: TEXCOORD3;
	float2 TexCoord0			: TEXCOORD0;
	float4 Position				: SV_Position;
	#else
	float4 Colour				: COLOR0;
	float4 MyWorldPosition		: TEXCOORD3;
	float4 Position				: POSITION;
	float2 TexCoord0			: TEXCOORD0;
	#endif
};


VS_OUTPUT main( VS_INPUT IN )
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float2 position = float2(IN.Position) * float2(1.0f / 32767.0f, 1.0f / 32767.0f);
	float2 texCoord = float2(IN.TexCoord0) * float2(1.0f / 32767.0f, 1.0f / 32767.0f);

	float4 newPos = float4( position.xy, 0.0f, 1.0f );
	float4 pos = mul( newPos, g_PolygonTransform );
	
	OUT.TexCoord0 = g_TextureVector.xy + (texCoord * g_TextureVector.zw);

	OUT.MyWorldPosition = pos;
	
	OUT.Position = mul( OUT.MyWorldPosition, g_UiTransform );
	
	OUT.Colour = g_PolygonColour;
	
	return OUT;
}