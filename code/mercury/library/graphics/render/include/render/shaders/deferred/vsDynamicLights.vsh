#include "../vsCommon.h"
#include "../vsConstants.h"

struct VS_INPUT
{
	float2	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float2	m_TexCoord	: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	output.m_Position = float4(input.m_Position, 0.0f, 1.0f);
	output.m_TexCoord = input.m_Position * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
	
	return output;
}
