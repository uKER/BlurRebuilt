#ifndef MATHS_TRIGONOMETRY_H
#define MATHS_TRIGONOMETRY_H

#ifdef _MSC_VER
#pragma once
#endif

#define TRIG_INLINE	inline

#if defined (PLATFORM_PS3)
#include <fastmath.h>
#endif


namespace Maths
{

enum E_PI_CONSTANTS
{
    E_PI_CONSTANTS_PI              = 0, // PI
    E_PI_CONSTANTS_TWO_PI          = 1, // PI * 2.0f
    E_PI_CONSTANTS_RCP_PI          = 2, // 1.0f / PI
    E_PI_CONSTANTS_RCP_TWO_PI      = 3, // 2.0f / PI
    E_PI_CONSTANTS_PI_DIV_TWO      = 4, // PI / 2.0f
    E_PI_CONSTANTS_PI_DIV_FOUR     = 5, // PI / 4.0f
    E_PI_CONSTANTS_SIN_PI_DIV_FOUR = 6, // sin( PI / 4.0f )
};

#define MATHS_EXP_TABLE_SIZE        176
#define MATHS_EXP_TABLE_ZERO_OFFSET 88

extern ALIGN16 const F32 g_Const_PI[];

extern ALIGN16 const F32 g_Const_BernoulliNumber[];
extern ALIGN16 const F32 g_Const_SinSeries[];
extern ALIGN16 const F32 g_Const_CosSeries[];
extern ALIGN16 const F32 g_Const_TanSeries[];
extern ALIGN16 const F32 g_Const_ArcSinSeries[];
extern ALIGN16 const F32 g_Const_ArcTanSeries[];
extern ALIGN16 const F32 g_Const_LogSeries[];
extern ALIGN16 const F32 g_Const_ExpSeries[];
extern ALIGN16 const F32 g_Const_ExpTable[];


#if defined( _SIMD_ )

/// SIMD scalar Sin: Using power series expansion
NOALIAS const V128 SIMD_ScalarSin( const V128 PCREF theta ) __attribute__(( const ));

/// SIMD scalar Cos: Using power series expansion
NOALIAS const V128 SIMD_ScalarCos( const V128 PCREF theta ) __attribute__(( const ));

/// SIMD scalar Tan: Using power series expansion
NOALIAS const V128 SIMD_ScalarTan( const V128 PCREF theta ) __attribute__(( const ));

/// SIMD vector Sin: Using power series expansion
NOALIAS const V128 SIMD_VectorSin( const V128 PCREF theta ) __attribute__(( const ));

/// SIMD vector Cos: Using power series expansion
NOALIAS const V128 SIMD_VectorCos( const V128 PCREF theta ) __attribute__(( const ));

/// SIMD vector Tan: Using power series expansion
NOALIAS const V128 SIMD_VectorTan( const V128 PCREF theta ) __attribute__(( const ));


/// SIMD scalar Sin: Using power series expansion
NOALIAS const V128 SIMD_ScalarArcSin( const V128 PCREF x ) __attribute__(( const ));

/// SIMD scalar Cos: Using power series expansion
NOALIAS __forceinline const V128 SIMD_ScalarArcCos( const V128 PCREF x ) __attribute__(( const ));

/// SIMD scalar Tan: Using power series expansion
NOALIAS const V128 SIMD_ScalarArcTan( const V128 PCREF x ) __attribute__(( const ));

/// SIMD scalar Tan: Using power series expansion
NOALIAS const V128 SIMD_ScalarArcTan( const V128 PCREF x, const V128 PCREF y ) __attribute__(( const ));

/// SIMD vector Sin: Using power series expansion
NOALIAS const V128 SIMD_VectorArcSin( const V128 PCREF x ) __attribute__(( const ));

/// SIMD vector Cos: Using power series expansion
NOALIAS const V128 SIMD_VectorArcCos( const V128 PCREF x ) __attribute__(( const ));

/// SIMD vector Tan: Using power series expansion
NOALIAS const V128 SIMD_VectorArcTan( const V128 PCREF x ) __attribute__(( const ));

/// SIMD vector Tan: Using power series expansion
NOALIAS const V128 SIMD_VectorArcTan( const V128 PCREF x, const V128 PCREF y ) __attribute__(( const ));

/// SIMD scalar Natural Logarithm: Using power series expansion
NOALIAS const V128 SIMD_ScalarLog( const V128 PCREF z ) __attribute__(( const ));

/// SIMD vector Natural Logarithm: Using power series expansion
NOALIAS const V128 SIMD_VectorLog( const V128 PCREF z ) __attribute__(( const ));

/// SIMD scalar Exponential e ^ z : Using power series expansion
NOALIAS const V128 SIMD_ScalarExp( const V128 PCREF z ) __attribute__(( const ));

/// SIMD vector Exponential e ^ z : Using power series expansion
NOALIAS const V128 SIMD_VectorExp( const V128 PCREF z ) __attribute__(( const ));

/// SIMD scalar Power x ^ y : Using power series expansion
//NOALIAS const V128 SIMD_ScalarPow( const V128 PCREF x, const V128 PCREF y ) __attribute__(( const ));

#endif



/// HF32 Calculate Sin( theta ) from Cos( theta )
NOALIAS __forceinline const HF32 CosTheta_To_SinTheta( const HF32 PCREF cosTheta );

/// HF32 Calculate Cos( theta ) from Sin( theta )
NOALIAS __forceinline const HF32 SinTheta_To_CosTheta( const HF32 PCREF sinTheta );

/// HF32 Calculate Sin( theta ) from Tan( theta )
NOALIAS __forceinline const HF32 TanTheta_To_SinTheta( const HF32 PCREF tanTheta );

/// HF32 Calculate Cos( theta ) from Tan( theta )
NOALIAS __forceinline const HF32 TanTheta_To_CosTheta( const HF32 PCREF tanTheta );

/// HF32 Calculate Sin( theta / 2 ) from Cos( theta )
NOALIAS __forceinline const HF32 CosTheta_To_SinHalfTheta( const HF32 PCREF cosTheta );

/// HF32 calculate Tan( theta / 2 ) from Cos( theta )
NOALIAS __forceinline const HF32 CosTheta_To_TanHalfTheta( const HF32 PCREF cosTheta );

/// HF32 Calculate Tan( 2 * theta ) from Tan( theta )
NOALIAS __forceinline const HF32 TanTheta_To_TanTwoTheta( const HF32 PCREF tanTheta );



/// HF32 Sin: Using power series expansion
//NOALIAS const HF32 Sin( const HF32 PCREF theta );
#if defined (PLATFORM_PS3)
NOALIAS __forceinline const HF32 Sin(const HF32 PCREF theta) { return f_sinf(theta); }
#else
NOALIAS __forceinline const HF32 Sin(const HF32 PCREF theta) { return sinf(theta); }
#endif

/// HF32 Tan: Using power series expansion
NOALIAS const HF32 Tan( const HF32 PCREF theta );

/// HF32 ArcSin: Using power series expansion
NOALIAS const HF32 ArcSin( const HF32 PCREF z );

/// HF32 ArcTan: Using power series expansion
NOALIAS const HF32 ArcTan( const HF32 PCREF z );


/// HVec4 Sin: Using power series expansion
#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU)
NOALIAS __forceinline const HVec4 Sin(const HVec4 PCREF theta ) { return HVec4(sinf4(theta)); }
#else
NOALIAS const HVec4 Sin( const HVec4 PCREF theta );
#endif

/// HVec4 ArcSin: Using power series expansion
NOALIAS const HVec4 ArcSin( const HVec4 PCREF z );


/// HF32 Cos: Using power series expansion
NOALIAS __forceinline const HF32 Cos( const HF32 PCREF theta );

/// HF32 ArcCos: Using power series expansion
NOALIAS __forceinline const HF32 ArcCos( const HF32 PCREF z );

/// HF32 ArcTan: Using power series expansion
NOALIAS __forceinline const HF32 ArcTan( const HF32 PCREF y, const HF32 PCREF x );


/// HVec4 Cos: Using power series expansion
NOALIAS __forceinline const HVec4 Cos( const HVec4 PCREF theta );

/// HVec4 ArcCos: Using power series expansion
NOALIAS __forceinline const HVec4 ArcCos( const HVec4 PCREF z );



/// HVec4 Calculate Sine from Cosine, using fundamental identity: Sin(theta) = Sqrt( 1 - Cos^2(theta) )
NOALIAS __forceinline const HVec4 CosTheta_To_SinTheta( const HVec4 PCREF cosTheta );

/// HVec4 Calculate Cosine from Sine, using fundamental identity: Cos(theta) = Sqrt( 1 - Sin^2(theta) )
NOALIAS __forceinline const HVec4 SinTheta_To_CosTheta( const HVec4 PCREF sinTheta );




// Expose power series for explicitely inlined versions
//extern ALIGN16 const F32 g_Const_PI[8];
//extern ALIGN16 const F32 g_Const_BernoulliNumber[64];
//extern ALIGN16 const F32 g_Const_SinSeries[16];
//extern ALIGN16 const F32 g_Const_TanSeries[16];
//extern ALIGN16 const F32 g_Const_ArcSinSeries[16];
//extern ALIGN16 const F32 g_Const_ArcTanSeries[32];


}; // namespace Maths



#endif



