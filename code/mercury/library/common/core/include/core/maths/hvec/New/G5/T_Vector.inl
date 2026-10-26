#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_T_VECTOR_INL
#define SOFT_MATHS_HVEC_NEW_GA_T_VECTOR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_T_VECTOR_INL
#define MATHS_HVEC_NEW_GA_T_VECTOR_INL
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

/// T_Vector Debug Display
template < ElementBits t_ElementBits, unsigned int t_Dimensions, typename t_Metric >
NOALIAS void Display( const T_Vector< t_ElementBits, t_Dimensions, t_Metric >& v )
{
    typedef T_Vector< t_ElementBits, t_Dimensions, t_Metric > TypeVector;
    
    printf( "Elements = %2d, bitfield = 0x%08X\n", TypeVector::m_Elements, TypeVector::m_ElementBits );
    
    const BasisBits bb_0 = T_GetBasis< t_ElementBits,  0 >::m_BasisBits;
    const BasisBits bb_1 = T_GetBasis< t_ElementBits,  1 >::m_BasisBits;
    const BasisBits bb_2 = T_GetBasis< t_ElementBits,  2 >::m_BasisBits;
    const BasisBits bb_3 = T_GetBasis< t_ElementBits,  3 >::m_BasisBits;
    const BasisBits bb_4 = T_GetBasis< t_ElementBits,  4 >::m_BasisBits;
    const BasisBits bb_5 = T_GetBasis< t_ElementBits,  5 >::m_BasisBits;
    const BasisBits bb_6 = T_GetBasis< t_ElementBits,  6 >::m_BasisBits;
    const BasisBits bb_7 = T_GetBasis< t_ElementBits,  7 >::m_BasisBits;
    const BasisBits bb_8 = T_GetBasis< t_ElementBits,  8 >::m_BasisBits;
    const BasisBits bb_9 = T_GetBasis< t_ElementBits,  9 >::m_BasisBits;
    const BasisBits bb10 = T_GetBasis< t_ElementBits, 10 >::m_BasisBits;
    const BasisBits bb11 = T_GetBasis< t_ElementBits, 11 >::m_BasisBits;
    const BasisBits bb12 = T_GetBasis< t_ElementBits, 12 >::m_BasisBits;
    const BasisBits bb13 = T_GetBasis< t_ElementBits, 13 >::m_BasisBits;
    const BasisBits bb14 = T_GetBasis< t_ElementBits, 14 >::m_BasisBits;
    const BasisBits bb15 = T_GetBasis< t_ElementBits, 15 >::m_BasisBits;
    const BasisBits bb16 = T_GetBasis< t_ElementBits, 16 >::m_BasisBits;
    const BasisBits bb17 = T_GetBasis< t_ElementBits, 17 >::m_BasisBits;
    const BasisBits bb18 = T_GetBasis< t_ElementBits, 18 >::m_BasisBits;
    const BasisBits bb19 = T_GetBasis< t_ElementBits, 19 >::m_BasisBits;
    const BasisBits bb20 = T_GetBasis< t_ElementBits, 20 >::m_BasisBits;
    const BasisBits bb21 = T_GetBasis< t_ElementBits, 21 >::m_BasisBits;
    const BasisBits bb22 = T_GetBasis< t_ElementBits, 22 >::m_BasisBits;
    const BasisBits bb23 = T_GetBasis< t_ElementBits, 23 >::m_BasisBits;
    const BasisBits bb24 = T_GetBasis< t_ElementBits, 24 >::m_BasisBits;
    const BasisBits bb25 = T_GetBasis< t_ElementBits, 25 >::m_BasisBits;
    const BasisBits bb26 = T_GetBasis< t_ElementBits, 26 >::m_BasisBits;
    const BasisBits bb27 = T_GetBasis< t_ElementBits, 27 >::m_BasisBits;
    const BasisBits bb28 = T_GetBasis< t_ElementBits, 28 >::m_BasisBits;
    const BasisBits bb29 = T_GetBasis< t_ElementBits, 29 >::m_BasisBits;
    const BasisBits bb30 = T_GetBasis< t_ElementBits, 30 >::m_BasisBits;
    const BasisBits bb31 = T_GetBasis< t_ElementBits, 31 >::m_BasisBits;
    
    printf( "Basis blade bits  0 = %s\n", GetString( bb_0 ) ); if ( bb_1 == BasisBits_Null ) return;
    printf( "Basis blade bits  1 = %s\n", GetString( bb_1 ) ); if ( bb_2 == BasisBits_Null ) return;
    printf( "Basis blade bits  2 = %s\n", GetString( bb_2 ) ); if ( bb_3 == BasisBits_Null ) return;
    printf( "Basis blade bits  3 = %s\n", GetString( bb_3 ) ); if ( bb_4 == BasisBits_Null ) return;
    printf( "Basis blade bits  4 = %s\n", GetString( bb_4 ) ); if ( bb_5 == BasisBits_Null ) return;
    printf( "Basis blade bits  5 = %s\n", GetString( bb_5 ) ); if ( bb_6 == BasisBits_Null ) return;
    printf( "Basis blade bits  6 = %s\n", GetString( bb_6 ) ); if ( bb_7 == BasisBits_Null ) return;
    printf( "Basis blade bits  7 = %s\n", GetString( bb_7 ) ); if ( bb_8 == BasisBits_Null ) return;
    printf( "Basis blade bits  8 = %s\n", GetString( bb_8 ) ); if ( bb_9 == BasisBits_Null ) return;
    printf( "Basis blade bits  9 = %s\n", GetString( bb_9 ) ); if ( bb10 == BasisBits_Null ) return;
    printf( "Basis blade bits 10 = %s\n", GetString( bb10 ) ); if ( bb11 == BasisBits_Null ) return;
    printf( "Basis blade bits 11 = %s\n", GetString( bb11 ) ); if ( bb12 == BasisBits_Null ) return;
    printf( "Basis blade bits 12 = %s\n", GetString( bb12 ) ); if ( bb13 == BasisBits_Null ) return;
    printf( "Basis blade bits 13 = %s\n", GetString( bb13 ) ); if ( bb14 == BasisBits_Null ) return;
    printf( "Basis blade bits 14 = %s\n", GetString( bb14 ) ); if ( bb15 == BasisBits_Null ) return;
    printf( "Basis blade bits 15 = %s\n", GetString( bb15 ) ); if ( bb16 == BasisBits_Null ) return;
    printf( "Basis blade bits 16 = %s\n", GetString( bb16 ) ); if ( bb17 == BasisBits_Null ) return;
    printf( "Basis blade bits 17 = %s\n", GetString( bb17 ) ); if ( bb18 == BasisBits_Null ) return;
    printf( "Basis blade bits 18 = %s\n", GetString( bb18 ) ); if ( bb19 == BasisBits_Null ) return;
    printf( "Basis blade bits 19 = %s\n", GetString( bb19 ) ); if ( bb20 == BasisBits_Null ) return;
    printf( "Basis blade bits 20 = %s\n", GetString( bb20 ) ); if ( bb21 == BasisBits_Null ) return;
    printf( "Basis blade bits 21 = %s\n", GetString( bb21 ) ); if ( bb22 == BasisBits_Null ) return;
    printf( "Basis blade bits 22 = %s\n", GetString( bb22 ) ); if ( bb23 == BasisBits_Null ) return;
    printf( "Basis blade bits 23 = %s\n", GetString( bb23 ) ); if ( bb24 == BasisBits_Null ) return;
    printf( "Basis blade bits 24 = %s\n", GetString( bb24 ) ); if ( bb25 == BasisBits_Null ) return;
    printf( "Basis blade bits 25 = %s\n", GetString( bb25 ) ); if ( bb26 == BasisBits_Null ) return;
    printf( "Basis blade bits 26 = %s\n", GetString( bb26 ) ); if ( bb27 == BasisBits_Null ) return;
    printf( "Basis blade bits 27 = %s\n", GetString( bb27 ) ); if ( bb28 == BasisBits_Null ) return;
    printf( "Basis blade bits 28 = %s\n", GetString( bb28 ) ); if ( bb29 == BasisBits_Null ) return;
    printf( "Basis blade bits 29 = %s\n", GetString( bb29 ) ); if ( bb30 == BasisBits_Null ) return;
    printf( "Basis blade bits 30 = %s\n", GetString( bb30 ) ); if ( bb31 == BasisBits_Null ) return;
    printf( "Basis blade bits 31 = %s\n", GetString( bb31 ) );
}


