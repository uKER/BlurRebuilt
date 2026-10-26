#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR6_INL
#define SOFT_MATHS_HVEC_NEW_VECTOR6_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR6_INL
#define MATHS_HVEC_NEW_VECTOR6_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector6
/// @{

/// Load Vector6 from F32 ptr
template <> NOALIAS __forceinline const Vector6 Load( F32 const * const __restrict pSrc )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Load<Vec8F32>( pSrc );
    #else
    result.m_V[0] = pSrc[0];
    result.m_V[1] = pSrc[1];
    result.m_V[2] = pSrc[2];
    result.m_V[3] = pSrc[3];
    result.m_V[4] = pSrc[4];
    result.m_V[5] = pSrc[5];
    #endif
    return result;
}

/// Store Vector6 to F32 ptr
NOALIAS __forceinline void Store( const Vector6& data, F32 * const __restrict pDst )
{
    #if defined( _SIMD_ )
    Store( data.m_V.m_V0, (StorageVec4F32*)pDst );
    Store( data.m_V.m_V1, (StorageVec2F32*)(pDst+4) );
    #else
    pDst[0] = data.m_V[0];
    pDst[1] = data.m_V[1];
    pDst[2] = data.m_V[2];
    pDst[3] = data.m_V[3];
    pDst[4] = data.m_V[4];
    pDst[5] = data.m_V[5];
    #endif
}


/// Load Vector6 from StorageVector6
NOALIAS __forceinline const Vector6 Load( StorageVector6 const * const __restrict pSrc )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Load<Vec8F32>( (F32 const *)pSrc );
    #else
    result.m_V[0] = pSrc->m_Data.v[0];
    result.m_V[1] = pSrc->m_Data.v[1];
    result.m_V[2] = pSrc->m_Data.v[2];
    result.m_V[3] = pSrc->m_Data.v[3];
    result.m_V[4] = pSrc->m_Data.v[4];
    result.m_V[5] = pSrc->m_Data.v[5];
    #endif
    return result;
}

/// Store Vector6 in StorageVector6
NOALIAS __forceinline void Store( const Vector6& data, StorageVector6 * const __restrict pDst )
{
    Store( data, (F32*)pDst );
}


/// Make Vector6 from broadcast VF32
NOALIAS __forceinline const Vector6 MakeVector6( const VF32 VECREF src )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = MakeVec8F32( src );
    #else
    result.m_V[0] =
    result.m_V[1] =
    result.m_V[2] =
    result.m_V[3] =
    result.m_V[4] =
    result.m_V[5] = src.m_V;
    #endif
    return result;
}

/// Make Vector6 from 6 VF32's
NOALIAS __forceinline const Vector6 MakeVector6( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4, const VF32 VECREF e5 )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = MakeVec8F32( e0, e1, e2, e3, e4, e5 );
    #else
    result.m_V[0] = e0.m_V;
    result.m_V[1] = e1.m_V;
    result.m_V[2] = e2.m_V;
    result.m_V[3] = e3.m_V;
    result.m_V[4] = e4.m_V;
    result.m_V[5] = e5.m_V;
    #endif
    return result;
}

/// Make Vector6 from concatenation of two Vector3's
NOALIAS __forceinline const Vector6 MakeVector6( const Vector3 PCREF v012, const Vector3 PCREF v345 )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = Permute<0,1,2,4>( v012.m_V, v345.m_V );
    result.m_V.m_V1 = Permute<1,2,3,3>( v345.m_V );
    #else
    result.m_V[0] = v012.m_V.m_V[0];
    result.m_V[1] = v012.m_V.m_V[1];
    result.m_V[2] = v012.m_V.m_V[2];
    result.m_V[3] = v345.m_V.m_V[0];
    result.m_V[4] = v345.m_V.m_V[1];
    result.m_V[5] = v345.m_V.m_V[2];
    #endif
    return result;
}

/// Make Vector6 from concatenation of a Vector3 and a Normal3
NOALIAS __forceinline const Vector6 MakeVector6( const Vector3 PCREF v012, const Normal3 PCREF n345 )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = Permute<0,1,2,4>( v012.m_V, n345.m_V );
    result.m_V.m_V1 = Permute<1,2,3,3>( n345.m_V );
    #else
    result.m_V[0] = v012.m_V.m_V[0];
    result.m_V[1] = v012.m_V.m_V[1];
    result.m_V[2] = v012.m_V.m_V[2];
    result.m_V[3] = n345.m_V.m_V[0];
    result.m_V[4] = n345.m_V.m_V[1];
    result.m_V[5] = n345.m_V.m_V[2];
    #endif
    return result;
}

/// Make Vector6 from concatenation of a Normal3 and a Vector3
NOALIAS __forceinline const Vector6 MakeVector6( const Normal3 PCREF n012, const Vector3 PCREF v345 )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = Permute<0,1,2,4>( n012.m_V, v345.m_V );
    result.m_V.m_V1 = Permute<1,2,3,3>( v345.m_V );
    #else
    result.m_V[0] = n012.m_V.m_V[0];
    result.m_V[1] = n012.m_V.m_V[1];
    result.m_V[2] = n012.m_V.m_V[2];
    result.m_V[3] = v345.m_V.m_V[0];
    result.m_V[4] = v345.m_V.m_V[1];
    result.m_V[5] = v345.m_V.m_V[2];
    #endif
    return result;
}

