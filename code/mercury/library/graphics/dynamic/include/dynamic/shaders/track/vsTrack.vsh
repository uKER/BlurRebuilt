#include "dynamic\shaders\vsCommon.h"
#include "dynamic\shaders\constants.h"

float4x4 g_mViewProj			: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4 g_mView				: register(CCV_DISPLAY_VIEW_MATRIX);
float4	 g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);

struct VS_INPUT
{
	float4 position : POSITION0;
	float4 colour	: TEXCOORD0;
	float2 uv		: TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 position	: POSITION0;
	float4 colour	: TEXCOORD0;
	float2 uv		: TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.position = mul(input.position, g_mViewProj);
	output.colour = input.colour;
	output.uv = input.uv;
			
    return output;
}
