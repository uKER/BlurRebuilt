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
	float3 normal	: NORMAL0;
};

struct VS_OUTPUT
{
	float4 position			: POSITION0;
	float4 colour			: COLOR0;
	float2 uv				: TEXCOORD0;
	float3 world			: TEXCOORD1;
	float3 normal			: TEXCOORD2;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	float4 position = mul(float4(input.position, 1.0f), g_mWorld);
	position.w = 1.0f;
	
	output.world = position.xyz;
	output.position = mul(position, g_mViewProj);
	output.colour = input.colour;
	output.uv = input.uv;
	output.normal = input.normal;//, g_mWorld);
			
    return output;
}



