#include "../vsCommon.h"
#include "../vsConstants.h"

struct VS_INPUT
{
	float3	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

	output.m_Position	= float4(input.m_Position.xyz, 1.0f);

	return output;
}
