#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR8_INL
#define SOFT_MATHS_HVEC_NEW_VECTOR8_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR8_INL
#define MATHS_HVEC_NEW_VECTOR8_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector8
/// @{

/// Load Vector8 from F32 ptr
template <> NOALIAS __forceinline const Vector8 Load( F32 const * const __restrict pSrc )
{
    Vector8 result;
    result.m_V = Load<Vec8F32>( pSrc );
    return result;
}

/// Store Vector8 to F32 ptr
NOALIAS __forceinline void Store( const Vector8& data, F32 * const __restrict pDst )
{
    Store( data.m_V.m_V0, (StorageVec4F32*)pDst );
    Store( data.m_V.m_V1, (StorageVec4F32*)(pDst+4) );
}


/// Load Vector8 from StorageVector8
NOALIAS __forceinline const Vector8 Load( StorageVector8 const * const __restrict pSrc )
{
    Vector8 result;
    result.m_V = Load<Vec8F32>( (F32 const *)pSrc );
    return result;
}

/// Store Vector8 in StorageVector8
NOALIAS __forceinline void Store( const Vector8& data, StorageVector8 * const __restrict pDst )
{
    Store( data, (F32*)pDst );
}

//[Michelle] function needed
NOALIAS __forceinline const VF32 MakeVF32( const Vector8& src )
{
	return Element<0>( src.m_V );
}

//[Michelle] function needed
NOALIAS __forceinline const Vector2 MakeVector2( const Vector8& src )
{
	Maths::Vector2 res;
	res.m_V = src.m_V.m_V0;
	return res;
}

//[Michelle] function needed
NOALIAS __forceinline const Vector3 MakeVector3( const Vector8& src )
{
	Maths::Vector3 res;
	res.m_V = src.m_V.m_V0;
	return res;
}

//[Michelle] function needed
NOALIAS __forceinline const Vector4 MakeVector4( const Vector8& src )
{
	Maths::Vector4 res;
	res.m_V = src.m_V.m_V0;
	return res;
}

//[Michelle] function needed
NOALIAS __forceinline const Vector5 MakeVector5( const Vector8& src )
{
	Maths::Vector5 res;
#if defined( _SIMD_ )
	res.m_V = src.m_V;
#else
	res.m_V[0] = src.m_V.m_V0.m_V[0];
	res.m_V[1] = src.m_V.m_V0.m_V[1];
	res.m_V[2] = src.m_V.m_V0.m_V[2];
	res.m_V[3] = src.m_V.m_V0.m_V[3];
	res.m_V[4] = src.m_V.m_V1.m_V[0];
#endif
	return res;
}

//[Michelle] function needed
NOALIAS __forceinline const Vector6 MakeVector6( const Vector8& src )
{
	Maths::Vector6 res;
#if defined( _SIMD_ )
	res.m_V = src.m_V;
#else
	res.m_V[0] = src.m_V.m_V0.m_V[0];
	res.m_V[1] = src.m_V.m_V0.m_V[1];
	res.m_V[2] = src.m_V.m_V0.m_V[2];
	res.m_V[3] = src.m_V.m_V0.m_V[3];
	res.m_V[4] = src.m_V.m_V1.m_V[0];
	res.m_V[5] = src.m_V.m_V1.m_V[1];
#endif
	return res;
}

//[Michelle] function needed
NOALIAS __forceinline const Vector7 MakeVector7( const Vector8& src )
{
	Maths::Vector7 res;
#if defined( _SIMD_ )
	res.m_V = src.m_V;
#else
	res.m_V[0] = src.m_V.m_V0.m_V[0];
	res.m_V[1] = src.m_V.m_V0.m_V[1];
	res.m_V[2] = src.m_V.m_V0.m_V[2];
	res.m_V[3] = src.m_V.m_V0.m_V[3];
	res.m_V[4] = src.m_V.m_V1.m_V[0];
	res.m_V[5] = src.m_V.m_V1.m_V[1];
	res.m_V[6] = src.m_V.m_V1.m_V[2];
#endif
	return res;
}



