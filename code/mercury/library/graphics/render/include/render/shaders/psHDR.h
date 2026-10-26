#include "PostProcess\HDR\psLuminance.h"

// does HDR in the shader but still writes alpha as normal
// mainly for use with 7e3 or 6e4 compressed float surfaces
#ifdef HDR

struct PS_OUTPUT
{
	real4 colour	: COLOR0;
};

PS_OUTPUT GetExposure(real4 colour, real alpha)
{
	PS_OUTPUT output;

	output.colour.rgb = colour.rgb * g_fExposure;
	
	#ifndef ADDITIVE
		output.colour.rgb = sqrt(output.colour.rgb);
	#endif

	// need to do alpha premultiply after gamma
	#ifdef ALPHA_AND_ADDITIVE
		// lerp between alpha and additive based on alpha
		output.colour = real4(output.colour.rgb * colour.a, alpha * colour.a);
	#else
		#ifdef ALPHA_PREMULTIPLY
			// alpha
			output.colour = real4(output.colour.rgb * colour.a, colour.a);
		#else
			output.colour.a = colour.a;
		#endif
	#endif

	return output;
}
#endif

// uses a single U32 rendertarget for HDR tonemapping
#ifdef HDRU32

struct PS_OUTPUT
{
	real4 colour	: COLOR0;
};

PS_OUTPUT GetExposure(real4 colour, real alpha)
{
	PS_OUTPUT output;

#ifdef ALPHA
	// exposes value but doesnt contribute to luminance
	output.colour.rgb = colour.rgb * g_fExposure;
	
	#ifndef ADDITIVE
		output.colour.rgb = sqrt(output.colour.rgb);
	#endif

	// need to do alpha premultiply after gamma
	#ifdef ALPHA_AND_ADDITIVE
		// lerp between alpha and additive based on alpha
		output.colour = real4(output.colour.rgb * colour.a, alpha * colour.a);
	#else
		#ifdef ALPHA_PREMULTIPLY
			// alpha
			output.colour = real4(output.colour.rgb * colour.a, colour.a);
		#else
			output.colour.a = colour.a;
		#endif
	#endif

#else
	// exposes value and writes luminance to alpha channel
	output.colour.a = GetLuminanceScaled(colour.rgb);
	output.colour.rgb = colour.rgb * g_fExposure;
	#ifndef ADDITIVE
		output.colour.rgb = sqrt(output.colour.rgb);
	#endif
#endif

	return output;
}
#endif

#ifndef HDR
#ifndef HDRU32

struct PS_OUTPUT
{
	real4 colour	: COLOR0;
};

PS_OUTPUT GetExposure(real4 colour, real alpha)
{
	PS_OUTPUT output;

	output.colour = ApplyGain(colour);

	// need to do alpha premultiply after gamma
	#ifdef ALPHA_AND_ADDITIVE
		// lerp between alpha and additive based on alpha
		output.colour = real4(output.colour.rgb * colour.a, alpha * colour.a);
	#else
		#ifdef ALPHA_PREMULTIPLY
			// alpha
			output.colour = float4(output.colour.rgb * colour.a, colour.a);
		#else
			output.colour.a = colour.a;
		#endif
	#endif

	return output;
}

#endif
#endif

PS_OUTPUT GetExposure(real4 colour)
{
	return GetExposure(colour, 1.0h);
}
