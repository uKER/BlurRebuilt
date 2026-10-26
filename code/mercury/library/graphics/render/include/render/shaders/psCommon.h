#include "display\shaders\psCommon.h"
#include "render\shaders\psConstants.h" // need to add ..\..\render\shaders for the PIX debugger to work, see Steve or Jan

float4		g_vLuminance		: register(CCP_RENDER_POSTPROCESS_LUMINANCE); // place this at the end of the PS20 constants

float4 ApplyGain(float4 colour)
{
	colour.rgb = colour.rgb * g_vLuminance.x;
	return colour;
}

float3 ApplyGain(float3 colour)
{
	return colour * g_vLuminance.x;
}

float3 ApplyInverseGain(float3 colour)
{
	return colour * g_vLuminance.y;
}

float4 ApplyInverseGain(float4 colour)
{
	colour.rgb = colour.rgb * g_vLuminance.y;
	return colour;
}

float SampleDepthBuffer(sampler2D depth, float2 uv)
{
	#ifdef PS3
	float3 depthBytes = round(tex2D(depth, uv).rgb * 255.0f);
	float3 depthScale = float3(65536.0f, 256.0f, 1.0f) / 16777215.0f;		
	return dot(depthBytes, depthScale);
	#elif defined(BLUR2PC)
	// [Blur2PC] Retail PC depth samples carry a sign over vehicles. Recover
	// depth from the magnitude, as in retail psDoFFgBgDownsample/Combine.
	return abs(tex2D(depth, uv).r);
	#else
	return tex2D(depth, uv).r;
	#endif
}

float ScaleDepth(float depthBufferValue, float4 scaleParam)
{
	return scaleParam.y / (depthBufferValue - scaleParam.x);
}

float SampleDepthBuffer(sampler2D depth, float2 uv, float4 scaleParam)
{
	float depthBufferValue = SampleDepthBuffer(depth, uv);

	return ScaleDepth(depthBufferValue, scaleParam);
}

float3 GetWorldPosition(sampler2D depth, float2 uv, float4 scaleParam, float4x3 toWorld)
{
	float z = SampleDepthBuffer(depth, uv);
	float w = ScaleDepth(z, scaleParam);

	float4 screenPos = float4(uv, z, 1.0f);
	return mul(screenPos * w, toWorld);
}

float3 GetWorldPosition(sampler2D depth, float2 uv, float4x4 toWorld)
{
	float z = SampleDepthBuffer(depth, uv);
	float4 screenPos = float4(uv, z, 1.0f);
	float4 worldPos = mul(screenPos, toWorld);
	float wRcp = 1.0f / worldPos.w;
	return worldPos.xyz * wRcp;
}

float3 GetWorldPosition(sampler2D depth, float2 uv, float4 scaleParam, float3 vCamera, float3 vInterpolated)
{
	float fZ = SampleDepthBuffer(depth, uv);
	return (1.0f / (fZ - scaleParam.x)) * vInterpolated + vCamera;
}
