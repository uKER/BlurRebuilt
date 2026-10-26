#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_CLASS_INL
#define SOFT_MATHS_HVEC_NEW_GA_CLASS_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_CLASS_INL
#define MATHS_HVEC_NEW_GA_CLASS_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace GA /// 5d Geometric Algebra
{
/// @addtogroup GA
/// @{

template < BasisBits t_Basis > struct T_Basis  { static const BasisBits m_Prev = BasisBits_Null; };
template <> struct T_Basis< BasisBits_e0 >     { static const BasisBits m_Prev = BasisBits_s; };
template <> struct T_Basis< BasisBits_e1 >     { static const BasisBits m_Prev = BasisBits_e0; };
template <> struct T_Basis< BasisBits_e2 >     { static const BasisBits m_Prev = BasisBits_e1; };
template <> struct T_Basis< BasisBits_e3 >     { static const BasisBits m_Prev = BasisBits_e2; };
template <> struct T_Basis< BasisBits_e4 >     { static const BasisBits m_Prev = BasisBits_e3; };
template <> struct T_Basis< BasisBits_e01 >    { static const BasisBits m_Prev = BasisBits_e4; };
template <> struct T_Basis< BasisBits_e02 >    { static const BasisBits m_Prev = BasisBits_e01; };
template <> struct T_Basis< BasisBits_e03 >    { static const BasisBits m_Prev = BasisBits_e02; };
template <> struct T_Basis< BasisBits_e04 >    { static const BasisBits m_Prev = BasisBits_e03; };
template <> struct T_Basis< BasisBits_e12 >    { static const BasisBits m_Prev = BasisBits_e04; };
template <> struct T_Basis< BasisBits_e13 >    { static const BasisBits m_Prev = BasisBits_e12; };
template <> struct T_Basis< BasisBits_e14 >    { static const BasisBits m_Prev = BasisBits_e13; };
template <> struct T_Basis< BasisBits_e23 >    { static const BasisBits m_Prev = BasisBits_e14; };
template <> struct T_Basis< BasisBits_e24 >    { static const BasisBits m_Prev = BasisBits_e23; };
template <> struct T_Basis< BasisBits_e34 >    { static const BasisBits m_Prev = BasisBits_e24; };
template <> struct T_Basis< BasisBits_e012 >   { static const BasisBits m_Prev = BasisBits_e34; };
template <> struct T_Basis< BasisBits_e013 >   { static const BasisBits m_Prev = BasisBits_e012; };
template <> struct T_Basis< BasisBits_e014 >   { static const BasisBits m_Prev = BasisBits_e013; };
template <> struct T_Basis< BasisBits_e023 >   { static const BasisBits m_Prev = BasisBits_e014; };
template <> struct T_Basis< BasisBits_e024 >   { static const BasisBits m_Prev = BasisBits_e023; };
template <> struct T_Basis< BasisBits_e034 >   { static const BasisBits m_Prev = BasisBits_e024; };
template <> struct T_Basis< BasisBits_e123 >   { static const BasisBits m_Prev = BasisBits_e034; };
template <> struct T_Basis< BasisBits_e124 >   { static const BasisBits m_Prev = BasisBits_e123; };
template <> struct T_Basis< BasisBits_e134 >   { static const BasisBits m_Prev = BasisBits_e124; };
template <> struct T_Basis< BasisBits_e234 >   { static const BasisBits m_Prev = BasisBits_e134; };
template <> struct T_Basis< BasisBits_e0123 >  { static const BasisBits m_Prev = BasisBits_e234; };
template <> struct T_Basis< BasisBits_e0124 >  { static const BasisBits m_Prev = BasisBits_e0123; };
template <> struct T_Basis< BasisBits_e0134 >  { static const BasisBits m_Prev = BasisBits_e0124; };
template <> struct T_Basis< BasisBits_e0234 >  { static const BasisBits m_Prev = BasisBits_e0134; };
template <> struct T_Basis< BasisBits_e1234 >  { static const BasisBits m_Prev = BasisBits_e0234; };
template <> struct T_Basis< BasisBits_e01234 > { static const BasisBits m_Prev = BasisBits_e1234; };


template < unsigned int t_ElementBitfield, BasisBits t_Basis >
struct T_SubBasisIndex
{
    static const BasisBits    m_PrevBasis = T_Basis< t_Basis >::m_Prev;
    static const unsigned int m_Index     = T_SubBasisIndex< t_ElementBitfield, m_PrevBasis >::m_Index +
                                            ( ( t_ElementBitfield & (U32(1)<<m_PrevBasis) ) ? 1 : 0 );
};

template < unsigned int t_ElementBitfield >
struct T_SubBasisIndex< t_ElementBitfield, BasisBits_Null >
{
    static const unsigned int m_Index = -1;
};

template < unsigned int t_ElementBitfield >
struct T_SubBasisIndex< t_ElementBitfield, BasisBits_s >
{
    static const unsigned int m_Index = 0;
};

template < unsigned int t_ElementBitfield, BasisBits t_Basis >
struct T_BasisIndex
{
    static const unsigned int m_Index = ( t_ElementBitfield & (U32(1)<<t_Basis) ) ? T_SubBasisIndex< t_ElementBitfield, t_Basis >::m_Index : -1;
};

/// Default constructor - undefined
template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector()
{
}


/// Copy constructor
//template < ElementBits  t_ElementBits,
//           unsigned int t_Dimensions,
//           typename     t_Metric >
//NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const TypeVector& copy )
//{
//    //for ( U32 i=0; i<m_Vectors; i++ )
//    //{
//    //    m_V[i] = copy.m_V[i];
//    //}
//    ForEachVec4<0>( &TypeVector::CopyVec4, this, copy );
//}

/// Copy assignment
//template < ElementBits  t_ElementBits,
//           unsigned int t_Dimensions,
//           typename     t_Metric >
//NOALIAS __forceinline const T_Vector< t_ElementBits, t_Dimensions, t_Metric >&
//                            T_Vector< t_ElementBits, t_Dimensions, t_Metric >::operator = ( const TypeVector& copy )
//{
//    //for ( U32 i=0; i<m_Vectors; i++ )
//    //{
//    //    m_V[i] = copy.m_V[i];
//    //}
//    ForEachVec4<0>( &TypeVector::CopyVec4, this, copy );
//    return copy;
//}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
template < ElementBits  t_CopyElementBits >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const T_Vector< t_CopyElementBits, t_Dimensions, t_Metric >& copy )
{
    if ( t_ElementBits == t_CopyElementBits )
    {
        #if defined(PLATFORM_PC)
        ForEachVec4<0>( &TypeVector::CopyVec4, this, copy );
        #else
        for ( S32 i=0; i<TypeVector::m_Vectors; i++ )
            m_V[i] = copy.m_V[i];
        #endif
    }
    else
    {
        SetElement< BasisBits_s >( this, Element< BasisBits_s >( copy ) );
        
        SetElement< BasisBits_e0 >( this, Element< BasisBits_e0 >( copy ) );
        SetElement< BasisBits_e1 >( this, Element< BasisBits_e1 >( copy ) );
        SetElement< BasisBits_e2 >( this, Element< BasisBits_e2 >( copy ) );
        SetElement< BasisBits_e3 >( this, Element< BasisBits_e3 >( copy ) );
        SetElement< BasisBits_e4 >( this, Element< BasisBits_e4 >( copy ) );
        
        SetElement< BasisBits_e01 >( this, Element< BasisBits_e01 >( copy ) );
        SetElement< BasisBits_e02 >( this, Element< BasisBits_e02 >( copy ) );
        SetElement< BasisBits_e03 >( this, Element< BasisBits_e03 >( copy ) );
        SetElement< BasisBits_e04 >( this, Element< BasisBits_e04 >( copy ) );
        SetElement< BasisBits_e12 >( this, Element< BasisBits_e12 >( copy ) );
        SetElement< BasisBits_e13 >( this, Element< BasisBits_e13 >( copy ) );
        SetElement< BasisBits_e14 >( this, Element< BasisBits_e14 >( copy ) );
        SetElement< BasisBits_e23 >( this, Element< BasisBits_e23 >( copy ) );
        SetElement< BasisBits_e24 >( this, Element< BasisBits_e24 >( copy ) );
        SetElement< BasisBits_e34 >( this, Element< BasisBits_e34 >( copy ) );
        
        SetElement< BasisBits_e012 >( this, Element< BasisBits_e012 >( copy ) );
        SetElement< BasisBits_e013 >( this, Element< BasisBits_e013 >( copy ) );
        SetElement< BasisBits_e014 >( this, Element< BasisBits_e014 >( copy ) );
        SetElement< BasisBits_e023 >( this, Element< BasisBits_e023 >( copy ) );
        SetElement< BasisBits_e024 >( this, Element< BasisBits_e024 >( copy ) );
        SetElement< BasisBits_e034 >( this, Element< BasisBits_e034 >( copy ) );
        SetElement< BasisBits_e123 >( this, Element< BasisBits_e123 >( copy ) );
        SetElement< BasisBits_e124 >( this, Element< BasisBits_e124 >( copy ) );
        SetElement< BasisBits_e134 >( this, Element< BasisBits_e134 >( copy ) );
        SetElement< BasisBits_e234 >( this, Element< BasisBits_e234 >( copy ) );
        
        SetElement< BasisBits_e0123 >( this, Element< BasisBits_e0123 >( copy ) );
        SetElement< BasisBits_e0124 >( this, Element< BasisBits_e0124 >( copy ) );
        SetElement< BasisBits_e0134 >( this, Element< BasisBits_e0134 >( copy ) );
        SetElement< BasisBits_e0234 >( this, Element< BasisBits_e0234 >( copy ) );
        SetElement< BasisBits_e1234 >( this, Element< BasisBits_e1234 >( copy ) );
        
        SetElement< BasisBits_e01234 >( this, Element< BasisBits_e01234 >( copy ) );
    }
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
template < ElementBits  t_CopyElementBits >
NOALIAS __forceinline const T_Vector< t_ElementBits, t_Dimensions, t_Metric >&
                            T_Vector< t_ElementBits, t_Dimensions, t_Metric >::operator = ( const T_Vector< t_CopyElementBits, t_Dimensions, t_Metric >& copy )
{
    if ( t_ElementBits == t_CopyElementBits )
    {
        #if defined(PLATFORM_PC)
        ForEachVec4<0>( &TypeVector::CopyVec4, this, copy );
        #else
        for ( S32 i=0; i<TypeVector::m_Vectors; i++ )
            m_V[i] = copy.m_V[i];
        #endif
    }
    else
    {
        SetElement< BasisBits_s >( this, Element< BasisBits_s >( copy ) );
        
        SetElement< BasisBits_e0 >( this, Element< BasisBits_e0 >( copy ) );
        SetElement< BasisBits_e1 >( this, Element< BasisBits_e1 >( copy ) );
        SetElement< BasisBits_e2 >( this, Element< BasisBits_e2 >( copy ) );
        SetElement< BasisBits_e3 >( this, Element< BasisBits_e3 >( copy ) );
        SetElement< BasisBits_e4 >( this, Element< BasisBits_e4 >( copy ) );
        
        SetElement< BasisBits_e01 >( this, Element< BasisBits_e01 >( copy ) );
        SetElement< BasisBits_e02 >( this, Element< BasisBits_e02 >( copy ) );
        SetElement< BasisBits_e03 >( this, Element< BasisBits_e03 >( copy ) );
        SetElement< BasisBits_e04 >( this, Element< BasisBits_e04 >( copy ) );
        SetElement< BasisBits_e12 >( this, Element< BasisBits_e12 >( copy ) );
        SetElement< BasisBits_e13 >( this, Element< BasisBits_e13 >( copy ) );
        SetElement< BasisBits_e14 >( this, Element< BasisBits_e14 >( copy ) );
        SetElement< BasisBits_e23 >( this, Element< BasisBits_e23 >( copy ) );
        SetElement< BasisBits_e24 >( this, Element< BasisBits_e24 >( copy ) );
        SetElement< BasisBits_e34 >( this, Element< BasisBits_e34 >( copy ) );
        
        SetElement< BasisBits_e012 >( this, Element< BasisBits_e012 >( copy ) );
        SetElement< BasisBits_e013 >( this, Element< BasisBits_e013 >( copy ) );
        SetElement< BasisBits_e014 >( this, Element< BasisBits_e014 >( copy ) );
        SetElement< BasisBits_e023 >( this, Element< BasisBits_e023 >( copy ) );
        SetElement< BasisBits_e024 >( this, Element< BasisBits_e024 >( copy ) );
        SetElement< BasisBits_e034 >( this, Element< BasisBits_e034 >( copy ) );
        SetElement< BasisBits_e123 >( this, Element< BasisBits_e123 >( copy ) );
        SetElement< BasisBits_e124 >( this, Element< BasisBits_e124 >( copy ) );
        SetElement< BasisBits_e134 >( this, Element< BasisBits_e134 >( copy ) );
        SetElement< BasisBits_e234 >( this, Element< BasisBits_e234 >( copy ) );
        
        SetElement< BasisBits_e0123 >( this, Element< BasisBits_e0123 >( copy ) );
        SetElement< BasisBits_e0124 >( this, Element< BasisBits_e0124 >( copy ) );
        SetElement< BasisBits_e0134 >( this, Element< BasisBits_e0134 >( copy ) );
        SetElement< BasisBits_e0234 >( this, Element< BasisBits_e0234 >( copy ) );
        SetElement< BasisBits_e1234 >( this, Element< BasisBits_e1234 >( copy ) );
        
        SetElement< BasisBits_e01234 >( this, Element< BasisBits_e01234 >( copy ) );
    }
    return *this;
}


template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const Vec4F32 VECREF v0 )
{
    BIZ_STATIC_ASSERT( ( m_Elements >= 1 ) && ( m_Elements <= 4 ) );
    m_V[0] = v0;
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1 )
{
    BIZ_STATIC_ASSERT( ( m_Elements >= 5 ) && ( m_Elements <= 8 ) );
    m_V[0] = v0;
    m_V[1] = v1;
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2 )
{
    BIZ_STATIC_ASSERT( ( m_Elements >= 9 ) && ( m_Elements <= 12 ) );
    m_V[0] = v0;
    m_V[1] = v1;
    m_V[2] = v2;
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3 )
{
    BIZ_STATIC_ASSERT( ( m_Elements >= 13 ) && ( m_Elements <= 16 ) );
    m_V[0] = v0;
    m_V[1] = v1;
    m_V[2] = v2;
    m_V[3] = v3;
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3,
                                                                                   const Vec4F32 VECREF v4 )
{
    BIZ_STATIC_ASSERT( ( m_Elements >= 17 ) && ( m_Elements <= 20 ) );
    m_V[0] = v0;
    m_V[1] = v1;
    m_V[2] = v2;
    m_V[3] = v3;
    m_V[4] = v4;
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3,
                                                                                   const Vec4F32 VECREF v4, const Vec4F32 VECREF v5 )
{
    BIZ_STATIC_ASSERT( ( m_Elements >= 21 ) && ( m_Elements <= 24 ) );
    m_V[0] = v0;
    m_V[1] = v1;
    m_V[2] = v2;
    m_V[3] = v3;
    m_V[4] = v4;
    m_V[5] = v5;
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3,
                                                                                   const Vec4F32 VECREF v4, const Vec4F32 VECREF v5, const Vec4F32 VECREF v6 )
{
    BIZ_STATIC_ASSERT( ( m_Elements >= 25 ) && ( m_Elements <= 28 ) );
    m_V[0] = v0;
    m_V[1] = v1;
    m_V[2] = v2;
    m_V[3] = v3;
    m_V[4] = v4;
    m_V[5] = v5;
    m_V[6] = v6;
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3,
                                                                                   const Vec4F32 VECREF v4, const Vec4F32 VECREF v5, const Vec4F32 VECREF v6, const Vec4F32 VECREF v7 )
{
    BIZ_STATIC_ASSERT( ( m_Elements >= 29 ) && ( m_Elements <= 32 ) );
    m_V[0] = v0;
    m_V[1] = v1;
    m_V[2] = v2;
    m_V[3] = v3;
    m_V[4] = v4;
    m_V[5] = v5;
    m_V[6] = v6;
    m_V[7] = v7;
}



