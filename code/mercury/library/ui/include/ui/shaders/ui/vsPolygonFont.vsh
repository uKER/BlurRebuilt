#include "..\vsCommon.h"

float4x4	g_UiTransform						: register( CCV_UI_TRANSFORM );

#ifdef _FINNED_
float4		g_UiViewPosition					: register( CCV_UI_VIEW_POSITION );
#endif

float4x4	g_PolygonTransform					: register( CCV_POLYGON_TRANSFORM );
float4		g_PolygonColour						: register( CCV_POLYGON_COLOUR );

float4		g_PolygonFontOutlineSettings		: register( CCV_POLYGONFONT_OUTLINE_SETTINGS );

#ifdef _GRADIENT_
float4x4	g_PolygonGradientColourTransform	: register( CCV_POLYGON_COLOUR_TRANSFORM );
#endif

#define		g_fPolygonFontHeight				g_PolygonFontOutlineSettings.x
#define		g_fOutlineSize						g_PolygonFontOutlineSettings.y
#define		g_fExtrudeSize						g_PolygonFontOutlineSettings.z
#define		g_fExtrudeSizeBevel					g_PolygonFontOutlineSettings.w

struct VS_INPUT
{
	float2 Position				: POSITION;
	float4 Vector				: COLOR0;
};

struct VS_OUTPUT
{
	float4 Colour				: COLOR;
	float4 MyWorldPosition		: TEXCOORD3;
#ifdef _TEXTURED_
	float2 TexCoord0			: TEXCOORD0;
#endif
	float4 Position				: POSITION;
};

#if defined(BLUR_DX11)
// [Blur2PC][Blur1PCDX11] The PC declaration is SHORT2/UBYTE4. D3D9
// converts these to float; native integer fetch requires a numeric cast.
struct VS_PACKED_INPUT
{
	int2 Position : POSITION;
	uint4 Vector : COLOR0;
};
VS_OUTPUT main( VS_PACKED_INPUT packed )
#else
VS_OUTPUT main( VS_INPUT IN )
#endif
{
#if defined(BLUR_DX11)
	VS_INPUT IN;
	IN.Position = float2(packed.Position);
	IN.Vector = float4(packed.Vector);
#endif
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	IN.Position = IN.Position * float2(1.0f / 32767.0f, 1.0f / 32767.0f);	
	IN.Vector.xy = (IN.Vector.xy - float2(127.0f, 127.0f)) * float2(1.0f / 127.0f, 1.0f / 127.0f);
	
#ifdef _TEXTURED_
	// generate tex coords
	OUT.TexCoord0.x = IN.Position.x / g_fPolygonFontHeight;
	OUT.TexCoord0.y = IN.Position.y / g_fPolygonFontHeight;
#endif
	
#ifdef _OUTLINE_FIXED_
	float4 newPos = float4( IN.Position.xy, IN.Vector.z * g_fExtrudeSize, 1.0f );
	float4 pos = mul( newPos, g_PolygonTransform );
	// transform the vector too
	float2 v = mul( float4( IN.Vector.xy, 0.0f, 1.0f ), g_PolygonTransform ).xy;
	v = normalize( v );
	pos.xy += ( v * g_fOutlineSize * length( IN.Vector.xy ) );

#elif defined( _FINNED_ )
	float4 newPos = float4( IN.Position.xy, 0.0f, 1.0f );
	float4 pos = mul( newPos, g_PolygonTransform );

	// **** THIS IS ALL TEMP
	// this is a bit ewww - push it down instead?
	// - or just make two shaders?
	bool is_2d = ( g_UiTransform[2][3] == 0.0f && g_UiTransform[3][3] == 1.0f );	
	
	float outline_size = 0.0f;
	
	if ( is_2d )
		outline_size = 1.0f;	// should this be sqrt(2) instead?
	else if ( !is_2d )
	{
		// this is fudge - figure it out properly?
		outline_size = length( g_UiViewPosition.xyz - pos.xyz ) * 0.0025f;
	}
	// *** END OF TEMP
	
	// transform the vector too
	float2 v = mul( float3( IN.Vector.xy, 0.0f ), (float4x3)g_PolygonTransform ).xy;
	v = normalize( v + 0.00001f ); // Temp bodge so we don't normalise a zero y component

#ifdef _FINNED_OUTER_
	v.x *= IN.Vector.w;
	v.y *= IN.Vector.w;
#endif

	pos.xy += ( v * outline_size * length( IN.Vector.xy ) );

#else
	float4 newPos = float4( IN.Position.xy + ( IN.Vector.xy * g_fOutlineSize ), 0.0f, 1.0f );
	
	if ( IN.Vector.z < -0.5f )
		newPos.z = ( IN.Vector.z + 0.5f ) * g_fExtrudeSize - g_fExtrudeSizeBevel;	// bevel
	else
		newPos.z = IN.Vector.z * g_fExtrudeSize;
	
	float4 pos = mul( newPos, g_PolygonTransform );
#endif
	
	OUT.Position = mul( pos, g_UiTransform );
	
#ifdef _GRADIENT_
	// calculate colour (positions in it's own 0-1 space)
	float4 p = mul( float4( IN.Position.xy, 0.0f, 1.0f ), g_PolygonTransform );
	OUT.Colour = mul( p, g_PolygonGradientColourTransform );
#else
	OUT.Colour = g_PolygonColour;
#endif

#ifdef _FINNED_
	OUT.Colour.a *= IN.Vector.z;
#endif
	
#ifdef _OUTLINE_FADE_
	if ( length( IN.Vector.xy ) != 0.0f )
		OUT.Colour.a = 0.0f;
#endif
	OUT.MyWorldPosition = pos;
	OUT.Position = mul( OUT.MyWorldPosition, g_UiTransform );

	return OUT;
}
