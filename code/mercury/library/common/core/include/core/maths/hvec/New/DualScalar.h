#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_DUAL_SCALAR_H
#define SOFT_MATHS_HVEC_NEW_DUAL_SCALAR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_DUAL_SCALAR_H
#define MATHS_HVEC_NEW_DUAL_SCALAR_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup DualScalar
/// @{

/// Make DualScalar from Real F32, dual part is zeroed
NOALIAS __forceinline const DualScalar MakeDualScalar( const F32& real );

/// Make DualScalar from Real & Dual pair of F32's
NOALIAS __forceinline const DualScalar MakeDualScalar( const F32& real, const F32& dual );


/// Make DualScalar from Real VF32, dual part is zeroed
NOALIAS __forceinline const DualScalar MakeDualScalar( const VF32 VECREF real );

/// Make DualScalar from Real & Dual pair of VF32's
NOALIAS __forceinline const DualScalar MakeDualScalar( const VF32 VECREF real, const VF32 VECREF dual );


/// DualScalar get Real part
NOALIAS __forceinline const VF32 Real( const DualScalar PCREF a );

/// DualScalar get Dual part
NOALIAS __forceinline const VF32 Dual( const DualScalar PCREF a );


/// DualScalar Select
NOALIAS __forceinline const DualScalar Select( const VBool PCREF condition, const DualScalar PCREF result_if_true, const DualScalar PCREF result_if_false );


/// DualScalar Conjugate
NOALIAS __forceinline const DualScalar Conjugate( const DualScalar PCREF a );


/// DualScalar Inverse: 1/a, a * inverse(a) = 1 (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Inverse( const DualScalar PCREF a );

/// DualScalar Inverse: 1/a, a * inverse(a) = 1 (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Inverse24( const DualScalar PCREF a );


/// DualScalar Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Div( const DualScalar PCREF a, const DualScalar PCREF b );

/// DualScalar Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Div24( const DualScalar PCREF a, const DualScalar PCREF b );


/// DualScalar-Scalar Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Div( const DualScalar PCREF a, const VF32 VECREF b0 );

/// DualScalar-Scalar Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Div24( const DualScalar PCREF a, const VF32 VECREF b0 );


/// Scalar-DualScalar Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Div( const VF32 VECREF a0, const DualScalar PCREF b );

/// Scalar-DualScalar Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Div24( const VF32 VECREF a0, const DualScalar PCREF b );


/// DualScalar Square
NOALIAS __forceinline const DualScalar Square( const DualScalar PCREF a );


/// DualScalar Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Sqrt( const DualScalar PCREF a );

/// DualScalar Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Sqrt24( const DualScalar PCREF a );


/// DualScalar Conjugate
NOALIAS __forceinline const DualScalar operator ! ( const DualScalar PCREF a );

/// DualScalar Negation
NOALIAS __forceinline const DualScalar operator - ( const DualScalar PCREF a );


/// DualScalar Addition
NOALIAS __forceinline const DualScalar operator + ( const DualScalar PCREF a, const DualScalar PCREF b );

/// DualScalar Subtraction
NOALIAS __forceinline const DualScalar operator - ( const DualScalar PCREF a, const DualScalar PCREF b );

/// DualScalar Multiplication
NOALIAS __forceinline const DualScalar operator * ( const DualScalar PCREF a, const DualScalar PCREF b );

/// DualScalar Division
NOALIAS __forceinline const DualScalar operator / ( const DualScalar PCREF a, const DualScalar PCREF b );


/// DualScalar-Scalar Addition
NOALIAS __forceinline const DualScalar operator + ( const DualScalar PCREF a, const VF32 VECREF b0 );

/// Scalar-DualScalar Addition
NOALIAS __forceinline const DualScalar operator + ( const VF32 VECREF a0, const DualScalar PCREF b );


/// DualScalar-Scalar Subtraction
NOALIAS __forceinline const DualScalar operator - ( const DualScalar PCREF a, const VF32 VECREF b0 );

/// Scalar-DualScalar Subtraction
NOALIAS __forceinline const DualScalar operator - ( const VF32 VECREF a0, const DualScalar PCREF b );


/// DualScalar-Scalar Multiplication
NOALIAS __forceinline const DualScalar operator * ( const DualScalar PCREF a, const VF32 VECREF b0 );

/// Scalar-DualScalar Multiplication
NOALIAS __forceinline const DualScalar operator * ( const VF32 VECREF a0, const DualScalar PCREF b );


/// DualScalar-Scalar Division
NOALIAS __forceinline const DualScalar operator / ( const DualScalar PCREF a, const VF32 VECREF b0 );

/// Scalar-DualScalar Division
NOALIAS __forceinline const DualScalar operator / ( const VF32 VECREF a0, const DualScalar PCREF b );


/// DualScalar Sin
NOALIAS __forceinline const DualScalar Sin( const DualScalar PCREF a );

/// DualScalar Cos
NOALIAS __forceinline const DualScalar Cos( const DualScalar PCREF a );

/// DualScalar SinCos
NOALIAS __forceinline void SinCos( const DualScalar PCREF a, DualScalar* const __restrict pSinA, DualScalar* const __restrict pCosA );

/// DualScalar ArcSin (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar ArcSin( const DualScalar PCREF a );

/// DualScalar ArcSin (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar ArcSin24( const DualScalar PCREF a );

/// DualScalar ArcCos (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar ArcCos( const DualScalar PCREF a );

/// DualScalar ArcCos (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar ArcCos24( const DualScalar PCREF a );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

