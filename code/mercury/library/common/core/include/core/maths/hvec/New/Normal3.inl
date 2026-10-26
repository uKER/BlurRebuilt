#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_NORMAL3_INL
#define SOFT_MATHS_HVEC_NEW_NORMAL3_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_NORMAL3_INL
#define MATHS_HVEC_NEW_NORMAL3_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Normal3
/// @{

/// Load Normal3 from StorageNormal3
NOALIAS __forceinline const Normal3 Load( StorageNormal3 const * const __restrict pSrc )
{
    Normal3 result;
    result.m_V = Load< Vec4F32 >( &pSrc->m_Data );
    return result;
}

/// Store Normal3 in StorageNormal3
NOALIAS __forceinline void Store( const Normal3 PCREF data, StorageNormal3 * const __restrict pDst )
{
    Store( data.m_V, &pDst->m_Data );
}



/// Make Normal3 from Vec4F32
/// WARNING: This does NOT normalise the input vector! It is assumed that the input vector is already normalised.
/// Use the Vector3 Normalise() or the Vec4F32 Normalise3() functions to ensure that the input vector is normalised!
NOALIAS __forceinline const Normal3 MakeNormal3( const Vec4F32 VECREF src )
{
    Normal3 result;
    result.m_V = src;
    return result;
}

/// Get Vec4F32 from Normal3
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Normal3 PCREF src )
{
    return src.m_V;
}

/// Make Normal3 from Vector3
/// WARNING: This does NOT normalise the input vector! It is assumed that the input vector is already normalised.
/// Use the Vector3 Normalise() function instead if the input vector is not guaranteed to be normalised!
NOALIAS __forceinline const Normal3 MakeNormal3( const Vector3 PCREF src )
{
    return MakeNormal3( src.m_V );
}


/// Normal3 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Normal3 PCREF src )
{
    return X( src.m_V );
}

/// Normal3 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Normal3 PCREF src )
{
    return Y( src.m_V );
}

/// Normal3 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Normal3 PCREF src )
{
    return Z( src.m_V );
}



/// Normal3 Write to Stream
template < typename T_Stream > void Serialise( const Normal3 PCREF src, T_Stream& stream )
{
    StorageNormal3 storage;
    Store( src, &storage );
    stream << storage.m_Data.x;
    stream << storage.m_Data.y;
    stream << storage.m_Data.z;
}

/// Normal3 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Normal3* const __restrict pDst )
{
    StorageNormal3 storage;
    stream >> storage.m_Data.x;
    stream >> storage.m_Data.y;
    stream >> storage.m_Data.z;
    *pDst = Load( &storage );
}




/// Normal3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Normal3 Permute( const Normal3 PCREF a )
{
    StaticAssert( IX < 3, IX_MUST_BE_LESS_THAN_3 );
    StaticAssert( IY < 3, IY_MUST_BE_LESS_THAN_3 );
    StaticAssert( IZ < 3, IZ_MUST_BE_LESS_THAN_3 );
    StaticAssert( IX != IY, NO_ELEMENT_DUPLICATION_ALLOWED );
    StaticAssert( IX != IZ, NO_ELEMENT_DUPLICATION_ALLOWED );
    StaticAssert( IY != IZ, NO_ELEMENT_DUPLICATION_ALLOWED );
    
    return MakeNormal3( Permute<IX,IY,IZ>( a.m_V ) );
}


/// Normal3 Unary Positive Operator
NOALIAS __forceinline const Normal3 operator + ( const Normal3 PCREF x )
{
    return x;
}

/// Normal3 Unary Negation Operator
NOALIAS __forceinline const Normal3 operator - ( const Normal3 PCREF x )
{
    return MakeNormal3( -x.m_V );
}

/// Normal3 Sum
NOALIAS __forceinline const Vector3 operator + ( const Normal3 PCREF a, const Normal3 PCREF b )
{
    return MakeVector3( a.m_V + b.m_V );
}

/// Normal3 Difference
NOALIAS __forceinline const Vector3 operator - ( const Normal3 PCREF a, const Normal3 PCREF b )
{
    return MakeVector3( a.m_V - b.m_V );
}


