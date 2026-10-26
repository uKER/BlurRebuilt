#ifndef __LIGHTFLARE_PARAMETERS_H__
#define __LIGHTFLARE_PARAMETERS_H__

// parameters
shared float4	g_vNearFarLightFlareColour	: register(CCV_SHADER_USER_0) = float4(10.0f, 300.0f, 0.0f, 0.f);

shared float3	g_vGlobalLightFlareSize		: register(CCV_SHADER_USER_1) = float3(1.0f, 1.0f, 1.0f);
#define			g_fGlobalLightFlareSize		g_vGlobalLightFlareSize.x
#define			g_fGlobalLightFlareSizeX	g_vGlobalLightFlareSize.y
#define			g_fGlobalLightFlareSizeY	g_vGlobalLightFlareSize.z

shared float3	g_vGlobalLightFlareColour	: register(CCV_SHADER_USER_2) = float3(1.0f, 1.0f, 1.0f);

texture2D		g_tColour	< string Source = "Colour"; string Default = "Black"; >;

sampler ColourSampler : register(SSP_SHADER_ALBEDO) = 
sampler_state
{
    Texture = <g_tColour>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

#if !defined(CFG_CONVERTER)
shared texture2D	g_tDepthBuffer;
sampler DepthBufferSampler		: register(SSP_SHADER_DEPTH_BUFFER) = 
sampler_state
{
	Texture = <g_tDepthBuffer>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};
#endif

#if defined(PLATFORM_PS3)
#pragma texformat g_tColour COMPRESSED_RGBA_S3TC_DXT5
#pragma texformat g_tDepth RGBA8
#endif

float3		g_vLightFlareColour				: register(CCV_SHADER_USER_3) < string Source = "MultiplyColour"; string Default = "1.0, 1.0, 1.0"; string Type = "Float3"; >	= float3(1.0f, 1.0f, 1.0f);
float		g_fLightFlareIntensity			: register(CCV_SHADER_USER_4) < string Source = "MultiplyIntensity"; string Default = "1.0"; string Type = "Float"; >			= 1.0f;
float		g_fLightFlareSize				: register(CCV_SHADER_USER_5) < string Source = "MultiplySize"; string Default = "1.0"; string Type = "Float"; >				= 1.0f;

float		g_fFrequency					: register(CCV_SHADER_USER_6) < string Source = "Frequency"; string Default = "1.0"; >	= 1.0f;
float		g_fThresholdOn					: register(CCV_SHADER_USER_7) < string Source = "ThresholdOn"; string Default = "1.0"; >	= 1.0f;
float		g_fThresholdOff					: register(CCV_SHADER_USER_8) < string Source = "ThresholdOff"; string Default = "0.0"; >	= 0.0f;

// vertex shader

#ifdef CFG_DEBUG
	float		g_fLightFlareDepthBias			: register(CCV_SHADER_USER_9) < string DebugVariable = "DepthBias"; string Range = "(0,1)"; string Type = "Float"; > = 0.45f;
	float		g_fLightFlareBorderSizeBias		: register(CCV_SHADER_USER_10) < string DebugVariable = "BorderSizeBias"; string Range = "(0,2)"; string Type = "Float"; > = 1.5f;
	float		g_fLightFlareBorderIntensityBias: register(CCV_SHADER_USER_11) < string DebugVariable = "BorderIntensityBias"; string Range = "(0,2)"; string Type = "Float"; > = 1.5f;
#else
#if defined(ENVIRONMENT)
	const real		g_fLightFlareDepthBias				= 0.45h;
	const real		g_fLightFlareBorderSizeBias			= 1.0h;
	const real		g_fLightFlareBorderIntensityBias	= 1.0h;
#else
	const real		g_fLightFlareDepthBias				= 0.45h;
	const real		g_fLightFlareBorderSizeBias			= 1.50h;
	const real		g_fLightFlareBorderIntensityBias	= 1.50h;
#endif
#endif
#endif