/// Make Vector6 from concatenation of two Normal3's
NOALIAS __forceinline const Vector6 MakeVector6( const Normal3 PCREF n012, const Normal3 PCREF n345 )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = Permute<0,1,2,4>( n012.m_V, n345.m_V );
    result.m_V.m_V1 = Permute<1,2,3,3>( n345.m_V );
    #else
    result.m_V[0] = n012.m_V.m_V[0];
    result.m_V[1] = n012.m_V.m_V[1];
    result.m_V[2] = n012.m_V.m_V[2];
    result.m_V[3] = n345.m_V.m_V[0];
    result.m_V[4] = n345.m_V.m_V[1];
    result.m_V[5] = n345.m_V.m_V[2];
    #endif
    return result;
}

/// Make Vector6 from a Vector5 (elements 0-4) & a VF32 (element 5)
NOALIAS __forceinline const Vector6 MakeVector6( const Vector5& e01234, const VF32 VECREF e5 )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = e01234.m_V.m_V0;
    result.m_V.m_V1 = e01234.m_V.m_V1;
    SetElement<5>( &result, e5 );
    #else
    result.m_V[0] = e01234.m_V[0];
    result.m_V[1] = e01234.m_V[1];
    result.m_V[2] = e01234.m_V[2];
    result.m_V[3] = e01234.m_V[3];
    result.m_V[4] = e01234.m_V[4];
    result.m_V[5] = e5.m_V;
    #endif
    return result;
}

/// Make Vector6 from a VF32 (element 0) & a Vector5 (elements 1-5)
NOALIAS __forceinline const Vector6 MakeVector6( const VF32 VECREF e0, const Vector5& e12345 )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V.m_V0 = RotateRight<1>( e12345.m_V.m_V0 );
    result.m_V.m_V1 = RotateRight<1>( e12345.m_V.m_V1 );
    SetElement<0>( &result, e0 );
    #else
    result.m_V[0] = e0.m_V;
    result.m_V[1] = e12345.m_V[0];
    result.m_V[2] = e12345.m_V[1];
    result.m_V[3] = e12345.m_V[2];
    result.m_V[4] = e12345.m_V[3];
    result.m_V[5] = e12345.m_V[4];
    #endif
    return result;
}


/// Split Vector6 into two Vector3's
NOALIAS __forceinline void Split( const Vector6& v, Vector3* const __restrict pV012, Vector3* const __restrict pV345 )
{
    #if defined( _SIMD_ )
    //*pV012 = MakeVector3( Element<0>( v ), Element<1>( v ), Element<2>( v ) );
    //*pV345 = MakeVector3( Element<3>( v ), Element<4>( v ), Element<5>( v ) );
    pV012->m_V = v.m_V.m_V0;
    pV345->m_V = Permute<3,4,5,7>( v.m_V.m_V0, v.m_V.m_V1 );
    #else
    pV012->m_V.m_V[0] = v.m_V[0];
    pV012->m_V.m_V[1] = v.m_V[1];
    pV012->m_V.m_V[2] = v.m_V[2];
    pV345->m_V.m_V[0] = v.m_V[0];
    pV345->m_V.m_V[1] = v.m_V[1];
    pV345->m_V.m_V[2] = v.m_V[2];
    #endif
}

/// Vector6 Extract low Vector3 (elements 0, 1 & 2)
NOALIAS __forceinline const Vector3 ExtractLoVector3( const Vector6& v )
{
    Vector3 result;
    #if defined( _SIMD_ )
    //result = MakeVector3( Element<0>( v ), Element<1>( v ), Element<2>( v ) );
    result.m_V = v.m_V.m_V0;
    #else
    result.m_V.m_V[0] = v.m_V[0];
    result.m_V.m_V[1] = v.m_V[1];
    result.m_V.m_V[2] = v.m_V[2];
    #endif
    return result;
}

/// Vector6 Extract high Vector3 (elements 3, 4 & 5)
NOALIAS __forceinline const Vector3 ExtractHiVector3( const Vector6& v )
{
    Vector3 result;
    #if defined( _SIMD_ )
    //result = MakeVector3( Element<3>( v ), Element<4>( v ), Element<5>( v ) );
    result.m_V = Permute<3,4,5,7>( v.m_V.m_V0, v.m_V.m_V1 );
    #else
    result.m_V.m_V[0] = v.m_V[3];
    result.m_V.m_V[1] = v.m_V[4];
    result.m_V.m_V[2] = v.m_V[5];
    #endif
    return result;
}



/// Vector6 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector6& v )
{
    BIZ_STATIC_ASSERT( I < 6 );
    #if defined( _SIMD_ )
    return Element<I>( v.m_V );
    #else
    return MakeVF32( v.m_V[I] );
    #endif
}

/// Vector6 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector6* const __restrict pDst, const VF32 VECREF ei )
{
    BIZ_STATIC_ASSERT( I < 6 );
    #if defined( _SIMD_ )
    SetElement<I>( &pDst->m_V, ei );
    #else
    pDst->m_V[I] = ei.m_V;
    #endif
}