/// Make Vector8 from broadcast VF32
NOALIAS __forceinline const Vector8 MakeVector8( const VF32 VECREF src )
{
    Vector8 result;
    result.m_V = MakeVec8F32( src );
    return result;
}

/// Make Vector8 from 7 VF32's
NOALIAS __forceinline const Vector8 MakeVector8( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6, const VF32 VECREF e7 )
{
    Vector8 result;
    result.m_V = MakeVec8F32( e0, e1, e2, e3, e4, e5, e6, e7 );
    return result;
}

//[Michelle] function needed
NOALIAS __forceinline const Vector8 MakeVector8( const Vector6& src )
{
    Vector8 result;
#if defined( _SIMD_ )
	result.m_V = src.m_V;
#else
	result.m_V.m_V0.m_V[0] = src.m_V[0];
	result.m_V.m_V0.m_V[1] = src.m_V[1];
	result.m_V.m_V0.m_V[2] = src.m_V[2];
	result.m_V.m_V0.m_V[3] = src.m_V[3];
	result.m_V.m_V1.m_V[0] = src.m_V[4];
	result.m_V.m_V1.m_V[1] = src.m_V[5];
	result.m_V.m_V1.m_V[2] = 0.f;
	result.m_V.m_V1.m_V[3] = 0.f;
#endif
    return result;
}


/// Make Vector8 from 2 Vector4's
NOALIAS __forceinline const Vector8 MakeVector8( const Vector4 PCREF v0123, const Vector4 PCREF v4567 )
{
    Vector8 result;
    result.m_V = MakeVec8F32( v0123.m_V, v4567.m_V );
    return result;
}

/// Split Vector8 into 2 Vector4's
NOALIAS __forceinline void Split( const Vector8& src, Vector4* const __restrict pV0123, Vector4* const __restrict pV4567 )
{
    Split( src.m_V, &pV0123->m_V, &pV4567->m_V );
}



/// Vector8 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector8& v )
{
    BIZ_STATIC_ASSERT( I < 8 );
    return Element<I>( v.m_V );
}

/// Vector8 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector8* const __restrict pDst, const VF32 VECREF ei )
{
    BIZ_STATIC_ASSERT( I < 8 );
    SetElement<I>( &pDst->m_V, ei );
}


/// Vector8 Write to Stream
template < typename T_Stream >
void Serialise( const Vector8& src, T_Stream& stream )
{
    Serialise( src.m_V, stream );
}

/// Vector8 Read from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, Vector8* const __restrict pDst )
{
    Deserialise( stream, &pDst->m_V );
}


/// Vector8 Permute : Indices select 8 result elements [I0,I1,I2,I3,I4,I5,I6,I7] from [ 0:v0, 1:v1, 2:v2, 3:v3, 4:v4, 5:v5, 6:v6, 7:v7 ]
template < unsigned int I0, unsigned int I1, unsigned int I2, unsigned int I3,
           unsigned int I4, unsigned int I5, unsigned int I6, unsigned int I7 >
NOALIAS __forceinline const Vector8 Permute( const Vector8& v )
{
    Vector8 result;
    result.m_V = Permute<I0,I1,I2,I3,I4,I5,I6,I7>( v.m_V );
    return result;
}


/// Vector8 Abs of each element
NOALIAS __forceinline const Vector8 Abs( const Vector8& v )
{
    Vector8 result;
    result.m_V = Abs( v.m_V );
    return result;
}

/// Vector8 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector8 Sign( const Vector8& v )
{
    Vector8 result;
    result.m_V = Sign( v.m_V );
    return result;
}

/// Vector8 CopySign of each element : Returns a Vector8 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector8 CopySign( const Vector8& magnitude, const Vector8& sign )
{
    Vector8 result;
    result.m_V = CopySign( magnitude.m_V, sign.m_V );
    return result;
}


/// Vector8 Negation : Indices select elements to negate [I0,I1,I2,I3,I4,I5,I6,I7]
template < bool I0, bool I1, bool I2, bool I3,
           bool I4, bool I5, bool I6, bool I7 >
NOALIAS __forceinline const Vector8 Negate( const Vector8& v )
{
    Vector8 result;
    result.m_V = Negate<I0,I1,I2,I3,I4,I5,I6,I7>( v.m_V );
    return result;
}


