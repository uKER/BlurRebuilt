// functions to filter variance shadow maps

float4 DepthToVariance(float fDepth)
{
	return float4(fDepth, fDepth * fDepth, 0.0f, 0.0f);
}

float4 VarianceGaussianBlur_5Taps(sampler2D tShadowMap, float2 vTexCoord, float2 vPixelOffset)
{
	float4	vSamples01;
	float4	vSamples23;
	float2	vSamples4_;

	vSamples01.xy	= tex2D(tShadowMap, vTexCoord - 2.0f * vPixelOffset).xy;
	vSamples01.zw	= tex2D(tShadowMap, vTexCoord - 1.0f * vPixelOffset).xy;
	vSamples23.xy	= tex2D(tShadowMap, vTexCoord).xy;
	vSamples23.zw	= tex2D(tShadowMap, vTexCoord + 1.0f * vPixelOffset).xy;
	vSamples4_.xy	= tex2D(tShadowMap, vTexCoord + 2.0f * vPixelOffset).xy;

	vSamples01		= vSamples01 * float4(1.0f, 1.0f, 4.0f, 4.0f) / 16.0f;
	vSamples23		= vSamples23 * float4(6.0f, 6.0f, 4.0f, 4.0f) / 16.0f;
	vSamples4_		= vSamples4_ * float2(1.0f, 1.0f) / 16.0f;

	float4	vSamples0123	= vSamples01 + vSamples23;

	return float4(vSamples0123.xy + vSamples0123.zw + vSamples4_.xy, 0.0f, 0.0f);
}

float4 VarianceGaussianBlur_7Taps(sampler2D tShadowMap, float2 vTexCoord, float2 vPixelOffset)
{
	float4	vSamples01;
	float4	vSamples23;
	float4	vSamples45;
	float4	vSamples6_;

	vSamples01.xy	= tex2D(tShadowMap, vTexCoord - 3.0f * vPixelOffset).xy;
	vSamples01.zw	= tex2D(tShadowMap, vTexCoord - 2.0f * vPixelOffset).xy;
	vSamples23.xy	= tex2D(tShadowMap, vTexCoord - 1.0f * vPixelOffset).xy;
	vSamples23.zw	= tex2D(tShadowMap, vTexCoord).xy;
	vSamples45.xy	= tex2D(tShadowMap, vTexCoord + 1.0f * vPixelOffset).xy;
	vSamples45.zw	= tex2D(tShadowMap, vTexCoord + 2.0f * vPixelOffset).xy;
	vSamples6_.xy	= tex2D(tShadowMap, vTexCoord + 3.0f * vPixelOffset).xy;
	vSamples6_.zw	= 0.0f;

	vSamples01		= vSamples01 * float4(1.0f, 1.0f, 6.0f, 6.0f) / 64.0f;
	vSamples23		= vSamples23 * float4(15.0f, 15.0f, 20.0f, 20.0f) / 64.0f;
	vSamples45		= vSamples45 * float4(15.0f, 15.0f, 6.0f, 6.0f) / 64.0f;
	vSamples6_		= vSamples6_ * float4(1.0f, 1.0f, 1.0f, 1.0f) / 64.0f;

	float4	vSamples0123	= vSamples01 + vSamples23;
	float4	vSamples456_	= vSamples45 + vSamples6_;

	return float4(vSamples0123.xy + vSamples0123.zw + vSamples456_.xy + vSamples456_.zw, 0.0f, 0.0f);
}

float4 VarianceGaussianBlurFromDepth_5Taps(sampler2D tShadowMap, float2 vTexCoord, float2 vPixelOffset)
{
	float4	vSamples0123;
	float	vSamples4___;
	vSamples0123.x	= SampleDepthBuffer(tShadowMap, vTexCoord - 2.0f * vPixelOffset);
	vSamples0123.y	= SampleDepthBuffer(tShadowMap, vTexCoord - 1.0f * vPixelOffset);
	vSamples0123.z	= SampleDepthBuffer(tShadowMap, vTexCoord);
	vSamples0123.w	= SampleDepthBuffer(tShadowMap, vTexCoord + 1.0f * vPixelOffset);
	vSamples4___.x	= SampleDepthBuffer(tShadowMap, vTexCoord + 2.0f * vPixelOffset);

	float	fDepth	= dot(vSamples0123, float4(1.0f / 16.0f, 4.0f / 16.0f, 6.0f / 16.0f, 4.0f / 16.0f)) + vSamples4___.x * (1.0f / 16.0f);
	
	vSamples0123	= vSamples0123 * vSamples0123;
	vSamples4___	= vSamples4___ * vSamples4___;

	float	fDepth2	= dot(vSamples0123, float4(1.0f / 16.0f, 4.0f / 16.0f, 6.0f / 16.0f, 4.0f / 16.0f)) + vSamples4___.x * (1.0f / 16.0f);

	return float4(fDepth, fDepth2, 0.0f, 0.0f);
}

float4 VarianceGaussianBlurFromDepth_7Taps(sampler2D tShadowMap, float2 vTexCoord, float2 vPixelOffset)
{
	float4	vSamples0123;
	float3	vSamples456_;
	vSamples0123.x	= SampleDepthBuffer(tShadowMap, vTexCoord - 3.0f * vPixelOffset);
	vSamples0123.y	= SampleDepthBuffer(tShadowMap, vTexCoord - 2.0f * vPixelOffset);
	vSamples0123.z	= SampleDepthBuffer(tShadowMap, vTexCoord - 1.0f * vPixelOffset);
	vSamples0123.w	= SampleDepthBuffer(tShadowMap, vTexCoord - 0.0f * vPixelOffset);
	vSamples456_.x	= SampleDepthBuffer(tShadowMap, vTexCoord + 1.0f * vPixelOffset);
	vSamples456_.y	= SampleDepthBuffer(tShadowMap, vTexCoord + 2.0f * vPixelOffset);
	vSamples456_.z	= SampleDepthBuffer(tShadowMap, vTexCoord + 3.0f * vPixelOffset);

	float	fDepth	= dot(vSamples0123, float4(1.0f / 64.0f, 6.0f / 64.0f, 15.0f / 64.0f, 20.0f / 64.0f))
					+ dot(vSamples456_.xyz, float3(15.0f / 64.0f, 6.0f / 64.0f, 1.0f / 64.0f));
	
	vSamples0123	= vSamples0123 * vSamples0123;
	vSamples456_	= vSamples456_ * vSamples456_;

	float	fDepth2	= dot(vSamples0123, float4(1.0f / 64.0f, 6.0f / 64.0f, 15.0f / 64.0f, 20.0f / 64.0f))
					+ dot(vSamples456_.xyz, float3(15.0f / 64.0f, 6.0f / 64.0f, 1.0f / 64.0f));

	return float4(fDepth, fDepth2, 0.0f, 0.0f);
}