/// Vector6 Permute : Indices select 6 result elements [I0,I1,I2,I3,I4,I5] from [ 0:v0, 1:v1, 2:v2, 3:v3, 4:v4, 5:v5 ]
template < unsigned int I0, unsigned int I1, unsigned int I2, unsigned int I3,
           unsigned int I4, unsigned int I5 >
NOALIAS __forceinline const Vector6 Permute( const Vector6& v )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Permute<I0,I1,I2,I3,I4,I5,I5,I5>( v.m_V );
    #else
    result.m_V[0] = v.m_V[I0];
    result.m_V[1] = v.m_V[I1];
    result.m_V[2] = v.m_V[I2];
    result.m_V[3] = v.m_V[I3];
    result.m_V[4] = v.m_V[I4];
    result.m_V[5] = v.m_V[I5];
    #endif
    return result;
}


/// Vector6 Abs of each element
NOALIAS __forceinline const Vector6 Abs( const Vector6& v )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Abs( v.m_V );
    #else
    result.m_V[0] = Abs( v.m_V[0] );
    result.m_V[1] = Abs( v.m_V[1] );
    result.m_V[2] = Abs( v.m_V[2] );
    result.m_V[3] = Abs( v.m_V[3] );
    result.m_V[4] = Abs( v.m_V[4] );
    result.m_V[5] = Abs( v.m_V[5] );
    #endif
    return result;
}

/// Vector6 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector6 Sign( const Vector6& v )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Sign( v.m_V );
    #else
    result.m_V[0] = ( v.m_V[0] < 0.0f ? -1.0f : ( v.m_V[0] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[1] = ( v.m_V[1] < 0.0f ? -1.0f : ( v.m_V[1] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[2] = ( v.m_V[2] < 0.0f ? -1.0f : ( v.m_V[2] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[3] = ( v.m_V[3] < 0.0f ? -1.0f : ( v.m_V[3] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[4] = ( v.m_V[4] < 0.0f ? -1.0f : ( v.m_V[4] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[5] = ( v.m_V[5] < 0.0f ? -1.0f : ( v.m_V[5] > 0.0f ? 1.0f : 0.0f ) );
    #endif
    return result;
}

/// Vector6 CopySign of each element : Returns a Vector6 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector6 CopySign( const Vector6& magnitude, const Vector6& sign )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = CopySign( magnitude.m_V, sign.m_V );
    #else
    result.m_V[0] = (F32)_copysign( magnitude.m_V[0], sign.m_V[0] );
    result.m_V[1] = (F32)_copysign( magnitude.m_V[1], sign.m_V[1] );
    result.m_V[2] = (F32)_copysign( magnitude.m_V[2], sign.m_V[2] );
    result.m_V[3] = (F32)_copysign( magnitude.m_V[3], sign.m_V[3] );
    result.m_V[4] = (F32)_copysign( magnitude.m_V[4], sign.m_V[4] );
    result.m_V[5] = (F32)_copysign( magnitude.m_V[5], sign.m_V[5] );
    #endif
    return result;
}


/// Vector6 Negation
NOALIAS __forceinline const Vector6 operator - ( const Vector6& a )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = -a.m_V;
    #else
    result.m_V[0] = -a.m_V[0];
    result.m_V[1] = -a.m_V[1];
    result.m_V[2] = -a.m_V[2];
    result.m_V[3] = -a.m_V[3];
    result.m_V[4] = -a.m_V[4];
    result.m_V[5] = -a.m_V[5];
    #endif
    return result;
}

/// Vector6 Sum
NOALIAS __forceinline const Vector6 operator + ( const Vector6& a, const Vector6& b )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V + b.m_V;
    #else
    result.m_V[0] = a.m_V[0] + b.m_V[0];
    result.m_V[1] = a.m_V[1] + b.m_V[1];
    result.m_V[2] = a.m_V[2] + b.m_V[2];
    result.m_V[3] = a.m_V[3] + b.m_V[3];
    result.m_V[4] = a.m_V[4] + b.m_V[4];
    result.m_V[5] = a.m_V[5] + b.m_V[5];
    #endif
    return result;
}

/// Vector6 Difference
NOALIAS __forceinline const Vector6 operator - ( const Vector6& a, const Vector6& b )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V - b.m_V;
    #else
    result.m_V[0] = a.m_V[0] - b.m_V[0];
    result.m_V[1] = a.m_V[1] - b.m_V[1];
    result.m_V[2] = a.m_V[2] - b.m_V[2];
    result.m_V[3] = a.m_V[3] - b.m_V[3];
    result.m_V[4] = a.m_V[4] - b.m_V[4];
    result.m_V[5] = a.m_V[5] - b.m_V[5];
    #endif
    return result;
}

/// Vector6 Addition Update
NOALIAS __forceinline void operator += ( Vector6& a, const Vector6& b )
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
    #endif
}

/// Vector6 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector6& a, const Vector6& b )
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
    #endif
}


/// Vector6 Scalar Right Multiplication
NOALIAS __forceinline const Vector6 operator * ( const Vector6& a, const VF32 VECREF b )
{
    Vector6 result;
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
    #endif
    return result;
}

/// Vector6 Scalar Left Multiplication
NOALIAS __forceinline const Vector6 operator * ( const VF32 VECREF a, const Vector6& b )
{
    Vector6 result;
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
    #endif
    return result;
}

/// Vector6 Scalar Right Division
NOALIAS __forceinline const Vector6 operator / ( const Vector6& a, const VF32 VECREF b )
{
    Vector6 result;
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
    #endif
    return result;
}

/// Vector6 Scalar Left Division
NOALIAS __forceinline const Vector6 operator / ( const VF32 VECREF a, const Vector6& b )
{
    Vector6 result;
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
    #endif
    return result;
}

/// Vector6 Scalar Right Multiplication Update
NOALIAS __forceinline void operator *= ( Vector6& a, const VF32 VECREF b )
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
    #endif
}

/// Vector6 Scalar Right Division Update
NOALIAS __forceinline void operator /= ( Vector6& a, const VF32 VECREF b )
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
    a.m_V[5] *= rcp_b.m_V;
    #endif
}


/// Vector6 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector6& a, const Vector6& b )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V * b.m_V;
    #else
    result.m_V[0] = a.m_V[0] * b.m_V[0];
    result.m_V[1] = a.m_V[1] * b.m_V[1];
    result.m_V[2] = a.m_V[2] * b.m_V[2];
    result.m_V[3] = a.m_V[3] * b.m_V[3];
    result.m_V[4] = a.m_V[4] * b.m_V[4];
    result.m_V[5] = a.m_V[5] * b.m_V[5];
    #endif
    return result;
}

