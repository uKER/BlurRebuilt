#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VBOOL_INL
#define SOFT_MATHS_HVEC_NEW_VBOOL_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VBOOL_INL
#define MATHS_HVEC_NEW_VBOOL_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean
/// @{

/// VBool cast to bool
NOALIAS __forceinline VBool::operator const bool () const
{
    bool result;
    #if defined( _SPU_ )
    result = (bool)( spu_extract( m_V, VX ) & 1 );
    #elif defined( _PPU_ )
    result = (bool)( vec_all_eq( m_V, Const< vector bool int >::True() ) & 1 ); // & 1 necessary?
    #elif defined( _VMX128_ )
    //__stvebx( __vand( m_V, __vspltisb( 1 ) ), &result, 0x0 );
    U32 CR;
    __vcmpequwR( m_V, __vzero(), &CR );
    const char char_result( (char)( ( CR & VMX_CR6_ALL_FALSE ) >> VMX_CR6_ALL_FALSE_BIT ) );
    result = (bool&)char_result;
    #elif defined( _SSE_ )
    result = _mm_movemask_ps( m_V ) & E_SSE_MaskBits_X;
    #else
    result = m_V;
    #endif
    return result;
}


/// Make VBool from a bool template bool ( compile time constant )
template < bool x >
NOALIAS __forceinline const VBool MakeVBool()
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::Bool< x >();
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool int >::Bool< x >();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool< x >();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::Bool< x >();
    #else
    result.m_V = x;
    #endif
    return result;
}

/// Make VBool from a bool ( runtime variable )
NOALIAS __forceinline const VBool MakeVBool( const bool x )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::Bool( x );
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool int >::Bool( x );
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool( x );
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::Bool( x );
    #else
    result.m_V = x;
    #endif
    return result;
}


/// VBool negation operator
NOALIAS __forceinline const VBool operator ~ ( const VBool PCREF x )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_not( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_nor( x.m_V, x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vnor( x.m_V, x.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_ps( x.m_V, Const< __m128 >::Mask<1,1,1,1>() );
    #else
    result.m_V = !x.m_V;
    #endif
    return result;
}

/// VBool logical and operator
NOALIAS __forceinline const VBool operator & ( const VBool PCREF x, const VBool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V && y.m_V;
    #endif
    return result;
}

/// VBool logical or operator
NOALIAS __forceinline const VBool operator | ( const VBool PCREF x, const VBool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V || y.m_V;
    #endif
    return result;
}

/// VBool logical xor operator
NOALIAS __forceinline const VBool operator ^ ( const VBool PCREF x, const VBool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V != y.m_V;
    #endif
    return result;
}

/// VBool equal operator
NOALIAS __forceinline const VBool operator == ( const VBool PCREF x, const VBool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpeq( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpeq( (vector unsigned int)x.m_V, (vector unsigned int)y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpequw( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = (__m128&)_mm_cmpeq_epi32( (__m128i&)x.m_V, (__m128i&)y.m_V );
    #else
    result.m_V = x.m_V == y.m_V;
    #endif
    return result;
}

/// VBool not-equal operator
NOALIAS __forceinline const VBool operator != ( const VBool PCREF x, const VBool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V != y.m_V;
    #endif
    return result;
}

/// VBool logical and assignment operator
NOALIAS __forceinline void operator &= ( VBool& x, const VBool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_and_ps( x.m_V, y.m_V );
    #else
    x.m_V = x.m_V && y.m_V;
    #endif
}

/// VBool logical or assignment operator
NOALIAS __forceinline void operator |= ( VBool& x, const VBool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_or_ps( x.m_V, y.m_V );
    #else
    x.m_V = x.m_V || y.m_V;
    #endif
}
    

/// VBool Bitwise NOT: ~a
NOALIAS __forceinline const VBool NOT( const VBool PCREF a )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_not( a.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_not( a.m_V );
    #elif defined( _VMX128_ )                    
    //result.m_V = __vnor( a.m_V, a.m_V );
    result.m_V = vmx_not( a.m_V );
    #elif defined( _SSE_ )                      
    //result.m_V = _mm_xor_ps( a.m_V, Const< __m128 >::Mask<1,1,1,1>() );
    result.m_V = sse_not( a.m_V );
    #else
    result.m_V = !a.m_V;
    #endif
    return result;
}

/// VBool Bitwise OR: a | b
NOALIAS __forceinline const VBool OR( const VBool PCREF a, const VBool PCREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_or( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_or( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_or( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V || b.m_V;
    #endif
    return result;
}

/// VBool Bitwise AND: a & b
NOALIAS __forceinline const VBool AND( const VBool PCREF a, const VBool PCREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_and( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_and( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_and( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V && b.m_V;
    #endif
    return result;
}

/// VBool Bitwise XOR: a ^ b : a != b : (a & ~b) | (~a & b)
NOALIAS __forceinline const VBool XOR( const VBool PCREF a, const VBool PCREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_xor( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_xor( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_xor( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V != b.m_V;
    #endif
    return result;
}

/// VBool Bitwise NOR: ~(a | b)
NOALIAS __forceinline const VBool NOR( const VBool PCREF a, const VBool PCREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_nor( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_nor( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_nor( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_nor( a.m_V, b.m_V );
    #else
    result.m_V = !( a.m_V || b.m_V );
    #endif
    return result;
}

/// VBool Bitwise NAND: ~(a & b)
NOALIAS __forceinline const VBool NAND( const VBool PCREF a, const VBool PCREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_nand( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_nand( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_nand( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_nand( a.m_V, b.m_V );
    #else
    result.m_V = !( a.m_V && b.m_V );
    #endif
    return result;
}

/// VBool Bitwise EQV: ~(a ^ b) : a == b : (a & b) | (~a & ~b)
NOALIAS __forceinline const VBool EQV( const VBool PCREF a, const VBool PCREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_eqv( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_eqv( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_eqv( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_eqv( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V == b.m_V;
    #endif
    return result;
}

/// VBool Bitwise ORC: a | ~b
NOALIAS __forceinline const VBool ORC( const VBool PCREF a, const VBool PCREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_orc( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_orc( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_orc( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_orc( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V || !b.m_V;
    #endif
    return result;
}

/// VBool Bitwise ANDC: a & ~b
NOALIAS __forceinline const VBool ANDC( const VBool PCREF a, const VBool PCREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_andc( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_andc( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_andc( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_andc( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V && !b.m_V;
    #endif
    return result;
}






/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


