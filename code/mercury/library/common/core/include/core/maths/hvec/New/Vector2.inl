#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR2_INL
#define SOFT_MATHS_HVEC_NEW_VECTOR2_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR2_INL
#define MATHS_HVEC_NEW_VECTOR2_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector2
/// @{

/// Load Vector2 from F32 ptr
template <> NOALIAS __forceinline const Vector2 Load( F32 const * const __restrict pSrc )
{
    Vector2 result;
    result.m_V = Load<Vec4F32>( pSrc );
    return result;
}

/// Store Vector2 to F32 ptr
NOALIAS __forceinline void Store( const Vector2 PCREF data, F32 * const __restrict pDst )
{
    Store( data.m_V, (StorageVec2F32*)pDst );
}


/// Load Vector2 from StorageVector2
NOALIAS __forceinline const Vector2 Load( StorageVector2 const * const __restrict pSrc )
{
    Vector2 result;
    //result.m_V = Load<Vec4F32>( &pSrc->m_Data );
    result.m_V = Load<Vec4F32>( (StorageVec2F32*)pSrc );
    return result;
}

/// Store Vector2 in StorageVector2
NOALIAS __forceinline void Store( const Vector2 PCREF data, StorageVector2 * const __restrict pDst )
{
    //Store( data.m_V, &pDst->m_Data );
    Store( data.m_V, (StorageVec2F32*)pDst );
}



/// Make Vector2 from broadcast VF32
NOALIAS __forceinline const Vector2 MakeVector2( const VF32 VECREF src )
{
    Vector2 result;
    result.m_V = MakeVec4F32( src );
    return result;
}

/// Make Vector2 from Vec4F32
NOALIAS __forceinline const Vector2 MakeVector2( const Vec4F32 VECREF src )
{
    Vector2 result;
    result.m_V = src;
    return result;
}

/// Get Vec4F32 from Vector2
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Vector2 PCREF src )
{
    return src.m_V;
}


/// Make Vector2 from Vector3
NOALIAS __forceinline const Vector2 MakeVector2( const Vector3 PCREF v3 )
{
    Vector2 result;
    result.m_V = v3.m_V;
    return result;
}

/// Make Vector2 from Vector4
NOALIAS __forceinline const Vector2 MakeVector2( const Vector4 PCREF v4 )
{
    Vector2 result;
    result.m_V = v4.m_V;
    return result;
}


/// Make Vector2 from 2 F32's
NOALIAS __forceinline const Vector2 MakeVector2( const F32& x, const F32& y )
{
    Vector2 result;
    result.m_V = MakeVec4F32( x, y );
    return result;
}

/// Make Vector2 from 2 VF32's
NOALIAS __forceinline const Vector2 MakeVector2( const VF32 VECREF x, const VF32 VECREF y )
{
    Vector2 result;
    result.m_V = MakeVec4F32( x, y );
    return result;
}



/// Vector2 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Vector2 PCREF src )
{
    return X( src.m_V );
}

/// Vector2 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Vector2 PCREF src )
{
    return Y( src.m_V );
}


/// Vector2 Accessor: Set X
NOALIAS __forceinline void SetX( Vector2* const __restrict pDst, const VF32 VECREF x )
{
    SetX( &pDst->m_V, x );
}

/// Vector2 Accessor: Set Y
NOALIAS __forceinline void SetY( Vector2* const __restrict pDst, const VF32 VECREF y )
{
    SetY( &pDst->m_V, y );
}


/// Vector2 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector2& v )
{
    BIZ_STATIC_ASSERT( I < 2 );
    return Element<I>( v.m_V );
}

/// Vector2 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector2* const __restrict pDst, const VF32 VECREF ei )
{
    BIZ_STATIC_ASSERT( I < 2 );
    SetElement<I>( &pDst->m_V, ei );
}


