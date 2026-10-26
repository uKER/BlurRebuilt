#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR5_INL
#define SOFT_MATHS_HVEC_NEW_VECTOR5_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR5_INL
#define MATHS_HVEC_NEW_VECTOR5_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector5
/// @{

/// Load Vector5 from F32 ptr
template <> NOALIAS __forceinline const Vector5 Load( F32 const * const __restrict pSrc )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Load<Vec8F32>( pSrc );
    #else
    result.m_V[0] = pSrc[0];
    result.m_V[1] = pSrc[1];
    result.m_V[2] = pSrc[2];
    result.m_V[3] = pSrc[3];
    result.m_V[4] = pSrc[4];
    #endif
    return result;
}

/// Store Vector5 to F32 ptr
NOALIAS __forceinline void Store( const Vector5& data, F32 * const __restrict pDst )
{
    #if defined( _SIMD_ )
    Store( data.m_V.m_V0, (StorageVec4F32*)pDst );
    Store( data.m_V.m_V1, (StorageVec1F32*)(pDst+4) );
    #else
    pDst[0] = data.m_V[0];
    pDst[1] = data.m_V[1];
    pDst[2] = data.m_V[2];
    pDst[3] = data.m_V[3];
    pDst[4] = data.m_V[4];
    #endif
}


/// Load Vector5 from StorageVector5
NOALIAS __forceinline const Vector5 Load( StorageVector5 const * const __restrict pSrc )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Load<Vec8F32>( (F32 const *)pSrc );
    #else
    result.m_V[0] = pSrc->m_Data.v[0];
    result.m_V[1] = pSrc->m_Data.v[1];
    result.m_V[2] = pSrc->m_Data.v[2];
    result.m_V[3] = pSrc->m_Data.v[3];
    result.m_V[4] = pSrc->m_Data.v[4];
    #endif
    return result;
}

/// Store Vector5 in StorageVector5
NOALIAS __forceinline void Store( const Vector5& data, StorageVector5 * const __restrict pDst )
{
    Store( data, (F32*)pDst );
}


/// Make Vector5 from broadcast VF32
NOALIAS __forceinline const Vector5 MakeVector5( const VF32 VECREF src )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = MakeVec8F32( src );
    #else
    result.m_V[0] =
    result.m_V[1] =
    result.m_V[2] =
    result.m_V[3] =
    result.m_V[4] = src.m_V;
    #endif
    return result;
}

/// Make Vector5 from 5 VF32's
NOALIAS __forceinline const Vector5 MakeVector5( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4 )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = MakeVec8F32( e0, e1, e2, e3, e4 );
    #else
    result.m_V[0] = e0.m_V;
    result.m_V[1] = e1.m_V;
    result.m_V[2] = e2.m_V;
    result.m_V[3] = e3.m_V;
    result.m_V[4] = e4.m_V;
    #endif
    return result;
}

/// Make Vector5 from a Vector6
NOALIAS __forceinline const Vector5 MakeVector5( const Vector6& v6 )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = v6.m_V.m_V0;
    result.m_V.m_V1 = v6.m_V.m_V1;
    #else
    result.m_V[0] = v6.m_V[0];
    result.m_V[1] = v6.m_V[1];
    result.m_V[2] = v6.m_V[2];
    result.m_V[3] = v6.m_V[3];
    result.m_V[4] = v6.m_V[4];
    #endif
    return result;
}

/// Make Vector5 from a Vector4 (elements 0-3) & a VF32 (element 4)
NOALIAS __forceinline const Vector5 MakeVector5( const Vector4 PCREF e0123, const VF32 VECREF e4 )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = e0123.m_V;
    result.m_V.m_V1 = MakeVec4F32( e4 );
    #else
    result.m_V[0] = e0123.m_V.m_V[0];
    result.m_V[1] = e0123.m_V.m_V[1];
    result.m_V[2] = e0123.m_V.m_V[2];
    result.m_V[3] = e0123.m_V.m_V[3];
    result.m_V[4] = e4.m_V;
    #endif
    return result;
}

