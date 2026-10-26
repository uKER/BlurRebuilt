#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_BIVECTOR_INL
#define SOFT_MATHS_HVEC_NEW_G3V_BIVECTOR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_BIVECTOR_INL
#define MATHS_HVEC_NEW_G3V_BIVECTOR_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup G3V_BiVector
/// @{

NOALIAS __forceinline const G3V::BiVector Const< G3V::BiVector >::Zero() ///< Constant BiVector(0,0,0)
{
    return G3V::MakeBiVector( Maths::Const<Vec4F32>::Zero() );
}

NOALIAS __forceinline const G3V::BiVector Const< G3V::BiVector >::One() ///< Constant BiVector(1,1,1)
{
    return G3V::MakeBiVector( Maths::Const<Vec4F32>::One() );
}

NOALIAS __forceinline const G3V::BiVector Const< G3V::BiVector >::UnitX() ///< Constant BiVector(1,0,0)
{
    return G3V::MakeBiVector( Maths::Const<Vec4F32>::UnitX() );
}

NOALIAS __forceinline const G3V::BiVector Const< G3V::BiVector >::UnitY() ///< Constant BiVector(0,1,0)
{
    return G3V::MakeBiVector( Maths::Const<Vec4F32>::UnitY() );
}

NOALIAS __forceinline const G3V::BiVector Const< G3V::BiVector >::UnitZ() ///< Constant BiVector(0,0,1)
{
    return G3V::MakeBiVector( Maths::Const<Vec4F32>::UnitZ() );
}

/// @}

namespace G3V
{
/// @addtogroup G3V_BiVector
/// @{

/// Make BiVector from 3 F32's
NOALIAS __forceinline const BiVector MakeBiVector( const F32& x, const F32& y, const F32& z )
{
    BiVector result;
    result.m_B = MakeVector3( x, y, z );
    return result;
}

/// Make BiVector from 3 VF32's
NOALIAS __forceinline const BiVector MakeBiVector( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    BiVector result;
    result.m_B = MakeVector3( x, y, z );
    return result;
}

/// Make BiVector from Vec4F32
NOALIAS __forceinline const BiVector MakeBiVector( const Vec4F32 VECREF v4 )
{
    BiVector result;
    result.m_B = MakeVector3( v4 );
    return result;
}

/// Make BiVector from Vector3
NOALIAS __forceinline const BiVector MakeBiVector( const Vector3 PCREF v3 )
{
    BiVector result;
    result.m_B = v3;
    return result;
}


/// BiVector Accessor: Get X
NOALIAS __forceinline const VF32 X( const BiVector PCREF src )
{
    return X( src.m_B );
}

/// BiVector Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const BiVector PCREF src )
{
    return Y( src.m_B );
}

/// BiVector Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const BiVector PCREF src )
{
    return Z( src.m_B );
}


/// BiVector Accessor: Set X
NOALIAS __forceinline void SetX( BiVector* const __restrict pDst, const VF32 VECREF x )
{
    SetX( &pDst->m_B, x );
}

/// BiVector Accessor: Set Y
NOALIAS __forceinline void SetY( BiVector* const __restrict pDst, const VF32 VECREF y )
{
    SetY( &pDst->m_B, y );
}

/// BiVector Accessor: Set Z
NOALIAS __forceinline void SetZ( BiVector* const __restrict pDst, const VF32 VECREF z )
{
    SetZ( &pDst->m_B, z );
}


/// BiVector Permute: Returns a permuted copy of it's Vec4F32 for internal use in implementing the product operators!
template < BiVectorElementIndex IX, BiVectorElementIndex IY, BiVectorElementIndex IZ >
NOALIAS __forceinline const Vector3 BPermute( const BiVector PCREF src )
{
    return Maths::Permute<IX,IY,IZ>( src.m_B );
}


/// BiVector Negation Operator
NOALIAS __forceinline const BiVector operator - ( const BiVector PCREF x )
{
    return MakeBiVector( -x.m_B );
}

/// BiVector Addition Operator
NOALIAS __forceinline const BiVector operator + ( const BiVector PCREF a, const BiVector PCREF b )
{
    return MakeBiVector( a.m_B + b.m_B );
}

/// BiVector Subtraction Operator
NOALIAS __forceinline const BiVector operator - ( const BiVector PCREF a, const BiVector PCREF b )
{
    return MakeBiVector( a.m_B - b.m_B );
}

/// BiVector Addition Update Operator
NOALIAS __forceinline void operator += ( BiVector& a, const BiVector PCREF b )
{
    a.m_B += b.m_B;
}

/// BiVector Subtraction Update Operator
NOALIAS __forceinline void operator -= ( BiVector& a, const BiVector PCREF b )
{
    a.m_B -= b.m_B;
}


/// BiVector Reversion
NOALIAS __forceinline const BiVector Reverse( const BiVector PCREF bv )
{
    return -bv;
}

/// BiVector Grade-Involution
NOALIAS __forceinline const BiVector Involution( const BiVector PCREF bv )
{
    return bv;
}









/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


