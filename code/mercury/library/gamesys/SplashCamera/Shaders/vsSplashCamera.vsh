#include "../../graphics/display/include/display/shaders/vsCommon.h"
#include "../../graphics/display/include/display/shaders/vsConstants.h"

struct VS_INPUT
{
	float2	m_Position	: POSITION;
	float4	m_Colour	: COLOR0;
	float2	m_TexCoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float3	m_TexCoord	: TEXCOORD0; //z-> Alpha
#ifdef REFRACT
	float4	m_ScreenPos : TEXCOORD1;
#endif
};


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	output.m_Position = float4(input.m_Position.xy, 0.f, 1.f);
	output.m_TexCoord.xy = input.m_TexCoord;
	output.m_TexCoord.z = input.m_Colour.a;
	
#ifdef REFRACT
	output.m_ScreenPos = output.m_Position * float4(0.5f, - 0.5, 1.f, 1.f) + float4(0.5f * output.m_Position.w, 0.5f * output.m_Position.w, 0.f, 0.f);
#endif
	
	return output;
}
