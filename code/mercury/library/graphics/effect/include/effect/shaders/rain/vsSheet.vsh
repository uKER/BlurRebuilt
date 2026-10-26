#include "Effect/Shaders/Constants.h"
#include "..\vsCommon.h"

float4x4 g_mWorldViewProjOrigin	: register(CCV_RENDER_VIEWPROJORIGIN_MATRIX);

float4	g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);
float4	g_vColourScale			: register(CCV_EFFECT_RAIN_COLOURSCALE);
float4	g_vUVScale				: register(CCV_EFFECT_RAIN_SIZESCALE);

float4	g_vHeight				: register(CCV_EFFECT_RAIN_UP);
float4	g_vRight				: register(CCV_EFFECT_RAIN_RIGHT);

struct VS_OUTPUT
{
    float4 position		: POSITION;		//Pixel position (world-view-proj space)
    float4 uv0			: TEXCOORD0;
    float4 uv1			: TEXCOORD1;
    float4 colour		: COLOR0;		//Diffuse texture
};

struct VS_INPUT
{
	float4 position		: POSITION;
	float2 uv			: TEXCOORD0;
	float4 colour		: COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	
	input.position.w = 1.0f;
	
	output.position = mul(input.position, g_mWorldViewProjOrigin);
	output.colour = input.colour;
	output.colour.rgb *= g_vColourScale.rgb;
	output.colour.a = pow(output.colour.a, g_vColourScale.a);
	
	output.uv0.xy = input.uv * g_vUVScale.xy + float2(g_vRight.x, g_vHeight.x);
	output.uv0.zw = input.uv * g_vUVScale.xy + float2(g_vRight.y, g_vHeight.y);
	output.uv1.xy = input.uv * g_vUVScale.xy + float2(g_vRight.z, g_vHeight.z);
	output.uv1.zw = input.uv * g_vUVScale.xy + float2(g_vRight.w, g_vHeight.w);
	
	return output;
}


