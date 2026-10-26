#include "..\\vsConstants.h"
#include "..\\vsCommon.h"

float4x4 g_WorldMatrix			: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4 g_ViewProjMatrix		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);

struct VS_INPUT
{
	float3 Position				: POSITION;
};

struct VS_OUTPUT
{
	float4 Position				: POSITION;
	//#ifndef XB2
	float2 Texcoord0			: TEXCOORD0;
	//#endif
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	float4 objectPosition = float4(IN.Position, 1.f);
	float4 worldPosition = mul(objectPosition, g_WorldMatrix);
	OUT.Position = mul(worldPosition, g_ViewProjMatrix);
	//#ifndef XB2
	OUT.Texcoord0 = OUT.Position.zw;
	//#endif
	
	//OUT.Texcoord0.x = OUT.Position.w;
	//OUT.Texcoord0.y = 10;
	
	return(OUT);
}