/// Vector2 Write to Stream
template < typename T_Stream > void Serialise( const Vector2 PCREF src, T_Stream& stream )
{
    StorageVector2 storage;
    Store( src, &storage );
    stream << storage.m_Data.x;
    stream << storage.m_Data.y;
}

/// Vector2 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Vector2* const __restrict pDst )
{
    StorageVector2 storage;
    stream >> storage.m_Data.x;
    stream >> storage.m_Data.y;
    *pDst = Load( &storage );
}



/// Vector2 Permute : Indices select 2 result elements [x,y] from [ 0:ax, 1:ay ]
template < unsigned int IX, unsigned int IY >
NOALIAS __forceinline const Vector2 Permute( const Vector2 PCREF a )
{
    StaticAssert( IX < 2, IX_MUST_BE_LESS_THAN_3 );
    StaticAssert( IY < 2, IY_MUST_BE_LESS_THAN_3 );
    
    return MakeVector2( Permute<IX,IY,VZ>( a.m_V ) );
}

/// Vector2 Permute : Indices select 2 result elements [x,y] from [ 0:ax, 1:ay, 2:bx, 3:by ]
template < unsigned int IX, unsigned int IY >
NOALIAS __forceinline const Vector2 Permute( const Vector2 PCREF a, const Vector2 PCREF b )
{
    StaticAssert( IX < 4, IX_MUST_BE_LESS_THAN_6 );
    StaticAssert( IY < 4, IY_MUST_BE_LESS_THAN_6 );
    
    return MakeVector2( Permute< IX<2?IX:IX+2, IY<2?IY:IY+2, VZ >( a.m_V, b.m_V ) );
}



/// Vector2 Abs of each element
NOALIAS __forceinline const Vector2 Abs( const Vector2 PCREF v )
{
    return MakeVector2( Abs( v.m_V ) );
}

/// Vector2 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector2 Sign( const Vector2 PCREF v )
{
    return MakeVector2( Sign( v.m_V ) );
}

/// Vector2 CopySign of each element : Returns a Vector2 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector2 CopySign( const Vector2 PCREF magnitude, const Vector2 PCREF sign )
{
    return MakeVector2( CopySign( magnitude.m_V, sign.m_V ) );
}


/// Vector2 Negation : Indices select elements to negate [X,Y]
template < bool X, bool Y >
NOALIAS __forceinline const Vector2 Negate( const Vector2& src )
{
    Vector2 result;
    result.m_V = Negate<X,Y,false,false>( src.m_V );
    return result;
}

/// Vector2 Negate X
NOALIAS __forceinline const Vector2 NegateX( const Vector2 PCREF src )
{
    Vector2 result;
    result.m_V = NegateX( src.m_V );
    return result;
}

/// Vector2 Negate Y
NOALIAS __forceinline const Vector2 NegateY( const Vector2 PCREF src )
{
    Vector2 result;
    result.m_V = NegateY( src.m_V );
    return result;
}


/// Vector2 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Rcp( const Vector2 PCREF v )
{
    return MakeVector2( Rcp( v.m_V ) );
}

/// Vector2 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Rcp24( const Vector2 PCREF v )
{
    return MakeVector2( Rcp24( v.m_V ) );
}

/// Vector2 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Div( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return MakeVector2( Div( a.m_V, b.m_V ) );
}

/// Vector2 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Div24( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return MakeVector2( Div24( a.m_V, b.m_V ) );
}

/// Vector2 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Div( const VF32 VECREF s, const Vector2 PCREF v )
{
    return MakeVector2( Div( s, v.m_V ) );
}

/// Vector2 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Div24( const VF32 VECREF s, const Vector2 PCREF v )
{
    return MakeVector2( Div24( s, v.m_V ) );
}

/// Vector2 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Div( const Vector2 PCREF v, const VF32 VECREF s )
{
    return MakeVector2( Div( v.m_V, s ) );
}

/// Vector2 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Div24( const Vector2 PCREF v, const VF32 VECREF s )
{
    return MakeVector2( Div24( v.m_V, s ) );
}

