#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_VECTOR_INL
#define SOFT_MATHS_HVEC_NEW_G3V_VECTOR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_VECTOR_INL
#define MATHS_HVEC_NEW_G3V_VECTOR_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup G3V_Vector
/// @{

NOALIAS __forceinline const G3V::Vector Const< G3V::Vector >::Zero() ///< Constant Vector(0,0,0)
{
    return G3V::MakeVector( Maths::Const<Vec4F32>::Zero() );
}

NOALIAS __forceinline const G3V::Vector Const< G3V::Vector >::One() ///< Constant Vector(1,1,1)
{
    return G3V::MakeVector( Maths::Const<Vec4F32>::One() );
}

NOALIAS __forceinline const G3V::Vector Const< G3V::Vector >::UnitX() ///< Constant Vector(1,0,0)
{
    return G3V::MakeVector( Maths::Const<Vec4F32>::UnitX() );
}

NOALIAS __forceinline const G3V::Vector Const< G3V::Vector >::UnitY() ///< Constant Vector(0,1,0)
{
    return G3V::MakeVector( Maths::Const<Vec4F32>::UnitY() );
}

NOALIAS __forceinline const G3V::Vector Const< G3V::Vector >::UnitZ() ///< Constant Vector(0,0,1)
{
    return G3V::MakeVector( Maths::Const<Vec4F32>::UnitZ() );
}

/// @}

namespace G3V
{
/// @addtogroup G3V_Vector
/// @{

/// Make Vector from 3 F32's
NOALIAS __forceinline const Vector MakeVector( const F32& x, const F32& y, const F32& z )
{
    Vector result;
    result.m_V = MakeVector3( x, y, z );
    return result;
}

/// Make Vector from 3 VF32's
NOALIAS __forceinline const Vector MakeVector( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    Vector result;
    result.m_V = MakeVector3( x, y, z );
    return result;
}

/// Make Vector from Vec4F32
NOALIAS __forceinline const Vector MakeVector( const Vec4F32 VECREF v4 )
{
    Vector result;
    result.m_V = MakeVector3( v4 );
    return result;
}

/// Make Vector from Vector3
NOALIAS __forceinline const Vector MakeVector( const Vector3 PCREF v3 )
{
    Vector result;
    result.m_V = v3;
    return result;
}

/// Make Vector from Point3
NOALIAS __forceinline const Vector MakeVector( const Point3 PCREF p3 )
{
    Vector result;
    result.m_V = MakeVector3( p3 );
    return result;
}

/// Make Vector from Normal3
NOALIAS __forceinline const Vector MakeVector( const Normal3 PCREF n3 )
{
    Vector result;
    result.m_V = MakeVector3( n3 );
    return result;
}



/// Vector Accessor: Get X
NOALIAS __forceinline const VF32 X( const Vector PCREF src )
{
    return X( src.m_V );
}

/// Vector Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Vector PCREF src )
{
    return Y( src.m_V );
}

/// Vector Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Vector PCREF src )
{
    return Z( src.m_V );
}


/// Vector Accessor: Set X
NOALIAS __forceinline void SetX( Vector* const __restrict pDst, const VF32 VECREF x )
{
    SetX( &pDst->m_V, x );
}

/// Vector Accessor: Set Y
NOALIAS __forceinline void SetY( Vector* const __restrict pDst, const VF32 VECREF y )
{
    SetY( &pDst->m_V, y );
}

/// Vector Accessor: Set Z
NOALIAS __forceinline void SetZ( Vector* const __restrict pDst, const VF32 VECREF z )
{
    SetZ( &pDst->m_V, z );
}


/// Vector Permute: Returns a permuted copy of it's Vec4F32 for internal use in implementing the product operators!
template < VectorElementIndex IX, VectorElementIndex IY, VectorElementIndex IZ >
NOALIAS __forceinline const Vector3 VPermute( const Vector PCREF src )
{
    return Maths::Permute<IX,IY,IZ>( src.m_V );
}


/// Vector Negation Operator
NOALIAS __forceinline const Vector operator - ( const Vector PCREF x )
{
    return MakeVector( -x.m_V );
}

/// Vector Addition Operator
NOALIAS __forceinline const Vector operator + ( const Vector PCREF a, const Vector PCREF b )
{
    return MakeVector( a.m_V + b.m_V );
}

/// Vector Subtraction Operator
NOALIAS __forceinline const Vector operator - ( const Vector PCREF a, const Vector PCREF b )
{
    return MakeVector( a.m_V - b.m_V );
}

/// Vector Addition Update Operator
NOALIAS __forceinline void operator += ( Vector& a, const Vector PCREF b )
{
    a.m_V += b.m_V;
}

/// Vector Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Vector& a, const Vector PCREF b )
{
    a.m_V -= b.m_V;
}


/// Vector Reversion
NOALIAS __forceinline const Vector Reverse( const Vector PCREF v )
{
    return v;
}

/// Vector Grade-Involution
NOALIAS __forceinline const Vector Involution( const Vector PCREF v )
{
    return -v;
}






/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


