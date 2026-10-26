#include "dynamic\shaders\vsCommon.h"
#include "dynamic\shaders\constants.h"

float4x4 g_mViewProj			: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4 g_mView				: register(CCV_DISPLAY_VIEW_MATRIX);
float4	 g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);

float4	 g_vFxParams			: register(CCV_RENDER_SETTINGS);



struct VS_INPUT
{
#if defined(VS_2D)
	float2 position	: POSITION0;
#elif defined(VS_3D)
	float3 position : POSITION0;
#endif
	float4 colour	: COLOR0;
	float3 uvw		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position			: POSITION0;
	float4 colour			: COLOR0;
	float3 uvw				: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

#if defined(VS_2D)
	output.position = float4(input.position.xy, 0.0f, 1.0f);
#elif defined(VS_3D)	
	output.position = mul(float4(input.position, 1.0f), g_mViewProj);
#endif
	output.colour = input.colour;
	output.uvw = input.uvw;
			
    return output;
}