/// Vector2 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 RcpSqrt( const Vector2 PCREF v )
{
    return MakeVector2( RcpSqrt( v.m_V ) );
}

/// Vector2 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 RcpSqrt24( const Vector2 PCREF v )
{
    return MakeVector2( RcpSqrt24( v.m_V ) );
}

/// Vector2 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Sqrt( const Vector2 PCREF v )
{
    return MakeVector2( Sqrt( v.m_V ) );
}

/// Vector2 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Sqrt24( const Vector2 PCREF v )
{
    return MakeVector2( Sqrt24( v.m_V ) );
}

/// Vector2 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vector2 PCREF v,
                                       Vector2* const __restrict pRcp,
                                       Vector2* const __restrict pSqrt,
                                       Vector2* const __restrict pRcpSqrt )
{
    RcpAndSqrt( v.m_V, &pRcp->m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}

/// Vector2 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vector2 PCREF v,
                                         Vector2* const __restrict pRcp,
                                         Vector2* const __restrict pSqrt,
                                         Vector2* const __restrict pRcpSqrt )
{
    RcpAndSqrt24( v.m_V, &pRcp->m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}


/// Vector2 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vector2 PCREF v,
                                           Vector2* const __restrict pSqrt,
                                           Vector2* const __restrict pRcpSqrt )
{
    SqrtAndRcpSqrt( v.m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}

/// Vector2 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vector2 PCREF v,
                                             Vector2* const __restrict pSqrt,
                                             Vector2* const __restrict pRcpSqrt )
{
    SqrtAndRcpSqrt24( v.m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}



/// Vector2 Unary Positive Operator
NOALIAS __forceinline const Vector2 operator + ( const Vector2 PCREF x )
{
    return x;
}

/// Vector2 Unary Negation Operator
NOALIAS __forceinline const Vector2 operator - ( const Vector2 PCREF x )
{
    return MakeVector2( -x.m_V );
}


/// Vector2 Sum
NOALIAS __forceinline const Vector2 operator + ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    Vector2 result;
    result.m_V = a.m_V + b.m_V;
    return result;
}

/// Vector2 Difference
NOALIAS __forceinline const Vector2 operator - ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    Vector2 result;
    result.m_V = a.m_V - b.m_V;
    return result;
}

/// Vector2 Multiplication Operator
NOALIAS __forceinline const Vector2 operator * ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    Vector2 result;
    result.m_V = a.m_V * b.m_V;
    return result;
}

/// Vector2 Division Operator
NOALIAS __forceinline const Vector2 operator / ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    Vector2 result;
    result.m_V = a.m_V / b.m_V;
    return result;
}


/// Vector2 Addition Update
NOALIAS __forceinline void operator += ( Vector2& a, const Vector2 PCREF b )
{
    a.m_V += b.m_V;
}

/// Vector2 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector2& a, const Vector2 PCREF b )
{
    a.m_V -= b.m_V;
}

/// Vector2 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector2& a, const Vector2 PCREF b )
{
    a.m_V *= b.m_V;
}

/// Vector2 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector2& a, const Vector2 PCREF b )
{
    a.m_V /= b.m_V;
}


/// Vector2 Scalar Multiplication: Vector2 * VF32
NOALIAS __forceinline const Vector2 operator * ( const Vector2 PCREF v, const VF32 VECREF s )
{
    Vector2 result;
    result.m_V = v.m_V * s;
    return result;
}

/// Vector2 Scalar Multiplication: VF32 * Vector2
NOALIAS __forceinline const Vector2 operator * ( const VF32 VECREF s, const Vector2 PCREF v )
{
    Vector2 result;
    result.m_V = s * v.m_V;
    return result;
}

/// Vector2 Scalar Division: Vector2 / VF32
NOALIAS __forceinline const Vector2 operator / ( const Vector2 PCREF v, const VF32 VECREF s )
{
    Vector2 result;
    result.m_V = v.m_V / s;
    return result;
}