/// Vector8 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector8 Rcp( const Vector8& v )
{
    Vector8 result;
    result.m_V = Rcp( v.m_V );
    return result;
}

/// Vector8 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector8 Rcp24( const Vector8& v )
{
    Vector8 result;
    result.m_V = Rcp24( v.m_V );
    return result;
}

/// Vector8 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector8 Div( const Vector8& x, const Vector8& y )
{
    Vector8 result;
    result.m_V = Div( x.m_V, y.m_V );
    return result;
}

/// Vector8 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector8 Div24( const Vector8& x, const Vector8& y )
{
    Vector8 result;
    result.m_V = Div24( x.m_V, y.m_V );
    return result;
}

/// Vector8 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector8 Div( const VF32 VECREF x, const Vector8& y )
{
    Vector8 result;
    result.m_V = Div( x, y.m_V );
    return result;
}

/// Vector8 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector8 Div24( const VF32 VECREF x, const Vector8& y )
{
    Vector8 result;
    result.m_V = Div24( x, y.m_V );
    return result;
}

/// Vector8 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vector8 Div( const Vector8& x, const VF32 VECREF y )
{
    Vector8 result;
    result.m_V = Div( x.m_V, y );
    return result;
}

/// Vector8 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vector8 Div24( const Vector8& x, const VF32 VECREF y )
{
    Vector8 result;
    result.m_V = Div24( x.m_V, y );
    return result;
}

/// Vector8 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector8 RcpSqrt( const Vector8& v )
{
    Vector8 result;
    result.m_V = RcpSqrt( v.m_V );
    return result;
}

/// Vector8 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector8 RcpSqrt24( const Vector8& v )
{
    Vector8 result;
    result.m_V = RcpSqrt24( v.m_V );
    return result;
}

/// Vector8 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector8 Sqrt( const Vector8& v )
{
    Vector8 result;
    result.m_V = Sqrt( v.m_V );
    return result;
}

/// Vector8 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector8 Sqrt24( const Vector8& v )
{
    Vector8 result;
    result.m_V = Sqrt24( v.m_V );
    return result;
}