template < ElementBits t_ElementBits, unsigned int t_Dimensions, typename t_Metric >
NOALIAS void Print( const T_Vector< t_ElementBits, t_Dimensions, t_Metric >& v )
{
    printf( "Vector( " );
    for ( U32 i=0; i<v.m_Elements; i++ )
    {
        if ( i != 0 )
        {
            printf( ", " );
        }
        
        switch ( i )
        {
            case  0 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  0 >::m_BasisBits ) ); break;
            case  1 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  1 >::m_BasisBits ) ); break;
            case  2 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  2 >::m_BasisBits ) ); break;
            case  3 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  3 >::m_BasisBits ) ); break;
            case  4 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  4 >::m_BasisBits ) ); break;
            case  5 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  5 >::m_BasisBits ) ); break;
            case  6 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  6 >::m_BasisBits ) ); break;
            case  7 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  7 >::m_BasisBits ) ); break;
            case  8 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  8 >::m_BasisBits ) ); break;
            case  9 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits,  9 >::m_BasisBits ) ); break;
            case 10 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 10 >::m_BasisBits ) ); break;
            case 11 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 11 >::m_BasisBits ) ); break;
            case 12 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 12 >::m_BasisBits ) ); break;
            case 13 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 13 >::m_BasisBits ) ); break;
            case 14 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 14 >::m_BasisBits ) ); break;
            case 15 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 15 >::m_BasisBits ) ); break;
            case 16 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 16 >::m_BasisBits ) ); break;
            case 17 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 17 >::m_BasisBits ) ); break;
            case 18 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 18 >::m_BasisBits ) ); break;
            case 19 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 19 >::m_BasisBits ) ); break;
            case 20 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 20 >::m_BasisBits ) ); break;
            case 21 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 21 >::m_BasisBits ) ); break;
            case 22 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 22 >::m_BasisBits ) ); break;
            case 23 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 23 >::m_BasisBits ) ); break;
            case 24 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 24 >::m_BasisBits ) ); break;
            case 25 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 25 >::m_BasisBits ) ); break;
            case 26 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 26 >::m_BasisBits ) ); break;
            case 27 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 27 >::m_BasisBits ) ); break;
            case 28 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 28 >::m_BasisBits ) ); break;
            case 29 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 29 >::m_BasisBits ) ); break;
            case 30 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 30 >::m_BasisBits ) ); break;
            case 31 : printf( "%s:", GetShortString( T_GetBasis< t_ElementBits, 31 >::m_BasisBits ) ); break;
        }
        
        switch ( i % 4 )
        {
            case 0 : printf( "%+6.3f", GetF32( Maths::Element<0>( v.m_V[i/4] ) ) ); break;
            case 1 : printf( "%+6.3f", GetF32( Maths::Element<1>( v.m_V[i/4] ) ) ); break;
            case 2 : printf( "%+6.3f", GetF32( Maths::Element<2>( v.m_V[i/4] ) ) ); break;
            case 3 : printf( "%+6.3f", GetF32( Maths::Element<3>( v.m_V[i/4] ) ) ); break;
            default : break;
        }
    }
    printf( " )\n" );
}

