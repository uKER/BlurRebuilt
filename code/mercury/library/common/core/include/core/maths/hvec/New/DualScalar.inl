#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_DUAL_SCALAR_INL
#define SOFT_MATHS_HVEC_NEW_DUAL_SCALAR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_DUAL_SCALAR_INL
#define MATHS_HVEC_NEW_DUAL_SCALAR_INL
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
NOALIAS __forceinline const DualScalar MakeDualScalar( const F32& real )
{
    DualScalar result;
    result.m_V = MakeVec4F32( real, 0.0f );
    return result;
}

/// Make DualScalar from Real & DualScalar pair of F32's
NOALIAS __forceinline const DualScalar MakeDualScalar( const F32& real, const F32& dual )
{
    DualScalar result;
    result.m_V = MakeVec4F32( real, dual );
    return result;
}


/// Make DualScalar from Real VF32, dual part is zeroed
NOALIAS __forceinline const DualScalar MakeDualScalar( const VF32 VECREF real )
{
    DualScalar result;
    const VF32 zero( Const<VF32>::Zero() );
    result.m_V = MakeVec4F32( real, zero );
    return result;
}

/// Make DualScalar from Real & DualScalar pair of VF32's
NOALIAS __forceinline const DualScalar MakeDualScalar( const VF32 VECREF real, const VF32 VECREF dual )
{
    DualScalar result;
    result.m_V = MakeVec4F32( real, dual );
    return result;
}

/// DualScalar get Real part
NOALIAS __forceinline const VF32 Real( const DualScalar PCREF a )
{
    return X( a.m_V );
}

/// DualScalar get Dual part
NOALIAS __forceinline const VF32 Dual( const DualScalar PCREF a )
{
    return Y( a.m_V );
}

/// DualScalar Select
NOALIAS __forceinline const DualScalar Select( const VBool PCREF condition, const DualScalar PCREF result_if_true, const DualScalar PCREF result_if_false )
{
    DualScalar result;
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    return result;
}


/// DualScalar Conjugate
NOALIAS __forceinline const DualScalar Conjugate( const DualScalar PCREF a )
{
    DualScalar result;
    result.m_V = NegateY( a.m_V );
    return result;
}

/// DualScalar Inverse: 1/a, a * inverse(a) = 1 (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Inverse( const DualScalar PCREF a )
{
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 rcp_a0( Rcp( a0 ) );
    return MakeDualScalar( rcp_a0, -ae * Square( rcp_a0 ) );
}

/// DualScalar Inverse: 1/a, a * inverse(a) = 1 (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Inverse24( const DualScalar PCREF a )
{
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 rcp_a0( Rcp24( a0 ) );
    return MakeDualScalar( rcp_a0, -ae * Square( rcp_a0 ) );
}


/// DualScalar Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Div( const DualScalar PCREF a, const DualScalar PCREF b )
{
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 b0( Real( b ) );
    const VF32 be( Dual( b ) );
    const VF32 rcp_b0( Rcp( b0 ) );
    const VF32 rcp_be( -be * Square( rcp_b0 ) );
    DualScalar result;
    result.m_V = MakeVec4F32( a0 * rcp_b0, MulAdd( a0, rcp_be, ae * rcp_b0 ) );
    return result;
}

/// DualScalar Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Div24( const DualScalar PCREF a, const DualScalar PCREF b )
{
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 b0( Real( b ) );
    const VF32 be( Dual( b ) );
    const VF32 rcp_b0( Rcp24( b0 ) );
    const VF32 rcp_be( -be * Square( rcp_b0 ) );
    DualScalar result;
    result.m_V = MakeVec4F32( a0 * rcp_b0, MulAdd( a0, rcp_be, ae * rcp_b0 ) );
    return result;
}


/// DualScalar-Scalar Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Div( const DualScalar PCREF a, const VF32 VECREF b0 )
{
    //const VF32 a0( Real( a ) );
    //const VF32 ae( Dual( a ) );
    const VF32 rcp_b0( Rcp( b0 ) );
    DualScalar result;
    result.m_V = a.m_V * rcp_b0;
    return result;
}

/// DualScalar-Scalar Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Div24( const DualScalar PCREF a, const VF32 VECREF b0 )
{
    //const VF32 a0( Real( a ) );
    //const VF32 ae( Dual( a ) );
    const VF32 rcp_b0( Rcp24( b0 ) );
    DualScalar result;
    result.m_V = a.m_V * rcp_b0;
    return result;
}


