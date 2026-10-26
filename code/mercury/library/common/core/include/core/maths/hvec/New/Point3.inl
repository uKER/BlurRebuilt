#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_POINT3_INL
#define SOFT_MATHS_HVEC_NEW_POINT3_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_POINT3_INL
#define MATHS_HVEC_NEW_POINT3_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Point3
/// @{

/// Load Point3 from StoragePoint3
NOALIAS __forceinline const Point3 Load( StoragePoint3 const * const __restrict pSrc )
{
    Point3 result;
    result.m_V = Load< Vec4F32 >( &pSrc->m_Data );
    return result;
}

/// Store Point3 in StoragePoint3
NOALIAS __forceinline void Store( const Point3 PCREF data, StoragePoint3 * const __restrict pDst )
{
    Store( data.m_V, &pDst->m_Data );
}



/// Make Point3 from Vec4F32
NOALIAS __forceinline const Point3 MakePoint3( const Vec4F32 VECREF src )
{
    Point3 result;
    result.m_V = src;
    return result;
}

/// Get Vec4F32 from Point3
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Point3 PCREF src )
{
    return src.m_V;
}


/// Make Point3 from Vector3
NOALIAS __forceinline const Point3 MakePoint3( const Vector3 PCREF v3 )
{
    return MakePoint3( v3.m_V );
}

/// Make Point3 from Normal3
NOALIAS __forceinline const Point3 MakePoint3( const Normal3 PCREF n3 )
{
    return MakePoint3( n3.m_V );
}

/// Make Point3 from Vector4: returns ( x/w, y/w, z/w )
//NOALIAS __forceinline const Point3 MakePoint3( const Vector4 PCREF v4 )
//{
//    return MakePoint3( v4.m_V ) * Rcp( W( v4 ) );
//}



/// Make Point3 from 3 F32's
NOALIAS __forceinline const Point3 MakePoint3( const F32& x, const F32& y, const F32& z )
{
    Point3 result;
    result.m_V = MakeVec4F32( x, y, z, 0.0f );
    return result;
}

/// Make Point3 from 3 VF32's
NOALIAS __forceinline const Point3 MakePoint3( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    Point3 result;
    result.m_V = MakeVec4F32( x, y, z, Const<VF32>::Zero() );
    return result;
}



/// Point3 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Point3 PCREF src )
{
    return X( src.m_V );
}

/// Point3 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Point3 PCREF src )
{
    return Y( src.m_V );
}

/// Point3 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Point3 PCREF src )
{
    return Z( src.m_V );
}

/// Point3 Accessor: Set X
NOALIAS __forceinline void SetX( Point3* const __restrict pDst, const VF32 VECREF x )
{
    SetX( &pDst->m_V, x );
}

/// Point3 Accessor: Set Y
NOALIAS __forceinline void SetY( Point3* const __restrict pDst, const VF32 VECREF y )
{
    SetY( &pDst->m_V, y );
}

/// Point3 Accessor: Set Z
NOALIAS __forceinline void SetZ( Point3* const __restrict pDst, const VF32 VECREF z )
{
    SetZ( &pDst->m_V, z );
}



/// Point3 Write to Stream
template < typename T_Stream >
void Serialise( const Point3 PCREF src, T_Stream& stream )
{
    StoragePoint3 storage;
    Store( src, &storage );
    stream << storage.m_Data.x;
    stream << storage.m_Data.y;
    stream << storage.m_Data.z;
}

/// Point3 Read from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, Point3* const __restrict pDst )
{
    StoragePoint3 storage;
    stream >> storage.m_Data.x;
    stream >> storage.m_Data.y;
    stream >> storage.m_Data.z;
    *pDst = Load( &storage );
}



/// Point3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Point3 Permute( const Point3 PCREF a )
{
    StaticAssert( IX < 3, IX_MUST_BE_LESS_THAN_3 );
    StaticAssert( IY < 3, IY_MUST_BE_LESS_THAN_3 );
    StaticAssert( IZ < 3, IZ_MUST_BE_LESS_THAN_3 );
    
    return MakePoint3( Permute<IX,IY,IZ>( a.m_V ) );
}

