#include "../psCommon.h"
#include "../psConstants.h"

float4		g_vSize		: register(CCP_RENDER_SETTINGS);
float4		g_vSettings	: register(CCP_RENDER_SETTINGS_1);

static const float4		g_vTaps[8]
						=	{	float4(0.44323325, -0.97511554, 0.53742981, -0.47373420),
								float4(-0.24188840, 0.99706507, -0.81409955, 0.91437590),
								float4(-0.094184101, -0.92938870, 0.34495938, 0.29387760),
								float4(-0.94201624, -0.39906216, 0.94558609, -0.76890725),
								float4(-0.38277543, 0.27676845, 0.97484398, 0.75648379),
								float4(0.19984126, 0.78641367, 0.14383161, -0.14100790),
								float4(-0.91588581, 0.45771432, -0.81544232, -0.87912464),
								float4(-0.26496911, -0.41893023, 0.79197514, 0.19090188)
							};

#define		g_fShadowWidth		g_vSize.x
#define		g_fShadowHeight		g_vSize.y
#define		g_vShadowSize		g_vSize.xy
#define		g_fPixelWidth		g_vSize.z
#define		g_fPixelHeight		g_vSize.w
#define		g_vPixelSize		g_vSize.zw
#define		g_fDepthBias		g_vSettings.x
#define		g_fVarianceBias		g_vSettings.x
#define		g_fVariancePower	g_vSettings.y
#define		g_fExponentialQ		g_vSettings.x
#define		g_fExponentialK		g_vSettings.y
#define		g_fShadowScale		g_vSettings.z
#define		g_fShadowBias		g_vSettings.w
#define		g_fJitterScale		g_vSettings.y
#define		g_vNoiseScale		g_vSettings.zw

sampler2D	g_tNoise			: register(s3);

float GetShadowSimple(sampler2D tShadow, float3 vShadowCoord)
{
	vShadowCoord.z = vShadowCoord.z + g_fDepthBias;
#if defined(PS3)
	float	fShadow	= tex2D(tShadow, vShadowCoord).r;
#else
	float	fDepth	= tex2D(tShadow, vShadowCoord.xy).r;
	float	fShadow	= step(vShadowCoord.z, fDepth);
#endif
	return 1.0f - fShadow;
}

float GetShadowJitter(sampler2D tShadow, float3 vShadowCoord, float2 vScreenCoord)
{
	// get noise
	float2		vNoise	= tex2D(g_tNoise, vScreenCoord.xy * g_vNoiseScale).rg * 2.0f - 1.0f;
	float2x4	mRot	= float2x4(vNoise.x, -vNoise.y, vNoise.y, vNoise.x, vNoise.x, -vNoise.y, vNoise.y, vNoise.x);
	
	float4	vShadowCoord0	= vShadowCoord.xyxy + mul(g_vTaps[0], mRot) * g_vSize.zwzw * g_fJitterScale;
	float4	vShadowCoord1	= vShadowCoord.xyxy + mul(g_vTaps[1], mRot) * g_vSize.zwzw * g_fJitterScale;
	float4	vShadowCoord2	= vShadowCoord.xyxy + mul(g_vTaps[2], mRot) * g_vSize.zwzw * g_fJitterScale;
	float	fZ				= vShadowCoord.z + g_fDepthBias;
	
	float4	vShadow0, vShadow1;
#if defined(PS3)
	vShadow0.x	= tex2D(tShadow, float3(vShadowCoord0.xy, fZ)).r;
	vShadow0.y	= tex2D(tShadow, float3(vShadowCoord0.zw, fZ)).r;
	vShadow0.z	= tex2D(tShadow, float3(vShadowCoord1.xy, fZ)).r;
	vShadow0.w	= tex2D(tShadow, float3(vShadowCoord1.zw, fZ)).r;
	vShadow1.x	= tex2D(tShadow, float3(vShadowCoord2.xy, fZ)).r;
	vShadow1.y	= tex2D(tShadow, float3(vShadowCoord2.zw, fZ)).r;
#else
	float4	vDepth;
	vDepth.x	= tex2D(tShadow, vShadowCoord0.xy).r;
	vDepth.y	= tex2D(tShadow, vShadowCoord0.zw).r;
	vDepth.z	= tex2D(tShadow, vShadowCoord1.xy).r;
	vDepth.w	= tex2D(tShadow, vShadowCoord1.zw).r;
	vShadow0	= step(fZ, vDepth);
	vDepth.x	= tex2D(tShadow, vShadowCoord2.xy).r;
	vDepth.y	= tex2D(tShadow, vShadowCoord2.zw).r;
	vShadow1.xy	= step(fZ, vDepth.xy);
#endif
	return 1.0f - dot(vShadow0 + float4(vShadow1.xy, 0.0f, 0.0f), 1.0f / 6.0f);
}

