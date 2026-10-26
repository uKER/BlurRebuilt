#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_ELEMENTARY_H
#define SOFT_MATHS_HVEC_NEW_ELEMENTARY_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_ELEMENTARY_H
#define MATHS_HVEC_NEW_ELEMENTARY_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup ElementaryFunctions
/// @{

//
// VF32 Elementary Functions
//

/// VF32 Sine
NOALIAS __forceinline const VF32 Sin( const VF32 VECREF theta );

/// VF32 Cosine
NOALIAS __forceinline const VF32 Cos( const VF32 VECREF theta );

/// VF32 Sine & Cosine
NOALIAS __forceinline void SinCos( const VF32 VECREF theta, VF32* const __restrict pSinTheta, VF32* const __restrict pCosTheta );

/// VF32 Sine & Cosine of 2 input angles
NOALIAS __forceinline void SinCos2( const VF32 VECREF alpha, const VF32 VECREF beta,
                                    VF32* const __restrict pSinAlpha, VF32* const __restrict pCosAlpha,
                                    VF32* const __restrict pSinBeta,  VF32* const __restrict pCosBeta );

/// VF32 Tangent
NOALIAS __forceinline const VF32 Tan( const VF32 VECREF theta );


/// VF32 Arc-Sine
NOALIAS __forceinline const VF32 ArcSin( const VF32 VECREF x );

/// VF32 Arc-Cosine
NOALIAS __forceinline const VF32 ArcCos( const VF32 VECREF x );

/// VF32 Arc-Tangent
NOALIAS __forceinline const VF32 ArcTan( const VF32 VECREF x );

/// VF32 Arc-Tangent
NOALIAS __forceinline const VF32 ArcTan( const VF32 VECREF x, const VF32 VECREF y );


/// VF32 Natural Logarithm ( Base e )
NOALIAS __forceinline const VF32 Log( const VF32 VECREF x );

/// VF32 Logarithm Base N
/// NOTE: It's more efficient to multiply Log(x) by a pre-computed 1/Log(n) for a large number of calculations with the same base!
NOALIAS __forceinline const VF32 Log( const VF32 VECREF n, const VF32 VECREF x );

/// VF32 Exponential e ^ z
NOALIAS __forceinline const VF32 Exp( const VF32 VECREF x );

/// VF32 Power x ^ y
/// WARNING: This implementation gives UNDEFINED results for negative x!
/// (If you need a power function that handles negative integer x correctly then let me know! JASOND)
NOALIAS __forceinline const VF32 Pow( const VF32 VECREF x, const VF32 VECREF y );





//
// Vec4F32 Elementary Functions
//

/// Vec4F32 Sine
NOALIAS __forceinline const Vec4F32 Sin( const Vec4F32 VECREF theta );

/// Vec4F32 Cosine
NOALIAS __forceinline const Vec4F32 Cos( const Vec4F32 VECREF theta );

/// Vec4F32 Tangent
NOALIAS __forceinline const Vec4F32 Tan( const Vec4F32 VECREF theta );

/// Vec4F32 Sine & Cosine : returns Vec4F32 with elements x=sin(theta.x), y=cos(theta.y), z=sin(theta.z), w=cos(theta.w)
NOALIAS __forceinline const Vec4F32 SinCos( const Vec4F32 VECREF theta );

/// Vec4F32 Arc-Sine
NOALIAS __forceinline const Vec4F32 ArcSin( const Vec4F32 VECREF x );

/// Vec4F32 Arc-Cosine
NOALIAS __forceinline const Vec4F32 ArcCos( const Vec4F32 VECREF x );

/// Vec4F32 Arc-Tangent
NOALIAS __forceinline const Vec4F32 ArcTan( const Vec4F32 VECREF x );

/// Vec4F32 Arc-Tangent
NOALIAS __forceinline const Vec4F32 ArcTan( const Vec4F32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Natural Logarithm
NOALIAS __forceinline const Vec4F32 Log( const Vec4F32 VECREF x );

/// Vec4F32 Logarithm Base N
/// NOTE: It's more efficient to multiply Log(x) by a pre-computed 1/Log(n) for a large number of calculations with the same base!
NOALIAS __forceinline const Vec4F32 Log( const Vec4F32 VECREF n, const Vec4F32 VECREF x );

/// Vec4F32 Exponential e ^ z
NOALIAS __forceinline const Vec4F32 Exp( const Vec4F32 VECREF x );

/// Vec4F32 Power x ^ y
/// WARNING: This implementation gives UNDEFINED results for negative x!
/// (If you need a power function that handles negative integer x correctly then let me know! JASOND)
NOALIAS __forceinline const Vec4F32 Pow( const Vec4F32 VECREF x, const Vec4F32 VECREF y );





//
// Vector3 Elementary Functions
//

/// Vector3 Sine
NOALIAS __forceinline const Vector3 Sin( const Vector3 PCREF theta );

/// Vector3 Cosine
NOALIAS __forceinline const Vector3 Cos( const Vector3 PCREF theta );

/// Vector3 Tangent
NOALIAS __forceinline const Vector3 Tan( const Vector3 PCREF theta );

/// Vector3 Arc-Sine
NOALIAS __forceinline const Vector3 ArcSin( const Vector3 PCREF x );

/// Vector3 Arc-Cosine
NOALIAS __forceinline const Vector3 ArcCos( const Vector3 PCREF x );

/// Vector3 Arc-Tangent
NOALIAS __forceinline const Vector3 ArcTan( const Vector3 PCREF x );

/// Vector3 Arc-Tangent
NOALIAS __forceinline const Vector3 ArcTan( const Vector3 PCREF x, const Vector3 PCREF y );

/// Vector3 Natural Logarithm
NOALIAS __forceinline const Vector3 Log( const Vector3 PCREF x );

/// Vector3 Logarithm Base N
/// NOTE: It's more efficient to multiply Log(x) by a pre-computed 1/Log(n) for a large number of calculations with the same base!
NOALIAS __forceinline const Vector3 Log( const Vector3 PCREF n, const Vector3 PCREF x );

/// Vector3 Exponential e ^ z
NOALIAS __forceinline const Vector3 Exp( const Vector3 PCREF x );

/// Vector3 Power x ^ y
/// WARNING: This implementation gives UNDEFINED results for negative x!
/// (If you need a power function that handles negative integer x correctly then let me know! JASOND)
NOALIAS __forceinline const Vector3 Pow( const Vector3 PCREF x, const Vector3 PCREF y );




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

