
#include "Common.h"
#include "vsCommon.h"

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4 worldPosition = mul(float4(IN.Position.xyz,1.0f), g_WorldMatrix);

#ifdef NORM
	float3 worldNormal = mul(IN.Normal.xyz, (float3x3)g_WorldMatrix/*g_WorldInvTransMatrix*/);
	worldNormal = normalize(worldNormal);
	OUT.Texcoord2.xyz = worldNormal;
	OUT.Texcoord2.w = 1.f;
#else
	OUT.Texcoord2.xyz = float4(1.f, 1.f, 1.f, 1.f);
#endif

	OUT.Position = mul(worldPosition, g_ViewProjMatrix);
	
#ifdef COLOUR	
	OUT.Colour = IN.Colour;
#else
	OUT.Colour = float4(1.f, 1.f, 1.f, 1.f);
#endif

#ifdef UV
	OUT.Texcoord0.xy = IN.Texcoord0;
#endif

#ifdef NORM
	/*
	float4 lightPosition = float4(g_WorldViewPosition, 1.f);
	float4 lightVector;
	OUT.Texcoord1.xyz = Lambert(worldPosition, worldNormal, lightPosition, lightVector);
	*/
	float3 lightVector = normalize(g_WorldViewPosition - worldPosition.xyz);
	OUT.Texcoord1.xyz = saturate(abs(dot(worldNormal, lightVector))).xxx;
	
	OUT.Texcoord1.xyz += 0.1f;
	OUT.Texcoord1.w = 1.f;
#else
	OUT.Texcoord1.w = float4(1.f, 1.f, 1.f, 1.f);
#endif
	return(OUT);
}