/// Vector2 Scalar Division: VF32 / Vector2
NOALIAS __forceinline const Vector2 operator / ( const VF32 VECREF s, const Vector2 PCREF v )
{
    Vector2 result;
    result.m_V = s / v.m_V;
    return result;
}


/// Vector2 Scalar Multiplication Update: Vector2 *= VF32
NOALIAS __forceinline void operator *= ( Vector2& v, const VF32 VECREF s )
{
    v.m_V *= s;
}

/// Vector2 Scalar Division Update: Vector2 /= VF32
NOALIAS __forceinline void operator /= ( Vector2& v, const VF32 VECREF s )
{
    v.m_V /= s;
}




/// Vector2 MulAdd : a * b + c
NOALIAS __forceinline const Vector2 MulAdd( const Vector2 PCREF a, const Vector2 PCREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = MulAdd( a.m_V, b.m_V, c.m_V );
    return result;
}

/// Vector2 MulSub : a * b - c
NOALIAS __forceinline const Vector2 MulSub( const Vector2 PCREF a, const Vector2 PCREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = MulSub( a.m_V, b.m_V, c.m_V );
    return result;
}

/// Vector2 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector2 NegMulAdd( const Vector2 PCREF a, const Vector2 PCREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = NegMulAdd( a.m_V, b.m_V, c.m_V );
    return result;
}

/// Vector2 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector2 NegMulSub( const Vector2 PCREF a, const Vector2 PCREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = NegMulSub( a.m_V, b.m_V, c.m_V );
    return result;
}


/// Vector2 MulAdd : a * b + c
NOALIAS __forceinline const Vector2 MulAdd( const Vector2 PCREF a, const VF32 VECREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = MulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Vector2 MulSub : a * b - c
NOALIAS __forceinline const Vector2 MulSub( const Vector2 PCREF a, const VF32 VECREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = MulSub( a.m_V, b, c.m_V );
    return result;
}

/// Vector2 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector2 NegMulAdd( const Vector2 PCREF a, const VF32 VECREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = NegMulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Vector2 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector2 NegMulSub( const Vector2 PCREF a, const VF32 VECREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = NegMulSub( a.m_V, b, c.m_V );
    return result;
}


/// Vector2 MulAdd : a * b + c
NOALIAS __forceinline const Vector2 MulAdd( const VF32 VECREF a, const Vector2 PCREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = MulAdd( a, b.m_V, c.m_V );
    return result;
}

/// Vector2 MulSub : a * b - c
NOALIAS __forceinline const Vector2 MulSub( const VF32 VECREF a, const Vector2 PCREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = MulSub( a, b.m_V, c.m_V );
    return result;
}

/// Vector2 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector2 NegMulAdd( const VF32 VECREF a, const Vector2 PCREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = NegMulAdd( a, b.m_V, c.m_V );
    return result;
}

/// Vector2 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector2 NegMulSub( const VF32 VECREF a, const Vector2 PCREF b, const Vector2 PCREF c )
{
    Vector2 result;
    result.m_V = NegMulSub( a, b.m_V, c.m_V );
    return result;
}


/// Vector2 Lerp
NOALIAS __forceinline const Vector2 Lerp( const Vector2 PCREF src, const Vector2 PCREF dst, const VF32 VECREF t )
{
    Vector2 result;
    result.m_V = Lerp( src.m_V, dst.m_V, t );
    return result;
}


/// Vector2 Min
NOALIAS __forceinline const Vector2 Min( const Vector2 PCREF a, const Vector2 PCREF b )
{
    Vector2 result;
    result.m_V = Min( a.m_V, b.m_V );
    return result;
}

/// Vector2 Max
NOALIAS __forceinline const Vector2 Max( const Vector2 PCREF a, const Vector2 PCREF b )
{
    Vector2 result;
    result.m_V = Max( a.m_V, b.m_V );
    return result;
}

