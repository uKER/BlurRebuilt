
#include "Common.h"
#include "vsCommon.h"


VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4 worldPosition = float4(IN.Position.xyz,1.0f);
	
	//remove offset - position == leaf card centre
	worldPosition.xy -= (IN.Texcoord1.xy);
	worldPosition = mul(worldPosition, g_WorldMatrix);
	
	// Calculate right and up vectors.
	float3 viewVector		= g_WorldViewPosition.xyz - worldPosition.xyz;
	float3 viewForward      = normalize(viewVector);
	
	float3 worldUp       = float3(0.f, 1.f, 0.f);
	float3 rightVector  = normalize(cross(viewForward, worldUp));
	float3 upVector     = cross(rightVector, viewForward);
	
	worldPosition.xyz += rightVector * IN.Texcoord1.x;
	worldPosition.xyz += upVector * IN.Texcoord1.y;

	float3 worldNormal = viewVector;//mul(IN.Normal, (float3x3)g_WorldInvTransMatrix/*g_WorldInvTransMatrix*/);
	worldNormal = normalize(worldNormal);

	OUT.Position = mul(worldPosition, g_ViewProjMatrix);
#ifdef COLOUR	
	OUT.Colour = IN.Colour;
#else
	OUT.Colour = float4(1.f, 1.f, 1.f, 1.f);
#endif
	
	OUT.Texcoord0.xy = IN.Texcoord0;

	/*
	float4 lightPosition = float4(g_WorldViewPosition, 1.f);
	float4 lightVector;
	OUT.Texcoord1.xyz = Lambert(worldPosition, worldNormal, lightPosition, lightVector);
	*/
	float3 lightVector = normalize(g_WorldViewPosition - worldPosition.xyz);
	OUT.Texcoord1.xyz = saturate(abs(dot(worldNormal, lightVector))).xxx;
	OUT.Texcoord1.xyz += 0.1f;
	
	OUT.Texcoord2.xyz = normalize(viewVector);
	OUT.Texcoord2.w = 1.f; 
	return(OUT);
}