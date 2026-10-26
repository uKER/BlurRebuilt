#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_REVERSE_INL
#define SOFT_MATHS_HVEC_NEW_GA_REVERSE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_REVERSE_INL
#define MATHS_HVEC_NEW_GA_REVERSE_INL
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

template < BasisBits    t_Basis,
           ElementBits  t_Element,
           unsigned int t_Dimensions,
           typename     t_Metric >
struct T_ElementReverse
{
    NOALIAS __forceinline static const U32 Process(
        const T_Vector< t_Element, t_Dimensions, t_Metric >& src,
              T_Vector< t_Element, t_Dimensions, t_Metric >* pResult,
        const bool                                           generateCode,
        const U32                                            index )
    {
        if ( t_Element & (1<<t_Basis) )
        {
            if ( ( T_BitCount< t_Basis >::m_Bits & 0x2 ) == 0x2 ) // ++--++-- ...
            {
                SetElement< t_Basis >( pResult, -Element< t_Basis >( src ) );
                if ( generateCode )
                {
                    const char* basisString = GA::GetString( t_Basis );
                    printf( "    SetElement<%s>( &result, -Element<%s>(src) );\n", basisString, basisString );
                }
            }
            else
            {
                SetElement< t_Basis >( pResult, Element< t_Basis >( src ) );
                if ( generateCode )
                {
                    const char* basisString = GA::GetString( t_Basis );
                    printf( "    SetElement<%s>( &result, Element<%s>(src) );\n", basisString, basisString );
                }
            }
            return 1;
        }
        return 0;
    }
};

/// T_Vector Reverse
template < ElementBits  t_Element,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS GA_FUNCTION_INLINE
    const T_Vector< t_Element, t_Dimensions, t_Metric >
    Reverse( const T_Vector< t_Element, t_Dimensions, t_Metric >& src )
{
    T_Vector< t_Element, t_Dimensions, t_Metric > result;
    
    #if defined( MATHS_GA_CODEGEN )
    static bool generateCode = true;
    if ( generateCode )
    {
        Util::C_String metricString = GetString< t_Metric >();
        printf( "\n****** GA CODEGEN START ******\n" );
        //printf( "template <>\n" );
        printf( "NOALIAS GA_CODEGEN_INLINE\n" );
        printf( "const T_Vector< (ElementBits)0x%X, %d, %s >\n", t_Element, t_Dimensions, metricString.Chars() );
        printf( "Reverse( const T_Vector< (ElementBits)0x%X, %d, %s >& src )\n", t_Element, t_Dimensions, metricString.Chars() );
        printf( "{\n" );
        printf( "    T_Vector< (ElementBits)0x%X, %d, %s > result;\n", t_Element, t_Dimensions, metricString.Chars() );
    }
    #else
    const bool generateCode = false;
    #endif
    
    U32 elements = 0;
    
    elements += T_ElementReverse< BasisBits_s, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementReverse< BasisBits_e0, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e1, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e2, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e3, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e4, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementReverse< BasisBits_e01, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e02, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e03, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e04, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e12, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e13, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e14, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e23, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e24, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e34, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementReverse< BasisBits_e012, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e013, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e014, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e023, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e024, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e034, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e123, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e124, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e134, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e234, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementReverse< BasisBits_e0123, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e0124, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e0134, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e0234, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementReverse< BasisBits_e1234, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementReverse< BasisBits_e01234, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
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
const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >
Reverse( const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >& src )
{
    T_Vector< (ElementBits)0x1140001, 5, Conformal3D > result;
    SetElement<BasisBits_s>( &result, Element<BasisBits_s>(src) );
    SetElement<BasisBits_e14>( &result, -Element<BasisBits_e14>(src) );
    SetElement<BasisBits_e24>( &result, -Element<BasisBits_e24>(src) );
    SetElement<BasisBits_e34>( &result, -Element<BasisBits_e34>(src) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1441, 5, Conformal3D >
Reverse( const T_Vector< (ElementBits)0x1441, 5, Conformal3D >& src )
{
    T_Vector< (ElementBits)0x1441, 5, Conformal3D > result;
    SetElement<BasisBits_s>( &result, Element<BasisBits_s>(src) );
    SetElement<BasisBits_e12>( &result, -Element<BasisBits_e12>(src) );
    SetElement<BasisBits_e13>( &result, -Element<BasisBits_e13>(src) );
    SetElement<BasisBits_e23>( &result, -Element<BasisBits_e23>(src) );
    return result;
}

NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x80000000, 5, Conformal3D >
Reverse( const T_Vector< (ElementBits)0x80000000, 5, Conformal3D >& src )
{
    T_Vector< (ElementBits)0x80000000, 5, Conformal3D > result;
    SetElement<BasisBits_e01234>( &result, Element<BasisBits_e01234>(src) );
    return result;
}

NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x4000, 5, Conformal3D >
Reverse( const T_Vector< (ElementBits)0x4000, 5, Conformal3D >& src )
{
    T_Vector< (ElementBits)0x4000, 5, Conformal3D > result;
    SetElement<BasisBits_e123>( &result, -Element<BasisBits_e123>(src) );
    return result;
}




/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