/// T_Vector Get Element
template < BasisBits t_Basis, ElementBits t_ElementBitfield, unsigned int t_Dimensions, typename t_Metric >
NOALIAS __forceinline const VF32 Element( const T_Vector< t_ElementBitfield, t_Dimensions, t_Metric >& v )
{
    if ( ( t_Basis != BasisBits_Null ) && ( t_ElementBitfield & (1<<t_Basis) ) )
    {
        static const U32 I = T_BasisIndex< t_ElementBitfield, t_Basis >::m_Index;
        //return Maths::Element<I%4>( v.m_V[I/4] );
        return Maths::Element<I&0x3>( v.m_V[I>>2] ); // NOTE: Win32 Debug: "warning C4307: '*' : integral constant overflow". Why???
    }
    else
    {
        return Const<VF32>::Zero();
    }
}

/// T_Vector Set Element
template < BasisBits t_Basis, ElementBits t_ElementBitfield, unsigned int t_Dimensions, typename t_Metric >
NOALIAS __forceinline void SetElement( T_Vector< t_ElementBitfield, t_Dimensions, t_Metric >* const __restrict pV, const VF32 VECREF e )
{
    if ( ( t_Basis != BasisBits_Null ) && ( t_ElementBitfield & (1<<t_Basis) ) )
    {
        static const U32 I = T_BasisIndex< t_ElementBitfield, t_Basis >::m_Index;
        //Maths::SetElement<I%4>( &pV->m_V[I/4], e );
        Maths::SetElement<I&0x3>( &pV->m_V[I>>2], e ); // NOTE: Win32 Debug: "warning C4307: '*' : integral constant overflow". Why???
    }
}




template < ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS,
           unsigned int t_Dimensions,
           typename     t_Metric >
struct T_AddSubResult
{
    typedef T_Vector< (ElementBits)( t_ElementLHS | t_ElementRHS ), t_Dimensions, t_Metric > TypeResult;
};

template < BasisBits    t_Basis,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS,
           ElementBits  t_ElementResult,
           unsigned int t_Dimensions,
           typename     t_Metric >
struct T_ElementAdd
{
    NOALIAS __forceinline static const U32 Process(
        const T_Vector< t_ElementLHS,    t_Dimensions, t_Metric >& lhs,
        const T_Vector< t_ElementRHS,    t_Dimensions, t_Metric >& rhs,
              T_Vector< t_ElementResult, t_Dimensions, t_Metric >* pResult,
        const bool                                                 generateCode,
        const U32                                                  index )
    {
        if ( t_ElementResult & (1<<t_Basis) )
        {
            if ( !( t_ElementRHS & (1<<t_Basis) ) ) // LHS only
            {
                SetElement< t_Basis >( pResult, Element< t_Basis >( lhs ) );
                if ( generateCode )
                {
                    const char* basisString = GA::GetString( t_Basis );
                    printf( "    SetElement<%s>( &result, Element<%s>(lhs) );\n", basisString, basisString );
                }
            }
            else if ( !( t_ElementLHS & (1<<t_Basis) ) ) // RHS only
            {
                SetElement< t_Basis >( pResult, Element< t_Basis >( rhs ) );
                if ( generateCode )
                {
                    const char* basisString = GA::GetString( t_Basis );
                    printf( "    SetElement<%s>( &result, Element<%s>(rhs) );\n", basisString, basisString );
                }
            }
            else // LHS + RHS
            {
                SetElement< t_Basis >( pResult, Element< t_Basis >( lhs ) + Element< t_Basis >( rhs ) );
                if ( generateCode )
                {
                    const char* basisString = GA::GetString( t_Basis );
                    printf( "    SetElement<%s>( &result, Element<%s>(lhs) + Element<%s>(rhs) );\n", basisString, basisString, basisString );
                }
            }
            return 1;
        }
        return 0;
    }
};

template < BasisBits    t_Basis,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS,
           ElementBits  t_ElementResult,
           unsigned int t_Dimensions,
           typename     t_Metric >
struct T_ElementSub
{
    NOALIAS __forceinline static const U32 Process(
        const T_Vector< t_ElementLHS,    t_Dimensions, t_Metric >& lhs,
        const T_Vector< t_ElementRHS,    t_Dimensions, t_Metric >& rhs,
              T_Vector< t_ElementResult, t_Dimensions, t_Metric >* pResult,
        const bool                                                 generateCode,
        const U32                                                  index )
    {
        if ( t_ElementResult & (1<<t_Basis) )
        {
            if ( !( t_ElementRHS & (1<<t_Basis) ) ) // LHS only
            {
                SetElement< t_Basis >( pResult, Element< t_Basis >( lhs ) );
                if ( generateCode )
                {
                    const char* basisString = GA::GetString( t_Basis );
                    printf( "    SetElement<%s>( &result, Element<%s>(lhs) );\n", basisString, basisString );
                }
            }
            else if ( !( t_ElementLHS & (1<<t_Basis) ) ) // RHS only
            {
                SetElement< t_Basis >( pResult, -Element< t_Basis >( rhs ) );
                if ( generateCode )
                {
                    const char* basisString = GA::GetString( t_Basis );
                    printf( "    SetElement<%s>( &result, -Element<%s>(rhs) );\n", basisString, basisString );
                }
            }
            else // LHS - RHS
            {
                SetElement< t_Basis >( pResult, Element< t_Basis >( lhs ) - Element< t_Basis >( rhs ) );
                if ( generateCode )
                {
                    const char* basisString = GA::GetString( t_Basis );
                    printf( "    SetElement<%s>( &result, Element<%s>(lhs) - Element<%s>(rhs) );\n", basisString, basisString, basisString );
                }
            }
            return 1;
        }
        return 0;
    }
};

