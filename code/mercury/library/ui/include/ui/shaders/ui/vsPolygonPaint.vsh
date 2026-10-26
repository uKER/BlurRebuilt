/*---------------------------------------------------------------------------------------
	File	: vsPolygonPaint.vsh

	Desc	: 
---------------------------------------------------------------------------------------*/

#include "..\vsCommon.h"

float4x4	g_UiTransform						: register( CCV_UI_TRANSFORM );
float4x4	g_PolygonTransform					: register( CCV_POLYGON_TRANSFORM );

struct VS_INPUT
{
	float4 Position				: POSITION;
	float4 Colour				: COLOR;
	float2 TexCoord0			: TEXCOORD0;
};


struct VS_OUTPUT
{
	float4	Position			: POSITION;
	float4	Colour				: COLOR0;
	float2	TexCoord0			: TEXCOORD0;
};

VS_OUTPUT main( VS_INPUT IN )
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = mul( IN.Position, g_PolygonTransform );
	OUT.TexCoord0 = IN.TexCoord0;
	OUT.Colour = IN.Colour;
	
	return OUT;
}