template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const VF32 VECREF v0 )
{
    BIZ_STATIC_ASSERT( m_Elements == 1 );
    m_V[0] = MakeVec4F32( v0 );
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const VF32 VECREF v0, const VF32 VECREF v1 )
{
    BIZ_STATIC_ASSERT( m_Elements == 2 );
    m_V[0] = MakeVec4F32( v0, v1 );
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2 )
{
    BIZ_STATIC_ASSERT( m_Elements == 3 );
    m_V[0] = MakeVec4F32( v0, v1, v2 );
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3 )
{
    BIZ_STATIC_ASSERT( m_Elements == 4 );
    m_V[0] = MakeVec4F32( v0, v1, v2, v3 );
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector
    ( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3,
      const VF32 VECREF v4 )
{
    BIZ_STATIC_ASSERT( m_Elements == 5 );
    m_V[0] = MakeVec4F32( v0, v1, v2, v3 );
    m_V[1] = MakeVec4F32( v4 );
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector
    ( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3,
      const VF32 VECREF v4, const VF32 VECREF v5 )
{
    BIZ_STATIC_ASSERT( m_Elements == 6 );
    m_V[0] = MakeVec4F32( v0, v1, v2, v3 );
    m_V[1] = MakeVec4F32( v4, v5 );
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector
    ( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3,
      const VF32 VECREF v4, const VF32 VECREF v5, const VF32 VECREF v6 )
{
    BIZ_STATIC_ASSERT( m_Elements == 7 );
    m_V[0] = MakeVec4F32( v0, v1, v2, v3 );
    m_V[1] = MakeVec4F32( v4, v5, v6 );
}

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS __forceinline T_Vector< t_ElementBits, t_Dimensions, t_Metric >::T_Vector
    ( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3,
      const VF32 VECREF v4, const VF32 VECREF v5, const VF32 VECREF v6, const VF32 VECREF v7 )
{
    BIZ_STATIC_ASSERT( m_Elements == 8 );
    m_V[0] = MakeVec4F32( v0, v1, v2, v3 );
    m_V[1] = MakeVec4F32( v4, v5, v6, v7 );
}


/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


