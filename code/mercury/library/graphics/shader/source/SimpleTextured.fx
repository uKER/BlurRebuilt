#include <Core.h>
#include <Globals.h>

texture2D	g_tAlbedo	< string Source = "Texture"; string Default = "Black"; bool AlphaDetect = true; >;

sampler AlbedoSampler = 
sampler_state
{
    Texture = <g_tAlbedo>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

struct C_VertexInput
{
	float3	m_vPosition	: POSITION;
	float2	m_vTexCoord	: TEXCOORD0 annotation(xy(uv:0) | format(F16_2));
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

C_VertexOutput MainVS(in C_VertexInput input)
{
	C_VertexOutput output = (C_VertexOutput) 0;
	
	float4	vObjectPosition	= float4(input.m_vPosition, 1.0f);
	float4	vWorldPosition	= mul(vObjectPosition, g_mWorld);
	float4	vProjPosition	= mul(vWorldPosition, g_mViewProj);
	
	output.m_vPosition	= vProjPosition;
	output.m_vTexCoord	= input.m_vTexCoord;

	return output;
}

C_PixelOutput MainPS(in C_VertexOutput input)
{
	C_PixelOutput output = (C_PixelOutput) 0;
	
	output.m_vColour = tex2D(AlbedoSampler, input.m_vTexCoord);

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