/// Point3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:bx, 4:by, 5:bz ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Point3 Permute( const Point3 PCREF a, const Point3 PCREF b )
{
    StaticAssert( IX < 6, IX_MUST_BE_LESS_THAN_6 );
    StaticAssert( IY < 6, IY_MUST_BE_LESS_THAN_6 );
    StaticAssert( IZ < 6, IZ_MUST_BE_LESS_THAN_6 );
    
    return MakePoint3( Permute< IX<3?IX:IX+1, IY<3?IY:IY+1, IZ<3?IZ:IZ+1 >( a.m_V, b.m_V ) );
}



/// Point3 Sum
NOALIAS __forceinline const Point3 operator + ( const Point3 PCREF a, const Point3 PCREF b )
{
    Point3 result;
    result.m_V = a.m_V + b.m_V;
    return result;
}

/// Point3 Difference
NOALIAS __forceinline const Vector3 operator - ( const Point3 PCREF a, const Point3 PCREF b )
{
    Vector3 result;
    result.m_V = a.m_V - b.m_V;
    return result;
}

/// Point3 Addition Update
NOALIAS __forceinline void operator += ( Point3& a, const Point3 PCREF b )
{
    a.m_V += b.m_V;
}


/// Point3 Scalar Multiplication Point3 * VF32
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const VF32 VECREF s )
{
    Point3 result;
    result.m_V = p.m_V * s;
    return result;
}

/// Point3 Scalar Multiplication VF32 * Point3
NOALIAS __forceinline const Point3 operator * ( const VF32 VECREF s, const Point3 PCREF p )
{
    Point3 result;
    result.m_V = s * p.m_V;
    return result;
}

/// Point3 Scalar Division Point3 / VF32
NOALIAS __forceinline const Point3 operator / ( const Point3 PCREF p, const VF32 VECREF s )
{
    Point3 result;
    result.m_V = p.m_V / s;
    return result;
}

/// Point3 Scalar Multiplication Update: Point3 *= VF32
NOALIAS __forceinline void operator *= ( Point3& p, const VF32 VECREF s )
{
    p.m_V *= s;
}

/// Point3 Scalar Division Update: Point3 /= VF32
NOALIAS __forceinline void operator /= ( Point3& p, const VF32 VECREF s )
{
    p.m_V /= s;
}



/// Point3 Unary Positive Operator
NOALIAS __forceinline const Point3 operator + ( const Point3 PCREF x )
{
    return x;
}

/// Point3 Unary Negation Operator
NOALIAS __forceinline const Point3 operator - ( const Point3 PCREF x )
{
    return MakePoint3( -x.m_V );
}

/// Addition: Point3 + Vector3
NOALIAS __forceinline const Point3 operator + ( const Point3 PCREF p, const Vector3 PCREF v )
{
    Point3 result;
    result.m_V = p.m_V + v.m_V;
    return result;
}

/// Subtraction: Point3 - Vector3
NOALIAS __forceinline const Point3 operator - ( const Point3 PCREF p, const Vector3 PCREF v )
{
    Point3 result;
    result.m_V = p.m_V - v.m_V;
    return result;
}

/// Point3 Addition Update: Point3 += Vector3
NOALIAS __forceinline void operator += ( Point3& p, const Vector3 PCREF v )
{
    p.m_V += v.m_V;
}

/// Point3 Subtraction Update: Point3 -= Vector3
NOALIAS __forceinline void operator -= ( Point3& p, const Vector3 PCREF v )
{
    p.m_V -= v.m_V;
}



/// Addition: Point3 + Normal3
NOALIAS __forceinline const Point3 operator + ( const Point3 PCREF p, const Normal3 PCREF n )
{
    Point3 result;
    result.m_V = p.m_V + n.m_V;
    return result;
}

/// Subtraction: Point3 - Normal3
NOALIAS __forceinline const Point3 operator - ( const Point3 PCREF p, const Normal3 PCREF n )
{
    Point3 result;
    result.m_V = p.m_V - n.m_V;
    return result;
}

/// Point3 Addition Update: Point3 += Normal3
NOALIAS __forceinline void operator += ( Point3& p, const Normal3 PCREF n )
{
    p.m_V += n.m_V;
}

/// Point3 Subtraction Update: Point3 -= Normal3
NOALIAS __forceinline void operator -= ( Point3& p, const Normal3 PCREF n )
{
    p.m_V -= n.m_V;
}