/// Vector2 Min
NOALIAS __forceinline const Vector2 Min( const Vector2 PCREF a, const VF32 VECREF b )
{
    Vector2 result;
    result.m_V = Min( a.m_V, b );
    return result;
}

/// Vector2 Max
NOALIAS __forceinline const Vector2 Max( const Vector2 PCREF a, const VF32 VECREF b )
{
    Vector2 result;
    result.m_V = Max( a.m_V, b );
    return result;
}

/// Vector2 Min Update
NOALIAS __forceinline void Min( Vector2* const __restrict pA, const Vector2 PCREF b )
{
    Min( &pA->m_V, b.m_V );
}

/// Vector2 Max Update
NOALIAS __forceinline void Max( Vector2* const __restrict pA, const Vector2 PCREF b )
{
    Max( &pA->m_V, b.m_V );
}

/// Vector2 Min Update
NOALIAS __forceinline void Min( Vector2* const __restrict pA, const VF32 VECREF b )
{
    Min( &pA->m_V, b );
}

/// Vector2 Max Update
NOALIAS __forceinline void Max( Vector2* const __restrict pA, const VF32 VECREF b )
{
    Max( &pA->m_V, b );
}


/// Vector2 Clamp
NOALIAS __forceinline const Vector2 Clamp( const Vector2 PCREF a, const Vector2 PCREF lo, const Vector2 PCREF hi )
{
    Vector2 result;
    result.m_V = Clamp( a.m_V, lo.m_V, hi.m_V );
    return result;
}

/// Vector2 Clamp
NOALIAS __forceinline const Vector2 Clamp( const Vector2 PCREF a, const VF32 VECREF lo, const VF32 VECREF hi )
{
    Vector2 result;
    result.m_V = Clamp( a.m_V, lo, hi );
    return result;
}

/// Vector2 Clamp Update
NOALIAS __forceinline void Clamp( Vector2* const __restrict pA, const Vector2 PCREF lo, const Vector2 PCREF hi )
{
    Clamp( &pA->m_V, lo.m_V, hi.m_V );
}

/// Vector2 Clamp Update
NOALIAS __forceinline void Clamp( Vector2* const __restrict pA, const VF32 VECREF lo, const VF32 VECREF hi )
{
    Clamp( &pA->m_V, lo, hi );
}


/// Vector2 Round to integer towards minus infinity
NOALIAS __forceinline const Vector2 Floor( const Vector2 PCREF a )
{
    Vector2 result;
    result.m_V = Floor( a.m_V );
    return result;
}

/// Vector2 Round to integer towards plus infinity
NOALIAS __forceinline const Vector2 Ceil( const Vector2 PCREF a )
{
    Vector2 result;
    result.m_V = Ceil( a.m_V );
    return result;
}

/// Vector2 Round to integer towards nearest
NOALIAS __forceinline const Vector2 Round( const Vector2 PCREF a )
{
    Vector2 result;
    result.m_V = Round( a.m_V );
    return result;
}

/// Vector2 Round to integer towards zero
NOALIAS __forceinline const Vector2 Trunc( const Vector2 PCREF a )
{
    Vector2 result;
    result.m_V = Trunc( a.m_V );
    return result;
}


/// Vector2 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y
NOALIAS __forceinline const VBool operator == ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return Permute<VX,VY,VY,VY>( a.m_V ) == Permute<VX,VY,VY,VY>( b.m_V );
}

/// Vector2 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y
NOALIAS __forceinline const VBool operator != ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return Permute<VX,VY,VY,VY>( a.m_V ) != Permute<VX,VY,VY,VY>( b.m_V );
}

/// Vector2 Comparison a <= b : Returns Vec4Bool representing results of a.i <= b.i, for each element i [x,y] (Z&W are undefined)
NOALIAS __forceinline const Vec4Bool operator <= ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return a.m_V <= b.m_V;
}

/// Vector2 Comparison a >= b : Returns VBool representing results of a.i >= b.i, for each element i [x,y] (Z&W are undefined)
NOALIAS __forceinline const Vec4Bool operator >= ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return a.m_V >= b.m_V;
}

