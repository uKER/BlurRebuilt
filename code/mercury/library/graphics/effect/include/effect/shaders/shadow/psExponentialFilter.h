// functions to filter exponential shadow maps

float ExponentialConvolution(float fWeight0, float fX, float fWeight1, float fY)
{
	return fX + log(fWeight0 + fWeight1 * exp(fY - fX));
}

float ExponentialGaussianBlur_5Taps(sampler2D tShadowMap, float2 vTexCoord, float2 vPixelOffset)
{
	float3	vWeights = float3(1.0f, 4.0f, 6.0f) / 16.0f;

	float4	vSamples0123;
	vSamples0123.x	= tex2D(tShadowMap, vTexCoord - 2.0f * vPixelOffset).r;
	vSamples0123.y	= tex2D(tShadowMap, vTexCoord - 1.0f * vPixelOffset).r;
	vSamples0123.z	= tex2D(tShadowMap, vTexCoord).r;
	vSamples0123.w	= tex2D(tShadowMap, vTexCoord + 1.0f * vPixelOffset).r;
	
	float	vSamples4___;
	vSamples4___.x	= tex2D(tShadowMap, vTexCoord + 2.0f * vPixelOffset).r;

	float2	vSum;
	vSum.x	= ExponentialConvolution(vWeights.x, vSamples0123.x, vWeights.y, vSamples0123.y);
	vSum.y	= ExponentialConvolution(vWeights.y, vSamples0123.w, vWeights.x, vSamples4___.x);
	vSum.x	= ExponentialConvolution(1.0f, vSum.x, vWeights.z, vSamples0123.z);

	return ExponentialConvolution(1.0f, vSum.x, 1.0f, vSum.y);
}

float ExponentialGaussianBlur_7Taps(sampler2D tShadowMap, float2 vTexCoord, float2 vPixelOffset)
{
	float4	vWeights = float4(1.0f, 6.0f, 15.0f, 20.0f) / 64.0f;

	float4	vSamples0123;
	vSamples0123.x	= tex2D(tShadowMap, vTexCoord - 3.0f * vPixelOffset).r;
	vSamples0123.y	= tex2D(tShadowMap, vTexCoord - 2.0f * vPixelOffset).r;
	vSamples0123.z	= tex2D(tShadowMap, vTexCoord - 1.0f * vPixelOffset).r;
	vSamples0123.w	= tex2D(tShadowMap, vTexCoord).r;
	
	float3	vSamples456_;
	vSamples456_.x	= tex2D(tShadowMap, vTexCoord + 1.0f * vPixelOffset).r;
	vSamples456_.y	= tex2D(tShadowMap, vTexCoord + 2.0f * vPixelOffset).r;
	vSamples456_.z	= tex2D(tShadowMap, vTexCoord + 3.0f * vPixelOffset).r;

	float2	vSum;
	vSum.x	= ExponentialConvolution(vWeights.x, vSamples0123.x, vWeights.y, vSamples0123.y);
	vSum.y	= ExponentialConvolution(vWeights.y, vSamples456_.y, vWeights.x, vSamples456_.z);
	vSum.x	= ExponentialConvolution(1.0f, vSum.x, vWeights.z, vSamples0123.z);
	vSum.y	= ExponentialConvolution(1.0f, vSum.y, vWeights.z, vSamples456_.x);
	vSum.x	= ExponentialConvolution(1.0f, vSum.x, vWeights.w, vSamples0123.w);

	return ExponentialConvolution(1.0f, vSum.x, 1.0f, vSum.y);
}

float ExponentialGaussianBlurFromDepth_5Taps(sampler2D tShadowMap, float2 vTexCoord, float2 vPixelOffset, float2 vScaleBias)
{
	float3	vWeights = float3(1.0f, 4.0f, 6.0f) / 16.0f;

	float4	vSamples0123;
	vSamples0123.x	= SampleDepthBuffer(tShadowMap, vTexCoord - 2.0f * vPixelOffset).r;
	vSamples0123.y	= SampleDepthBuffer(tShadowMap, vTexCoord - 1.0f * vPixelOffset).r;
	vSamples0123.z	= SampleDepthBuffer(tShadowMap, vTexCoord).r;
	vSamples0123.w	= SampleDepthBuffer(tShadowMap, vTexCoord + 1.0f * vPixelOffset).r;
	
	float	vSamples4___;
	vSamples4___.x	= SampleDepthBuffer(tShadowMap, vTexCoord + 2.0f * vPixelOffset).r;

	vSamples0123 = vScaleBias.x * vSamples0123 + vScaleBias.y;
	vSamples4___ = vScaleBias.x * vSamples4___ + vScaleBias.y;	

	float2	vSum;
	vSum.x	= ExponentialConvolution(vWeights.x, vSamples0123.x, vWeights.y, vSamples0123.y).r;
	vSum.y	= ExponentialConvolution(vWeights.y, vSamples0123.w, vWeights.x, vSamples4___.x).r;
	vSum.x	= ExponentialConvolution(1.0f, vSum.x, vWeights.z, vSamples0123.z).r;

	return ExponentialConvolution(1.0f, vSum.x, 1.0f, vSum.y);
}

float ExponentialGaussianBlurFromDepth_7Taps(sampler2D tShadowMap, float2 vTexCoord, float2 vPixelOffset, float2 vScaleBias)
{
	float4	vWeights = float4(1.0f, 6.0f, 15.0f, 20.0f) / 64.0f;

	float4	vSamples0123;
	vSamples0123.x	= SampleDepthBuffer(tShadowMap, vTexCoord - 3.0f * vPixelOffset).r;
	vSamples0123.y	= SampleDepthBuffer(tShadowMap, vTexCoord - 2.0f * vPixelOffset).r;
	vSamples0123.z	= SampleDepthBuffer(tShadowMap, vTexCoord - 1.0f * vPixelOffset).r;
	vSamples0123.w	= SampleDepthBuffer(tShadowMap, vTexCoord).r;
	
	float3	vSamples456_;
	vSamples456_.x	= SampleDepthBuffer(tShadowMap, vTexCoord + 1.0f * vPixelOffset).r;
	vSamples456_.y	= SampleDepthBuffer(tShadowMap, vTexCoord + 2.0f * vPixelOffset).r;
	vSamples456_.z	= SampleDepthBuffer(tShadowMap, vTexCoord + 3.0f * vPixelOffset).r;

	vSamples0123 = vScaleBias.x * vSamples0123 + vScaleBias.y;
	vSamples456_ = vScaleBias.x * vSamples456_ + vScaleBias.y;

	float2	vSum;
	vSum.x	= ExponentialConvolution(vWeights.x, vSamples0123.x, vWeights.y, vSamples0123.y);
	vSum.y	= ExponentialConvolution(vWeights.y, vSamples456_.y, vWeights.x, vSamples456_.z);
	vSum.x	= ExponentialConvolution(1.0f, vSum.x, vWeights.z, vSamples0123.z);
	vSum.y	= ExponentialConvolution(1.0f, vSum.y, vWeights.z, vSamples456_.x);
	vSum.x	= ExponentialConvolution(1.0f, vSum.x, vWeights.w, vSamples0123.w);

	return ExponentialConvolution(1.0f, vSum.x, 1.0f, vSum.y);
}

// Xbox 360-specific optimisations