/// Point3 MulAdd : a * b + c
NOALIAS __forceinline const Point3 MulAdd( const Vector3 PCREF a, const VF32 VECREF b, const Point3 PCREF c )
{
    Point3 result;
    result.m_V = MulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Point3 MulSub : a * b - c
NOALIAS __forceinline const Point3 MulSub( const Vector3 PCREF a, const VF32 VECREF b, const Point3 PCREF c )
{
    Point3 result;
    result.m_V = MulSub( a.m_V, b, c.m_V );
    return result;
}

/// Point3 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Point3 NegMulAdd( const Vector3 PCREF a, const VF32 VECREF b, const Point3 PCREF c )
{
    Point3 result;
    result.m_V = NegMulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Point3 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Point3 NegMulSub( const Vector3 PCREF a, const VF32 VECREF b, const Point3 PCREF c )
{
    Point3 result;
    result.m_V = NegMulSub( a.m_V, b, c.m_V );
    return result;
}


/// Point3 MulAdd : a * b + c
NOALIAS __forceinline const Point3 MulAdd( const Normal3 PCREF a, const VF32 VECREF b, const Point3 PCREF c )
{
    Point3 result;
    result.m_V = MulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Point3 MulSub : a * b - c
NOALIAS __forceinline const Point3 MulSub( const Normal3 PCREF a, const VF32 VECREF b, const Point3 PCREF c )
{
    Point3 result;
    result.m_V = MulSub( a.m_V, b, c.m_V );
    return result;
}

/// Point3 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Point3 NegMulAdd( const Normal3 PCREF a, const VF32 VECREF b, const Point3 PCREF c )
{
    Point3 result;
    result.m_V = NegMulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Point3 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Point3 NegMulSub( const Normal3 PCREF a, const VF32 VECREF b, const Point3 PCREF c )
{
    Point3 result;
    result.m_V = NegMulSub( a.m_V, b, c.m_V );
    return result;
}



/// Point3 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z
NOALIAS __forceinline const VBool operator == ( const Point3 PCREF a, const Point3 PCREF b )
{
    return Permute<VX,VY,VZ,VZ>( a.m_V ) == Permute<VX,VY,VZ,VZ>( b.m_V );
}

/// Point3 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z
NOALIAS __forceinline const VBool operator != ( const Point3 PCREF a, const Point3 PCREF b )
{
    return Permute<VX,VY,VZ,VZ>( a.m_V ) != Permute<VX,VY,VZ,VZ>( b.m_V );
}

/// Point3 Comparison a <= b : Returns Vec4Bool representing results of a.i <= b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator <= ( const Point3 PCREF a, const Point3 PCREF b )
{
    return a.m_V <= b.m_V;
}

/// Point3 Comparison a >= b : Returns VBool representing results of a.i >= b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator >= ( const Point3 PCREF a, const Point3 PCREF b )
{
    return a.m_V >= b.m_V;
}

/// Point3 Comparison a < b : Returns VBool representing results of a.i < b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator < ( const Point3 PCREF a, const Point3 PCREF b )
{
    return a.m_V < b.m_V;
}

/// Point3 Comparison a > b : Returns VBool representing results of a.i > b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator > ( const Point3 PCREF a, const Point3 PCREF b )
{
    return a.m_V > b.m_V;
}

/// Point3 Ordered Test : true if neither a nor b contains elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Point3 PCREF a, const Point3 PCREF b )
{
    return Ordered( Permute<VX,VY,VZ,VZ>( a.m_V ), Permute<VX,VY,VZ,VZ>( b.m_V ) );
}

/// Point3 Unordered Test : true if a or b contains elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Point3 PCREF a, const Point3 PCREF b )
{
    return Unordered( Permute<VX,VY,VZ,VZ>( a.m_V ), Permute<VX,VY,VZ,VZ>( b.m_V ) );
}




/// Point3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Point3 Select( const VBool PCREF condition, const Point3 PCREF result_if_true, const Point3 PCREF result_if_false )
{
    return MakePoint3( Select( condition, result_if_true.m_V, result_if_false.m_V ) );
}

/// Point3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Point3 Select( const Vec4Bool PCREF condition, const Point3 PCREF result_if_true, const Point3 PCREF result_if_false )
{
    return MakePoint3( Select( condition, result_if_true.m_V, result_if_false.m_V ) );
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