float GetShadowPCF(sampler2D tShadow, float3 vShadowCoord)
{
	vShadowCoord.z = vShadowCoord.z + g_fDepthBias;
#if defined(PS3)
	float4	vShadow;
	vShadow.x	= tex2D(tShadow, vShadowCoord + 0.5f * float3(-g_fPixelWidth, -g_fPixelHeight, 0.0f)).r;
	vShadow.y	= tex2D(tShadow, vShadowCoord + 0.5f * float3(-g_fPixelWidth, +g_fPixelHeight, 0.0f)).r;
	vShadow.z	= tex2D(tShadow, vShadowCoord + 0.5f * float3(+g_fPixelWidth, -g_fPixelHeight, 0.0f)).r;
	vShadow.w	= tex2D(tShadow, vShadowCoord + 0.5f * float3(+g_fPixelWidth, +g_fPixelHeight, 0.0f)).r;
	
	float2	vShadowFrac		= frac(vShadowCoord.xy * g_vShadowSize);
	float2	vShadowLerp		= lerp(vShadow.xy, vShadow.zw, vShadowFrac.x);
	float	fShadow			= lerp(vShadowLerp.x, vShadowLerp.y, vShadowFrac.y);
#else
	float3	vSamples012;
	vSamples012.x	= tex2D(tShadow, vShadowCoord.xy + float2(-g_fPixelWidth, -g_fPixelHeight)).r;
	vSamples012.y	= tex2D(tShadow, vShadowCoord.xy + float2(0.0f, -g_fPixelHeight)).r;
	vSamples012.z	= tex2D(tShadow, vShadowCoord.xy + float2(+g_fPixelWidth, -g_fPixelHeight)).r;
	float3	vSamples456;
	vSamples456.x	= tex2D(tShadow, vShadowCoord.xy + float2(-g_fPixelWidth, 0.0f)).r;
	vSamples456.y	= tex2D(tShadow, vShadowCoord.xy + float2(0.0f, 0.0f)).r;
	vSamples456.z	= tex2D(tShadow, vShadowCoord.xy + float2(+g_fPixelWidth, 0.0f)).r;
	float3	vSamples789;
	vSamples789.x	= tex2D(tShadow, vShadowCoord.xy + float2(-g_fPixelWidth, +g_fPixelHeight)).r;
	vSamples789.y	= tex2D(tShadow, vShadowCoord.xy + float2(0.0f, +g_fPixelHeight)).r;
	vSamples789.z	= tex2D(tShadow, vShadowCoord.xy + float2(+g_fPixelWidth, +g_fPixelHeight)).r;

	float2	vShadowFrac		= frac(vShadowCoord.xy * g_vShadowSize);
	float3	vShadowLerp0	= lerp(vSamples012, vSamples456, vShadowFrac.y);
	float3	vShadowLerp1	= lerp(vSamples456, vSamples789, vShadowFrac.y);
	
	float4	vShadowLerp;
	vShadowLerp.xy			= lerp(vShadowLerp0.xy, vShadowLerp0.yz, vShadowFrac.x);
	vShadowLerp.zw			= lerp(vShadowLerp1.xy, vShadowLerp1.yz, vShadowFrac.x);
	
	float	fShadow			= dot(vShadowLerp, float4(0.25f, 0.25f, 0.25f, 0.25f));
#endif
	return 1.0f - fShadow;
}

float GetShadowVariance(sampler2D tShadow, float3 vShadowCoord)
{
	float2	vVarianceShadow	= tex2D(tShadow, vShadowCoord.xy);
	float	fAvgZ			= vVarianceShadow.r;
	float	fAvgZ2			= vVarianceShadow.g;
	
	float	fVariance		= fAvgZ2 - fAvgZ * fAvgZ;
			fVariance		= saturate(fVariance + g_fVarianceBias);
	
	float	fMean			= fAvgZ;
	float	fD				= vShadowCoord.z - fMean;
	float	fPMax			= fVariance / (fVariance + fD * fD);
	
	if (vShadowCoord.z <= fAvgZ2)
		return 0.0f;
	else
		return 1.0f - pow(fPMax, g_fVariancePower);
}

float GetShadowExponential(sampler2D tShadow, float3 vShadowCoord)
{
	float2	vInterp		= frac(vShadowCoord.xy * g_vShadowSize);

	float4	vMoments0123;
	vMoments0123.x	= tex2D(tShadow, vShadowCoord.xy).r;
	vMoments0123.y	= tex2D(tShadow, vShadowCoord.xy + float2(g_fPixelWidth, 0.0f)).r;
	vMoments0123.z	= tex2D(tShadow, vShadowCoord.xy + float2(0.0f, g_fPixelHeight)).r;
	vMoments0123.w	= tex2D(tShadow, vShadowCoord.xy + float2(g_fPixelWidth, g_fPixelHeight)).r;
	
	float2	vMoments01	= lerp(vMoments0123.xz, vMoments0123.yw, vInterp.x);
	float	fMoment		= lerp(vMoments01.x, vMoments01.y, vInterp.y);

	vShadowCoord.z = g_fShadowScale * vShadowCoord.z + g_fShadowBias;
	
	float	fShadow = saturate(g_fExponentialQ - exp(g_fExponentialK * vShadowCoord.z - fMoment));
	return fShadow;
}
