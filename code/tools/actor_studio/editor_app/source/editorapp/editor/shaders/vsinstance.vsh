
#include "Common.h"
#include "..\..\..\..\..\..\aftkrender\aftkrender\shaders\vsCommon.h"

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4x4 worldMatrix;
	worldMatrix[0] = IN.Texcoord1;
	worldMatrix[1] = IN.Texcoord2;
	worldMatrix[2] = IN.Texcoord3;
	worldMatrix[3] = IN.Texcoord4;
	
	float4 worldPosition = mul(float4(IN.Position.xyz,1.0f), worldMatrix);

	OUT.Position = mul(worldPosition, g_ViewProjMatrix);
	OUT.Colour0 = IN.Colour;

#ifdef SOLID
	OUT.Texcoord0 = IN.Texcoord0;

	float3 worldNormal = mul(IN.Normal.xyz, (float3x3)worldMatrix);	//we're assuming uniform scaling
	worldNormal = normalize(worldNormal);

	float4 lightPosition = float4(g_WorldViewPosition, 1.f);
	float3 lightVector;
	OUT.Colour1.xyz = Lambert(worldPosition, worldNormal, lightPosition, lightVector);
	OUT.Colour1.xyz += 0.1f;
	OUT.Colour1.w = 1.f;
#endif	
	return(OUT);
}