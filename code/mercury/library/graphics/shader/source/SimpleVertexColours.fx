#include <Core.h>
#include <Globals.h>

struct C_VertexInput
{
	float3	m_vPosition	: POSITION;
	float4	m_vColour	: COLOR0 annotation(xyzw(colour:vertexColour));
};

struct C_VertexOutput
{
	float4	m_vPosition	: POSITION;
	float4	m_vColour	: COLOR0;
};

struct C_PixelOutput
{
	float4	m_vColour	: COLOR0;
};

C_VertexOutput MainVS(in C_VertexInput input)
{
	C_VertexOutput output = (C_VertexOutput) 0;
	
	float4	vObjectPosition	= float4(input.m_vPosition, 1.0f);
	float4	vWorldPosition	= mul(vObjectPosition, g_mWorld);
	float4	vProjPosition	= mul(vWorldPosition, g_mViewProj);
	
	output.m_vPosition	= vProjPosition;
	output.m_vColour	= input.m_vColour;

	return output;
}

C_PixelOutput MainPS(in C_VertexOutput input)
{
	C_PixelOutput output = (C_PixelOutput) 0;
	
	output.m_vColour = input.m_vColour;

	return output;
}

technique Default
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
