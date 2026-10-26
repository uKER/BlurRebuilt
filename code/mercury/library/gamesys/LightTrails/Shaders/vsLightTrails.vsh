#include "../../graphics/display/include/display/shaders/vsCommon.h"
#include "../../graphics/display/include/display/shaders/vsConstants.h"

float4x4			g_mViewProj	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4			g_mView		: register(CCV_DISPLAY_VIEW_MATRIX);
float4				g_vViewPosition	: register(CCV_DISPLAY_VIEW_POSITION);

struct VS_INPUT
{
	float3	m_Position	: POSITION;
	float4	m_Colour	: TEXCOORD0;
	float3	m_DirectionA	: NORMAL;
	float3	m_DirectionB	: TANGENT;
};

struct VS_OUTPUT
{
	float4	m_Position		: POSITION;
	float4	m_Colour		: TEXCOORD0;
	float3	m_Texcoords		: TEXCOORD1; //x-> height, y->projected Z, w -> projected W
};


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	//Calculate world position
	float4 vCentrePosition = float4(input.m_Position, 1.0f);
	
	float3 vWorldDirection = normalize(cross(input.m_DirectionA, input.m_DirectionB));
	vWorldDirection = (input.m_Colour.r < 0.f) ? -vWorldDirection : vWorldDirection;
	
	float3 viewVector = normalize(vCentrePosition.xyz - g_vViewPosition.xyz);
	
	float3 referenceVector = normalize(cross(viewVector, vWorldDirection));
	
	//Calculate best normal
	float normal0product = dot(referenceVector, input.m_DirectionA);
	float normal1product = dot(referenceVector, input.m_DirectionB);
	

	float maxFactor = normal0product;	
	maxFactor = max(maxFactor, normal1product);
	maxFactor = max(maxFactor, -normal0product);
	maxFactor = max(maxFactor, -normal1product);
	
	float alpha = input.m_Colour.a;
	
	float4 vWorldPosition = vCentrePosition;
	vWorldPosition.xyz += referenceVector * maxFactor;
	
	
	float4  vProjPos = mul(vWorldPosition, g_mViewProj);
	output.m_Position = vProjPos;
	output.m_Colour.rgb = input.m_Colour.rgb;
	output.m_Colour.r = abs(output.m_Colour.r);
	output.m_Colour.a = alpha * saturate((1.f - abs(dot(vWorldDirection, viewVector))) * 10.f );
	
	output.m_Texcoords.x = (input.m_Colour.r < 0.f) ? 0.f : 1.f;
	output.m_Texcoords.y = vProjPos.z;
	output.m_Texcoords.z = vProjPos.w;
	
	return output;
}