/// Vector6 Division Operator
NOALIAS __forceinline const Vector6 operator / ( const Vector6& a, const Vector6& b )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = a.m_V / b.m_V;
    #else
    result.m_V[0] = a.m_V[0] / b.m_V[0];
    result.m_V[1] = a.m_V[1] / b.m_V[1];
    result.m_V[2] = a.m_V[2] / b.m_V[2];
    result.m_V[3] = a.m_V[3] / b.m_V[3];
    result.m_V[4] = a.m_V[4] / b.m_V[4];
    result.m_V[5] = a.m_V[5] / b.m_V[5];
    #endif
    return result;
}

/// Vector6 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector6& a, const Vector6& b )
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
    #endif
}

/// Vector6 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector6& a, const Vector6& b )
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
    #endif
}


/// Vector6 MulAdd: a * b + c
NOALIAS __forceinline const Vector6 MulAdd( const Vector6& a, const Vector6& b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = MulAdd( a.m_V, b.m_V, c.m_V );
    #else
    result.m_V[0] = a.m_V[0] * b.m_V[0] + c.m_V[0];
    result.m_V[1] = a.m_V[1] * b.m_V[1] + c.m_V[1];
    result.m_V[2] = a.m_V[2] * b.m_V[2] + c.m_V[2];
    result.m_V[3] = a.m_V[3] * b.m_V[3] + c.m_V[3];
    result.m_V[4] = a.m_V[4] * b.m_V[4] + c.m_V[4];
    result.m_V[5] = a.m_V[5] * b.m_V[5] + c.m_V[5];
    #endif
    return result;
}

/// Vector6 MulAdd: a * b + c
NOALIAS __forceinline const Vector6 MulAdd( const Vector6& a, const VF32 VECREF b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = MulAdd( a.m_V, b, c.m_V );
    #else
    result.m_V[0] = a.m_V[0] * b.m_V + c.m_V[0];
    result.m_V[1] = a.m_V[1] * b.m_V + c.m_V[1];
    result.m_V[2] = a.m_V[2] * b.m_V + c.m_V[2];
    result.m_V[3] = a.m_V[3] * b.m_V + c.m_V[3];
    result.m_V[4] = a.m_V[4] * b.m_V + c.m_V[4];
    result.m_V[5] = a.m_V[5] * b.m_V + c.m_V[5];
    #endif
    return result;
}

/// Vector6 MulAdd: a * b + c
NOALIAS __forceinline const Vector6 MulAdd( const VF32 VECREF a, const Vector6& b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = MulAdd( a, b.m_V, c.m_V );
    #else
    result.m_V[0] = a.m_V * b.m_V[0] + c.m_V[0];
    result.m_V[1] = a.m_V * b.m_V[1] + c.m_V[1];
    result.m_V[2] = a.m_V * b.m_V[2] + c.m_V[2];
    result.m_V[3] = a.m_V * b.m_V[3] + c.m_V[3];
    result.m_V[4] = a.m_V * b.m_V[4] + c.m_V[4];
    result.m_V[5] = a.m_V * b.m_V[5] + c.m_V[5];
    #endif
    return result;
}