/// Make Vector5 from a VF32 (element 0) & a Vector4 (elements 1-4)
NOALIAS __forceinline const Vector5 MakeVector5( const VF32 VECREF e0, const Vector4 PCREF e1234 )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = MakeVec4F32( e0, MakeVector3( e1234 ) );
    result.m_V.m_V1 = MakeVec4F32( W( e1234 ) );
    #else
    result.m_V[0] = e0.m_V;
    result.m_V[1] = e1234.m_V.m_V[0];
    result.m_V[2] = e1234.m_V.m_V[1];
    result.m_V[3] = e1234.m_V.m_V[2];
    result.m_V[4] = e1234.m_V.m_V[3];
    #endif
    return result;
}

/// Make Vector5 from a Vector3 (elements 0-2) & two VF32's (elements 3 & 4)
NOALIAS __forceinline const Vector5 MakeVector5( const Vector3 PCREF e012, const VF32 VECREF e3, const VF32 VECREF e4 )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = MakeVec4F32( e012, e3 );
    result.m_V.m_V1 = MakeVec4F32( e4 );
    #else
    result.m_V[0] = e012.m_V.m_V[0];
    result.m_V[1] = e012.m_V.m_V[1];
    result.m_V[2] = e012.m_V.m_V[2];
    result.m_V[3] = e3.m_V;
    result.m_V[4] = e4.m_V;
    #endif
    return result;
}


/// Vector5 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector5& v )
{
    BIZ_STATIC_ASSERT( I < 5 );
    #if defined( _SIMD_ )
    return Element<I>( v.m_V );
    #else
    return MakeVF32( v.m_V[I] );
    #endif
}

/// Vector5 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector5* const __restrict pDst, const VF32 VECREF ei )
{
    BIZ_STATIC_ASSERT( I < 5 );
    #if defined( _SIMD_ )
    SetElement<I>( &pDst->m_V, ei );
    #else
    pDst->m_V[I] = ei.m_V;
    #endif
}


/// Vector5 Permute : Indices select 5 result elements [I0,I1,I2,I3,I4] from [ 0:v0, 1:v1, 2:v2, 3:v3, 4:v4 ]
template < unsigned int I0, unsigned int I1, unsigned int I2, unsigned int I3,
           unsigned int I4 >
NOALIAS __forceinline const Vector5 Permute( const Vector5& v )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Permute<I0,I1,I2,I3,I4,I4,I4,I4>( v.m_V );
    #else
    result.m_V[0] = v.m_V[I0];
    result.m_V[1] = v.m_V[I1];
    result.m_V[2] = v.m_V[I2];
    result.m_V[3] = v.m_V[I3];
    result.m_V[4] = v.m_V[I4];
    #endif
    return result;
}


/// Vector5 Abs of each element
NOALIAS __forceinline const Vector5 Abs( const Vector5& v )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Abs( v.m_V );
    #else
    result.m_V[0] = Abs( v.m_V[0] );
    result.m_V[1] = Abs( v.m_V[1] );
    result.m_V[2] = Abs( v.m_V[2] );
    result.m_V[3] = Abs( v.m_V[3] );
    result.m_V[4] = Abs( v.m_V[4] );
    #endif
    return result;
}

