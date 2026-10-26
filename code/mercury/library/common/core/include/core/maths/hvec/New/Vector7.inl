#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR7_INL
#define SOFT_MATHS_HVEC_NEW_VECTOR7_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR7_INL
#define MATHS_HVEC_NEW_VECTOR7_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector7
/// @{

/// Load Vector7 from F32 ptr
template <> NOALIAS __forceinline const Vector7 Load( F32 const * const __restrict pSrc )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Load<Vec8F32>( pSrc );
    #else
    result.m_V[0] = pSrc[0];
    result.m_V[1] = pSrc[1];
    result.m_V[2] = pSrc[2];
    result.m_V[3] = pSrc[3];
    result.m_V[4] = pSrc[4];
    result.m_V[5] = pSrc[5];
    result.m_V[6] = pSrc[6];
    #endif
    return result;
}

/// Store Vector7 to F32 ptr
NOALIAS __forceinline void Store( const Vector7& data, F32 * const __restrict pDst )
{
    #if defined( _SIMD_ )
    Store( data.m_V.m_V0, (StorageVec4F32*)pDst );
    Store( data.m_V.m_V1, (StorageVec3F32*)(pDst+4) );
    #else
    pDst[0] = data.m_V[0];
    pDst[1] = data.m_V[1];
    pDst[2] = data.m_V[2];
    pDst[3] = data.m_V[3];
    pDst[4] = data.m_V[4];
    pDst[5] = data.m_V[5];
    pDst[6] = data.m_V[6];
    #endif
}


/// Load Vector7 from StorageVector7
NOALIAS __forceinline const Vector7 Load( StorageVector7 const * const __restrict pSrc )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Load<Vec8F32>( (F32 const *)pSrc );
    #else
    result.m_V[0] = pSrc->m_Data.v[0];
    result.m_V[1] = pSrc->m_Data.v[1];
    result.m_V[2] = pSrc->m_Data.v[2];
    result.m_V[3] = pSrc->m_Data.v[3];
    result.m_V[4] = pSrc->m_Data.v[4];
    result.m_V[5] = pSrc->m_Data.v[5];
    result.m_V[6] = pSrc->m_Data.v[6];
    #endif
    return result;
}

/// Store Vector7 in StorageVector7
NOALIAS __forceinline void Store( const Vector7& data, StorageVector7 * const __restrict pDst )
{
    Store( data, (F32*)pDst );
}


/// Make Vector7 from broadcast VF32
NOALIAS __forceinline const Vector7 MakeVector7( const VF32 VECREF src )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = MakeVec8F32( src );
    #else
    result.m_V[0] =
    result.m_V[1] =
    result.m_V[2] =
    result.m_V[3] =
    result.m_V[4] =
    result.m_V[5] =
    result.m_V[6] = src.m_V;
    #endif
    return result;
}

/// Make Vector7 from 7 VF32's
NOALIAS __forceinline const Vector7 MakeVector7( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6 )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = MakeVec8F32( e0, e1, e2, e3, e4, e5, e6 );
    #else
    result.m_V[0] = e0.m_V;
    result.m_V[1] = e1.m_V;
    result.m_V[2] = e2.m_V;
    result.m_V[3] = e3.m_V;
    result.m_V[4] = e4.m_V;
    result.m_V[5] = e5.m_V;
    result.m_V[6] = e6.m_V;
    #endif
    return result;
}


/// Vector7 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector7& v )
{
    BIZ_STATIC_ASSERT( I < 7 );
    #if defined( _SIMD_ )
    return Element<I>( v.m_V );
    #else
    return MakeVF32( v.m_V[I] );
    #endif
}

/// Vector7 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector7* const __restrict pDst, const VF32 VECREF ei )
{
    BIZ_STATIC_ASSERT( I < 7 );
    #if defined( _SIMD_ )
    SetElement<I>( &pDst->m_V, ei );
    #else
    pDst->m_V[I] = ei.m_V;
    #endif
}


