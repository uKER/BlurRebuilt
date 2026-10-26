#ifndef SHADER_MISC_RESOLVE_H
#define SHADER_MISC_RESOLVE_H

float SampleDepthBuffer(sampler DepthSampler, float2 uv)
{
#if !defined(CFG_CONVERTER)
	#ifdef PLATFORM_PS3
	float3 depthBytes = round(h4tex2D(DepthSampler, uv).rgb * 255.0f);
	float3 depthScale = float3(65536.0f, 256.0f, 1.0f) / 16777215.0f;		
	return dot(depthBytes, depthScale);
	#else
	return tex2D(DepthSampler, uv).r;
	#endif
#else
	return 0.0f;
#endif
}

float ScaleDepth(float depthBufferValue, float4 scaleParam)
{
	return scaleParam.y / (depthBufferValue - scaleParam.x);
}

float SampleDepthBuffer(sampler DepthSampler, float2 uv, float4 scaleParam)
{
	float depthBufferValue = SampleDepthBuffer(DepthSampler, uv);
#if defined(BLUR_DX11)
	// [Blur2PC][Blur1PCDX11][retail-recovered] Every shipped PC effect that scales
	// a depth-buffer sample (water, mesh effects, flares) takes abs() first.
	depthBufferValue = abs(depthBufferValue);
#endif

	return ScaleDepth(depthBufferValue, scaleParam);
}

real3 SampleColourBuffer(sampler ColourSampler, float2 uv)
{
#if !defined(CFG_CONVERTER)
	real3 vColour = h4tex2D(ColourSampler, uv).rgb;
#else
	real3 vColour = 0.0h;
#endif
	vColour = InverseExposure(vColour);

	return vColour;
}

#endif // SHADER_MISC_RESOLVE_H
