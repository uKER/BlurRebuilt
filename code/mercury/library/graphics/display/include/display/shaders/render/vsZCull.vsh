#include "../vsConstants.h"
#include "../vsCommon.h"

struct VS_INPUT
{
	float2	m_Position	: POSITION;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;

#if defined(XB2) || defined(PS3)
	output.m_Position = float4(input.m_Position, 1.0f, 1.0f);
#else	
	output.m_Position = float4(input.m_Position, 0.0f, 1.0f);
#endif

	return output; 
}