#include "vsConstants.h"


float4x4 g_WorldMatrix			: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4 g_ViewProjMatrix		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4 g_WorldInvTransMatrix  : register(CCV_AFTKRENDER_WORLD_INV_TRANS_MATRIX);
float4x4 g_NormalMatrix			: register(CCV_SPEEDTREE_BILLBOARD_WORLDVIEW_INV_TRANS_MATRIX);

float3   g_WorldViewPosition	: register(CCV_DISPLAY_VIEW_POSITION);


struct VS_INPUT
{
	float3 Position				: POSITION;
	float3 Normal				: NORMAL;

	float2 Texcoord0			: TEXCOORD0;
	float2 Texcoord1			: TEXCOORD1;
	
#ifdef NORMAL_MAP
	float3 Tangent				: TANGENT;
	float3 Binormal				: BINORMAL;
#endif
};


struct VS_OUTPUT
{
	float4 Position				: POSITION;
	float2 Texcoord0			: TEXCOORD0;

#ifdef GEOMETRIC_NORMAL
	float3 Normal				: TEXCOORD1;
#endif

#ifdef NORMAL_MAP
	float3 Normal				: TEXCOORD1;
	float3 Tangent				: TEXCOORD2;
	float3 Binormal				: TEXCOORD3;
#endif
};



VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

#ifdef BILLBOARD
	float4 worldPosition = float4(IN.Position.xyz,1.0f);
	
	//remove offset - position == leaf card centre
	worldPosition.xy -= (IN.Texcoord1.xy);
	worldPosition = mul(worldPosition, g_WorldMatrix);
	
	// Calculate right and up vectors.
	float3 viewVector		= g_WorldViewPosition.xyz - worldPosition.xyz;
	float3 viewForward      = normalize(viewVector);
	
	float3 worldUp       = float3(0.0f, 1.0f, 0.0f);
	float3 rightVector  = normalize(cross(viewForward, worldUp));
	float3 upVector     = cross(rightVector, viewForward);
	
	float3 centrePosition = worldPosition.xyz;
	worldPosition.xyz -= rightVector * IN.Texcoord1.x;
	worldPosition.xyz -= upVector * IN.Texcoord1.y;
	
	float3 viewNormal = float3(0.0f,0.0f,-1.0f);
	
	#ifdef NORMAL_MAP
    //float3 viewTangent = IN.Texcoord1.x*rightVector + IN.Texcoord1.y*upVector;
    //viewTangent = normalize(viewTangent);
    //float3 viewBinormal = IN.Texcoord1.y*rightVector + IN.Texcoord1.x*upVector;
	//viewBinormal = normalize(viewBinormal);
	float3 viewTangent = rightVector;
	float3 viewBinormal = upVector;
	#endif
#else
	float4 worldPosition = mul(float4(IN.Position.xyz,1.0f), g_WorldMatrix);
	float3 viewNormal = mul(IN.Normal, (float3x3)g_NormalMatrix);
	viewNormal = normalize(viewNormal);	
	
	#ifdef NORMAL_MAP
    float3 viewTangent = mul(IN.Tangent, (float3x3)g_NormalMatrix);
    viewTangent = normalize(viewTangent);
    float3 viewBinormal = mul(IN.Binormal, (float3x3)g_NormalMatrix);
    viewBinormal = normalize(viewBinormal);	
	#endif
#endif

	OUT.Position = mul(worldPosition, g_ViewProjMatrix);

#ifdef GEOMETRIC_NORMAL
	OUT.Normal = viewNormal;
#endif

#ifdef NORMAL_MAP
	OUT.Normal = viewNormal;
	OUT.Tangent = viewTangent;
	OUT.Binormal = viewBinormal;
#endif

	OUT.Texcoord0 = IN.Texcoord0;
	return OUT;
}