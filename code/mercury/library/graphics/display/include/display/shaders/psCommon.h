#if defined(PS3) // || defined(PCD3D9)
#define USE_HALFPRECISION
#endif

#if defined(USE_HALFPRECISION)

#define	colour1		half
#define	colour2		half2
#define	colour3		half3
#define	colour4		half4
#define	real		half
#define	real2		half2
#define	real3		half3
#define	real4		half4
#define	real3x3		half3x3
#define	real4x3		half4x3
#define	real4x4		half4x4

#else

#define	colour1		float
#define	colour2		float2
#define	colour3		float3
#define	colour4		float4
#define	real		float
#define	real2		float2
#define	real3		float3
#define	real4		float4
#define	real3x3		float3x3
#define	real4x3		float4x3
#define	real4x4		float4x4

#endif

#if !defined(PS3)

#define h4tex1D(A, B)		tex1D(A, B)
#define h4tex1Dproj(A, B)	tex1Dproj(A, B)
#define h4tex2D(A, B)		tex2D(A, B)
#define h4tex2Dproj(A, B)	tex2Dproj(A, B)
#define h4tex3D(A, B)		tex3D(A, B)
#define h4tex3Dproj(A, B)	tex3Dproj(A, B)
#define h4texCUBE(A, B)		texCUBE(A, B)
#define h4texCUBEproj(A, B)	texCUBEproj(A, B)

#endif

#include "psConstants.h"
