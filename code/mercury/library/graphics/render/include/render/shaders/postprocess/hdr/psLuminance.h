static const real3 vLuminance = real3(0.2125h, 0.7154h, 0.0721h);
static const real3 vHalfLuminance = real3(0.10625h, 0.3577h, 0.03605h);
static const real3 vTwoLuminance = real3(0.4250h, 1.4038h, 0.1442h);
static const real3 vFourLuminance = real3(0.8500h, 2.8616h, 0.2884h);
static const real fDelta = 0.000h;
static const real fFrameBufferDelta = 0.004h;

#define		g_fExposure			g_vLuminance.x
#define		g_fInvExposure		g_vLuminance.y
#define		g_fLuminanceScale	g_vLuminance.z

float GetLogLuminance(float3 colour)
{
	return log(dot(colour, vLuminance) + fDelta);
}

float GetLogLuminanceScaled(float3 colour)
{
	return log(dot(colour, vLuminance) + fDelta) * g_fLuminanceScale;
}

float GetLuminance(float3 colour)
{
	// make sure the lowest value written to the alpha channel is 1/255
	return max((dot(colour, vLuminance) + fDelta), fFrameBufferDelta);
}

float GetLuminanceScaled(float3 colour)
{
	return saturate(1.0f - dot(colour, vFourLuminance));
}

half GetLogLuminance(half3 colour)
{
	return log(dot(colour, vLuminance) + fDelta);
}

half GetLogLuminanceScaled(half3 colour)
{
	return log(dot(colour, vLuminance) + fDelta) * g_fLuminanceScale;
}

half GetLuminance(half3 colour)
{
	// make sure the lowest value written to the alpha channel is 1/255
	return max((dot(colour, vLuminance) + fDelta), fFrameBufferDelta);
}

half GetLuminanceScaled(half3 colour)
{
	return saturate(1.0h - dot(colour, vFourLuminance));
}