/// Vector6 MulSub: a * b - c
NOALIAS __forceinline const Vector6 MulSub( const Vector6& a, const Vector6& b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = MulSub( a.m_V, b.m_V, c.m_V );
    #else
    result.m_V[0] = a.m_V[0] * b.m_V[0] - c.m_V[0];
    result.m_V[1] = a.m_V[1] * b.m_V[1] - c.m_V[1];
    result.m_V[2] = a.m_V[2] * b.m_V[2] - c.m_V[2];
    result.m_V[3] = a.m_V[3] * b.m_V[3] - c.m_V[3];
    result.m_V[4] = a.m_V[4] * b.m_V[4] - c.m_V[4];
    result.m_V[5] = a.m_V[5] * b.m_V[5] - c.m_V[5];
    #endif
    return result;
}

/// Vector6 MulSub: a * b - c
NOALIAS __forceinline const Vector6 MulSub( const Vector6& a, const VF32 VECREF b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = MulSub( a.m_V, b, c.m_V );
    #else
    result.m_V[0] = a.m_V[0] * b.m_V - c.m_V[0];
    result.m_V[1] = a.m_V[1] * b.m_V - c.m_V[1];
    result.m_V[2] = a.m_V[2] * b.m_V - c.m_V[2];
    result.m_V[3] = a.m_V[3] * b.m_V - c.m_V[3];
    result.m_V[4] = a.m_V[4] * b.m_V - c.m_V[4];
    result.m_V[5] = a.m_V[5] * b.m_V - c.m_V[5];
    #endif
    return result;
}

/// Vector6 MulSub: a * b - c
NOALIAS __forceinline const Vector6 MulSub( const VF32 VECREF a, const Vector6& b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = MulSub( a, b.m_V, c.m_V );
    #else
    result.m_V[0] = a.m_V * b.m_V[0] - c.m_V[0];
    result.m_V[1] = a.m_V * b.m_V[1] - c.m_V[1];
    result.m_V[2] = a.m_V * b.m_V[2] - c.m_V[2];
    result.m_V[3] = a.m_V * b.m_V[3] - c.m_V[3];
    result.m_V[4] = a.m_V * b.m_V[4] - c.m_V[4];
    result.m_V[5] = a.m_V * b.m_V[5] - c.m_V[5];
    #endif
    return result;
}


/// Vector6 NegMulAdd: -( a * b + c )
NOALIAS __forceinline const Vector6 NegMulAdd( const Vector6& a, const Vector6& b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = NegMulAdd( a.m_V, b.m_V, c.m_V );
    #else
    result.m_V[0] = -( a.m_V[0] * b.m_V[0] + c.m_V[0] );
    result.m_V[1] = -( a.m_V[1] * b.m_V[1] + c.m_V[1] );
    result.m_V[2] = -( a.m_V[2] * b.m_V[2] + c.m_V[2] );
    result.m_V[3] = -( a.m_V[3] * b.m_V[3] + c.m_V[3] );
    result.m_V[4] = -( a.m_V[4] * b.m_V[4] + c.m_V[4] );
    result.m_V[5] = -( a.m_V[5] * b.m_V[5] + c.m_V[5] );
    #endif
    return result;
}

/// Vector6 NegMulAdd: -( a * b + c )
NOALIAS __forceinline const Vector6 NegMulAdd( const Vector6& a, const VF32 VECREF b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = NegMulAdd( a.m_V, b, c.m_V );
    #else
    result.m_V[0] = -( a.m_V[0] * b.m_V + c.m_V[0] );
    result.m_V[1] = -( a.m_V[1] * b.m_V + c.m_V[1] );
    result.m_V[2] = -( a.m_V[2] * b.m_V + c.m_V[2] );
    result.m_V[3] = -( a.m_V[3] * b.m_V + c.m_V[3] );
    result.m_V[4] = -( a.m_V[4] * b.m_V + c.m_V[4] );
    result.m_V[5] = -( a.m_V[5] * b.m_V + c.m_V[5] );
    #endif
    return result;
}

/// Vector6 NegMulAdd: -( a * b + c )
NOALIAS __forceinline const Vector6 NegMulAdd( const VF32 VECREF a, const Vector6& b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = NegMulAdd( a, b.m_V, c.m_V );
    #else
    result.m_V[0] = -( a.m_V * b.m_V[0] + c.m_V[0] );
    result.m_V[1] = -( a.m_V * b.m_V[1] + c.m_V[1] );
    result.m_V[2] = -( a.m_V * b.m_V[2] + c.m_V[2] );
    result.m_V[3] = -( a.m_V * b.m_V[3] + c.m_V[3] );
    result.m_V[4] = -( a.m_V * b.m_V[4] + c.m_V[4] );
    result.m_V[5] = -( a.m_V * b.m_V[5] + c.m_V[5] );
    #endif
    return result;
}


/// Vector6 NegMulSub: -( a * b - c ) = c - a * b
NOALIAS __forceinline const Vector6 NegMulSub( const Vector6& a, const Vector6& b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = NegMulSub( a.m_V, b.m_V, c.m_V );
    #else
    result.m_V[0] = c.m_V[0] - a.m_V[0] * b.m_V[0];
    result.m_V[1] = c.m_V[1] - a.m_V[1] * b.m_V[1];
    result.m_V[2] = c.m_V[2] - a.m_V[2] * b.m_V[2];
    result.m_V[3] = c.m_V[3] - a.m_V[3] * b.m_V[3];
    result.m_V[4] = c.m_V[4] - a.m_V[4] * b.m_V[4];
    result.m_V[5] = c.m_V[5] - a.m_V[5] * b.m_V[5];
    #endif
    return result;
}

