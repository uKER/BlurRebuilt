/*---------------------------------------------------------------------------------------
	File	: vsPolygonTrail.vsh

	Desc	: 
---------------------------------------------------------------------------------------*/

#include "..\vsCommon.h"

float4x4	g_UiTransform						: register( CCV_UI_TRANSFORM );
float4x4	g_PolygonTransform					: register( CCV_POLYGON_TRANSFORM );

float4		g_PolygonColour						: register( CCV_POLYGON_COLOUR );
float4		g_TextureVector						: register( CCV_POLYGON_TEXTURE_VECTOR );
float4		g_WaveSettings						: register( CCV_POLYGON_WAVE );

// For usability.
#define		g_Time								g_WaveSettings.x 
#define		g_Intensity							g_WaveSettings.y 
#define		g_Frequency							g_WaveSettings.z 

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
	float4 Normal				: NORMAL;
};


struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 links stages by register: interpolants from zero, SV_Position last.
	float4	Colour				: COLOR0;
	float2	TexCoord0			: TEXCOORD0;
	float4	Position			: SV_Position;
	#else
	float4	Position			: POSITION;
	float4	Colour				: COLOR0;
	float2	TexCoord0			: TEXCOORD0;
	//float4	WorldPosition		: TEXCOORD0;
	#endif
};


VS_OUTPUT main( VS_INPUT IN )
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	float2 position = float2(IN.Position);
	float2 texCoord = float2(IN.TexCoord0);

	float4 pos = float4( position.xy, 0.0f, 1.0f );
	
	IN.Normal = normalize( IN.Normal );

	pos.x += ( sin( ( texCoord.x * g_Frequency ) + g_Time ) * g_Intensity ) * abs( IN.Normal.x );
	pos.y += ( sin( ( texCoord.x * g_Frequency ) + g_Time ) * g_Intensity ) * abs( IN.Normal.y );

	/*
	pos.y += sin( ( pos.x * g_Frequency ) + g_Time ) * g_Intensity;
	pos.y += sin( ( pos.x * g_Frequency * 0.3f ) + cos( g_Time ) ) * g_Intensity ;
	pos.y += sin( ( pos.x * cos( g_Frequency ) * 0.3f ) + g_Time + 3.2f ) * g_Intensity;
	*/

	// Pass to the pixel shader
	OUT.Position = mul( mul( pos, g_PolygonTransform ), g_UiTransform );
	OUT.TexCoord0 = g_TextureVector.xy + (texCoord * g_TextureVector.zw);
	OUT.Colour = g_PolygonColour;
	
	return OUT;
}