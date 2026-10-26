
#include "Common.h"

float4x4 g_WorldMatrix			: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4 g_ViewProjMatrix		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4 g_ViewMatrix 			: register(CCV_DISPLAY_VIEW_MATRIX);
float4x4 g_ProjMatrix 			: register(CCV_DISPLAY_PROJ_MATRIX);
float3   g_WorldViewPosition	: register(CCV_DISPLAY_VIEW_POSITION);


VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	float4 worldPosition = float4(IN.Position.xyz,1.0f);

	//float3 viewVector		= normalize(g_WorldViewPosition.xyz - worldPosition.xyz);
	float3 viewVector		= normalize(float3(g_ViewMatrix[0][2], g_ViewMatrix[1][2], g_ViewMatrix[2][2]));
	float3 upVector			= normalize(float3(g_ViewMatrix[0][1], g_ViewMatrix[1][1], g_ViewMatrix[2][1]));
	float3 rightVector		= normalize(cross(viewVector, upVector));
	
	worldPosition.xyz += IN.Texcoord0.x*rightVector;
	worldPosition.xyz += IN.Texcoord0.y*upVector;
	
	OUT.Position = mul(worldPosition, g_ViewProjMatrix);
	OUT.Texcoord0.xy = IN.Texcoord0.zw;

#ifdef DEPTHBIAS
	float4 biasedPosition = float4(worldPosition.xyz - 2.f*viewVector, 1.f);
	biasedPosition = mul(biasedPosition, g_ViewProjMatrix);
	
	OUT.Depth = saturate(biasedPosition.z/biasedPosition.w);
#endif
	return(OUT);
}