/// Vector7 Abs of each element
NOALIAS __forceinline const Vector7 Abs( const Vector7& v )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Abs( v.m_V );
    #else
    result.m_V[0] = Abs( v.m_V[0] );
    result.m_V[1] = Abs( v.m_V[1] );
    result.m_V[2] = Abs( v.m_V[2] );
    result.m_V[3] = Abs( v.m_V[3] );
    result.m_V[4] = Abs( v.m_V[4] );
    result.m_V[5] = Abs( v.m_V[5] );
    result.m_V[6] = Abs( v.m_V[6] );
    #endif
    return result;
}

/// Vector7 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector7 Sign( const Vector7& v )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Sign( v.m_V );
    #else
    result.m_V[0] = ( v.m_V[0] < 0.0f ? -1.0f : ( v.m_V[0] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[1] = ( v.m_V[1] < 0.0f ? -1.0f : ( v.m_V[1] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[2] = ( v.m_V[2] < 0.0f ? -1.0f : ( v.m_V[2] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[3] = ( v.m_V[3] < 0.0f ? -1.0f : ( v.m_V[3] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[4] = ( v.m_V[4] < 0.0f ? -1.0f : ( v.m_V[4] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[5] = ( v.m_V[5] < 0.0f ? -1.0f : ( v.m_V[5] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[6] = ( v.m_V[6] < 0.0f ? -1.0f : ( v.m_V[6] > 0.0f ? 1.0f : 0.0f ) );
    #endif
    return result;
}

/// Vector7 CopySign of each element : Returns a Vector7 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector7 CopySign( const Vector7& magnitude, const Vector7& sign )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = CopySign( magnitude.m_V, sign.m_V );
    #else
    result.m_V[0] = (F32)_copysign( magnitude.m_V[0], sign.m_V[0] );
    result.m_V[1] = (F32)_copysign( magnitude.m_V[1], sign.m_V[1] );
    result.m_V[2] = (F32)_copysign( magnitude.m_V[2], sign.m_V[2] );
    result.m_V[3] = (F32)_copysign( magnitude.m_V[3], sign.m_V[3] );
    result.m_V[4] = (F32)_copysign( magnitude.m_V[4], sign.m_V[4] );
    result.m_V[5] = (F32)_copysign( magnitude.m_V[5], sign.m_V[5] );
    result.m_V[6] = (F32)_copysign( magnitude.m_V[6], sign.m_V[6] );
    #endif
    return result;
}


/// Vector7 Negation
NOALIAS __forceinline const Vector7 operator - ( const Vector7& a )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = -a.m_V;
    #else
    result.m_V[0] = -a.m_V[0];
    result.m_V[1] = -a.m_V[1];
    result.m_V[2] = -a.m_V[2];
    result.m_V[3] = -a.m_V[3];
    result.m_V[4] = -a.m_V[4];
    result.m_V[5] = -a.m_V[5];
    result.m_V[6] = -a.m_V[6];
    #endif
    return result;
}

/// Vector7 Sum
NOALIAS __forceinline const Vector7 operator + ( const Vector7& a, const Vector7& b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V + b.m_V;
    #else
    result.m_V[0] = a.m_V[0] + b.m_V[0];
    result.m_V[1] = a.m_V[1] + b.m_V[1];
    result.m_V[2] = a.m_V[2] + b.m_V[2];
    result.m_V[3] = a.m_V[3] + b.m_V[3];
    result.m_V[4] = a.m_V[4] + b.m_V[4];
    result.m_V[5] = a.m_V[5] + b.m_V[5];
    result.m_V[6] = a.m_V[6] + b.m_V[6];
    #endif
    return result;
}

/// Vector7 Difference
NOALIAS __forceinline const Vector7 operator - ( const Vector7& a, const Vector7& b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V - b.m_V;
    #else
    result.m_V[0] = a.m_V[0] - b.m_V[0];
    result.m_V[1] = a.m_V[1] - b.m_V[1];
    result.m_V[2] = a.m_V[2] - b.m_V[2];
    result.m_V[3] = a.m_V[3] - b.m_V[3];
    result.m_V[4] = a.m_V[4] - b.m_V[4];
    result.m_V[5] = a.m_V[5] - b.m_V[5];
    result.m_V[6] = a.m_V[6] - b.m_V[6];
    #endif
    return result;
}

/// Vector7 Addition Update
NOALIAS __forceinline void operator += ( Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    a.m_V += b.m_V;
    #else
    a.m_V[0] += b.m_V[0];
    a.m_V[1] += b.m_V[1];
    a.m_V[2] += b.m_V[2];
    a.m_V[3] += b.m_V[3];
    a.m_V[4] += b.m_V[4];
    a.m_V[5] += b.m_V[5];
    a.m_V[6] += b.m_V[6];
    #endif
}

/// Vector7 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    a.m_V -= b.m_V;
    #else
    a.m_V[0] -= b.m_V[0];
    a.m_V[1] -= b.m_V[1];
    a.m_V[2] -= b.m_V[2];
    a.m_V[3] -= b.m_V[3];
    a.m_V[4] -= b.m_V[4];
    a.m_V[5] -= b.m_V[5];
    a.m_V[6] -= b.m_V[6];
    #endif
}


/// Vector7 Scalar Right Multiplication
NOALIAS __forceinline const Vector7 operator * ( const Vector7& a, const VF32 VECREF b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    //result.m_V.m_V0 = a.m_V.m_V0 * b;
    //result.m_V.m_V1 = a.m_V.m_V1 * b;
    result.m_V = a.m_V * b;
    #else
    result.m_V[0] = a.m_V[0] * b.m_V;
    result.m_V[1] = a.m_V[1] * b.m_V;
    result.m_V[2] = a.m_V[2] * b.m_V;
    result.m_V[3] = a.m_V[3] * b.m_V;
    result.m_V[4] = a.m_V[4] * b.m_V;
    result.m_V[5] = a.m_V[5] * b.m_V;
    result.m_V[6] = a.m_V[6] * b.m_V;
    #endif
    return result;
}

/// Vector7 Scalar Left Multiplication
NOALIAS __forceinline const Vector7 operator * ( const VF32 VECREF a, const Vector7& b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    //result.m_V.m_V0 = a * b.m_V.m_V0;
    //result.m_V.m_V1 = a * b.m_V.m_V1;
    result.m_V = a * b.m_V;
    #else
    result.m_V[0] = a.m_V * b.m_V[0];
    result.m_V[1] = a.m_V * b.m_V[1];
    result.m_V[2] = a.m_V * b.m_V[2];
    result.m_V[3] = a.m_V * b.m_V[3];
    result.m_V[4] = a.m_V * b.m_V[4];
    result.m_V[5] = a.m_V * b.m_V[5];
    result.m_V[6] = a.m_V * b.m_V[6];
    #endif
    return result;
}

/// Vector7 Scalar Right Division
NOALIAS __forceinline const Vector7 operator / ( const Vector7& a, const VF32 VECREF b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    //result.m_V.m_V0 = a.m_V.m_V0 * rcp_b;
    //result.m_V.m_V1 = a.m_V.m_V1 * rcp_b;
    result.m_V = a.m_V / b;
    #else
    const VF32 rcp_b( Rcp( b ) );
    result.m_V[0] = a.m_V[0] * rcp_b.m_V;
    result.m_V[1] = a.m_V[1] * rcp_b.m_V;
    result.m_V[2] = a.m_V[2] * rcp_b.m_V;
    result.m_V[3] = a.m_V[3] * rcp_b.m_V;
    result.m_V[4] = a.m_V[4] * rcp_b.m_V;
    result.m_V[5] = a.m_V[5] * rcp_b.m_V;
    result.m_V[6] = a.m_V[6] * rcp_b.m_V;
    #endif
    return result;
}

/// Vector7 Scalar Left Division
NOALIAS __forceinline const Vector7 operator / ( const VF32 VECREF a, const Vector7& b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    //result.m_V.m_V0 = a / b.m_V.m_V0;
    //result.m_V.m_V1 = a / b.m_V.m_V1;
    result.m_V = a / b.m_V;
    #else
    result.m_V[0] = a.m_V / b.m_V[0];
    result.m_V[1] = a.m_V / b.m_V[1];
    result.m_V[2] = a.m_V / b.m_V[2];
    result.m_V[3] = a.m_V / b.m_V[3];
    result.m_V[4] = a.m_V / b.m_V[4];
    result.m_V[5] = a.m_V / b.m_V[5];
    result.m_V[6] = a.m_V / b.m_V[6];
    #endif
    return result;
}

/// Vector7 Scalar Right Multiplication Update
NOALIAS __forceinline void operator *= ( Vector7& a, const VF32 VECREF b )
{
    #if defined( _SIMD_ )
    //a.m_V.m_V0 *= b;
    //a.m_V.m_V1 *= b;
    a.m_V *= b;
    #else
    a.m_V[0] *= b.m_V;
    a.m_V[1] *= b.m_V;
    a.m_V[2] *= b.m_V;
    a.m_V[3] *= b.m_V;
    a.m_V[4] *= b.m_V;
    a.m_V[5] *= b.m_V;
    a.m_V[6] *= b.m_V;
    #endif
}

/// Vector7 Scalar Right Division Update
NOALIAS __forceinline void operator /= ( Vector7& a, const VF32 VECREF b )
{
    const VF32 rcp_b( Rcp( b ) );
    #if defined( _SIMD_ )
    //a.m_V.m_V0 *= rcp_b;
    //a.m_V.m_V1 *= rcp_b;
    a.m_V *= rcp_b;
    #else
    a.m_V[0] *= rcp_b.m_V;
    a.m_V[1] *= rcp_b.m_V;
    a.m_V[2] *= rcp_b.m_V;
    a.m_V[3] *= rcp_b.m_V;
    a.m_V[4] *= rcp_b.m_V;
    a.m_V[5] *= rcp_b.m_V;
    a.m_V[6] *= rcp_b.m_V;
    #endif
}


/// Vector7 Multiplication Operator
NOALIAS __forceinline const Vector7 operator * ( const Vector7& a, const Vector7& b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V * b.m_V;
    #else
    result.m_V[0] = a.m_V[0] * b.m_V[0];
    result.m_V[1] = a.m_V[1] * b.m_V[1];
    result.m_V[2] = a.m_V[2] * b.m_V[2];
    result.m_V[3] = a.m_V[3] * b.m_V[3];
    result.m_V[4] = a.m_V[4] * b.m_V[4];
    result.m_V[5] = a.m_V[5] * b.m_V[5];
    result.m_V[6] = a.m_V[6] * b.m_V[6];
    #endif
    return result;
}

/// Vector7 Division Operator
NOALIAS __forceinline const Vector7 operator / ( const Vector7& a, const Vector7& b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V / b.m_V;
    #else
    result.m_V[0] = a.m_V[0] / b.m_V[0];
    result.m_V[1] = a.m_V[1] / b.m_V[1];
    result.m_V[2] = a.m_V[2] / b.m_V[2];
    result.m_V[3] = a.m_V[3] / b.m_V[3];
    result.m_V[4] = a.m_V[4] / b.m_V[4];
    result.m_V[5] = a.m_V[5] / b.m_V[5];
    result.m_V[6] = a.m_V[6] / b.m_V[6];
    #endif
    return result;
}

/// Vector7 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    a.m_V *= b.m_V;
    #else
    a.m_V[0] *= b.m_V[0];
    a.m_V[1] *= b.m_V[1];
    a.m_V[2] *= b.m_V[2];
    a.m_V[3] *= b.m_V[3];
    a.m_V[4] *= b.m_V[4];
    a.m_V[5] *= b.m_V[5];
    a.m_V[6] *= b.m_V[6];
    #endif
}

/// Vector7 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    a.m_V /= b.m_V;
    #else
    a.m_V[0] /= b.m_V[0];
    a.m_V[1] /= b.m_V[1];
    a.m_V[2] /= b.m_V[2];
    a.m_V[3] /= b.m_V[3];
    a.m_V[4] /= b.m_V[4];
    a.m_V[5] /= b.m_V[5];
    a.m_V[6] /= b.m_V[6];
    #endif
}


/// Vector7 Comparison a == b : Returns VBool representing results of a[0] == b[0] && ... && a[6] == b[6]
NOALIAS __forceinline const VBool operator == ( const Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    return Permute<0,1,2,3,4,5,6,6>( a.m_V ) == Permute<0,1,2,3,4,5,6,6>( b.m_V );
    #else
    VBool result;
    result.m_V = a.m_V[0] == b.m_V[0] &&
                 a.m_V[1] == b.m_V[1] &&
                 a.m_V[2] == b.m_V[2] &&
                 a.m_V[3] == b.m_V[3] &&
                 a.m_V[4] == b.m_V[4] &&
                 a.m_V[5] == b.m_V[5] &&
                 a.m_V[6] == b.m_V[6];
    return result;
    #endif
}

/// Vector7 Comparison a != b : Returns VBool representing results of a[0] != b[0] || ... || a[6] != b[6]
NOALIAS __forceinline const VBool operator != ( const Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    return Permute<0,1,2,3,4,5,6,6>( a.m_V ) != Permute<0,1,2,3,4,5,6,6>( b.m_V );
    #else
    VBool result;
    result.m_V = a.m_V[0] != b.m_V[0] ||
                 a.m_V[1] != b.m_V[1] ||
                 a.m_V[2] != b.m_V[2] ||
                 a.m_V[3] != b.m_V[3] ||
                 a.m_V[4] != b.m_V[4] ||
                 a.m_V[5] != b.m_V[5] ||
                 a.m_V[6] != b.m_V[6];
    return result;
    #endif
}

/// Vector7 Comparison a <= b : Returns Vec8Bool representing results of a[i] <= b[i], for each element i [0,6]
NOALIAS __forceinline const Vec8Bool operator <= ( const Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    return a.m_V <= b.m_V;
    #else
    Vec8Bool result;
    result.m_V[0] = a.m_V[0] <= b.m_V[0];
    result.m_V[1] = a.m_V[1] <= b.m_V[1];
    result.m_V[2] = a.m_V[2] <= b.m_V[2];
    result.m_V[3] = a.m_V[3] <= b.m_V[3];
    result.m_V[4] = a.m_V[4] <= b.m_V[4];
    result.m_V[5] = a.m_V[5] <= b.m_V[5];
    result.m_V[6] = a.m_V[6] <= b.m_V[6];
    return result;
    #endif
}

/// Vector7 Comparison a >= b : Returns Vec8Bool representing results of a[i] >= b[i], for each element i [0,6]
NOALIAS __forceinline const Vec8Bool operator >= ( const Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    return a.m_V >= b.m_V;
    #else
    Vec8Bool result;
    result.m_V[0] = a.m_V[0] >= b.m_V[0];
    result.m_V[1] = a.m_V[1] >= b.m_V[1];
    result.m_V[2] = a.m_V[2] >= b.m_V[2];
    result.m_V[3] = a.m_V[3] >= b.m_V[3];
    result.m_V[4] = a.m_V[4] >= b.m_V[4];
    result.m_V[5] = a.m_V[5] >= b.m_V[5];
    result.m_V[6] = a.m_V[6] >= b.m_V[6];
    return result;
    #endif
}

/// Vector7 Comparison a < b : Returns Vec8Bool representing results of a[i] < b[i], for each element i [0,6]
NOALIAS __forceinline const Vec8Bool operator < ( const Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    return a.m_V < b.m_V;
    #else
    Vec8Bool result;
    result.m_V[0] = a.m_V[0] < b.m_V[0];
    result.m_V[1] = a.m_V[1] < b.m_V[1];
    result.m_V[2] = a.m_V[2] < b.m_V[2];
    result.m_V[3] = a.m_V[3] < b.m_V[3];
    result.m_V[4] = a.m_V[4] < b.m_V[4];
    result.m_V[5] = a.m_V[5] < b.m_V[5];
    result.m_V[6] = a.m_V[6] < b.m_V[6];
    return result;
    #endif
}

/// Vector7 Comparison a > b : Returns Vec8Bool representing results of a[i] > b[i], for each element i [0,6]
NOALIAS __forceinline const Vec8Bool operator > ( const Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    return a.m_V > b.m_V;
    #else
    Vec8Bool result;
    result.m_V[0] = a.m_V[0] > b.m_V[0];
    result.m_V[1] = a.m_V[1] > b.m_V[1];
    result.m_V[2] = a.m_V[2] > b.m_V[2];
    result.m_V[3] = a.m_V[3] > b.m_V[3];
    result.m_V[4] = a.m_V[4] > b.m_V[4];
    result.m_V[5] = a.m_V[5] > b.m_V[5];
    result.m_V[6] = a.m_V[6] > b.m_V[6];
    return result;
    #endif
}

/// Vector7 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    return Ordered( Permute<0,1,2,3,4,5,6,6>( a.m_V ), Permute<0,1,2,3,4,5,6,6>( b.m_V ) );
    #else
    VBool result;
    result.m_V = !_isnan( a.m_V[0] ) && !_isnan( b.m_V[0] ) &&
                 !_isnan( a.m_V[1] ) && !_isnan( b.m_V[1] ) &&
                 !_isnan( a.m_V[2] ) && !_isnan( b.m_V[2] ) &&
                 !_isnan( a.m_V[3] ) && !_isnan( b.m_V[3] ) &&
                 !_isnan( a.m_V[4] ) && !_isnan( b.m_V[4] ) &&
                 !_isnan( a.m_V[5] ) && !_isnan( b.m_V[5] ) &&
                 !_isnan( a.m_V[6] ) && !_isnan( b.m_V[6] );
    return result;
    #endif
}

/// Vector7 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    return Unordered( Permute<0,1,2,3,4,5,6,6>( a.m_V ), Permute<0,1,2,3,4,5,6,6>( b.m_V ) );
    #else
    VBool result;
    result.m_V = _isnan( a.m_V[0] ) || _isnan( b.m_V[0] ) ||
                 _isnan( a.m_V[1] ) || _isnan( b.m_V[1] ) ||
                 _isnan( a.m_V[2] ) || _isnan( b.m_V[2] ) ||
                 _isnan( a.m_V[3] ) || _isnan( b.m_V[3] ) ||
                 _isnan( a.m_V[4] ) || _isnan( b.m_V[4] ) ||
                 _isnan( a.m_V[5] ) || _isnan( b.m_V[5] ) ||
                 _isnan( a.m_V[6] ) || _isnan( b.m_V[6] );
    return result;
    #endif
}


/// Vector7 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vector7& a )
{
    #if defined( _SIMD_ )
    return IsNan( Permute<0,1,2,3,4,5,6,6>( a.m_V ) );
    #else
    VBool result;
    result.m_V = _isnan( a.m_V[0] ) ||
                 _isnan( a.m_V[1] ) ||
                 _isnan( a.m_V[2] ) ||
                 _isnan( a.m_V[3] ) ||
                 _isnan( a.m_V[4] ) ||
                 _isnan( a.m_V[5] ) ||
                 _isnan( a.m_V[6] );
    return result;
    #endif
}

/// Vector7 Finite Test : true if a contains no elements that are NaN's and for each element i [0,6], abs(x[i]) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vector7& a )
{
    #if defined( _SIMD_ )
    return Finite( Permute<0,1,2,3,4,5,6,6>( a.m_V ) );
    #else
    VBool result;
    result.m_V = !_isnan( a.m_V[0] ) && ( Maths::Abs( a.m_V[0] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[1] ) && ( Maths::Abs( a.m_V[1] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[2] ) && ( Maths::Abs( a.m_V[2] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[3] ) && ( Maths::Abs( a.m_V[3] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[4] ) && ( Maths::Abs( a.m_V[4] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[5] ) && ( Maths::Abs( a.m_V[5] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[6] ) && ( Maths::Abs( a.m_V[6] ) <= FLT_MAX );
    return result;
    #endif
}


/// Vector7 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector7 Select( const VBool PCREF condition, const Vector7& result_if_true, const Vector7& result_if_false )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    #else
    result.m_V[0] = ( condition.m_V ? result_if_true.m_V[0] : result_if_false.m_V[0] );
    result.m_V[1] = ( condition.m_V ? result_if_true.m_V[1] : result_if_false.m_V[1] );
    result.m_V[2] = ( condition.m_V ? result_if_true.m_V[2] : result_if_false.m_V[2] );
    result.m_V[3] = ( condition.m_V ? result_if_true.m_V[3] : result_if_false.m_V[3] );
    result.m_V[4] = ( condition.m_V ? result_if_true.m_V[4] : result_if_false.m_V[4] );
    result.m_V[5] = ( condition.m_V ? result_if_true.m_V[5] : result_if_false.m_V[5] );
    result.m_V[6] = ( condition.m_V ? result_if_true.m_V[6] : result_if_false.m_V[6] );
    #endif
    return result;
}

/// Vector7 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector7 Select( const Vec8Bool PCREF condition, const Vector7& result_if_true, const Vector7& result_if_false )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    #else
    result.m_V[0] = ( condition.m_V[0] ? result_if_true.m_V[0] : result_if_false.m_V[0] );
    result.m_V[1] = ( condition.m_V[1] ? result_if_true.m_V[1] : result_if_false.m_V[1] );
    result.m_V[2] = ( condition.m_V[2] ? result_if_true.m_V[2] : result_if_false.m_V[2] );
    result.m_V[3] = ( condition.m_V[3] ? result_if_true.m_V[3] : result_if_false.m_V[3] );
    result.m_V[4] = ( condition.m_V[4] ? result_if_true.m_V[4] : result_if_false.m_V[4] );
    result.m_V[5] = ( condition.m_V[5] ? result_if_true.m_V[5] : result_if_false.m_V[5] );
    result.m_V[6] = ( condition.m_V[6] ? result_if_true.m_V[6] : result_if_false.m_V[6] );
    #endif
    return result;
}


/// Vector7 Bitwise AND: a & b : for each element i, returns ( b ? a[i] : zero )
NOALIAS __forceinline const Vector7 AND( const Vector7& a, const VBool PCREF b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = AND( a.m_V, b );
    #else
    result.m_V[0] = b.m_V ? a.m_V[0] : 0.0f;
    result.m_V[1] = b.m_V ? a.m_V[1] : 0.0f;
    result.m_V[2] = b.m_V ? a.m_V[2] : 0.0f;
    result.m_V[3] = b.m_V ? a.m_V[3] : 0.0f;
    result.m_V[4] = b.m_V ? a.m_V[4] : 0.0f;
    result.m_V[5] = b.m_V ? a.m_V[5] : 0.0f;
    result.m_V[6] = b.m_V ? a.m_V[6] : 0.0f;
    #endif
    return result;
}

/// Vector7 Bitwise ANDC: a & ~b : for each element i, returns ( b ? zero : a[i] )
NOALIAS __forceinline const Vector7 ANDC( const Vector7& a, const VBool PCREF b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = ANDC( a.m_V, b );
    #else
    result.m_V[0] = !b.m_V ? a.m_V[0] : 0.0f;
    result.m_V[1] = !b.m_V ? a.m_V[1] : 0.0f;
    result.m_V[2] = !b.m_V ? a.m_V[2] : 0.0f;
    result.m_V[3] = !b.m_V ? a.m_V[3] : 0.0f;
    result.m_V[4] = !b.m_V ? a.m_V[4] : 0.0f;
    result.m_V[5] = !b.m_V ? a.m_V[5] : 0.0f;
    result.m_V[6] = !b.m_V ? a.m_V[6] : 0.0f;
    #endif
    return result;
}


/// Vector7 Bitwise AND: a & b : for each element i, returns ( b[i] ? a[i] : zero )
NOALIAS __forceinline const Vector7 AND( const Vector7& a, const Vec8Bool PCREF b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = AND( a.m_V, b );
    #else
    result.m_V[0] = b.m_V[0] ? a.m_V[0] : 0.0f;
    result.m_V[1] = b.m_V[1] ? a.m_V[1] : 0.0f;
    result.m_V[2] = b.m_V[2] ? a.m_V[2] : 0.0f;
    result.m_V[3] = b.m_V[3] ? a.m_V[3] : 0.0f;
    result.m_V[4] = b.m_V[4] ? a.m_V[4] : 0.0f;
    result.m_V[5] = b.m_V[5] ? a.m_V[5] : 0.0f;
    result.m_V[6] = b.m_V[6] ? a.m_V[6] : 0.0f;
    #endif
    return result;
}

/// Vector7 Bitwise ANDC: a & ~b : for each element i, returns ( b[i] ? zero : a[i] )
NOALIAS __forceinline const Vector7 ANDC( const Vector7& a, const Vec8Bool PCREF b )
{
    Vector7 result;
    #if defined( _SIMD_ )
    result.m_V = ANDC( a.m_V, b );
    #else
    result.m_V[0] = !b.m_V[0] ? a.m_V[0] : 0.0f;
    result.m_V[1] = !b.m_V[1] ? a.m_V[1] : 0.0f;
    result.m_V[2] = !b.m_V[2] ? a.m_V[2] : 0.0f;
    result.m_V[3] = !b.m_V[3] ? a.m_V[3] : 0.0f;
    result.m_V[4] = !b.m_V[4] ? a.m_V[4] : 0.0f;
    result.m_V[5] = !b.m_V[5] ? a.m_V[5] : 0.0f;
    result.m_V[6] = !b.m_V[6] ? a.m_V[6] : 0.0f;
    #endif
    return result;
}


/// Vector7 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector7 Rcp( const Vector7& v )
{
	Vector7 result;
    #if defined( _SIMD_ )
	result.m_V = Rcp( v.m_V );
    #else
	result.m_V[0] = Rcp( v.m_V[0] );
	result.m_V[1] = Rcp( v.m_V[1] );
	result.m_V[2] = Rcp( v.m_V[2] );
	result.m_V[3] = Rcp( v.m_V[3] );
	result.m_V[4] = Rcp( v.m_V[4] );
	result.m_V[5] = Rcp( v.m_V[5] );
	result.m_V[6] = Rcp( v.m_V[6] );
    #endif
	return result;
}

/// Vector7 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector7 Rcp24( const Vector7& v )
{
	Vector7 result;
    #if defined( _SIMD_ )
	result.m_V = Rcp24( v.m_V );
    #else
	result.m_V[0] = Rcp( v.m_V[0] );
	result.m_V[1] = Rcp( v.m_V[1] );
	result.m_V[2] = Rcp( v.m_V[2] );
	result.m_V[3] = Rcp( v.m_V[3] );
	result.m_V[4] = Rcp( v.m_V[4] );
	result.m_V[5] = Rcp( v.m_V[5] );
	result.m_V[6] = Rcp( v.m_V[6] );
    #endif
	return result;
}


/// Vector7 Dot Product
NOALIAS __forceinline const VF32 Dot( const Vector7& a, const Vector7& b )
{
    #if defined( _SIMD_ )
    return Dot7( a.m_V, b.m_V );
    #else
    return MakeVF32( a.m_V[0] * b.m_V[0] +
                     a.m_V[1] * b.m_V[1] +
                     a.m_V[2] * b.m_V[2] +
                     a.m_V[3] * b.m_V[3] +
                     a.m_V[4] * b.m_V[4] +
                     a.m_V[5] * b.m_V[5] +
                     a.m_V[6] * b.m_V[6] );
    #endif
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

