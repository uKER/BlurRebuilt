#ifndef AMAX_SHADERS_COMMON_REFLECTION_H
#define AMAX_SHADERS_COMMON_REFLECTION_H

shared float4		g_vPlaneReflection : register(CCV_SHADER_USER_0)			= float4(0.0f, 1.0f, 0.0f, 0.0f);
shared texture2D	g_tScreenReflectionTexture;
shared texture2D	g_tBlurScreenReflectionTexture;

shared float4	g_ReflectionViewMatrix0 : register(CCV_SHADER_USER_1);
shared float4	g_ReflectionViewMatrix1 : register(CCV_SHADER_USER_2);
shared float4	g_ReflectionViewMatrix3 : register(CCV_SHADER_USER_3);

#ifdef CFG_DEBUG
	float	g_fReflectionNormalScale			: register(CCV_SHADER_USER_4) < string DebugVariable = "ReflectionNormalScale"; string Range = "(0,20)"; string Type = "Float"; > = 10.f;
	float	g_fReflectionPlaneDistanceScale		: register(CCV_SHADER_USER_5) < string DebugVariable = "ReflectionPlaneDistanceScale"; string Range = "(0,5)"; string Type = "Float"; > = 1.0f;
#else
	const float	g_fReflectionNormalScale = 10.f;
	const float	g_fReflectionPlaneDistanceScale = 1.0f;
#endif

sampler ScreenReflectionSampler : register(SSP_SHADER_REFLECTION) = 
sampler_state
{
    Texture = <g_tScreenReflectionTexture>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

sampler BlurScreenReflectionSampler : register(s7) = 
sampler_state
{
	Texture = <g_tBlurScreenReflectionTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

#endif // AMAX_SHADERS_COMMON_REFLECTION_H