/// Vector5 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector5 Sign( const Vector5& v )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Sign( v.m_V );
    #else
    result.m_V[0] = ( v.m_V[0] < 0.0f ? -1.0f : ( v.m_V[0] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[1] = ( v.m_V[1] < 0.0f ? -1.0f : ( v.m_V[1] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[2] = ( v.m_V[2] < 0.0f ? -1.0f : ( v.m_V[2] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[3] = ( v.m_V[3] < 0.0f ? -1.0f : ( v.m_V[3] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[4] = ( v.m_V[4] < 0.0f ? -1.0f : ( v.m_V[4] > 0.0f ? 1.0f : 0.0f ) );
    #endif
    return result;
}

/// Vector5 CopySign of each element : Returns a Vector5 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector5 CopySign( const Vector5& magnitude, const Vector5& sign )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = CopySign( magnitude.m_V, sign.m_V );
    #else
    result.m_V[0] = (F32)_copysign( magnitude.m_V[0], sign.m_V[0] );
    result.m_V[1] = (F32)_copysign( magnitude.m_V[1], sign.m_V[1] );
    result.m_V[2] = (F32)_copysign( magnitude.m_V[2], sign.m_V[2] );
    result.m_V[3] = (F32)_copysign( magnitude.m_V[3], sign.m_V[3] );
    result.m_V[4] = (F32)_copysign( magnitude.m_V[4], sign.m_V[4] );
    #endif
    return result;
}


/// Vector5 Negation
NOALIAS __forceinline const Vector5 operator - ( const Vector5& a )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = -a.m_V;
    #else
    result.m_V[0] = -a.m_V[0];
    result.m_V[1] = -a.m_V[1];
    result.m_V[2] = -a.m_V[2];
    result.m_V[3] = -a.m_V[3];
    result.m_V[4] = -a.m_V[4];
    #endif
    return result;
}

/// Vector5 Sum
NOALIAS __forceinline const Vector5 operator + ( const Vector5& a, const Vector5& b )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V + b.m_V;
    #else
    result.m_V[0] = a.m_V[0] + b.m_V[0];
    result.m_V[1] = a.m_V[1] + b.m_V[1];
    result.m_V[2] = a.m_V[2] + b.m_V[2];
    result.m_V[3] = a.m_V[3] + b.m_V[3];
    result.m_V[4] = a.m_V[4] + b.m_V[4];
    #endif
    return result;
}

/// Vector5 Difference
NOALIAS __forceinline const Vector5 operator - ( const Vector5& a, const Vector5& b )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V - b.m_V;
    #else
    result.m_V[0] = a.m_V[0] - b.m_V[0];
    result.m_V[1] = a.m_V[1] - b.m_V[1];
    result.m_V[2] = a.m_V[2] - b.m_V[2];
    result.m_V[3] = a.m_V[3] - b.m_V[3];
    result.m_V[4] = a.m_V[4] - b.m_V[4];
    #endif
    return result;
}

/// Vector5 Addition Update
NOALIAS __forceinline void operator += ( Vector5& a, const Vector5& b )
{
    #if defined( _SIMD_ )
    a.m_V += b.m_V;
    #else
    a.m_V[0] += b.m_V[0];
    a.m_V[1] += b.m_V[1];
    a.m_V[2] += b.m_V[2];
    a.m_V[3] += b.m_V[3];
    a.m_V[4] += b.m_V[4];
    #endif
}

/// Vector5 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector5& a, const Vector5& b )
{
    #if defined( _SIMD_ )
    a.m_V -= b.m_V;
    #else
    a.m_V[0] -= b.m_V[0];
    a.m_V[1] -= b.m_V[1];
    a.m_V[2] -= b.m_V[2];
    a.m_V[3] -= b.m_V[3];
    a.m_V[4] -= b.m_V[4];
    #endif
}


/// Vector5 Scalar Right Multiplication
NOALIAS __forceinline const Vector5 operator * ( const Vector5& a, const VF32 VECREF b )
{
    Vector5 result;
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
    #endif
    return result;
}

/// Vector5 Scalar Left Multiplication
NOALIAS __forceinline const Vector5 operator * ( const VF32 VECREF a, const Vector5& b )
{
    Vector5 result;
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
    #endif
    return result;
}

/// Vector5 Scalar Right Division
NOALIAS __forceinline const Vector5 operator / ( const Vector5& a, const VF32 VECREF b )
{
    Vector5 result;
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
    #endif
    return result;
}

/// Vector5 Scalar Left Division
NOALIAS __forceinline const Vector5 operator / ( const VF32 VECREF a, const Vector5& b )
{
    Vector5 result;
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
    #endif
    return result;
}

/// Vector5 Scalar Right Multiplication Update
NOALIAS __forceinline void operator *= ( Vector5& a, const VF32 VECREF b )
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
    #endif
}

/// Vector5 Scalar Right Division Update
NOALIAS __forceinline void operator /= ( Vector5& a, const VF32 VECREF b )
{
    #if defined( _SIMD_ )
    //a.m_V.m_V0 *= rcp_b;
    //a.m_V.m_V1 *= rcp_b;
    a.m_V /= b;
    #else
    const VF32 rcp_b( Rcp( b ) );
    a.m_V[0] *= rcp_b.m_V;
    a.m_V[1] *= rcp_b.m_V;
    a.m_V[2] *= rcp_b.m_V;
    a.m_V[3] *= rcp_b.m_V;
    a.m_V[4] *= rcp_b.m_V;
    #endif
}


/// Vector5 Multiplication Operator
NOALIAS __forceinline const Vector5 operator * ( const Vector5& a, const Vector5& b )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V * b.m_V;
    #else
    result.m_V[0] = a.m_V[0] * b.m_V[0];
    result.m_V[1] = a.m_V[1] * b.m_V[1];
    result.m_V[2] = a.m_V[2] * b.m_V[2];
    result.m_V[3] = a.m_V[3] * b.m_V[3];
    result.m_V[4] = a.m_V[4] * b.m_V[4];
    #endif
    return result;
}

/// Vector5 Division Operator
NOALIAS __forceinline const Vector5 operator / ( const Vector5& a, const Vector5& b )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V / b.m_V;
    #else
    result.m_V[0] = a.m_V[0] / b.m_V[0];
    result.m_V[1] = a.m_V[1] / b.m_V[1];
    result.m_V[2] = a.m_V[2] / b.m_V[2];
    result.m_V[3] = a.m_V[3] / b.m_V[3];
    result.m_V[4] = a.m_V[4] / b.m_V[4];
    #endif
    return result;
}