/// Vector6 NegMulSub: -( a * b - c ) = c - a * b
NOALIAS __forceinline const Vector6 NegMulSub( const Vector6& a, const VF32 VECREF b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = NegMulSub( a.m_V, b, c.m_V );
    #else
    result.m_V[0] = c.m_V[0] - a.m_V[0] * b.m_V;
    result.m_V[1] = c.m_V[1] - a.m_V[1] * b.m_V;
    result.m_V[2] = c.m_V[2] - a.m_V[2] * b.m_V;
    result.m_V[3] = c.m_V[3] - a.m_V[3] * b.m_V;
    result.m_V[4] = c.m_V[4] - a.m_V[4] * b.m_V;
    result.m_V[5] = c.m_V[5] - a.m_V[5] * b.m_V;
    #endif
    return result;
}

/// Vector6 NegMulSub: -( a * b - c ) = c - a * b
NOALIAS __forceinline const Vector6 NegMulSub( const VF32 VECREF a, const Vector6& b, const Vector6& c )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = NegMulSub( a, b.m_V, c.m_V );
    #else
    result.m_V[0] = c.m_V[0] - a.m_V * b.m_V[0];
    result.m_V[1] = c.m_V[1] - a.m_V * b.m_V[1];
    result.m_V[2] = c.m_V[2] - a.m_V * b.m_V[2];
    result.m_V[3] = c.m_V[3] - a.m_V * b.m_V[3];
    result.m_V[4] = c.m_V[4] - a.m_V * b.m_V[4];
    result.m_V[5] = c.m_V[5] - a.m_V * b.m_V[5];
    #endif
    return result;
}




/// Vector6 Comparison a == b : Returns VBool representing results of a[0] == b[0] && ... && a[5] == b[5]
NOALIAS __forceinline const VBool operator == ( const Vector6& a, const Vector6& b )
{
    #if defined( _SIMD_ )
    return Permute<0,1,2,3,4,5,5,5>( a.m_V ) == Permute<0,1,2,3,4,5,5,5>( b.m_V );
    #else
    VBool result;
    result.m_V = a.m_V[0] == b.m_V[0] &&
                 a.m_V[1] == b.m_V[1] &&
                 a.m_V[2] == b.m_V[2] &&
                 a.m_V[3] == b.m_V[3] &&
                 a.m_V[4] == b.m_V[4] &&
                 a.m_V[5] == b.m_V[5];
    return result;
    #endif
}

/// Vector6 Comparison a != b : Returns VBool representing results of a[0] != b[0] || ... || a[5] != b[5]
NOALIAS __forceinline const VBool operator != ( const Vector6& a, const Vector6& b )
{
    #if defined( _SIMD_ )
    return Permute<0,1,2,3,4,5,5,5>( a.m_V ) != Permute<0,1,2,3,4,5,5,5>( b.m_V );
    #else
    VBool result;
    result.m_V = a.m_V[0] != b.m_V[0] ||
                 a.m_V[1] != b.m_V[1] ||
                 a.m_V[2] != b.m_V[2] ||
                 a.m_V[3] != b.m_V[3] ||
                 a.m_V[4] != b.m_V[4] ||
                 a.m_V[5] != b.m_V[5];
    return result;
    #endif
}

/// Vector6 Comparison a <= b : Returns Vec8Bool representing results of a[i] <= b[i], for each element i [0,5]
NOALIAS __forceinline const Vec8Bool operator <= ( const Vector6& a, const Vector6& b )
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
    return result;
    #endif
}

/// Vector6 Comparison a >= b : Returns Vec8Bool representing results of a[i] >= b[i], for each element i [0,5]
NOALIAS __forceinline const Vec8Bool operator >= ( const Vector6& a, const Vector6& b )
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
    return result;
    #endif
}

/// Vector6 Comparison a < b : Returns Vec8Bool representing results of a[i] < b[i], for each element i [0,5]
NOALIAS __forceinline const Vec8Bool operator < ( const Vector6& a, const Vector6& b )
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
    return result;
    #endif
}

/// Vector6 Comparison a > b : Returns Vec8Bool representing results of a[i] > b[i], for each element i [0,5]
NOALIAS __forceinline const Vec8Bool operator > ( const Vector6& a, const Vector6& b )
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
    return result;
    #endif
}

/// Vector6 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector6& a, const Vector6& b )
{
    #if defined( _SIMD_ )
    return Ordered( Permute<0,1,2,3,4,5,5,5>( a.m_V ), Permute<0,1,2,3,4,5,5,5>( b.m_V ) );
    #else
    VBool result;
    result.m_V = !_isnan( a.m_V[0] ) && !_isnan( b.m_V[0] ) &&
                 !_isnan( a.m_V[1] ) && !_isnan( b.m_V[1] ) &&
                 !_isnan( a.m_V[2] ) && !_isnan( b.m_V[2] ) &&
                 !_isnan( a.m_V[3] ) && !_isnan( b.m_V[3] ) &&
                 !_isnan( a.m_V[4] ) && !_isnan( b.m_V[4] ) &&
                 !_isnan( a.m_V[5] ) && !_isnan( b.m_V[5] );
    return result;
    #endif
}

