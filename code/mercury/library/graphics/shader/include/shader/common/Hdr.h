#ifndef SHADER_HDR_H
#define SHADER_HDR_H

#if !defined(CFG_CONVERTER)

#define HDRU32

// constants
shared real4		g_vLuminance		: register(CCP_HDR_LUMINANCE) = real4(1, 1, 1, 0);

static const real3	vLuminance			= real3(0.2125h, 0.7154h, 0.0721h);
static const real3	vHalfLuminance		= real3(0.10625h, 0.3577h, 0.03605h);
static const real3	vTwoLuminance		= real3(0.4250h, 1.4038h, 0.1442h);
static const real3	vFourLuminance		= real3(0.8500h, 2.8616h, 0.2884h);
static const real	fDelta				= 0.0001h;
static const real	fFrameBufferDelta	= 0.004h;

// [Blur2PC][HDR][retail-recovered] Retail ps_3_0 exposure emits rsq then
// rcp per channel (Default_LN_NB_NL_NV/MaterialLights). D3D9 rsq takes
// abs(source); SM5 sqrt of a negative lighting value instead produces NaN.
// Preserve the retail magnitude conversion without limiting HDR radiance.
real3 ExposureGamma(real3 value)
{
#if defined(BLUR_DX11)
    return sqrt(abs(value));
#else
    return sqrt(value);
#endif
}

// defines
#define			g_fGain						g_vLuminance.x
#define			g_fInvGain					g_vLuminance.y
#define			g_fExposure					g_vLuminance.x
#define			g_fInvExposure				g_vLuminance.y
#define			g_fLuminanceScale			g_vLuminance.z

// functions
real GetLogLuminance(real3 vColour)
{
	return log(dot(vColour, vLuminance) + fDelta);
}

real GetLogLuminanceScaled(real3 vColour)
{
	return log(dot(vColour, vLuminance) + fDelta) * g_fLuminanceScale;
}

real GetLuminance(real3 vColour)
{
	// make sure the lowest value written to the alpha channel is 1/255
	return max((dot(vColour, vLuminance) + fDelta), fFrameBufferDelta);
}

real GetLuminanceScaled(real3 vColour)
{
	// make sure the lowest value written to the alpha channel is 1/255
	return saturate(1.0h - (dot(vColour, vFourLuminance)));
}

// does HDR in the shader but still writes alpha as normal
// mainly for use with 7e3 or 6e4 compressed float surfaces
#ifdef HDR

real4 GetExposure(real4 vRGBA, real fAlpha)
{
	real4	vColour		= 0.0h;
	
	vColour.rgb = vRGBA.rgb * g_fExposure;
	vColour.rgb = ExposureGamma(vColour.rgb);

	#ifdef ALPHA_AND_ADDITIVE
		vColour = real4(vColour.rgb * vRGBA.a, (1.0h - fAlpha) * vRGBA.a); 
	#else
		#ifdef ALPHA_PREMULTIPLY
			vColour = real4(vColour.rgb * vRGBA.a, vRGBA.a);
		#else
			vColour.a = vRGBA.a;
		#endif
	#endif

	return vColour;
}

#endif

#if !defined(ALPHA_LUMINANCE) || defined(LOD)	// disable ALPHA during the opaque pass (in which we write out luminance in the alpha instead)
#define ALPHA
#endif

// uses a single U32 rendertarget for HDR tonemapping
#ifdef HDRU32

real4 GetExposure(real4 vRGBA, real fAlpha)
{
	real4	vColour		= 0.0h;
	
#ifdef ALPHA
	// exposes value but doesnt contribute to luminance
	vColour.rgb = vRGBA.rgb * g_fExposure;
	vColour.rgb = ExposureGamma(vColour.rgb);

	// need to do alpha premultiply after gamma
	#ifdef ALPHA_AND_ADDITIVE
		vColour = real4(vColour.rgb * vRGBA.a, (1.0h - fAlpha) * vRGBA.a); 
	#else
		#ifdef ALPHA_PREMULTIPLY
			vColour = real4(vColour.rgb * vRGBA.a, vRGBA.a);
		#else
			vColour.a = vRGBA.a;
		#endif
	#endif
#else
	// exposes value and writes luminance to alpha channel
	vColour.a = GetLuminanceScaled(vRGBA.rgb);
	vColour.rgb = vRGBA.rgb * g_fExposure;
	vColour.rgb = ExposureGamma(vColour.rgb);
#endif

	return vColour;
}
#endif

#ifndef HDR
#ifndef HDRU32
#ifndef HDRU32X2

real4 GetExposure(real4 vRGBA, real fAlpha)
{
	// apply gain
	real4	vColour = vRGBA * g_fGain;

	// need to do alpha premultiply after gamma
	#ifdef ALPHA_AND_ADDITIVE
		vColour = real4(vRGBA.rgb * vRGBA.a, (1.0h - fAlpha) * vRGBA.a);
	#else
		#ifdef ALPHA_PREMULTIPLY
			vColour = real4(vRGBA.rgb * vRGBA.a, vRGBA.a);
		#else
			vColour.a = vRGBA.a;
		#endif
	#endif

	return vColour;
}

#endif
#endif
#endif

real4 GetExposure(real4 vColour)
{
	return GetExposure(vColour, 1.0h);
}
#else // MAYA & XSI

real4 GetExposure(real4 vColour)
{
	return saturate(vColour);
}

#endif // MAYA & XSI

real3 InverseExposure(real3 vRGB)
{
#if defined(HDR) || defined(HDRU32)
	real3	vInversedColour	= vRGB * vRGB * g_fInvExposure;
#else
#if !defined(CFG_CONVERTER)
	real3	vInversedColour	= vRGB * g_fInvGain;
#else
	real3	vInversedColour	= vRGB;
#endif
#endif
	return vInversedColour;
}

#endif // SHADER_HDR_H