/// Scalar-DualScalar Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Div( const VF32 VECREF a0, const DualScalar PCREF b )
{
    const VF32 b0( Real( b ) );
    const VF32 be( Dual( b ) );
    const VF32 rcp_b0( Rcp( b0 ) );
    const VF32 rcp_be( -be * Square( rcp_b0 ) );
    DualScalar result;
    result.m_V = MakeVec4F32( a0 * rcp_b0, a0 * rcp_be );
    return result;
}

/// Scalar-DualScalar Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Div24( const VF32 VECREF a0, const DualScalar PCREF b )
{
    const VF32 b0( Real( b ) );
    const VF32 be( Dual( b ) );
    const VF32 rcp_b0( Rcp24( b0 ) );
    const VF32 rcp_be( -be * Square( rcp_b0 ) );
    DualScalar result;
    result.m_V = MakeVec4F32( a0 * rcp_b0, a0 * rcp_be );
    return result;
}


/// DualScalar Square
NOALIAS __forceinline const DualScalar Square( const DualScalar PCREF a )
{
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 a0a0( a0 * a0 );
    const VF32 a0ae( a0 * ae );
    DualScalar result;
    result.m_V = MakeVec4F32( a0a0, a0ae + a0ae );
    return result;
}

/// DualScalar Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar Sqrt( const DualScalar PCREF a )
{
    const VF32 half( Const<VF32>::Half() );
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    VF32 sqrt_a0, rcp_sqrt_a0;
    SqrtAndRcpSqrt( a0, &sqrt_a0, &rcp_sqrt_a0 );
    return MakeDualScalar( sqrt_a0, ae * half * rcp_sqrt_a0 );
}

/// DualScalar Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar Sqrt24( const DualScalar PCREF a )
{
    const VF32 half( Const<VF32>::Half() );
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    VF32 sqrt_a0, rcp_sqrt_a0;
    SqrtAndRcpSqrt24( a0, &sqrt_a0, &rcp_sqrt_a0 );
    return MakeDualScalar( sqrt_a0, ae * half * rcp_sqrt_a0 );
}


/// DualScalar Conjugate
NOALIAS __forceinline const DualScalar operator ! ( const DualScalar PCREF a )
{
    return Conjugate( a );
}

/// DualScalar Negation
NOALIAS __forceinline const DualScalar operator - ( const DualScalar PCREF a )
{
    DualScalar result;
    result.m_V = -a.m_V;
    return result;
}

/// DualScalar Addition
NOALIAS __forceinline const DualScalar operator + ( const DualScalar PCREF a, const DualScalar PCREF b )
{
    DualScalar result;
    result.m_V = a.m_V + b.m_V;
    return result;
}

/// DualScalar Subtraction
NOALIAS __forceinline const DualScalar operator - ( const DualScalar PCREF a, const DualScalar PCREF b )
{
    DualScalar result;
    result.m_V = a.m_V - b.m_V;
    return result;
}

/// DualScalar Multiplication
NOALIAS __forceinline const DualScalar operator * ( const DualScalar PCREF a, const DualScalar PCREF b )
{
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 b0( Real( b ) );
    const VF32 be( Dual( b ) );
    DualScalar result;
    result.m_V = MakeVec4F32( a0 * b0, MulAdd( a0, be, ae * b0 ) );
    return result;
}

/// DualScalar Division
NOALIAS __forceinline const DualScalar operator / ( const DualScalar PCREF a, const DualScalar PCREF b )
{
    return Div( a, b );
}


/// DualScalar-Scalar Addition
NOALIAS __forceinline const DualScalar operator + ( const DualScalar PCREF a, const VF32 VECREF b0 )
{
    return a + MakeDualScalar( b0 );
}

/// Scalar-DualScalar Addition
NOALIAS __forceinline const DualScalar operator + ( const VF32 VECREF a0, const DualScalar PCREF b )
{
    return MakeDualScalar( a0 ) + b;
}


/// DualScalar-Scalar Subtraction
NOALIAS __forceinline const DualScalar operator - ( const DualScalar PCREF a, const VF32 VECREF b0 )
{
    return a - MakeDualScalar( b0 );
}

/// Scalar-DualScalar Subtraction
NOALIAS __forceinline const DualScalar operator - ( const VF32 VECREF a0, const DualScalar PCREF b )
{
    return MakeDualScalar( a0 ) - b;
}


