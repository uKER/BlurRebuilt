#include "..\..\vsConstants.h"
#include "..\..\vsCommon.h"



float4	g_ClipVector	: register(CCV_DISPLAY_CLIP_VECTOR);



struct VS_INPUT
{
	float2	m_Position	: POSITION;
	float2	m_TexCoord	: TEXCOORD0;
};



struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float4	m_TexCoord	: TEXCOORD0;
};



VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output		=	(VS_OUTPUT) 0;
	
	output.m_Position		=	float4(input.m_Position.xy * g_ClipVector.xy + g_ClipVector.zw, 0.0f, 1.0f);
	output.m_TexCoord.xy	=	input.m_TexCoord;
	
	return output;
}