/// Vector6 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector6& a, const Vector6& b )
{
    #if defined( _SIMD_ )
    return Unordered( Permute<0,1,2,3,4,5,5,5>( a.m_V ), Permute<0,1,2,3,4,5,5,5>( b.m_V ) );
    #else
    VBool result;
    result.m_V = _isnan( a.m_V[0] ) || _isnan( b.m_V[0] ) ||
                 _isnan( a.m_V[1] ) || _isnan( b.m_V[1] ) ||
                 _isnan( a.m_V[2] ) || _isnan( b.m_V[2] ) ||
                 _isnan( a.m_V[3] ) || _isnan( b.m_V[3] ) ||
                 _isnan( a.m_V[4] ) || _isnan( b.m_V[4] ) ||
                 _isnan( a.m_V[5] ) || _isnan( b.m_V[5] );
    return result;
    #endif
}


/// Vector6 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vector6& a )
{
    #if defined( _SIMD_ )
    return IsNan( Permute<0,1,2,3,4,5,5,5>( a.m_V ) );
    #else
    VBool result;
    result.m_V = _isnan( a.m_V[0] ) ||
                 _isnan( a.m_V[1] ) ||
                 _isnan( a.m_V[2] ) ||
                 _isnan( a.m_V[3] ) ||
                 _isnan( a.m_V[4] ) ||
                 _isnan( a.m_V[5] );
    return result;
    #endif
}

/// Vector6 Finite Test : true if a contains no elements that are NaN's and for each element i [0,5], abs(x[i]) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vector6& a )
{
    #if defined( _SIMD_ )
    return Finite( Permute<0,1,2,3,4,5,5,5>( a.m_V ) );
    #else
    VBool result;
    result.m_V = !_isnan( a.m_V[0] ) && ( Maths::Abs( a.m_V[0] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[1] ) && ( Maths::Abs( a.m_V[1] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[2] ) && ( Maths::Abs( a.m_V[2] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[3] ) && ( Maths::Abs( a.m_V[3] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[4] ) && ( Maths::Abs( a.m_V[4] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[5] ) && ( Maths::Abs( a.m_V[5] ) <= FLT_MAX );
    return result;
    #endif
}


/// Vector6 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector6 Select( const VBool PCREF condition, const Vector6& result_if_true, const Vector6& result_if_false )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    #else
    result.m_V[0] = ( condition.m_V ? result_if_true.m_V[0] : result_if_false.m_V[0] );
    result.m_V[1] = ( condition.m_V ? result_if_true.m_V[1] : result_if_false.m_V[1] );
    result.m_V[2] = ( condition.m_V ? result_if_true.m_V[2] : result_if_false.m_V[2] );
    result.m_V[3] = ( condition.m_V ? result_if_true.m_V[3] : result_if_false.m_V[3] );
    result.m_V[4] = ( condition.m_V ? result_if_true.m_V[4] : result_if_false.m_V[4] );
    result.m_V[5] = ( condition.m_V ? result_if_true.m_V[5] : result_if_false.m_V[5] );
    #endif
    return result;
}

/// Vector6 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector6 Select( const Vec8Bool PCREF condition, const Vector6& result_if_true, const Vector6& result_if_false )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    #else
    result.m_V[0] = ( condition.m_V[0] ? result_if_true.m_V[0] : result_if_false.m_V[0] );
    result.m_V[1] = ( condition.m_V[1] ? result_if_true.m_V[1] : result_if_false.m_V[1] );
    result.m_V[2] = ( condition.m_V[2] ? result_if_true.m_V[2] : result_if_false.m_V[2] );
    result.m_V[3] = ( condition.m_V[3] ? result_if_true.m_V[3] : result_if_false.m_V[3] );
    result.m_V[4] = ( condition.m_V[4] ? result_if_true.m_V[4] : result_if_false.m_V[4] );
    result.m_V[5] = ( condition.m_V[5] ? result_if_true.m_V[5] : result_if_false.m_V[5] );
    #endif
    return result;
}


/// Vector6 Bitwise AND: a & b : for each element i, returns ( b ? a[i] : zero )
NOALIAS __forceinline const Vector6 AND( const Vector6& a, const VBool PCREF b )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = AND( a.m_V, b );
    #else
    result.m_V[0] = b.m_V ? a.m_V[0] : 0.0f;
    result.m_V[1] = b.m_V ? a.m_V[1] : 0.0f;
    result.m_V[2] = b.m_V ? a.m_V[2] : 0.0f;
    result.m_V[3] = b.m_V ? a.m_V[3] : 0.0f;
    result.m_V[4] = b.m_V ? a.m_V[4] : 0.0f;
    result.m_V[5] = b.m_V ? a.m_V[5] : 0.0f;
    #endif
    return result;
}

/// Vector6 Bitwise ANDC: a & ~b : for each element i, returns ( b ? zero : a[i] )
NOALIAS __forceinline const Vector6 ANDC( const Vector6& a, const VBool PCREF b )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = ANDC( a.m_V, b );
    #else
    result.m_V[0] = !b.m_V ? a.m_V[0] : 0.0f;
    result.m_V[1] = !b.m_V ? a.m_V[1] : 0.0f;
    result.m_V[2] = !b.m_V ? a.m_V[2] : 0.0f;
    result.m_V[3] = !b.m_V ? a.m_V[3] : 0.0f;
    result.m_V[4] = !b.m_V ? a.m_V[4] : 0.0f;
    result.m_V[5] = !b.m_V ? a.m_V[5] : 0.0f;
    #endif
    return result;
}


/// Vector6 Bitwise AND: a & b : for each element i, returns ( b[i] ? a[i] : zero )
NOALIAS __forceinline const Vector6 AND( const Vector6& a, const Vec8Bool PCREF b )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = AND( a.m_V, b );
    #else
    result.m_V[0] = b.m_V[0] ? a.m_V[0] : 0.0f;
    result.m_V[1] = b.m_V[1] ? a.m_V[1] : 0.0f;
    result.m_V[2] = b.m_V[2] ? a.m_V[2] : 0.0f;
    result.m_V[3] = b.m_V[3] ? a.m_V[3] : 0.0f;
    result.m_V[4] = b.m_V[4] ? a.m_V[4] : 0.0f;
    result.m_V[5] = b.m_V[5] ? a.m_V[5] : 0.0f;
    #endif
    return result;
}

