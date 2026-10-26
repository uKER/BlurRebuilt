// shared parameters
shared float4	g_Luminance : register(c27);
shared texture	g_LuminanceTexture;

#define		g_fGain				g_Luminance.x
#define		g_fGamma			g_Luminance.x
#define		g_fLuminanceScale	g_Luminance.y
#define		g_fExposureLow		g_Luminance.z
#define		g_fExposureHigh		g_Luminance.w

// shared samplers
sampler LuminanceSampler =
sampler_state
{
	Texture = <g_LuminanceTexture>;
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

// constants
static const float3	vLuminance			= float3(0.2125f, 0.7154f, 0.0721f);
static const float	fDelta				= 0.0001f;
static const float	fFrameBufferDelta	= 0.004f;

// functions
float GetLogLuminance(float3 colour)
{
	return log(dot(colour, vLuminance) + fDelta);
}

float GetLogLuminanceScaled(float3 colour)
{
	return log(dot(colour, vLuminance) + fDelta) * g_fLuminanceScale;
}

float GetLuminanceScaled(float3 colour)
{
	// make sure the lowest value written to the alpha channel is 1/255
	return max((dot(colour, vLuminance) + fDelta) * g_fLuminanceScale, fFrameBufferDelta);
}


float4 GetExposure(float4 vColour, float fAlpha)
{
	return float4(vColour.rgb, fAlpha);
}


float4 GetExposure(float4 vColour)
{
	return GetExposure(vColour, 1.0f);
}