/// Vector5 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector5& a, const Vector5& b )
{
    #if defined( _SIMD_ )
    a.m_V *= b.m_V;
    #else
    a.m_V[0] *= b.m_V[0];
    a.m_V[1] *= b.m_V[1];
    a.m_V[2] *= b.m_V[2];
    a.m_V[3] *= b.m_V[3];
    a.m_V[4] *= b.m_V[4];
    #endif
}

/// Vector5 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector5& a, const Vector5& b )
{
    #if defined( _SIMD_ )
    a.m_V /= b.m_V;
    #else
    a.m_V[0] /= b.m_V[0];
    a.m_V[1] /= b.m_V[1];
    a.m_V[2] /= b.m_V[2];
    a.m_V[3] /= b.m_V[3];
    a.m_V[4] /= b.m_V[4];
    #endif
}


/// Vector5 Comparison a == b : Returns VBool representing results of a[0] == b[0] && ... && a[4] == b[4]
NOALIAS __forceinline const VBool operator == ( const Vector5& a, const Vector5& b )
{
    #if defined( _SIMD_ )
    return Permute<0,1,2,3,4,4,4,4>( a.m_V ) == Permute<0,1,2,3,4,4,4,4>( b.m_V );
    #else
    VBool result;
    result.m_V = a.m_V[0] == b.m_V[0] &&
                 a.m_V[1] == b.m_V[1] &&
                 a.m_V[2] == b.m_V[2] &&
                 a.m_V[3] == b.m_V[3] &&
                 a.m_V[4] == b.m_V[4];
    return result;
    #endif
}

/// Vector5 Comparison a != b : Returns VBool representing results of a[0] != b[0] || ... || a[4] != b[4]
NOALIAS __forceinline const VBool operator != ( const Vector5& a, const Vector5& b )
{
    #if defined( _SIMD_ )
    return Permute<0,1,2,3,4,4,4,4>( a.m_V ) != Permute<0,1,2,3,4,4,4,4>( b.m_V );
    #else
    VBool result;
    result.m_V = a.m_V[0] != b.m_V[0] ||
                 a.m_V[1] != b.m_V[1] ||
                 a.m_V[2] != b.m_V[2] ||
                 a.m_V[3] != b.m_V[3] ||
                 a.m_V[4] != b.m_V[4];
    return result;
    #endif
}