/// Vector8 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vector8& v,
                                       Vector8* const __restrict pRcp,
                                       Vector8* const __restrict pSqrt,
                                       Vector8* const __restrict pRcpSqrt )
{
    RcpAndSqrt( v.m_V, &pRcp->m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}

/// Vector8 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vector8& v,
                                         Vector8* const __restrict pRcp,
                                         Vector8* const __restrict pSqrt,
                                         Vector8* const __restrict pRcpSqrt )
{
    RcpAndSqrt24( v.m_V, &pRcp->m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}

/// Vector8 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vector8& v,
                                           Vector8* const __restrict pSqrt,
                                           Vector8* const __restrict pRcpSqrt )
{
    SqrtAndRcpSqrt( v.m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}

/// Vector8 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vector8& v,
                                             Vector8* const __restrict pSqrt,
                                             Vector8* const __restrict pRcpSqrt )
{
    SqrtAndRcpSqrt24( v.m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}


/// Vector8 Unary Positive Operator
NOALIAS __forceinline const Vector8 operator + ( const Vector8& a )
{
    return a;
}

/// Vector8 Unary Negation Operator
NOALIAS __forceinline const Vector8 operator - ( const Vector8& a )
{
    Vector8 result;
    result.m_V = -a.m_V;
    return result;
}


/// Vector8 Sum
NOALIAS __forceinline const Vector8 operator + ( const Vector8& a, const Vector8& b )
{
    Vector8 result;
    result.m_V = a.m_V + b.m_V;
    return result;
}

/// Vector8 Difference
NOALIAS __forceinline const Vector8 operator - ( const Vector8& a, const Vector8& b )
{
    Vector8 result;
    result.m_V = a.m_V - b.m_V;
    return result;
}

/// Vector8 Multiplication Operator
NOALIAS __forceinline const Vector8 operator * ( const Vector8& a, const Vector8& b )
{
    Vector8 result;
    result.m_V = a.m_V * b.m_V;
    return result;
}

/// Vector8 Division Operator
NOALIAS __forceinline const Vector8 operator / ( const Vector8& a, const Vector8& b )
{
    Vector8 result;
    result.m_V = a.m_V / b.m_V;
    return result;
}


/// Vector8 Addition Update
NOALIAS __forceinline void operator += ( Vector8& a, const Vector8& b )
{
    a.m_V += b.m_V;
}

/// Vector8 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector8& a, const Vector8& b )
{
    a.m_V -= b.m_V;
}

/// Vector8 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector8& a, const Vector8& b )
{
    a.m_V *= b.m_V;
}

/// Vector8 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector8& a, const Vector8& b )
{
    a.m_V /= b.m_V;
}


/// Vector8 Scalar Right Multiplication
NOALIAS __forceinline const Vector8 operator * ( const Vector8& a, const VF32 VECREF b )
{
    Vector8 result;
    result.m_V = a.m_V * b;
    return result;
}

/// Vector8 Scalar Left Multiplication
NOALIAS __forceinline const Vector8 operator * ( const VF32 VECREF a, const Vector8& b )
{
    Vector8 result;
    result.m_V = a * b.m_V;
    return result;
}

/// Vector8 Scalar Right Division
NOALIAS __forceinline const Vector8 operator / ( const Vector8& a, const VF32 VECREF b )
{
    Vector8 result;
    result.m_V = a.m_V / b;
    return result;
}

/// Vector8 Scalar Left Division
NOALIAS __forceinline const Vector8 operator / ( const VF32 VECREF a, const Vector8& b )
{
    Vector8 result;
    result.m_V = a / b.m_V;
    return result;
}


/// Vector8 Scalar Right Multiplication Update
NOALIAS __forceinline void operator *= ( Vector8& a, const VF32 VECREF b )
{
    a.m_V *= b;
}

/// Vector8 Scalar Right Division Update
NOALIAS __forceinline void operator /= ( Vector8& a, const VF32 VECREF b )
{
    a.m_V /= b;
}


/// Vector8 MulAdd : a * b + c
NOALIAS __forceinline const Vector8 MulAdd( const Vector8& a, const Vector8& b, const Vector8& c )
{
    Vector8 result;
    result.m_V = MulAdd( a.m_V, b.m_V, c.m_V );
    return result;
}

/// Vector8 MulSub : a * b - c
NOALIAS __forceinline const Vector8 MulSub( const Vector8& a, const Vector8& b, const Vector8& c )
{
    Vector8 result;
    result.m_V = MulSub( a.m_V, b.m_V, c.m_V );
    return result;
}

/// Vector8 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector8 NegMulAdd( const Vector8& a, const Vector8& b, const Vector8& c )
{
    Vector8 result;
    result.m_V = NegMulAdd( a.m_V, b.m_V, c.m_V );
    return result;
}

/// Vector8 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector8 NegMulSub( const Vector8& a, const Vector8& b, const Vector8& c )
{
    Vector8 result;
    result.m_V = NegMulSub( a.m_V, b.m_V, c.m_V );
    return result;
}


/// Vector8 MulAdd : a * b + c : [ vector * scalar + vector ]
NOALIAS __forceinline const Vector8 MulAdd( const Vector8& a, const VF32 VECREF b, const Vector8& c )
{
    Vector8 result;
    result.m_V = MulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Vector8 MulSub : a * b - c : [ vector * scalar - vector ]
NOALIAS __forceinline const Vector8 MulSub( const Vector8& a, const VF32 VECREF b, const Vector8& c )
{
    Vector8 result;
    result.m_V = MulSub( a.m_V, b, c.m_V );
    return result;
}

/// Vector8 NegMulAdd : -( a * b + c ) : [ - ( vector * scalar + vector ) ]
NOALIAS __forceinline const Vector8 NegMulAdd( const Vector8& a, const VF32 VECREF b, const Vector8& c )
{
    Vector8 result;
    result.m_V = NegMulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Vector8 NegMulSub : -( a * b - c ) == c - a * b : [ - ( vector * scalar - vector ) ]
NOALIAS __forceinline const Vector8 NegMulSub( const Vector8& a, const VF32 VECREF b, const Vector8& c )
{
    Vector8 result;
    result.m_V = NegMulSub( a.m_V, b, c.m_V );
    return result;
}


/// Vector8 MulAdd : a * b + c : [ scalar * vector + vector ]
NOALIAS __forceinline const Vector8 MulAdd( const VF32 VECREF a, const Vector8& b, const Vector8& c )
{
    Vector8 result;
    result.m_V = MulAdd( a, b.m_V, c.m_V );
    return result;
}

/// Vector8 MulSub : a * b - c : [ scalar * vector - vector ]
NOALIAS __forceinline const Vector8 MulSub( const VF32 VECREF a, const Vector8& b, const Vector8& c )
{
    Vector8 result;
    result.m_V = MulSub( a, b.m_V, c.m_V );
    return result;
}

/// Vector8 NegMulAdd : -( a * b + c ) : [ - ( scalar * vector + vector ) ]
NOALIAS __forceinline const Vector8 NegMulAdd( const VF32 VECREF a, const Vector8& b, const Vector8& c )
{
    Vector8 result;
    result.m_V = NegMulAdd( a, b.m_V, c.m_V );
    return result;
}

/// Vector8 NegMulSub : -( a * b - c ) == c - a * b : [ - ( scalar * vector - vector ) ]
NOALIAS __forceinline const Vector8 NegMulSub( const VF32 VECREF a, const Vector8& b, const Vector8& c )
{
    Vector8 result;
    result.m_V = NegMulSub( a, b.m_V, c.m_V );
    return result;
}


/// Vector8 Min
NOALIAS __forceinline const Vector8 Min( const Vector8& a, const Vector8& b )
{
    Vector8 result;
    result.m_V = Min( a.m_V, b.m_V );
    return result;
}

/// Vector8 Max
NOALIAS __forceinline const Vector8 Max( const Vector8& a, const Vector8& b )
{
    Vector8 result;
    result.m_V = Max( a.m_V, b.m_V );
    return result;
}

/// Vector8 Min
NOALIAS __forceinline const Vector8 Min( const Vector8& a, const VF32 VECREF b )
{
    Vector8 result;
    result.m_V = Min( a.m_V, b );
    return result;
}

/// Vector8 Max
NOALIAS __forceinline const Vector8 Max( const Vector8& a, const VF32 VECREF b )
{
    Vector8 result;
    result.m_V = Max( a.m_V, b );
    return result;
}

/// Vector8 Min Update
NOALIAS __forceinline void Min( Vector8* const __restrict pA, const Vector8& b )
{
    Min( &pA->m_V, b.m_V );
}

/// Vector8 Max Update
NOALIAS __forceinline void Max( Vector8* const __restrict pA, const Vector8& b )
{
    Max( &pA->m_V, b.m_V );
}


/// Vector8 Clamp
NOALIAS __forceinline const Vector8 Clamp( const Vector8& a, const Vector8& lo, const Vector8& hi )
{
    Vector8 result;
    result.m_V = Clamp( a.m_V, lo.m_V, hi.m_V );
    return result;
}

/// Vector8 Clamp Update
NOALIAS __forceinline void Clamp( Vector8* const __restrict pA, const Vector8& lo, const Vector8& hi )
{
    Clamp( &pA->m_V, lo.m_V, hi.m_V );
}


/// Vector8 Round to integer towards minus infinity
NOALIAS __forceinline const Vector8 Floor( const Vector8& a )
{
    Vector8 result;
    result.m_V = Floor( a.m_V );
    return result;
}

/// Vector8 Round to integer towards plus infinity
NOALIAS __forceinline const Vector8 Ceil ( const Vector8& a )
{
    Vector8 result;
    result.m_V = Ceil( a.m_V );
    return result;
}

/// Vector8 Round to integer towards nearest
NOALIAS __forceinline const Vector8 Round( const Vector8& a )
{
    Vector8 result;
    result.m_V = Round( a.m_V );
    return result;
}

/// Vector8 Round to integer towards zero
NOALIAS __forceinline const Vector8 Trunc( const Vector8& a )
{
    Vector8 result;
    result.m_V = Trunc( a.m_V );
    return result;
}



/// Vector8 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w
NOALIAS __forceinline const VBool operator == ( const Vector8& a, const Vector8& b )
{
    return a.m_V == b.m_V;
}

/// Vector8 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w
NOALIAS __forceinline const VBool operator != ( const Vector8& a, const Vector8& b )
{
    return a.m_V != b.m_V;
}

/// Vector8 Comparison a <= b : Returns Vec8Bool representing results of a.i <= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec8Bool operator <= ( const Vector8& a, const Vector8& b )
{
    return a.m_V <= b.m_V;
}

/// Vector8 Comparison a >= b : Returns Vec8Bool representing results of a.i >= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec8Bool operator >= ( const Vector8& a, const Vector8& b )
{
    return a.m_V >= b.m_V;
}

/// Vector8 Comparison a < b : Returns Vec8Bool representing results of a.i < b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec8Bool operator < ( const Vector8& a, const Vector8& b )
{
    return a.m_V < b.m_V;
}

/// Vector8 Comparison a > b : Returns Vec8Bool representing results of a.i > b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec8Bool operator > ( const Vector8& a, const Vector8& b )
{
    return a.m_V > b.m_V;
}

/// Vector8 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector8& a, const Vector8& b )
{
    return Ordered( a.m_V, b.m_V );
}

/// Vector8 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector8& a, const Vector8& b )
{
    return Unordered( a.m_V, b.m_V );
}


/// Vector8 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vector8& a )
{
    return IsNan( a.m_V );
}

/// Vector8 Finite Test : true if a contains no elements that are NaN's and for each element i [x,y,z,w], abs(x.i) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vector8& a )
{
    return Finite( a.m_V );
}


/// Vector8 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector8 Select( const VBool PCREF condition, const Vector8& result_if_true, const Vector8& result_if_false )
{
    Vector8 result;
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    return result;
}

/// Vector8 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector8 Select( const Vec8Bool PCREF condition, const Vector8& result_if_true, const Vector8& result_if_false )
{
    Vector8 result;
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    return result;
}


/// Vector8 Bitwise AND: a & b : for each element i, returns ( b ? a[i] : zero )
NOALIAS __forceinline const Vector8 AND( const Vector8& a, const VBool PCREF b )
{
    Vector8 result;
    result.m_V = AND( a.m_V, b );
    return result;
}

/// Vector8 Bitwise ANDC: a & ~b : for each element i, returns ( b ? zero : a[i] )
NOALIAS __forceinline const Vector8 ANDC( const Vector8& a, const VBool PCREF b )
{
    Vector8 result;
    result.m_V = ANDC( a.m_V, b );
    return result;
}


/// Vector8 Bitwise AND: a & b : for each element i, returns ( b[i] ? a[i] : zero )
NOALIAS __forceinline const Vector8 AND( const Vector8& a, const Vec8Bool PCREF b )
{
    Vector8 result;
    result.m_V = AND( a.m_V, b );
    return result;
}

/// Vector8 Bitwise ANDC: a & ~b : for each element i, returns ( b[i] ? zero : a[i] )
NOALIAS __forceinline const Vector8 ANDC( const Vector8& a, const Vec8Bool PCREF b )
{
    Vector8 result;
    result.m_V = ANDC( a.m_V, b );
    return result;
}


/// Vector8 Dot Product
NOALIAS __forceinline const VF32 Dot( const Vector8& a, const Vector8& b )
{
    return Dot8( a.m_V, b.m_V );
}


/// Vector8 Flush To Zero: flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vector8 FlushToZero( const Vector8& v, const VF32 VECREF tolerance )
{
    Vector8 result;
    result.m_V = FlushToZero8( v.m_V, tolerance );
    return result;
}

/// Vector8 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vector8 FlushElementsToZero( const Vector8& v, const VF32 VECREF tolerance )
{
    Vector8 result;
    result.m_V = FlushElementsToZero( v.m_V, tolerance );
    return result;
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

