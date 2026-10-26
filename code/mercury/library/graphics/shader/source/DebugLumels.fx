#include <Core.h>
#include <Globals.h>

struct C_VertexInput
{
	float3	m_vPosition	: POSITION;
	float4	m_vTexCoord	: TEXCOORD0;
};

struct C_VertexOutput
{
	float4	m_vPosition	: POSITION;
	float2	m_vTexCoord	: TEXCOORD0;
};

struct C_PixelOutput
{
	float4	m_vColour	: COLOR0;
};

float g_fLightMapTextureSize = 512.0f;

C_VertexOutput MainVS(C_VertexInput input)
{
    C_VertexOutput output = (C_VertexOutput) 0;

	float4	vObjectPosition	= float4(input.m_vPosition, 1.0f);
	float4	vWorldPosition	= mul(vObjectPosition, g_mWorld);
	float4	vPosition		= mul(vWorldPosition, g_mViewProj);

	output.m_vPosition	= vPosition;
	output.m_vTexCoord	= input.m_vTexCoord.zw;

    return output;    
}

C_PixelOutput MainPS(C_VertexOutput input)
{
	C_PixelOutput output = (C_PixelOutput) 0;
	
	float2 vGrid = frac(input.m_vTexCoord * g_fLightMapTextureSize);
	if ((vGrid.x > 0.9f) || (vGrid.y > 0.9f))
	{
		output.m_vColour = float4(0.75f, 0.75f, 0.75f, 1.0f);
	}
	else
	{
		output.m_vColour = float4(0.25f, 0.25f, 0.25f, 1.0f);
	}
	
	return output;
}

technique Debug
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