/// Vector5 Comparison a <= b : Returns Vec8Bool representing results of a[i] <= b[i], for each element i [0,4]
NOALIAS __forceinline const Vec8Bool operator <= ( const Vector5& a, const Vector5& b )
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
    return result;
    #endif
}

/// Vector5 Comparison a >= b : Returns Vec8Bool representing results of a[i] >= b[i], for each element i [0,4]
NOALIAS __forceinline const Vec8Bool operator >= ( const Vector5& a, const Vector5& b )
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
    return result;
    #endif
}

/// Vector5 Comparison a < b : Returns Vec8Bool representing results of a[i] < b[i], for each element i [0,4]
NOALIAS __forceinline const Vec8Bool operator < ( const Vector5& a, const Vector5& b )
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
    return result;
    #endif
}

/// Vector5 Comparison a > b : Returns Vec8Bool representing results of a[i] > b[i], for each element i [0,4]
NOALIAS __forceinline const Vec8Bool operator > ( const Vector5& a, const Vector5& b )
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
    return result;
    #endif
}

/// Vector5 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector5& a, const Vector5& b )
{
    #if defined( _SIMD_ )
    return Ordered( Permute<0,1,2,3,4,4,4,4>( a.m_V ), Permute<0,1,2,3,4,4,4,4>( b.m_V ) );
    #else
    VBool result;
    result.m_V = !_isnan( a.m_V[0] ) && !_isnan( b.m_V[0] ) &&
                 !_isnan( a.m_V[1] ) && !_isnan( b.m_V[1] ) &&
                 !_isnan( a.m_V[2] ) && !_isnan( b.m_V[2] ) &&
                 !_isnan( a.m_V[3] ) && !_isnan( b.m_V[3] ) &&
                 !_isnan( a.m_V[4] ) && !_isnan( b.m_V[4] );
    return result;
    #endif
}

/// Vector5 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector5& a, const Vector5& b )
{
    #if defined( _SIMD_ )
    return Unordered( Permute<0,1,2,3,4,4,4,4>( a.m_V ), Permute<0,1,2,3,4,4,4,4>( b.m_V ) );
    #else
    VBool result;
    result.m_V = _isnan( a.m_V[0] ) || _isnan( b.m_V[0] ) ||
                 _isnan( a.m_V[1] ) || _isnan( b.m_V[1] ) ||
                 _isnan( a.m_V[2] ) || _isnan( b.m_V[2] ) ||
                 _isnan( a.m_V[3] ) || _isnan( b.m_V[3] ) ||
                 _isnan( a.m_V[4] ) || _isnan( b.m_V[4] );
    return result;
    #endif
}


/// Vector5 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vector5& a )
{
    #if defined( _SIMD_ )
    return IsNan( Permute<0,1,2,3,4,4,4,4>( a.m_V ) );
    #else
    VBool result;
    result.m_V = _isnan( a.m_V[0] ) ||
                 _isnan( a.m_V[1] ) ||
                 _isnan( a.m_V[2] ) ||
                 _isnan( a.m_V[3] ) ||
                 _isnan( a.m_V[4] );
    return result;
    #endif
}