//template < ElementBits  t_ElementLHS,
//           ElementBits  t_ElementRHS,
//           ElementBits  t_ElementResult,
//           unsigned int t_Dimensions,
//           typename     t_Metric >
//NOALIAS __forceinline static void T_VectorAdd(
//    const T_Vector< t_ElementLHS,    t_Dimensions, t_Metric >& lhs,
//    const T_Vector< t_ElementRHS,    t_Dimensions, t_Metric >& rhs,
//          T_Vector< t_ElementResult, t_Dimensions, t_Metric >* pResult,
//    const bool                                                 generateCode )
//{
//    U32 elements = 0;
//    
//    elements += T_ElementAdd< BasisBits_s, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementAdd< BasisBits_e0, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e1, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e2, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e3, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e4, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementAdd< BasisBits_e01, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e02, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e03, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e04, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e12, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e13, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e14, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e23, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e24, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e34, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementAdd< BasisBits_e012, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e013, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e014, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e023, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e024, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e034, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e123, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e124, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e134, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e234, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementAdd< BasisBits_e0123, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e0124, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e0134, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e0234, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementAdd< BasisBits_e1234, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementAdd< BasisBits_e01234, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//}

//template < ElementBits  t_ElementLHS,
//           ElementBits  t_ElementRHS,
//           ElementBits  t_ElementResult,
//           unsigned int t_Dimensions,
//           typename     t_Metric >
//NOALIAS __forceinline static void T_VectorSub(
//    const T_Vector< t_ElementLHS,    t_Dimensions, t_Metric >& lhs,
//    const T_Vector< t_ElementRHS,    t_Dimensions, t_Metric >& rhs,
//          T_Vector< t_ElementResult, t_Dimensions, t_Metric >* pResult,
//    const bool                                                 generateCode )
//{
//    U32 elements = 0;
//    
//    elements += T_ElementSub< BasisBits_s, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementSub< BasisBits_e0, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e1, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e2, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e3, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e4, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementSub< BasisBits_e01, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e02, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e03, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e04, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e12, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e13, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e14, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e23, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e24, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e34, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementSub< BasisBits_e012, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e013, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e014, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e023, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e024, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e034, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e123, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e124, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e134, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e234, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementSub< BasisBits_e0123, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e0124, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e0134, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e0234, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    elements += T_ElementSub< BasisBits_e1234, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//    
//    elements += T_ElementSub< BasisBits_e01234, t_ElementLHS, t_ElementRHS, t_ElementResult, t_Dimensions, t_Metric >::Process( lhs, rhs, pResult, generateCode, elements );
//}


/// T_Vector Negation
template < ElementBits  t_Element,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS GA_OPERATOR_INLINE
    const T_Vector< t_Element, t_Dimensions, t_Metric >
    operator - ( const T_Vector< t_Element, t_Dimensions, t_Metric >& lhs )
{
    #if defined( MATHS_GA_CODEGEN )
    static bool generateCode = true;
    if ( generateCode )
    {
        Util::C_String metricString = GetString< t_Metric >();
        printf( "\n****** GA CODEGEN START ******\n" );
        //printf( "template <>\n" );
        printf( "NOALIAS GA_CODEGEN_INLINE\n" );
        printf( "const T_Vector< (ElementBits)0x%X, %d, %s >\n", t_Element, t_Dimensions, metricString.Chars() );
        printf( "operator - ( const T_Vector< (ElementBits)0x%X, %d, %s >& lhs )\n", t_Element, t_Dimensions, metricString.Chars() );
        printf( "{\n" );
        printf( "    T_Vector< (ElementBits)0x%X, %d, %s > result;\n", t_Element, t_Dimensions, metricString.Chars() );
        for ( U32 i=0; i<lhs.m_Vectors; i++ )
        {
            printf( "    result.m_V[%d] = -lhs.m_V[%d];\n", i, i );
        }
        printf( "    return result;\n" );
        printf( "}\n" );
        printf( "****** GA CODEGEN END ******\n\n" );
        generateCode = false;
    }
    #endif
    
    T_Vector< t_Element, t_Dimensions, t_Metric > result;
    for ( U32 i=0; i<lhs.m_Vectors; i++ )
    {
        result.m_V[i] = -lhs.m_V[i];
    }
    return result;
}

