#include <Core.h>
#include <Globals.h>

struct C_VertexInput
{
	float3	m_vPosition	: POSITION;
	float4	m_vCol0		: COLOR0 annotation(format(F32_4));
	float4	m_vCol1		: COLOR1 annotation(format(U8_4N));
	float4	m_vNorm0	: NORMAL0 annotation(format(F32_4));
	float4	m_vNorm1	: TEXCOORD2 annotation(xyz(normal) | format(SHHD_3N));
	float4	m_vTex0		: TEXCOORD0 annotation(format(F32_4));
	float4	m_vTex1		: TEXCOORD1 annotation(format(U8_4N));
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

C_VertexOutput GetPosition(C_VertexInput input)
{
    C_VertexOutput output = (C_VertexOutput) 0;

	float4	vObjectPosition	= float4(input.m_vPosition, 1.0f);
	float4	vWorldPosition = mul(vObjectPosition, g_mWorld);
	
	output.m_vPosition = mul(vWorldPosition, g_mViewProj);

    return output;
}

C_VertexOutput VCol_F32_4(C_VertexInput input)
{
    C_VertexOutput output = GetPosition(input);
	output.m_vColour = input.m_vCol0;
    return output;
}

C_VertexOutput VCol_U8_4N(C_VertexInput input)
{
    C_VertexOutput output = GetPosition(input);
	output.m_vColour = input.m_vCol1;
    return output;
}

C_VertexOutput VNorm_F32_4(C_VertexInput input)
{
    C_VertexOutput output = GetPosition(input);
    float3 normal = mul(input.m_vNorm0, (float3x3) g_mWorld);
	output.m_vColour.xyz = (normal + 1.0) * 0.5;
	output.m_vColour.w = 1.f;
    return output;
}

C_VertexOutput VNorm_SHHD_3N(C_VertexInput input)
{
    C_VertexOutput output = GetPosition(input);
    
// PC does not support this format, converts to U8_4N
#if defined(PLATFORM_PC)
	float3 normal = (input.m_vNorm1 - 0.5) * 2.0;
#else
	float3 normal = input.m_vNorm1;
#endif

    normal = mul(normal, (float3x3) g_mWorld);
	output.m_vColour.xyz = (normal + 1.0) * 0.5;
	output.m_vColour.w = 1.f;
    return output;
}

C_VertexOutput VTex_F32_4(C_VertexInput input)
{
    C_VertexOutput output = GetPosition(input);
	output.m_vColour = input.m_vTex0;
    return output;
}

C_VertexOutput VTex_U8_4N(C_VertexInput input)
{
    C_VertexOutput output = GetPosition(input);
	output.m_vColour = input.m_vTex1;
    return output;
}

C_PixelOutput MainPS(C_VertexOutput input) 
{ 
    C_PixelOutput output = (C_PixelOutput) 0;

	output.m_vColour = input.m_vColour;

    return output;
}

// technique

technique TCol_F32_4
{
	pass p0
	{
		VertexShader = compile vs_3_0 VCol_F32_4();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique TCol_U8_4N
{
	pass p0
	{
		VertexShader = compile vs_3_0 VCol_U8_4N();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique TNorm_F32_4
{
	pass p0
	{
		VertexShader = compile vs_3_0 VNorm_F32_4();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique TNorm_SHHD_3N
{
	pass p0
	{
		VertexShader = compile vs_3_0 VNorm_SHHD_3N();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique TTex_F32_4
{
	pass p0
	{
		VertexShader = compile vs_3_0 VTex_F32_4();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique TTex_U8_4N
{
	pass p0
	{
		VertexShader = compile vs_3_0 VTex_U8_4N();
		PixelShader = compile ps_3_0 MainPS();
	}
}
