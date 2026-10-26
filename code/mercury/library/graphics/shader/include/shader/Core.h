#ifndef SHADER_CORE_H
#define SHADER_CORE_H

/*****************************************************************************/
// Section: Build Configurations
/*****************************************************************************/

//Supported Build Configurations. Must be defined by project

// BUILD_CONFIG_DEBUG, BUILD_CONFIG_RELEASE, BUILD_CONFIG_PROFILE, 
// BUILD_CONFIG_SPEED, BUILD_CONFIG_MASTER

// The above configs should not be used directly for specifying conditional 
// code compilation. The following defines will provide the necessary 
// information such as "does this need to be removed for a Speed build" or
// "this should only be included for paranoid validation of structures".
// Some of these will be defined depending on the selected build config.

// Supported build related defines, which can be enabled as needed.

// CFG_VALIDATE, CFG_PROFILE, CFG_SPEED, CFG_GOLD, CFG_DEBUG

// #define CFG_DEBUG
// #define CFG_VALIDATE
// #define CFG_SPEED
// #define CFG_PROFILE
// #define CFG_GOLD

// The following sets up defines depending on build config. 

#if defined(BUILD_CONFIG_DEBUG)
#define CFG_DEBUG 1
#elif defined(BUILD_CONFIG_RELEASE)
#define CFG_RELEASE 1	
#elif defined(BUILD_CONFIG_PROFILE)
#define CFG_PROFILE 1
#define CFG_SPEED 1
#elif defined(BUILD_CONFIG_SPEED)
#define CFG_SPEED 1
#elif defined(BUILD_CONFIG_MASTER)
#define CFG_SPEED 1
#define CFG_GOLD 1
#else
//#error ("*** ERROR: Unknown build configuration")
#endif

//
#if !(PLATFORM_PC + 0 + PLATFORM_PS3 + MAYA)
#define MAYA
#define XSI //This means we can't have shaders that work in Maya AND Xsi, as the Maya plugin #defines MAYA
#endif

#if defined(MAYA) || defined(XSI)
#define CFG_CONVERTER
#endif

#ifndef FXFILE_PARSER
	#define annotation(x)
	#if PLATFORM_PS3 && !defined(__VARIABLE_PARSING_STAGE__)
		#ifndef shared
			#define shared
		#endif
	#endif
#else
	#define annotation(x) ___VERTEX_DECLARATION_ANNOTATION___ #x
#endif

#if defined(CFG_CONVERTER)
#define SRGB_TEXTURE
#else
#define SRGB_TEXTURE SRGBTEXTURE = TRUE;
#endif

#ifdef HALF_PRECISION
	#define	real		half
	#define	real2		half2
	#define	real3		half3
	#define	real4		half4
	#define real2x2		half2x2
	#define	real3x3		half3x3
	#define	real4x3		half4x3
	#define	real4x4		half4x4
#else
	#define	real		float
	#define	real2		float2
	#define	real3		float3
	#define	real4		float4
	#define real2x2		float2x2
	#define	real3x3		float3x3
	#define	real4x3		float4x3
	#define	real4x4		float4x4
#endif

#if !defined(PLATFORM_PS3) || !defined(__PIXEL__)
	#define h4tex2D(A, B)		tex2D(A, B)
	#define h4tex2Dbias(A, B)	tex2Dbias(A, B)
	#define h4tex2Dlod(A, B)	tex2Dlod(A, B)
	#define h4tex3D(A, B)		tex3D(A, B)
	#define h4tex3Dbias(A, B)	tex3Dbias(A, B)
	#define h4tex3Dlod(A, B)	tex3Dlod(A, B)
	#define h4texCUBE(A, B)		texCUBE(A, B)
	#define h4texCUBEbias(A, B)	texCUBEbias(A, B)
	#define h4texCUBElod(A, B)	texCUBElod(A, B)
#endif

// Stop PC HLSL compiler generating error "NaN and infinity literals not allowed by shader model"
#ifdef PLATFORM_PC
#define safe_pow(X,Y) pow(max(0.00001f,X),Y)
#else
#define safe_pow(X,Y) pow(X,Y)
#endif

#endif // SHADER_CORE_H