/// T_Vector Addition
template < ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS GA_OPERATOR_INLINE
    const typename T_AddSubResult< t_ElementLHS, t_ElementRHS, t_Dimensions, t_Metric >::TypeResult
    operator + ( const T_Vector< t_ElementLHS, t_Dimensions, t_Metric >& lhs,
                 const T_Vector< t_ElementRHS, t_Dimensions, t_Metric >& rhs )
{
    #if defined( MATHS_GA_CODEGEN )
    static bool generateCode = true;
    if ( generateCode )
    {
        Util::C_String metricString = GetString< t_Metric >();
        printf( "\n****** GA CODEGEN START ******\n" );
        //printf( "template <>\n" );
        printf( "NOALIAS GA_CODEGEN_INLINE\n" );
        printf( "const T_Vector< (ElementBits)0x%X, %d, %s >\n", t_ElementLHS | t_ElementRHS, t_Dimensions, metricString.Chars() );
        printf( "operator + ( const T_Vector< (ElementBits)0x%X, %d, %s >& lhs,\n", t_ElementLHS, t_Dimensions, metricString.Chars() );
        printf( "             const T_Vector< (ElementBits)0x%X, %d, %s >& rhs )\n", t_ElementRHS, t_Dimensions, metricString.Chars() );
        printf( "{\n" );
        printf( "    T_Vector< (ElementBits)0x%X, %d, %s > result;\n", t_ElementLHS | t_ElementRHS, t_Dimensions, metricString.Chars() );
        if ( t_ElementLHS == t_ElementRHS )
        {
            for ( U32 i=0; i<lhs.m_Vectors; i++ )
            {
                printf( "    result.m_V[%d] = lhs.m_V[%d] + rhs.m_V[%d];\n", i, i, i );
            }
        }
    }
    #else
    const bool generateCode = false;
    #endif
    
    typedef typename T_AddSubResult< t_ElementLHS, t_ElementRHS, t_Dimensions, t_Metric >::TypeResult TypeResult;
    TypeResult result;
    if ( t_ElementLHS == t_ElementRHS )
    {
        for ( U32 i=0; i<lhs.m_Vectors; i++ )
        {
            result.m_V[i] = lhs.m_V[i] + rhs.m_V[i];
        }
    }
    else
    {
        //T_VectorAdd< t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >( lhs, rhs, &result, generateCode );
        
        U32 elements = 0;
        
        elements += T_ElementAdd< BasisBits_s, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementAdd< BasisBits_e0, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e1, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e2, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e3, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e4, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementAdd< BasisBits_e01, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e02, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e03, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e04, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e12, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e13, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e14, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e23, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e24, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e34, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementAdd< BasisBits_e012, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e013, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e014, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e023, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e024, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e034, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e123, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e124, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e134, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e234, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementAdd< BasisBits_e0123, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e0124, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e0134, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e0234, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementAdd< BasisBits_e1234, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementAdd< BasisBits_e01234, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
    }
    
    #if defined( MATHS_GA_CODEGEN )
    if ( generateCode )
    {
        printf( "    return result;\n" );
        printf( "}\n" );
        printf( "****** GA CODEGEN END ******\n\n" );
        generateCode = false;
    }
    #endif
    
    return result;
}