/// Vector5 Finite Test : true if a contains no elements that are NaN's and for each element i [0,4], abs(x[i]) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vector5& a )
{
    #if defined( _SIMD_ )
    return Finite( Permute<0,1,2,3,4,4,4,4>( a.m_V ) );
    #else
    VBool result;
    result.m_V = !_isnan( a.m_V[0] ) && ( Maths::Abs( a.m_V[0] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[1] ) && ( Maths::Abs( a.m_V[1] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[2] ) && ( Maths::Abs( a.m_V[2] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[3] ) && ( Maths::Abs( a.m_V[3] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[4] ) && ( Maths::Abs( a.m_V[4] ) <= FLT_MAX );
    return result;
    #endif
}


/// Vector5 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector5 Select( const VBool PCREF condition, const Vector5& result_if_true, const Vector5& result_if_false )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    #else
    result.m_V[0] = ( condition.m_V ? result_if_true.m_V[0] : result_if_false.m_V[0] );
    result.m_V[1] = ( condition.m_V ? result_if_true.m_V[1] : result_if_false.m_V[1] );
    result.m_V[2] = ( condition.m_V ? result_if_true.m_V[2] : result_if_false.m_V[2] );
    result.m_V[3] = ( condition.m_V ? result_if_true.m_V[3] : result_if_false.m_V[3] );
    result.m_V[4] = ( condition.m_V ? result_if_true.m_V[4] : result_if_false.m_V[4] );
    #endif
    return result;
}

/// Vector5 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector5 Select( const Vec8Bool PCREF condition, const Vector5& result_if_true, const Vector5& result_if_false )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    #else
    result.m_V[0] = ( condition.m_V[0] ? result_if_true.m_V[0] : result_if_false.m_V[0] );
    result.m_V[1] = ( condition.m_V[1] ? result_if_true.m_V[1] : result_if_false.m_V[1] );
    result.m_V[2] = ( condition.m_V[2] ? result_if_true.m_V[2] : result_if_false.m_V[2] );
    result.m_V[3] = ( condition.m_V[3] ? result_if_true.m_V[3] : result_if_false.m_V[3] );
    result.m_V[4] = ( condition.m_V[4] ? result_if_true.m_V[4] : result_if_false.m_V[4] );
    #endif
    return result;
}


/// Vector5 Bitwise AND: a & b : for each element i, returns ( b ? a[i] : zero )
NOALIAS __forceinline const Vector5 AND( const Vector5& a, const VBool PCREF b )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = AND( a.m_V, b );
    #else
    result.m_V[0] = b.m_V ? a.m_V[0] : 0.0f;
    result.m_V[1] = b.m_V ? a.m_V[1] : 0.0f;
    result.m_V[2] = b.m_V ? a.m_V[2] : 0.0f;
    result.m_V[3] = b.m_V ? a.m_V[3] : 0.0f;
    result.m_V[4] = b.m_V ? a.m_V[4] : 0.0f;
    #endif
    return result;
}

/// Vector5 Bitwise ANDC: a & ~b : for each element i, returns ( b ? zero : a[i] )
NOALIAS __forceinline const Vector5 ANDC( const Vector5& a, const VBool PCREF b )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = ANDC( a.m_V, b );
    #else
    result.m_V[0] = !b.m_V ? a.m_V[0] : 0.0f;
    result.m_V[1] = !b.m_V ? a.m_V[1] : 0.0f;
    result.m_V[2] = !b.m_V ? a.m_V[2] : 0.0f;
    result.m_V[3] = !b.m_V ? a.m_V[3] : 0.0f;
    result.m_V[4] = !b.m_V ? a.m_V[4] : 0.0f;
    #endif
    return result;
}


/// Vector5 Bitwise AND: a & b : for each element i, returns ( b[i] ? a[i] : zero )
NOALIAS __forceinline const Vector5 AND( const Vector5& a, const Vec8Bool PCREF b )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = AND( a.m_V, b );
    #else
    result.m_V[0] = b.m_V[0] ? a.m_V[0] : 0.0f;
    result.m_V[1] = b.m_V[1] ? a.m_V[1] : 0.0f;
    result.m_V[2] = b.m_V[2] ? a.m_V[2] : 0.0f;
    result.m_V[3] = b.m_V[3] ? a.m_V[3] : 0.0f;
    result.m_V[4] = b.m_V[4] ? a.m_V[4] : 0.0f;
    #endif
    return result;
}

