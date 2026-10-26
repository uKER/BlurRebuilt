#include "deferred/shaders/vsCommon.h"
#include "deferred/shaders/vsConstants.h"

float4x4			g_mViewProj	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4				g_vCameraPosition : register(CCV_DISPLAY_VIEW_POSITION);

struct VS_INPUT
{
	float3	m_Position	: POSITION;
	float4	m_Colour	: TEXCOORD0;  //Colour-> RGB, Size->A
	float2	m_Offset	: TEXCOORD1;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float2	m_Offset	: TEXCOORD0;
	float3	m_Colour	: TEXCOORD1;
};


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	//Pivot
	float2	vPivot			= input.m_Offset.xy;
	float3	view			= normalize(g_vCameraPosition.xyz - input.m_Position);
	
	// grab vectors from view
	float3			vRight		= normalize(cross(view, float3(0.f, 1.f, 0.f)));
	float3			vUp			= normalize(cross(vRight, view ));
	
	//Calculate world position
	float4 vWorldPosition = float4(input.m_Position, 1.0f);
	
	vWorldPosition.xyz	+= vRight * vPivot.x * input.m_Colour.w;
	vWorldPosition.xyz	+= vUp * vPivot.y * input.m_Colour.w;
	
	//Calculate position
	float4  vProjPos = mul(vWorldPosition, g_mViewProj);
	output.m_Position = vProjPos;
	output.m_Position.z = max(output.m_Position.z, 0.01f);

	output.m_Offset.xy = input.m_Offset.xy;
	output.m_Colour = input.m_Colour.rgb;
	
	return output;
}