/// T_Vector Subtraction
template < ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS GA_OPERATOR_INLINE
    const typename T_AddSubResult< t_ElementLHS, t_ElementRHS, t_Dimensions, t_Metric >::TypeResult
    operator - ( const T_Vector< t_ElementLHS, t_Dimensions, t_Metric >& lhs,
                 const T_Vector< t_ElementRHS, t_Dimensions, t_Metric >& rhs )
{
    #if defined( MATHS_GA_CODEGEN )
    static bool generateCode = true;
    if ( generateCode )
    {
        Util::C_String metricString = GetString< t_Metric >();
        printf( "\n****** GA CODEGEN START ******\n" );
        //printf( "template <>\n" );
        printf( "NOALIAS GA_CODEGEN_INLINE\n" );
        printf( "const T_Vector< (ElementBits)0x%X, %d, %s >\n", t_ElementLHS | t_ElementRHS, t_Dimensions, metricString.Chars() );
        printf( "operator - ( const T_Vector< (ElementBits)0x%X, %d, %s >& lhs,\n", t_ElementLHS, t_Dimensions, metricString.Chars() );
        printf( "             const T_Vector< (ElementBits)0x%X, %d, %s >& rhs )\n", t_ElementRHS, t_Dimensions, metricString.Chars() );
        printf( "{\n" );
        printf( "    T_Vector< (ElementBits)0x%X, %d, %s > result;\n", t_ElementLHS | t_ElementRHS, t_Dimensions, metricString.Chars() );
        if ( t_ElementLHS == t_ElementRHS )
        {
            for ( U32 i=0; i<lhs.m_Vectors; i++ )
            {
                printf( "    result.m_V[%d] = lhs.m_V[%d] - rhs.m_V[%d];\n", i, i, i );
            }
        }
    }
    #else
    const bool generateCode = false;
    #endif
    
    typedef typename T_AddSubResult< t_ElementLHS, t_ElementRHS, t_Dimensions, t_Metric >::TypeResult TypeResult;
    TypeResult result;
    if ( t_ElementLHS == t_ElementRHS )
    {
        for ( U32 i=0; i<lhs.m_Vectors; i++ )
        {
            result.m_V[i] = lhs.m_V[i] - rhs.m_V[i];
        }
    }
    else
    {
        //T_VectorSub< t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >( lhs, rhs, &result, generateCode );
        
        U32 elements = 0;
        
        elements += T_ElementSub< BasisBits_s, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementSub< BasisBits_e0, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e1, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e2, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e3, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e4, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementSub< BasisBits_e01, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e02, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e03, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e04, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e12, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e13, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e14, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e23, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e24, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e34, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementSub< BasisBits_e012, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e013, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e014, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e023, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e024, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e034, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e123, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e124, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e134, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e234, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementSub< BasisBits_e0123, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e0124, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e0134, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e0234, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        elements += T_ElementSub< BasisBits_e1234, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
        
        elements += T_ElementSub< BasisBits_e01234, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits, t_Dimensions, t_Metric >::Process( lhs, rhs, &result, generateCode, elements );
    }
    
    #if defined( MATHS_GA_CODEGEN )
    if ( generateCode )
    {
        printf( "    return result;\n" );
        printf( "}\n" );
        printf( "****** GA CODEGEN END ******\n\n" );
        generateCode = false;
    }
    #endif
    
    return result;
}






//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x10000, 5, Conformal3D >
operator - ( const T_Vector< (ElementBits)0x10000, 5, Conformal3D >& lhs )
{
    T_Vector< (ElementBits)0x10000, 5, Conformal3D > result;
    result.m_V[0] = -lhs.m_V[0];
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1, 5, Conformal3D >
operator + ( const T_Vector< (ElementBits)0x1, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1, 5, Conformal3D > result;
    result.m_V[0] = lhs.m_V[0] + rhs.m_V[0];
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1, 5, Conformal3D >
operator - ( const T_Vector< (ElementBits)0x1, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1, 5, Conformal3D > result;
    result.m_V[0] = lhs.m_V[0] - rhs.m_V[0];
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >
operator - ( const T_Vector< (ElementBits)0x1, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1140000, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1140001, 5, Conformal3D > result;
    SetElement<BasisBits_s>( &result, Element<BasisBits_s>(lhs) );
    SetElement<BasisBits_e14>( &result, -Element<BasisBits_e14>(rhs) );
    SetElement<BasisBits_e24>( &result, -Element<BasisBits_e24>(rhs) );
    SetElement<BasisBits_e34>( &result, -Element<BasisBits_e34>(rhs) );
    return result;
}



/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