/// DualScalar-Scalar Multiplication
NOALIAS __forceinline const DualScalar operator * ( const DualScalar PCREF a, const VF32 VECREF b0 )
{
    DualScalar result;
    result.m_V = a.m_V * b0;
    return result;
}

/// Scalar-DualScalar Multiplication
NOALIAS __forceinline const DualScalar operator * ( const VF32 VECREF a0, const DualScalar PCREF b )
{
    DualScalar result;
    result.m_V = a0 * b.m_V;
    return result;
}


/// DualScalar-Scalar Division
NOALIAS __forceinline const DualScalar operator / ( const DualScalar PCREF a, const VF32 VECREF b0 )
{
    return Div( a, b0 );
}

/// Scalar-DualScalar Division
NOALIAS __forceinline const DualScalar operator / ( const VF32 VECREF a0, const DualScalar PCREF b )
{
    return Div( a0, b );
}


/// DualScalar Sin
NOALIAS __forceinline const DualScalar Sin( const DualScalar PCREF a )
{
    /// sin( a0 + e.ae ) = sin(a0) + e.ae.cos(a0)
    VF32 sin_a0, cos_a0;
    SinCos( Real( a ), &sin_a0, &cos_a0 );
    return MakeDualScalar( sin_a0, Dual( a ) * cos_a0 );
}

/// DualScalar Cos
NOALIAS __forceinline const DualScalar Cos( const DualScalar PCREF a )
{
    /// cos( a0 + e.ae ) = cos(a0) - e.ae.sin(a0)
    VF32 sin_a0, cos_a0;
    SinCos( Real( a ), &sin_a0, &cos_a0 );
    return MakeDualScalar( cos_a0, -Dual( a ) * sin_a0 );
}

/// DualScalar SinCos
NOALIAS __forceinline void SinCos( const DualScalar PCREF a, DualScalar* const __restrict pSinA, DualScalar* const __restrict pCosA )
{
    /// sin( a0 + e.ae ) = sin(a0) + e.ae.cos(a0)
    /// cos( a0 + e.ae ) = cos(a0) - e.ae.sin(a0)
    VF32 sin_a0, cos_a0;
    SinCos( Real( a ), &sin_a0, &cos_a0 );
    *pSinA = MakeDualScalar( sin_a0, Dual( a ) * cos_a0 );
    *pCosA = MakeDualScalar( cos_a0, -Dual( a ) * sin_a0 );
}

/// DualScalar ArcSin (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar ArcSin( const DualScalar PCREF a )
{
    // b = ArcSin( a )
    // a0 = Sin( b0 )
    // ae = be * Cos( b0 )
    // b0 = ArcSin( a0 )
    // be = ae / Cos( b0 )
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 b0( ArcSin( a0 ) );
    const VF32 be( ae / Cos( b0 ) );
    return MakeDualScalar( b0, be );
}

/// DualScalar ArcSin (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar ArcSin24( const DualScalar PCREF a )
{
    // b = ArcSin( a )
    // a0 = Sin( b0 )
    // ae = be * Cos( b0 )
    // b0 = ArcSin( a0 )
    // be = ae / Cos( b0 )
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 b0( ArcSin( a0 ) );
    const VF32 be( Div24( ae, Cos( b0 ) ) );
    return MakeDualScalar( b0, be );
}

/// DualScalar ArcCos (faster but only 22 bit precision)
NOALIAS __forceinline const DualScalar ArcCos( const DualScalar PCREF a )
{
    // b = ArcCos( a )
    // a0 = Cos( b0 )
    // ae = -be * Sin( b0 )
    // b0 = ArcCos( a0 )
    // be = -ae / Sin( b0 )
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 b0( ArcCos( a0 ) );
    const VF32 be( -ae / Sin( b0 ) );
    return MakeDualScalar( b0, be );
}

/// DualScalar ArcCos (slower but full 24 bit precision)
NOALIAS __forceinline const DualScalar ArcCos24( const DualScalar PCREF a )
{
    // b = ArcCos( a )
    // a0 = Cos( b0 )
    // ae = -be * Sin( b0 )
    // b0 = ArcCos( a0 )
    // be = -ae / Sin( b0 )
    const VF32 a0( Real( a ) );
    const VF32 ae( Dual( a ) );
    const VF32 b0( ArcCos( a0 ) );
    const VF32 be( Div24( -ae, Sin( b0 ) ) );
    return MakeDualScalar( b0, be );
}


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