/// Vector5 Bitwise ANDC: a & ~b : for each element i, returns ( b[i] ? zero : a[i] )
NOALIAS __forceinline const Vector5 ANDC( const Vector5& a, const Vec8Bool PCREF b )
{
    Vector5 result;
    #if defined( _SIMD_ )
    result.m_V = ANDC( a.m_V, b );
    #else
    result.m_V[0] = !b.m_V[0] ? a.m_V[0] : 0.0f;
    result.m_V[1] = !b.m_V[1] ? a.m_V[1] : 0.0f;
    result.m_V[2] = !b.m_V[2] ? a.m_V[2] : 0.0f;
    result.m_V[3] = !b.m_V[3] ? a.m_V[3] : 0.0f;
    result.m_V[4] = !b.m_V[4] ? a.m_V[4] : 0.0f;
    #endif
    return result;
}


/// Vector5 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector5 Rcp( const Vector5& v )
{
	Vector5 result;
    #if defined (_SIMD_)
	result.m_V = Rcp( v.m_V );
    #else
	result.m_V[0] = Rcp( v.m_V[0] );
	result.m_V[1] = Rcp( v.m_V[1] );
	result.m_V[2] = Rcp( v.m_V[2] );
	result.m_V[3] = Rcp( v.m_V[3] );
	result.m_V[4] = Rcp( v.m_V[4] );
    #endif
	return result;
}

/// Vector5 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector5 Rcp24( const Vector5& v )
{
	Vector5 result;
    #if defined (_SIMD_)
	result.m_V = Rcp24( v.m_V );
    #else
	result.m_V[0] = Rcp( v.m_V[0] );
	result.m_V[1] = Rcp( v.m_V[1] );
	result.m_V[2] = Rcp( v.m_V[2] );
	result.m_V[3] = Rcp( v.m_V[3] );
	result.m_V[4] = Rcp( v.m_V[4] );
    #endif
	return result;
}


/// Vector5 Dot Product
NOALIAS __forceinline const VF32 Dot( const Vector5& a, const Vector5& b )
{
    #if defined( _SIMD_ )
    return Dot5( a.m_V, b.m_V );
    #else
    return MakeVF32( a.m_V[0] * b.m_V[0] +
                     a.m_V[1] * b.m_V[1] +
                     a.m_V[2] * b.m_V[2] +
                     a.m_V[3] * b.m_V[3] +
                     a.m_V[4] * b.m_V[4] );
    #endif
}

/// Vector5 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Vector5& a, const Vector5& b )
{
    /// @par WARNING: Operator precedence of '|' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    #if defined( _SIMD_ )
    return Dot5( a.m_V, b.m_V );
    #else
    return MakeVF32( a.m_V[0] * b.m_V[0] +
                     a.m_V[1] * b.m_V[1] +
                     a.m_V[2] * b.m_V[2] +
                     a.m_V[3] * b.m_V[3] +
                     a.m_V[4] * b.m_V[4] );
    #endif
}

/// Vector5 Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Vector5& v )
{
    return Dot( v, v );
}

/// Vector5 Length
NOALIAS __forceinline const VF32 Length( const Vector5& v )
{
    return Sqrt( LengthSquared( v ) );
}

/// Vector5 Inverse: Returns A^-1, parallel to A and Dot( A, A^-1 ) == 1
NOALIAS __forceinline const Vector5 Inverse( const Vector5& v )
{
    return v * Rcp( LengthSquared( v ) );
}


/// Vector Space Tensor Product of a 5x1 column vector and a 1x5 row vector -> Mat55
NOALIAS __forceinline const Mat55 TensorProduct( const Vector5& a, const Vector5& b )
{
	Mat55 result;
	result.m_Row[0] = Element<0>( a ) * b;
	result.m_Row[1] = Element<1>( a ) * b;
	result.m_Row[2] = Element<2>( a ) * b;
	result.m_Row[3] = Element<3>( a ) * b;
	result.m_Row[4] = Element<4>( a ) * b;
	return result;
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