/// Vector2 Comparison a < b : Returns VBool representing results of a.i < b.i, for each element i [x,y] (Z&W are undefined)
NOALIAS __forceinline const Vec4Bool operator < ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return a.m_V < b.m_V;
}

/// Vector2 Comparison a > b : Returns VBool representing results of a.i > b.i, for each element i [x,y] (Z&W are undefined)
NOALIAS __forceinline const Vec4Bool operator > ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return a.m_V > b.m_V;
}

/// Vector2 Ordered Test : true if neither a nor b contains elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return Ordered( Permute<VX,VY,VY,VY>( a.m_V ), Permute<VX,VY,VY,VY>( b.m_V ) );
}

/// Vector2 Unordered Test : true if a or b contains elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return Unordered( Permute<VX,VY,VY,VY>( a.m_V ), Permute<VX,VY,VY,VY>( b.m_V ) );
}

/// Vector2 Finite test
NOALIAS __forceinline const VBool Finite( const Vector2 PCREF src )
{
    return Finite( src.m_V );
}




/// Vector2 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector2 Select( const VBool PCREF condition, const Vector2 PCREF result_if_true, const Vector2 PCREF result_if_false )
{
    return MakeVector2( Select( condition, result_if_true.m_V, result_if_false.m_V ) );
}

/// Vector2 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector2 Select( const Vec4Bool PCREF condition, const Vector2 PCREF result_if_true, const Vector2 PCREF result_if_false )
{
    return MakeVector2( Select( condition, result_if_true.m_V, result_if_false.m_V ) );
}


/// Vector2 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vector2 AND( const Vector2 PCREF a, const VBool PCREF b )
{
    Vector2 result;
    result.m_V = AND( a.m_V, b );
    return result;
}

/// Vector2 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vector2 ANDC( const Vector2 PCREF a, const VBool PCREF b )
{
    Vector2 result;
    result.m_V = ANDC( a.m_V, b );
    return result;
}


/// Vector2 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vector2 AND( const Vector2 PCREF a, const Vec4Bool PCREF b )
{
    Vector2 result;
    result.m_V = AND( a.m_V, b );
    return result;
}

/// Vector2 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vector2 ANDC( const Vector2 PCREF a, const Vec4Bool PCREF b )
{
    Vector2 result;
    result.m_V = ANDC( a.m_V, b );
    return result;
}




/// Vector2 dot product
NOALIAS __forceinline const VF32 Dot( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return Dot2( a.m_V, b.m_V );
}


/// Vector2 cross product
NOALIAS __forceinline const VF32 Cross( const Vector2 PCREF a, const Vector2 PCREF b )
{
    return Cross2( a.m_V, b.m_V );
}


/// Vector2 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    /// @par WARNING: Operator precedence of '|' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    return Dot2( a.m_V, b.m_V );
}


/// Vector2 cross product operator
NOALIAS __forceinline const VF32 operator ^ ( const Vector2 PCREF a, const Vector2 PCREF b )
{
    /// @par WARNING: Operator precedence of '^' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    return Cross2( a.m_V, b.m_V );
}


/// Vector2 Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Vector2 PCREF v )
{
    return LengthSquared2( v.m_V );
}


/// Vector2 Length (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length( const Vector2 PCREF v )
{
    return Length2( v.m_V );
}

/// Vector2 Length (slower but full 24 bit precision
NOALIAS __forceinline const VF32 Length24( const Vector2 PCREF v )
{
    return Length2_24( v.m_V );
}


/// Vector2 Set Length (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength( Vector2* const __restrict pDst, const VF32 VECREF length )
{
    SetLength2( &pDst->m_V, length );
}

/// Vector2 Set Length (slower but full 24 bit precision
NOALIAS __forceinline void SetLength24( Vector2* const __restrict pDst, const VF32 VECREF length )
{
    SetLength2_24( &pDst->m_V, length );
}


