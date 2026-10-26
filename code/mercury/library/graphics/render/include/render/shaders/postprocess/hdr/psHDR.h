sampler2D	colourTexture		: register(s0);
sampler2D	luminanceTexture	: register(s1);
sampler2D	noiseTexture		: register(s2);
sampler2D	bloomTexture		: register(s3);

float4		g_vSettings1		: register(CCP_RENDER_POSTPROCESS_PARAMETERS_0);
float4		g_vSettings2		: register(CCP_RENDER_POSTPROCESS_PARAMETERS_1);

#define		g_fMiddleGrey		g_vSettings1.x
#define		g_fGamma			g_vSettings1.y
#define		g_fInvGamma			g_vSettings1.z
#define		g_fInvBloomGamma	g_vSettings1.w

#define		g_vNoiseUVScale		g_vSettings2.xy
#define		g_fNoiseFactor		g_vSettings2.z

float3 ApplyGamma(float3 colour)
{
	return pow(colour, g_fGamma);
}

float3 ApplyInverseGamma(float3 colour)
{
	return pow(colour, g_fInvGamma);
}

float3 ApplyInverseBloomGamma(float3 colour)
{
	return pow(colour, g_fInvBloomGamma);
}

float3 ApplyToneMapping(float3 colour, float fLuminance)
{
	return colour * (g_fMiddleGrey / (fLuminance + 0.001f));
}

float3 GetBloomCombine(float2 uv)
{
	return tex2D(bloomTexture, uv).rgb;
}

half3 ApplyGamma(half3 colour)
{
	return pow(colour, g_fGamma);
}

half3 ApplyInverseGamma(half3 colour)
{
	return pow(colour, g_fInvGamma);
}

half3 ApplyInverseBloomGamma(half3 colour)
{
	return pow(colour, g_fInvBloomGamma);
}

half3 ApplyToneMapping(half3 colour, float fLuminance)
{
	return colour * (g_fMiddleGrey / (fLuminance + 0.001h));
}
