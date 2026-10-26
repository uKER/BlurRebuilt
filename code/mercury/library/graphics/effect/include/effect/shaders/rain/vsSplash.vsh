#include "Effect/Shaders/Constants.h"
#include "..\vsCommon.h"

float4x4 g_mWorldViewProj		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);

float4	 g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);
float4   g_vColourScale			: register(CCV_EFFECT_RAIN_COLOURSCALE);
float4	 g_vLighting			: register(CCV_EFFECT_RAIN_LIGHTING);

struct VS_OUTPUT
{
    float4 position		: POSITION;		//Pixel position (world-view-proj space)
    float2 uv			: TEXCOORD0;
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
	
	output.uv = input.uv;
	input.position.w = 1.0f;
	
	float colourScale = 1.0f - saturate(length(g_vViewPosition.xyz - input.position.xyz) / g_vColourScale.x);
	output.colour = input.colour * colourScale * g_vLighting;
	output.uv = input.uv;
	
	// work out position of rain drop
	output.position = mul(input.position, g_mWorldViewProj);
	
	return output;
}


