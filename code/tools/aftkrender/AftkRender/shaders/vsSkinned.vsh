
#include "Common.h"
#include "vsCommon.h"


VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4 position = float4(IN.Position.xyz,1.0f);
	float4 skinnedPosition = (0.0f, 0.0f, 0.0f, 0.0f);
	skinnedPosition += IN.Blendweight[0] * mul(position, g_MatrixPalette[IN.Blendindices[0]]);
	skinnedPosition += IN.Blendweight[1] * mul(position, g_MatrixPalette[IN.Blendindices[1]]);
	
	float4 normal = float4(IN.Normal.xyz, 1.0f);
	float4 skinnedNormal = (0.0f, 0.0f, 0.0f, 0.0f);
	skinnedNormal += IN.Blendweight[0] * mul(position, g_MatrixPalette[IN.Blendindices[0]]);
	skinnedNormal += IN.Blendweight[1] * mul(position, g_MatrixPalette[IN.Blendindices[1]]);
	skinnedNormal = normalize(skinnedNormal);
	
	OUT.Position = mul(skinnedPosition, g_WorldMatrix);
	OUT.Position = mul(skinnedPosition, g_ViewProjMatrix);

#ifdef COLOUR	
	OUT.Colour = IN.Colour;
#else
	OUT.Colour = float4(1.f, 1.f, 1.f, 1.f);
#endif

#ifdef UV
	OUT.Texcoord0.xy = IN.Texcoord0;
#endif
	/*
	float4 lightPosition = float4(g_WorldViewPosition, 1.f);
	float4 lightVector;
	OUT.Texcoord1.xyz = Lambert(skinnedPosition,skinnedNormal, lightPosition, lightVector);
	*/
	float3 lightVector = normalize(g_WorldViewPosition - skinnedPosition.xyz);
	OUT.Texcoord1.xyz = saturate(abs(dot(skinnedNormal, lightVector))).xxx;	
	OUT.Texcoord1.xyz += 0.1f;
	
	return(OUT);
}