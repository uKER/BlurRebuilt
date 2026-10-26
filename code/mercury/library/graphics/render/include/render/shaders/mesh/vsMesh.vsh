#include "..\\vsConstants.h"
#include "..\\vsCommon.h"

float4x4	g_WorldMatrix		: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4	g_ViewProjMatrix	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
#if defined(USE_STANDARDSHADOWMAP)
float4x4	g_LightMatrix		: register(CCV_DISPLAY_LIGHT_MATRIX);
#endif
#if defined(USE_STANDARDSHADOWMAP) || defined(USE_VARIANCESHADOWMAP)
float4x4	g_ShadowMatrix		: register(CCV_DISPLAY_SHADOW_MATRIX);
#endif

struct VS_INPUT
{
	float3 Position				: POSITION;
	float3 Normal				: NORMAL;
	#ifdef USE_ALBEDO
	float2 Albedo				: TEXCOORD0;
	#endif
};

struct VS_OUTPUT
{
	float4 Position				: POSITION;
	float4 Normal				: TEXCOORD0;
	#ifdef USE_ALBEDO
	float4 Albedo				: TEXCOORD1;
	#endif
	#if defined(USE_STANDARDSHADOWMAP) || defined(USE_VARIANCESHADOWMAP)
	float4 Shadow				: TEXCOORD2;
	#endif
	#if defined(USE_STANDARDSHADOWMAP)
	float4 Depth				: TEXCOORD3;
	#endif
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	float4 objectPosition = float4(IN.Position, 1.f);
	float4 worldPosition = mul(objectPosition, g_WorldMatrix);
	OUT.Position = mul(worldPosition, g_ViewProjMatrix);

	float4 objectNormal = float4(IN.Normal, 1.f);
	float4 worldNormal = normalize(mul(objectNormal, g_WorldMatrix)); 
	OUT.Normal.xyz = worldNormal.xyz;
	
	#ifdef USE_ALBEDO
	OUT.Albedo.xy = IN.Albedo;
	#endif

	#ifdef USE_STANDARDSHADOWMAP
	OUT.Shadow = mul(worldPosition, g_ShadowMatrix);
	OUT.Depth.xy = mul(worldPosition, g_LightMatrix).zw;
	#endif
	
	#ifdef USE_VARIANCESHADOWMAP
	OUT.Shadow = mul(worldPosition, g_ShadowMatrix);
	OUT.Shadow.x = (+OUT.Shadow.x + OUT.Shadow.w) / 2.f;
    OUT.Shadow.y = (-OUT.Shadow.y + OUT.Shadow.w) / 2.f;
	#endif

	return OUT;
}