/// Normal3 Scalar Multiplication
NOALIAS __forceinline const Vector3 operator * ( const Normal3 PCREF n, const VF32 VECREF s )
{
    return MakeVector3( n.m_V * s );
}

/// Normal3 Scalar Multiplication
NOALIAS __forceinline const Vector3 operator * ( const VF32 VECREF s, const Normal3 PCREF n )
{
    return MakeVector3( n.m_V * s );
}

/// Normal3 Scalar Division
NOALIAS __forceinline const Vector3 operator / ( const Normal3 PCREF n, const VF32 VECREF s )
{
    return MakeVector3( n.m_V / s );
}




/// Normal3 Matrix Multiplication: Normal3 * Mat33
NOALIAS __forceinline const Vector3 operator * ( const Normal3 PCREF n, const Mat33 MATREF m )
{
    Vector3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_vector3_matrix33( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_vector3_matrix33( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_vector3_matrix33( n.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_vector3_matrix33( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = n.m_V.m_V[0] * m.m[0][c] +
                            n.m_V.m_V[1] * m.m[1][c] +
                            n.m_V.m_V[2] * m.m[2][c];
    }
    #endif
    return result;
}

/// Normal3 Matrix Multiplication: Normal3 * Mat43
NOALIAS __forceinline const Vector3 operator * ( const Normal3 PCREF n, const Mat43 MATREF m )
{
    Vector3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_vector3_matrix43( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_vector3_matrix43( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_vector3_matrix43( n.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_vector3_matrix43( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = n.m_V.m_V[0] * m.m[0][c] +
                            n.m_V.m_V[1] * m.m[1][c] +
                            n.m_V.m_V[2] * m.m[2][c];
    }
    #endif
    return result;
}

/// Normal3 Matrix Multiplication: Normal3 * Mat44
NOALIAS __forceinline const Vector4 operator * ( const Normal3 PCREF n, const Mat44 MATREF m )
{
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_vector3_matrix44( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_vector3_matrix44( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_vector3_matrix44( n.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ, m.m_ColW );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_vector3_matrix44( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    for ( S32 c=0; c<4; c++ )
    {
        result.m_V.m_V[c] = n.m_V.m_V[0] * m.m[0][c] +
                            n.m_V.m_V[1] * m.m[1][c] +
                            n.m_V.m_V[2] * m.m[2][c];
    }
    #endif
    return result;
}

/// Normal3 Matrix Multiplication Update: Normal3 *= Mat33
/// WARNING: This does NOT normalise the transformed normal!
NOALIAS __forceinline void operator *= ( Normal3& n, const Mat33 MATREF m )
{
    #if defined( _SPU_ )
    n.m_V.m_V = spu_multiply_vector3_matrix33( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    n.m_V.m_V = vec_multiply_vector3_matrix33( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    n.m_V.m_V = vmx_multiply_vector3_matrix33( n.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    n.m_V.m_V = sse_multiply_vector3_matrix33( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    Normal3 result;
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = n.m_V.m_V[0] * m.m[0][c] +
                            n.m_V.m_V[1] * m.m[1][c] +
                            n.m_V.m_V[2] * m.m[2][c];
    }
    n = result;
    #endif
}

/// Normal3 Matrix Multiplication Update: Normal3 *= Mat43
/// WARNING: This does NOT normalise the transformed normal!
NOALIAS __forceinline void operator *= ( Normal3& n, const Mat43 MATREF m )
{
    #if defined( _SPU_ )
    n.m_V.m_V = spu_multiply_vector3_matrix43( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    n.m_V.m_V = vec_multiply_vector3_matrix43( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    n.m_V.m_V = vmx_multiply_vector3_matrix43( n.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    n.m_V.m_V = sse_multiply_vector3_matrix43( n.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    Normal3 result;
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = n.m_V.m_V[0] * m.m[0][c] +
                            n.m_V.m_V[1] * m.m[1][c] +
                            n.m_V.m_V[2] * m.m[2][c];
    }
    n = result;
    #endif
}



/// Normal3 dot product
NOALIAS __forceinline const VF32 Dot( const Normal3 PCREF a, const Normal3 PCREF b )
{
    return Dot3( a.m_V, b.m_V );
}

/// Normal3-Point3 dot product ( normal | ( point - origin ) )
NOALIAS __forceinline const VF32 Dot( const Normal3 PCREF a, const Point3 PCREF b )
{
    return Dot3( a.m_V, b.m_V );
}

/// Point3-Normal3 dot product ( normal | ( point - origin ) )
NOALIAS __forceinline const VF32 Dot( const Point3 PCREF a, const Normal3 PCREF b )
{
    return Dot3( a.m_V, b.m_V );
}


/// Normal3 cross product
NOALIAS __forceinline const Vector3 Cross( const Normal3 PCREF a, const Normal3 PCREF b )
{
    return MakeVector3( Cross3( a.m_V, b.m_V ) );
}


/// Normal3 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Normal3 PCREF a, const Normal3 PCREF b )
{
    /// @par WARNING: Operator precedence of '|' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    return Dot3( a.m_V, b.m_V );
}

/// Normal3-Point3 dot product operator ( normal | ( point - origin ) )
NOALIAS __forceinline const VF32 operator | ( const Normal3 PCREF a, const Point3 PCREF b )
{
    /// @par WARNING: Operator precedence of '|' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    return Dot3( a.m_V, b.m_V );
}

/// Point3-Normal3 dot product operator ( normal | ( point - origin ) )
NOALIAS __forceinline const VF32 operator | ( const Point3 PCREF a, const Normal3 PCREF b )
{
    /// @par WARNING: Operator precedence of '|' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    return Dot3( a.m_V, b.m_V );
}


/// Normal3 cross product operator
NOALIAS __forceinline const Vector3 operator ^ ( const Normal3 PCREF a, const Normal3 PCREF b )
{
    /// @par WARNING: Operator precedence of '^' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    return MakeVector3( Cross3( a.m_V, b.m_V ) );
}



/// Normal3 Renormalise (to correct for accumulated float errors) (faster but only 22 bit precision)
NOALIAS __forceinline const Normal3 Normalise( const Normal3 PCREF n )
{
    Normal3 result;
    result.m_V = Normalise3( n.m_V );
    return result;
}

/// Normal3 Renormalise (to correct for accumulated float errors) (slower but full 24 bit precision)
NOALIAS __forceinline const Normal3 Normalise24( const Normal3 PCREF n )
{
    Normal3 result;
    result.m_V = Normalise3_24( n.m_V );
    return result;
}

/// Normal3 Renormalise (to correct for accumulated float errors) (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Normal3* const __restrict pDst )
{
    Normalise3( &pDst->m_V );
}

/// Normal3 Renormalise (to correct for accumulated float errors) (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise24( Normal3* const __restrict pDst )
{
    Normalise3_24( &pDst->m_V );
}




/// Normal3 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z
NOALIAS __forceinline const VBool operator == ( const Normal3 PCREF a, const Normal3 PCREF b )
{
    return Permute<VX,VY,VZ,VZ>( a.m_V ) == Permute<VX,VY,VZ,VZ>( b.m_V );
}

/// Normal3 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z
NOALIAS __forceinline const VBool operator != ( const Normal3 PCREF a, const Normal3 PCREF b )
{
    return Permute<VX,VY,VZ,VZ>( a.m_V ) != Permute<VX,VY,VZ,VZ>( b.m_V );
}



/// Normal3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Normal3 Select( const VBool PCREF condition, const Normal3 PCREF result_if_true, const Normal3 PCREF result_if_false )
{
    return MakeNormal3( Select( condition, result_if_true.m_V, result_if_false.m_V ) );
}

/// Normal3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Normal3 Select( const Vec4Bool PCREF condition, const Normal3 PCREF result_if_true, const Normal3 PCREF result_if_false )
{
    return MakeNormal3( Select( condition, result_if_true.m_V, result_if_false.m_V ) );
}


/// Normal3: Calculate Perpendicular Axes
/// Output normals obey same handedness as matrices, with input as axis z
NOALIAS __forceinline void PerpendicularAxes( const Normal3 PCREF axisZ, Normal3* const __restrict pAxisX, Normal3* const __restrict pAxisY )
{
    const Mat33 m( MakeMat33_AxisZ( axisZ ) );
    *pAxisX = MakeNormal3( RowX( m ) );
    *pAxisY = MakeNormal3( RowY( m ) );
}


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