/// Vector6 Bitwise ANDC: a & ~b : for each element i, returns ( b[i] ? zero : a[i] )
NOALIAS __forceinline const Vector6 ANDC( const Vector6& a, const Vec8Bool PCREF b )
{
    Vector6 result;
    #if defined( _SIMD_ )
    result.m_V = ANDC( a.m_V, b );
    #else
    result.m_V[0] = !b.m_V[0] ? a.m_V[0] : 0.0f;
    result.m_V[1] = !b.m_V[1] ? a.m_V[1] : 0.0f;
    result.m_V[2] = !b.m_V[2] ? a.m_V[2] : 0.0f;
    result.m_V[3] = !b.m_V[3] ? a.m_V[3] : 0.0f;
    result.m_V[4] = !b.m_V[4] ? a.m_V[4] : 0.0f;
    result.m_V[5] = !b.m_V[5] ? a.m_V[5] : 0.0f;
    #endif
    return result;
}


/// Vector6 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector6 Rcp( const Vector6& v )
{
	Vector6 result;
    #if defined( _SIMD_ )
	result.m_V = Rcp( v.m_V );
    #else
	result.m_V[0] = Rcp( v.m_V[0] );
	result.m_V[1] = Rcp( v.m_V[1] );
	result.m_V[2] = Rcp( v.m_V[2] );
	result.m_V[3] = Rcp( v.m_V[3] );
	result.m_V[4] = Rcp( v.m_V[4] );
	result.m_V[5] = Rcp( v.m_V[5] );
    #endif
	return result;
}

/// Vector6 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector6 Rcp24( const Vector6& v )
{
	Vector6 result;
    #if defined( _SIMD_ )
	result.m_V = Rcp24( v.m_V );
    #else
	result.m_V[0] = Rcp( v.m_V[0] );
	result.m_V[1] = Rcp( v.m_V[1] );
	result.m_V[2] = Rcp( v.m_V[2] );
	result.m_V[3] = Rcp( v.m_V[3] );
	result.m_V[4] = Rcp( v.m_V[4] );
	result.m_V[5] = Rcp( v.m_V[5] );
    #endif
	return result;
}


/// Vector6 Dot Product
NOALIAS __forceinline const VF32 Dot( const Vector6& a, const Vector6& b )
{
    #if defined( _SIMD_ )
    return Dot6( a.m_V, b.m_V );
    #else
    return MakeVF32( a.m_V[0] * b.m_V[0] +
                     a.m_V[1] * b.m_V[1] +
                     a.m_V[2] * b.m_V[2] +
                     a.m_V[3] * b.m_V[3] +
                     a.m_V[4] * b.m_V[4] +
                     a.m_V[5] * b.m_V[5] );
    #endif
}

/// Vector6 Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Vector6& v )
{
    return Dot( v, v );
}

/// Vector6 Length
NOALIAS __forceinline const VF32 Length( const Vector6& v )
{
    return Sqrt( LengthSquared( v ) );
}

/// Vector6 Inverse: Returns A^-1, parallel to A and Dot( A, A^-1 ) == 1
NOALIAS __forceinline const Vector6 Inverse( const Vector6& v )
{
    return v * Rcp( LengthSquared( v ) );
}


/// Vector Space Tensor Product of a 6x1 column vector and a 1x6 row vector -> Mat66
NOALIAS __forceinline const Mat66 TensorProduct( const Vector6& a, const Vector6& b )
{
	Mat66 result;
	result.m_Row[0] = Element<0>( a ) * b;
	result.m_Row[1] = Element<1>( a ) * b;
	result.m_Row[2] = Element<2>( a ) * b;
	result.m_Row[3] = Element<3>( a ) * b;
	result.m_Row[4] = Element<4>( a ) * b;
	result.m_Row[5] = Element<5>( a ) * b;
	return result;
}


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