/// Vector2 Limit Length (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength( Vector2* const __restrict pV, const VF32 VECREF limit )
{
    LimitLength2( &pV->m_V, limit );
}

/// Vector2 Limit Length (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength24( Vector2* const __restrict pV, const VF32 VECREF limit )
{
    LimitLength2_24( &pV->m_V, limit );
}


/// Vector2 Normalise (faster but only 22 bit precision)
//NOALIAS __forceinline const Normal2 Normalise( const Vector2 PCREF v )
NOALIAS __forceinline const Vector2 Normalise( const Vector2 PCREF v )
{
    Vector2 result;
    result.m_V = Normalise2( v.m_V );
    return result;
}

/// Vector2 Normalise (slower but full 24 bit precision
//NOALIAS __forceinline const Normal2 Normalise24( const Vector2 PCREF v )
NOALIAS __forceinline const Vector2 Normalise24( const Vector2 PCREF v )
{
    Vector2 result;
    result.m_V = Normalise2_24( v.m_V );
    return result;
}


/// Vector2 Normalise (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Vector2* const __restrict pDst )
{
    Normalise2( &pDst->m_V );
}

/// Vector2 Normalise (slower but full 24 bit precision
NOALIAS __forceinline void Normalise24( Vector2* const __restrict pDst )
{
    Normalise2_24( &pDst->m_V );
}


/// Vector2 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Inverse( const Vector2 PCREF v )
{
    return v * Rcp( LengthSquared( v ) );
}

/// Vector2 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Inverse24( const Vector2 PCREF v )
{
    return v * Rcp24( LengthSquared( v ) );
}


/// Vector2 Flush To Zero: flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vector2 FlushToZero( const Vector2 PCREF v, const VF32 VECREF tolerance )
{
    Vector2 result;
    result.m_V = FlushToZero2( v.m_V, tolerance );
    return result;
}

/// Vector2 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vector2 FlushElementsToZero( const Vector2 PCREF v, const VF32 VECREF tolerance )
{
    Vector2 result;
    result.m_V = FlushElementsToZero( v.m_V, tolerance );
    return result;
}


/// Vector2 maximum absolute value of all elements
NOALIAS __forceinline const VF32 MaxAbsValue( const Vector2 PCREF src )
{
    const Vector2 abs_src( Abs( src ) );
    return Max( X( abs_src ),
                Y( abs_src ) );
}


/// Vector2 Project Onto Axis: returns the component of vector 'v' parallel to axis 'a'
NOALIAS __forceinline const Vector2 Project( const Vector2 PCREF v, const Vector2 PCREF a )
{
    return a * Dot( v, Normalise( a ) );
}

/// Vector2 Reject From Axis: returns the component of vector 'v' perpendicular to axis 'a'
NOALIAS __forceinline const Vector2 Reject( const Vector2 PCREF v, const Vector2 PCREF a )
{
    const Vector2 na( Normalise( a ) );
    return NegMulSub( na, Dot( v, na ), v );
}


/// Vector2 ZeroX: sets the X component to zero
NOALIAS __forceinline void ZeroX( Vector2* const __restrict pV )
{
    SetX( pV, Const<VF32>::Zero() );
}

/// Vector2 ZeroY: sets the Y component to zero
NOALIAS __forceinline void ZeroY( Vector2* const __restrict pV )
{
    SetY( pV, Const<VF32>::Zero() );
}


/// Vector2 ZeroX: returns copy of v with the X component set to zero
NOALIAS __forceinline const Vector2 ZeroX( const Vector2 PCREF v )
{
    Vector2 result( v );
    SetX( &result, Const<VF32>::Zero() );
    return result;
}

/// Vector2 ZeroY: returns copy of v with the Y component set to zero
NOALIAS __forceinline const Vector2 ZeroY( const Vector2 PCREF v )
{
    Vector2 result( v );
    SetY( &result, Const<VF32>::Zero() );
    return result;
}







/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

