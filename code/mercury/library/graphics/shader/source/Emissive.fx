#include <Core.h>
#include <Globals.h>

#define NO_BUMPMAP
#define NO_EMISSIVE
#define NO_LIGHTMAP
#define NO_SPECULAR
#define NO_VTXCOL

#define REPLACE_MATERIAL
#define REPLACE_LIGHTING_FUNCTIONS


#include <Common/Defines.h>
#include <Common/Material.h>
#include <Common/Lighting.h>

#include <Misc/ColourOp.h>

// parameters

struct C_VertexInput
{
	float3	m_vPosition	: POSITION;
	float3	m_vNormal	: NORMAL;
	float2	m_vTexCoord	: TEXCOORD0 annotation(xy(uv:0) | format(F16_2));
};

struct C_VertexOutput
{
	float4	m_vPosition	: POSITION;
	float2	m_vTexCoord	: TEXCOORD0;
	float4	m_vView		: TEXCOORD2;
};

struct C_PixelOutput
{
    float4	m_vColour	: COLOR0;
};

float		g_fBrightness		< string Source = "Brightness"; string Default = "1.0"; string Type = "Float"; >					= 1.0f;
float3		g_fTint				< string Source = "Tint"; string Default = "1, 1, 1"; >					= float3(1.0f, 1.0f, 1.0f);

// vertex shader

C_VertexOutput MainVS(C_VertexInput input)
{
    C_VertexOutput output = (C_VertexOutput) 0;

	float4	vObjectPosition	= float4(input.m_vPosition, 1.0f);
	float4	vWorldPosition	= mul(vObjectPosition, g_mWorld);
	
	output.m_vPosition		= mul(vWorldPosition, g_mViewProj);
	output.m_vTexCoord		= input.m_vTexCoord;
	output.m_vView.xyz		= vWorldPosition.xyz - g_vViewPosition.xyz;
	output.m_vView.w		= output.m_vPosition.w;
	
    return output;    
}

// pixel shader

C_PixelOutput MainPS(C_VertexOutput input) 
{ 
    C_PixelOutput output = (C_PixelOutput) 0;

	real4	vAlbedo		= tex2D(AlbedoSampler, input.m_vTexCoord.xy);
	
#if !defined(CFG_CONVERTER)	
	real3   vEmissiveColour = g_LightingInfo.m_vEmissiveColour.rgb;
#else
	real3   vEmissiveColour = real3(1.0f, 1.0f, 1.0f);
#endif
	
	real3	vDiffuse	= vAlbedo.rgb * vEmissiveColour * g_fBrightness * g_fTint;
	real	fAlpha		= vAlbedo.a;
	real4	vColour		= real4(vDiffuse, fAlpha);
#if !defined(NO_FOG)
	vColour		= EvaluateDepthFog(vColour, input.m_vView.w);
#endif

	output.m_vColour	= GetExposure(vColour);

    return output;
}

// technique

technique Material
{
	pass p0
	{	
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
