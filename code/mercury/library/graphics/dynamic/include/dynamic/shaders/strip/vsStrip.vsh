#include "dynamic\shaders\vsCommon.h"
#include "dynamic\shaders\constants.h"

float4x4 g_mWorld				: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4 g_mViewProj			: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4 g_mView				: register(CCV_DISPLAY_VIEW_MATRIX);
float4	 g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);
float4	 g_vLightDirection		: register(CCV_RENDER_LIGHT_DIRECTION);

struct VS_INPUT
{
	float3 position : POSITION0;
	float4 colour	: TEXCOORD0;
	float2 uv		: TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 position			: POSITION0;
	float4 colour			: TEXCOORD0;
	float2 uv				: TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	float4 position = float4(input.position, 1.0f);
	
	output.position = mul(position, g_mViewProj);
	output.colour = input.colour;
	output.uv = input.uv;
			
    return output;
}



