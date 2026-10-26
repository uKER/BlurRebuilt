#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_INVOLUTION_INL
#define SOFT_MATHS_HVEC_NEW_GA_INVOLUTION_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_INVOLUTION_INL
#define MATHS_HVEC_NEW_GA_INVOLUTION_INL
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
struct T_ElementInvolution
{
    NOALIAS __forceinline static const U32 Process(
        const T_Vector< t_Element, t_Dimensions, t_Metric >& src,
              T_Vector< t_Element, t_Dimensions, t_Metric >* pResult,
        const bool                                           generateCode,
        const U32                                            index )
    {
        if ( t_Element & (1<<t_Basis) )
        {
            if ( ( T_BitCount< t_Basis >::m_Bits & 0x1 ) == 0x1 ) // +-+-+-+- ...
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

/// T_Vector Involution
template < ElementBits  t_Element,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS GA_FUNCTION_INLINE
    const T_Vector< t_Element, t_Dimensions, t_Metric >
    Involution( const T_Vector< t_Element, t_Dimensions, t_Metric >& src )
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
        printf( "Involution( const T_Vector< (ElementBits)0x%X, %d, %s >& src )\n", t_Element, t_Dimensions, metricString.Chars() );
        printf( "{\n" );
        printf( "    T_Vector< (ElementBits)0x%X, %d, %s > result;\n", t_Element, t_Dimensions, metricString.Chars() );
    }
    #else
    const bool generateCode = false;
    #endif
    
    U32 elements = 0;
    
    elements += T_ElementInvolution< BasisBits_s, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementInvolution< BasisBits_e0, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e1, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e2, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e3, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e4, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementInvolution< BasisBits_e01, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e02, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e03, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e04, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e12, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e13, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e14, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e23, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e24, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e34, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementInvolution< BasisBits_e012, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e013, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e014, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e023, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e024, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e034, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e123, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e124, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e134, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e234, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementInvolution< BasisBits_e0123, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e0124, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e0134, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e0234, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    elements += T_ElementInvolution< BasisBits_e1234, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
    elements += T_ElementInvolution< BasisBits_e01234, t_Element, t_Dimensions, t_Metric >::Process( src, &result, generateCode, elements );
    
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











/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


