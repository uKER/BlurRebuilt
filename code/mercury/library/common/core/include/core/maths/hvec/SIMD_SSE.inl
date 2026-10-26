#ifndef SIMD_SSE_INL
#define SIMD_SSE_INL

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
#include <alloca.h>
#endif

#ifndef PLATFORM_SPU
#include "memory.h"
#endif

namespace Maths
{
#if defined( _SSE_ )

//
// SIMD Sign Bit Masks
//

enum E_SSE_MaskBits
{
    E_SSE_MaskBits_0 =    0x0,
    E_SSE_MaskBits_X =    0x1,
    E_SSE_MaskBits_Y =    0x2,
    E_SSE_MaskBits_Z =    0x4,
    E_SSE_MaskBits_W =    0x8,
    E_SSE_MaskBits_XY =   ( E_SSE_MaskBits_X | E_SSE_MaskBits_Y ),
    E_SSE_MaskBits_XZ =   ( E_SSE_MaskBits_X | E_SSE_MaskBits_Z ),
    E_SSE_MaskBits_XW =   ( E_SSE_MaskBits_X | E_SSE_MaskBits_W ),
    E_SSE_MaskBits_YZ =   ( E_SSE_MaskBits_Y | E_SSE_MaskBits_Z ),
    E_SSE_MaskBits_YW =   ( E_SSE_MaskBits_Y | E_SSE_MaskBits_W ),
    E_SSE_MaskBits_ZW =   ( E_SSE_MaskBits_Z | E_SSE_MaskBits_W ),
    E_SSE_MaskBits_XYZ =  ( E_SSE_MaskBits_X | E_SSE_MaskBits_Y | E_SSE_MaskBits_Z ),
    E_SSE_MaskBits_XYW =  ( E_SSE_MaskBits_X | E_SSE_MaskBits_Y | E_SSE_MaskBits_W ),
    E_SSE_MaskBits_XZW =  ( E_SSE_MaskBits_X | E_SSE_MaskBits_Z | E_SSE_MaskBits_W ),
    E_SSE_MaskBits_YZW =  ( E_SSE_MaskBits_Y | E_SSE_MaskBits_Z | E_SSE_MaskBits_W ),
    E_SSE_MaskBits_XYZW = ( E_SSE_MaskBits_X | E_SSE_MaskBits_Y | E_SSE_MaskBits_Z | E_SSE_MaskBits_W )
};



template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7,
           unsigned char I8, unsigned char I9, unsigned char IA, unsigned char IB,
           unsigned char IC, unsigned char ID, unsigned char IE, unsigned char IF >
class ShuffleByte
{
public:
    static const __m128i Pattern;
};

template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7,
           unsigned char I8, unsigned char I9, unsigned char IA, unsigned char IB,
           unsigned char IC, unsigned char ID, unsigned char IE, unsigned char IF >
const __m128i ShuffleByte<I0,I1,I2,I3,I4,I5,I6,I7,I8,I9,IA,IB,IC,ID,IE,IF>::Pattern =
{
    I0, I1, I2, I3,
    I4, I5, I6, I7,
    I8, I9, IA, IB,
    IC, ID, IE, IF
};



//
// SSE SIMD Scalar & Vector Maths Functions
//


#define sse_merge_xy _mm_unpacklo_ps
#define sse_merge_zw _mm_unpackhi_ps

/// Make vector3 from scalar x, y & z : the X component of each scalar must contain the value. (splatted scalars not required)
NOALIAS __forceinline const __m128 sse_make_vector3( const __m128& x, const __m128& y, const __m128& z )
{
    return sse_merge_xy( sse_merge_xy( x, z ), y ); // NOTE: w component is UNDEFINED, not zero!
}

/// Make vector4 from scalar x, y, z & w : the X component of each scalar must contain the value. (splatted scalars not required)
NOALIAS __forceinline const __m128 sse_make_vector4( const __m128& x, const __m128& y, const __m128& z, const __m128& w )
{
    return sse_merge_xy( sse_merge_xy( x, z ),
                         sse_merge_xy( y, w ) );
}


/// SSE not
NOALIAS __forceinline const __m128  sse_not( const __m128& a )  { return _mm_andnot_ps( a, Const< __m128 >::Bool<true>() ); }
NOALIAS __forceinline const __m128d sse_not( const __m128d& a ) { return _mm_andnot_pd( a, (__m128d&)Const< __m128 >::Bool<true>() ); }
NOALIAS __forceinline const __m128i sse_not( const __m128i& a ) { return _mm_andnot_si128( a, (__m128i&)Const< __m128 >::Bool<true>() ); }

#define sse_or   _mm_or_ps
#define sse_and  _mm_and_ps
#define sse_xor  _mm_xor_ps

NOALIAS __forceinline const __m128 sse_nor( const __m128& a, const __m128& b )
{
    return sse_not( sse_or( a, b ) );
}

NOALIAS __forceinline const __m128 sse_nand( const __m128& a, const __m128& b )
{
    return sse_not( sse_and( a, b ) );
}

NOALIAS __forceinline const __m128 sse_eqv( const __m128& a, const __m128& b )
{
    return sse_not( sse_xor( a, b ) );
}

NOALIAS __forceinline const __m128 sse_orc( const __m128& a, const __m128& b )
{
    return sse_or( a, sse_not( b ) );
}

NOALIAS __forceinline const __m128 sse_andc( const __m128& a, const __m128& b )
{
    return _mm_andnot_ps( b, a );
}


/// SSE splat
template < unsigned int I >
NOALIAS __forceinline const __m128 sse_splat( const __m128& a )
{
    StaticAssert( I < 4, I_MUST_BE_LESS_THAN_4 );
    return (__m128&)_mm_shuffle_epi32( (__m128i&)a, _MM_SHUFFLE( I%4, I%4, I%4, I%4 ) );
}

/// SSE splat (int)
template < unsigned int I >
NOALIAS __forceinline const __m128i sse_splat_int32( const __m128i& a )
{
    StaticAssert( I < 4, I_MUST_BE_LESS_THAN_4 );
    return _mm_shuffle_epi32( a, _MM_SHUFFLE( I%4, I%4, I%4, I%4 ) );
}

/// SSE splat (short)
template < unsigned int I >
NOALIAS __forceinline const __m128i sse_splat_int16( const __m128i& a )
{
    StaticAssert( I < 8, I_MUST_BE_LESS_THAN_8 );
    if ( I >= 4 )
    {
        const __m128i hi( _mm_shufflehi_epi16( a, _MM_SHUFFLE( I%4, I%4, I%4, I%4 ) ) );
        return (__m128i&)_mm_movehl_ps( (__m128&)hi, (__m128&)hi );
    }
    else
    {
        const __m128i lo( _mm_shufflelo_epi16( a, _MM_SHUFFLE( I%4, I%4, I%4, I%4 ) ) );
        return (__m128i&)_mm_movelh_ps( (__m128&)lo, (__m128&)lo );
    }
}

/// SSE splat (char)
//template < unsigned int I >
//NOALIAS __forceinline const __m128i sse_splat_int8( const __m128i& a )
//{
//    StaticAssert( I < 16, I_MUST_BE_LESS_THAN_16 );
//}

/// SSE permute
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const __m128 sse_permute( const __m128& a )
{
    StaticAssert( IX < 4, IX_MUST_BE_LESS_THAN_4 );
    StaticAssert( IY < 4, IY_MUST_BE_LESS_THAN_4 );
    StaticAssert( IZ < 4, IZ_MUST_BE_LESS_THAN_4 );
    StaticAssert( IW < 4, IW_MUST_BE_LESS_THAN_4 );
    
    if ( ( IX == VX ) && ( IY == VY ) && ( IZ == VZ ) && ( IW == VW ) )
        return a;
    else
        //return _mm_shuffle_ps( a, a, _MM_SHUFFLE( IW, IZ, IY, IX ) );
        return (__m128&)_mm_shuffle_epi32( (__m128i&)a, _MM_SHUFFLE( IW%4, IZ%4, IY%4, IX%4 ) );
}

/// SSE permute S32/U32
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const __m128i sse_permute_int32( const __m128i& a )
{
    StaticAssert( IX < 4, IX_MUST_BE_LESS_THAN_4 );
    StaticAssert( IY < 4, IY_MUST_BE_LESS_THAN_4 );
    StaticAssert( IZ < 4, IZ_MUST_BE_LESS_THAN_4 );
    StaticAssert( IW < 4, IW_MUST_BE_LESS_THAN_4 );
    
    if ( ( IX == VX ) && ( IY == VY ) && ( IZ == VZ ) && ( IW == VW ) )
        return a;
    else
        return _mm_shuffle_epi32( a, _MM_SHUFFLE( IW%4, IZ%4, IY%4, IX%4 ) );
}

template < unsigned char I0, unsigned char I1, unsigned char I2 >
NOALIAS __forceinline const __m128 sse_permute( const __m128& v0, const __m128& v1, const __m128& v2 )
{
    StaticAssert( I0 < 4, I0_MUST_BE_LESS_THAN_4 );
    StaticAssert( I1 < 4, I1_MUST_BE_LESS_THAN_4 );
    StaticAssert( I2 < 4, I2_MUST_BE_LESS_THAN_4 );
    
    const __m128 zero = Const< __m128 >::Zero();
    const __m128 x = sse_splat<I0>( v0 );
    const __m128 y = sse_splat<I1>( v1 );
    const __m128 z = sse_splat<I2>( v2 );
    
    return sse_merge_xy( sse_merge_xy( x, z ), sse_merge_xy( y, zero ) );
}

/// SSE permute
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const __m128 sse_permute( const __m128& a, const __m128& b )
{
    StaticAssert( IX < 8, IX_MUST_BE_LESS_THAN_8 );
    StaticAssert( IY < 8, IY_MUST_BE_LESS_THAN_8 );
    StaticAssert( IZ < 8, IZ_MUST_BE_LESS_THAN_8 );
    StaticAssert( IW < 8, IW_MUST_BE_LESS_THAN_8 );
    
    #define a0 0
    #define a1 1
    #define a2 2
    #define a3 3
    #define b0 4
    #define b1 5
    #define b2 6
    #define b3 7
    #define INDICES_EQ(i0,i1,i2,i3) (IX==i0) && (IY==i1) && (IZ==i2) && (IW==i3)
    #define INDICES_LE(i)           (IX<=i)  && (IY<=i)  && (IZ<=i)  && (IW<=i)
    #define INDICES_GE(i)           (IX>=i)  && (IY>=i)  && (IZ>=i)  && (IW>=i)
    
    if ( INDICES_EQ( a0, a1, a2, a3 ) ) return a;
    if ( INDICES_EQ( b0, b1, b2, b3 ) ) return b;
    //if ( INDICES_LE( a3 ) )             { return sse_permute<IX,IY,IZ,IW>( a ); }
    if ( INDICES_LE( a3 ) )             { return sse_permute<IX%4,IY%4,IZ%4,IW%4>( a ); }
    //if ( INDICES_GE( b0 ) )             { return sse_permute<IX-4,IY-4,IZ-4,IW-4>( b ); }
    if ( INDICES_GE( b0 ) )             { return sse_permute<IX%4,IY%4,IZ%4,IW%4>( b ); }
    if ( INDICES_EQ( a0, b0, a1, b1 ) ) return _mm_unpacklo_ps( a, b );
    if ( INDICES_EQ( a2, b2, a3, b3 ) ) return _mm_unpackhi_ps( a, b );
    if ( INDICES_EQ( b0, a0, b1, a1 ) ) return _mm_unpacklo_ps( b, a );
    if ( INDICES_EQ( b2, a2, b3, a3 ) ) return _mm_unpackhi_ps( b, a );
    if ( INDICES_EQ( a0, a1, b0, b1 ) ) return _mm_movelh_ps( a, b );
    if ( INDICES_EQ( a2, a3, b2, b3 ) ) return _mm_movehl_ps( b, a );
    if ( INDICES_EQ( b0, b1, a0, a1 ) ) return _mm_movelh_ps( b, a );
    if ( INDICES_EQ( b2, b3, a2, a3 ) ) return _mm_movehl_ps( a, b );
    
    {
        const __m128 x( IX==a0 ? a : ( IX==b0 ? b : ( IX<=a3 ? sse_splat<IX%4>( a ) : sse_splat<IX%4>( b ) ) ) );
        const __m128 y( IY==a0 ? a : ( IY==b0 ? b : ( IY<=a3 ? sse_splat<IY%4>( a ) : sse_splat<IY%4>( b ) ) ) );
        const __m128 z( IZ==a0 ? a : ( IZ==b0 ? b : ( IZ<=a3 ? sse_splat<IZ%4>( a ) : sse_splat<IZ%4>( b ) ) ) );
        const __m128 w( IW==a0 ? a : ( IW==b0 ? b : ( IW<=a3 ? sse_splat<IW%4>( a ) : sse_splat<IW%4>( b ) ) ) );
        
        const __m128 xz( IX==a0 && IZ==a1 ? a :
                         IX==b0 && IZ==b1 ? b :
                         //IX<=a3 && IZ<=a3 ? sse_permute<IX,IZ,IX,IZ>( a ) :
                         //IX>=b0 && IZ>=b0 ? sse_permute<IX-4,IZ-4,IX-4,IZ-4>( b ) :
                         IX<=a3 && IZ<=a3 ? sse_permute<IX%4,IZ%4,IX%4,IZ%4>( a ) :
                         IX>=b0 && IZ>=b0 ? sse_permute<IX%4,IZ%4,IX%4,IZ%4>( b ) :
                         IX==a0 && IZ==b0 ? _mm_unpacklo_ps( a, b ) :
                         IX==a2 && IZ==b2 ? _mm_unpackhi_ps( a, b ) :
                         IX==b0 && IZ==a0 ? _mm_unpacklo_ps( b, a ) :
                         IX==b2 && IZ==a2 ? _mm_unpackhi_ps( b, a ) :
                                            _mm_unpacklo_ps( x, z ) );
        
        const __m128 yw( IY==a0 && IW==a1 ? a :
                         IY==b0 && IW==b1 ? b :
                         //IY<=a3 && IW<=a3 ? sse_permute<IY,IW,IY,IW>( a ) :
                         //IY>=b0 && IW>=b0 ? sse_permute<IY-4,IW-4,IY-4,IW-4>( b ) :
                         IY<=a3 && IW<=a3 ? sse_permute<IY%4,IW%4,IY%4,IW%4>( a ) :
                         IY>=b0 && IW>=b0 ? sse_permute<IY%4,IW%4,IY%4,IW%4>( b ) :
                         IY==a0 && IW==b0 ? _mm_unpacklo_ps( a, b ) :
                         IY==a2 && IW==b2 ? _mm_unpackhi_ps( a, b ) :
                         IY==b0 && IW==a0 ? _mm_unpacklo_ps( b, a ) :
                         IY==b2 && IW==a2 ? _mm_unpackhi_ps( b, a ) :
                                            _mm_unpacklo_ps( y, w ) );
        
        return _mm_unpacklo_ps( xz, yw );
    }
    
    #undef INDICES_EQ
    #undef INDICES_LE
    #undef INDICES_GE
    #undef a0
    #undef a1
    #undef a2
    #undef a3
    #undef b0
    #undef b1
    #undef b2
    #undef b3
}

/// SSE horizontal or
NOALIAS __forceinline const __m128 sse_horizontal_or_int32( const __m128& a )
{
    return _mm_or_ps( _mm_or_ps( a,
                                 sse_permute<VY,VZ,VW,VX>( a ) ),
                      _mm_or_ps( sse_permute<VZ,VW,VX,VY>( a ),
                                 sse_permute<VW,VX,VY,VZ>( a ) ) );
}

/// SSE horizontal or
NOALIAS __forceinline const __m128i sse_horizontal_or_int16( const __m128i& a )
{
    #if 0
    const __m128i b( _mm_or_si128( a, _mm_srli_si128( a, 0x8 ) ) );
    const __m128i c( _mm_or_si128( b, _mm_srli_si128( b, 0x4 ) ) );
    const __m128i d( _mm_or_si128( c, _mm_srli_si128( c, 0x2 ) ) );
    const __m128i e( _mm_unpacklo_epi16( d, d ) );
    return _mm_shuffle_epi32( e, _MM_SHUFFLE(0,0,0,0) );
    #else
    const __m128i b( _mm_or_si128( a, sse_permute_int32<2,3,0,1>( a ) ) );
    const __m128i c( _mm_or_si128( b, sse_permute_int32<1,2,3,0>( b ) ) );
    const __m128i d( _mm_or_si128( c, _mm_shufflehi_epi16( _mm_shufflelo_epi16( c, _MM_SHUFFLE(1,2,3,0) ), _MM_SHUFFLE(1,2,3,0) ) ) );
    return d;
    #endif
}

/// SSE horizontal or
NOALIAS __forceinline const __m128i sse_horizontal_or_int8( const __m128i& a )
{
    #if 0
    const __m128i b( _mm_or_si128( a, _mm_srli_si128( a, 0x8 ) ) );
    const __m128i c( _mm_or_si128( b, _mm_srli_si128( b, 0x4 ) ) );
    const __m128i d( _mm_or_si128( c, _mm_srli_si128( c, 0x2 ) ) );
    const __m128i e( _mm_or_si128( d, _mm_srli_si128( d, 0x1 ) ) );
    const __m128i f( _mm_unpacklo_epi8( e, e ) );
    const __m128i g( _mm_unpacklo_epi16( f, f ) );
    return _mm_shuffle_epi32( g, _MM_SHUFFLE(0,0,0,0) );
    #else
    const __m128i b( _mm_or_si128( a, sse_permute_int32<2,3,0,1>( a ) ) );
    const __m128i c( _mm_or_si128( b, sse_permute_int32<1,2,3,0>( b ) ) );
    const __m128i d( _mm_or_si128( c, _mm_srli_si128( c, 0x2 ) ) );
    const __m128i e( _mm_or_si128( d, _mm_srli_si128( d, 0x1 ) ) );
    return sse_splat_int32<0>( e );
    #endif
}


/// SSE horizontal and
NOALIAS __forceinline const __m128 sse_horizontal_and_int32( const __m128& a )
{
    return _mm_and_ps( _mm_and_ps( a,
                                   sse_permute<VY,VZ,VW,VX>( a ) ),
                       _mm_and_ps( sse_permute<VZ,VW,VX,VY>( a ),
                                   sse_permute<VW,VX,VY,VZ>( a ) ) );
}

/// SSE horizontal and
NOALIAS __forceinline const __m128i sse_horizontal_and_int16( const __m128i& a )
{
    #if 0
    const __m128i b( _mm_and_si128( a, _mm_srli_si128( a, 0x8 ) ) );
    const __m128i c( _mm_and_si128( b, _mm_srli_si128( b, 0x4 ) ) );
    const __m128i d( _mm_and_si128( c, _mm_srli_si128( c, 0x2 ) ) );
    const __m128i e( _mm_unpacklo_epi16( d, d ) );
    return _mm_shuffle_epi32( e, _MM_SHUFFLE(0,0,0,0) );
    #else
    const __m128i b( _mm_and_si128( a, sse_permute_int32<2,3,0,1>( a ) ) );
    const __m128i c( _mm_and_si128( b, sse_permute_int32<1,2,3,0>( b ) ) );
    const __m128i d( _mm_and_si128( c, _mm_shufflehi_epi16( _mm_shufflelo_epi16( c, _MM_SHUFFLE(1,2,3,0) ), _MM_SHUFFLE(1,2,3,0) ) ) );
    return d;
    #endif
}

/// SSE horizontal and
NOALIAS __forceinline const __m128i sse_horizontal_and_int8( const __m128i& a )
{
    #if 0
    const __m128i b( _mm_and_si128( a, _mm_srli_si128( a, 0x8 ) ) );
    const __m128i c( _mm_and_si128( b, _mm_srli_si128( b, 0x4 ) ) );
    const __m128i d( _mm_and_si128( c, _mm_srli_si128( c, 0x2 ) ) );
    const __m128i e( _mm_and_si128( d, _mm_srli_si128( d, 0x1 ) ) );
    const __m128i f( _mm_unpacklo_epi8( e, e ) );
    const __m128i g( _mm_unpacklo_epi16( f, f ) );
    return _mm_shuffle_epi32( g, _MM_SHUFFLE(0,0,0,0) );
    #else
    const __m128i b( _mm_and_si128( a, sse_permute_int32<2,3,0,1>( a ) ) );
    const __m128i c( _mm_and_si128( b, sse_permute_int32<1,2,3,0>( b ) ) );
    //const __m128i d( _mm_and_si128( c, _mm_shufflehi_epi16( _mm_shufflelo_epi16( c, _MM_SHUFFLE(1,2,3,0) ), _MM_SHUFFLE(1,2,3,0) ) ) );
    const __m128i d( _mm_and_si128( c, _mm_srli_si128( c, 0x2 ) ) );
    const __m128i e( _mm_and_si128( d, _mm_srli_si128( d, 0x1 ) ) );
    return sse_splat_int32<0>( e );
    #endif
}


/// SSE rotate left
template < unsigned int N >
NOALIAS __forceinline const __m128 sse_rotate_left_qw_words( const __m128& a )
{
    return sse_permute< (VX+N)&3, (VY+N)&3, (VZ+N)&3, (VW+N)&3 >( a );
}

/// SSE rotate right
template < unsigned int N >
NOALIAS __forceinline const __m128 sse_rotate_right_qw_words( const __m128& a )
{
    return sse_permute< (VX-N)&3, (VY-N)&3, (VZ-N)&3, (VW-N)&3 >( a );
}

/// SSE select : result.x = x==0 ? a.x : b.x
///              result.y = y==0 ? a.y : b.y
///              result.z = z==0 ? a.z : b.z
///              result.w = w==0 ? a.w : b.w
template < bool x, bool y, bool z, bool w >
NOALIAS __forceinline const __m128 sse_select( const __m128& a, const __m128& b )
{
    const __m128 mask( Const< __m128 >::Mask< !x, !y, !z, !w >() );
    return _mm_or_ps( _mm_and_ps( mask, a ), _mm_andnot_ps( mask, b ) );
}

/// SSE Select
NOALIAS __forceinline const __m128 sse_select( const __m128& a, const __m128& b, const __m128& selector )
{
    return _mm_or_ps( _mm_andnot_ps( selector, a ), _mm_and_ps( selector, b ) );
}

/// SSE Select
NOALIAS __forceinline const __m128i sse_select( const __m128i& a, const __m128i& b, const __m128i& selector )
{
    return _mm_or_si128( _mm_andnot_si128( selector, a ), _mm_and_si128( selector, b ) );
}

/// SSE abs
NOALIAS __forceinline const __m128 sse_abs( const __m128& a )
{
    return _mm_and_ps( a, Const< __m128 >::AbsMask() );
}

/// SSE negate
NOALIAS __forceinline const __m128 sse_negate( const __m128& a )
{
    return _mm_xor_ps( a, Const< __m128 >::SignMask() );
}


/// SSE 2 element Sum
NOALIAS __forceinline const __m128 sse_sum2( const __m128& a )
{
    return _mm_add_ps( sse_splat<VX>( a ),
                       sse_splat<VY>( a ) );
}

/// SSE 3 element Sum
NOALIAS __forceinline const __m128 sse_sum3( const __m128& a )
{
    return _mm_add_ps( _mm_add_ps( sse_splat<VX>( a ),
                                   sse_splat<VY>( a ) ),
                                   sse_splat<VZ>( a ) );
}

/// SSE 4 element Sum
NOALIAS __forceinline const __m128 sse_sum4( const __m128& a )
{
    return _mm_add_ps( _mm_add_ps( sse_splat<VX>( a ),
                                   sse_splat<VY>( a ) ),
                       _mm_add_ps( sse_splat<VZ>( a ),
                                   sse_splat<VW>( a ) ) );
}

/// SSE 2 element dot product
NOALIAS __forceinline const __m128 sse_dot_product2( const __m128& a, const __m128& b )
{
    const __m128 ab( _mm_mul_ps( a, b ) );
    return _mm_add_ps( sse_splat<VX>( ab ),
                       sse_splat<VY>( ab ) );
}

/// SSE 3 element dot product
NOALIAS __forceinline const __m128 sse_dot_product3( const __m128& a, const __m128& b )
{
    const __m128 ab( _mm_mul_ps( a, b ) );
    return _mm_add_ps( _mm_add_ps( sse_splat<VX>( ab ),
                                   sse_splat<VY>( ab ) ),
                                   sse_splat<VZ>( ab ) );
}

/// SSE 4 element dot product
NOALIAS __forceinline const __m128 sse_dot_product4( const __m128& a, const __m128& b )
{
    const __m128 ab( _mm_mul_ps( a, b ) );
    return _mm_add_ps( _mm_add_ps( sse_splat<VX>( ab ),
                                   sse_splat<VY>( ab ) ),
                       _mm_add_ps( sse_splat<VZ>( ab ),
                                   sse_splat<VW>( ab ) ) );
}

/// SSE 2 element cross product : Z & W elements of result are UNDEFINED!
NOALIAS __forceinline const __m128 sse_cross_product2( const __m128& a, const __m128& b )
{
    return _mm_sub_ps( _mm_mul_ps( sse_splat<VX>( a ), sse_splat<VY>( b ) ),
                       _mm_mul_ps( sse_splat<VY>( a ), sse_splat<VX>( b ) ) );
}

/// SSE 3 element cross product : W element of result is UNDEFINED!
NOALIAS __forceinline const __m128 sse_cross_product3( const __m128& a, const __m128& b )
{
    static const unsigned __ = VW;
    const __m128 a_yzx = sse_permute<VY,VZ,VX,__>( a );
    const __m128 a_zxy = sse_permute<VZ,VX,VY,__>( a );
    const __m128 b_yzx = sse_permute<VY,VZ,VX,__>( b );
    const __m128 b_zxy = sse_permute<VZ,VX,VY,__>( b );
    return _mm_sub_ps( _mm_mul_ps( a_yzx, b_zxy ), _mm_mul_ps( a_zxy, b_yzx ) );
}


/// SSE Multiply Add: a * b + c
NOALIAS __forceinline const __m128 sse_madd( const __m128& a, const __m128& b, const __m128& c )
{
    return _mm_add_ps( _mm_mul_ps( a, b ), c );
}

/// SSE Multiply Subtract: a * b - c
NOALIAS __forceinline const __m128 sse_msub( const __m128& a, const __m128& b, const __m128& c )
{
    return _mm_sub_ps( _mm_mul_ps( a, b ), c );
}

/// SSE Negative Multiply Add: -( a * b + c )
NOALIAS __forceinline const __m128 sse_nmadd( const __m128& a, const __m128& b, const __m128& c )
{
    return sse_negate( _mm_add_ps( _mm_mul_ps( a, b ), c ) );
}

/// SSE Negative Multiply Subtract: -( a * b - c ) = c - a * b
NOALIAS __forceinline const __m128 sse_nmsub( const __m128& a, const __m128& b, const __m128& c )
{
    return _mm_sub_ps( c, _mm_mul_ps( a, b ) );
}


/// SSE unaligned load vector
NOALIAS __forceinline const __m128 sse_load_aligned( F32 const * const __restrict pSrc )
{
    return _mm_load_ps( pSrc );
}

/// SSE unaligned load vector
NOALIAS __forceinline const __m128 sse_load_unaligned( F32 const * const __restrict pSrc )
{
    return _mm_loadu_ps( pSrc );
}

/// SSE unaligned load vector
NOALIAS __forceinline const __m128i sse_load_unaligned( S32 const * const __restrict pSrc )
{
    return _mm_loadu_si128( (const __m128i*)pSrc );
}

/// SSE unaligned load vector
NOALIAS __forceinline const __m128i sse_load_unaligned( U32 const * const __restrict pSrc )
{
    return _mm_loadu_si128( (const __m128i*)pSrc );
}

/// SSE unaligned load vector
NOALIAS __forceinline const __m128i sse_load_unaligned( S16 const * const __restrict pSrc )
{
    return _mm_loadu_si128( (const __m128i*)pSrc );
}

/// SSE unaligned load vector
NOALIAS __forceinline const __m128i sse_load_unaligned( U16 const * const __restrict pSrc )
{
    return _mm_loadu_si128( (const __m128i*)pSrc );
}

/// SSE unaligned load vector
NOALIAS __forceinline const __m128i sse_load_unaligned( S8 const * const __restrict pSrc )
{
    return _mm_loadu_si128( (const __m128i*)pSrc );
}

/// SSE unaligned load vector
NOALIAS __forceinline const __m128i sse_load_unaligned( U8 const * const __restrict pSrc )
{
    return _mm_loadu_si128( (const __m128i*)pSrc );
}


/// SSE unaligned store vector4
NOALIAS __forceinline void sse_store_aligned_vec4( const __m128& a, F32* const __restrict pDst )
{
    _mm_store_ps( pDst, a );
}

/// SSE unaligned store vector4
NOALIAS __forceinline void sse_store_unaligned_vec4( const __m128& a, F32* const __restrict pDst )
{
    _mm_storeu_ps( pDst, a );
}

/// SSE unaligned store vector3
NOALIAS __forceinline void sse_store_unaligned_vec3( const __m128& a, F32* const __restrict pDst )
{
    _mm_store_ss( &pDst[0], a );
    _mm_store_ss( &pDst[1], sse_splat<VY>( a ) );
    _mm_store_ss( &pDst[2], sse_splat<VZ>( a ) );
}

/// SSE unaligned store vector2
NOALIAS __forceinline void sse_store_unaligned_vec2( const __m128& a, F32* const __restrict pDst )
{
    _mm_store_ss( (F32*)&pDst[0], a );
    _mm_store_ss( (F32*)&pDst[1], sse_splat<VY>( a ) );
}

/// SSE unaligned store vector1
NOALIAS __forceinline void sse_store_unaligned_vec1( const __m128& a, F32* const __restrict pDst )
{
    _mm_store_ss( (F32*)&pDst[0], a );
}


NOALIAS __forceinline void sse_transpose_matrix33(
    const __m128&            inX,
    const __m128&            inY,
    const __m128&            inZ,
    __m128* const __restrict pOutX,
    __m128* const __restrict pOutY,
    __m128* const __restrict pOutZ )
{
    const __m128 inW( Const< __m128 >::Zero() );
    const __m128 tmp0( sse_merge_xy( inX, inZ ) );
    const __m128 tmp1( sse_merge_xy( inY, inW ) );
    const __m128 tmp2( sse_merge_zw( inX, inZ ) );
    const __m128 tmp3( sse_merge_zw( inY, inW ) );
    *pOutX = sse_merge_xy( tmp0, tmp1 );
    *pOutY = sse_merge_zw( tmp0, tmp1 );
    *pOutZ = sse_merge_xy( tmp2, tmp3 );
}

NOALIAS __forceinline void sse_transpose_matrix43(
    const __m128&            inX,
    const __m128&            inY,
    const __m128&            inZ,
    const __m128&            inW,
    __m128* const __restrict pOutX,
    __m128* const __restrict pOutY,
    __m128* const __restrict pOutZ )
{
    const __m128 tmp0( sse_merge_xy( inX, inZ ) );
    const __m128 tmp1( sse_merge_xy( inY, inW ) );
    const __m128 tmp2( sse_merge_zw( inX, inZ ) );
    const __m128 tmp3( sse_merge_zw( inY, inW ) );
    *pOutX = sse_merge_xy( tmp0, tmp1 );
    *pOutY = sse_merge_zw( tmp0, tmp1 );
    *pOutZ = sse_merge_xy( tmp2, tmp3 );
}

NOALIAS __forceinline void sse_transpose_matrix44(
    const __m128&            inX,
    const __m128&            inY,
    const __m128&            inZ,
    const __m128&            inW,
    __m128* const __restrict pOutX,
    __m128* const __restrict pOutY,
    __m128* const __restrict pOutZ,
    __m128* const __restrict pOutW )
{
    const __m128 tmp0( sse_merge_xy( inX, inZ ) );
    const __m128 tmp1( sse_merge_xy( inY, inW ) );
    const __m128 tmp2( sse_merge_zw( inX, inZ ) );
    const __m128 tmp3( sse_merge_zw( inY, inW ) );
    *pOutX = sse_merge_xy( tmp0, tmp1 );
    *pOutY = sse_merge_zw( tmp0, tmp1 );
    *pOutZ = sse_merge_xy( tmp2, tmp3 );
    *pOutW = sse_merge_zw( tmp2, tmp3 );
}


NOALIAS __forceinline const __m128 sse_transpose_col_x( const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW )
{
    return sse_merge_xy( sse_merge_xy( rowX, rowZ ),
                         sse_merge_xy( rowY, rowW ) );
}

NOALIAS __forceinline const __m128 sse_transpose_col_y( const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW )
{
    return sse_merge_zw( sse_merge_xy( rowX, rowZ ),
                         sse_merge_xy( rowY, rowW ) );
}

NOALIAS __forceinline const __m128 sse_transpose_col_z( const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW )
{
    return sse_merge_xy( sse_merge_zw( rowX, rowZ ),
                         sse_merge_zw( rowY, rowW ) );
}

NOALIAS __forceinline const __m128 sse_transpose_col_w( const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW )
{
    return sse_merge_zw( sse_merge_zw( rowX, rowZ ),
                         sse_merge_zw( rowY, rowW ) );
}



/// vector3 <-- vector3 [Implicit W=0] * matrix33
NOALIAS __forceinline const __m128 sse_multiply_vector3_matrix33( const __m128& v, const __m128& rowX, const __m128& rowY, const __m128& rowZ )
{
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v ), rowX ),
                                   _mm_mul_ps( sse_splat<VY>( v ), rowY ) ),
                                   _mm_mul_ps( sse_splat<VZ>( v ), rowZ ) );
}

/// vector3 <-- vector3 [Implicit W=0] * matrix43 [ Implicit column W=(0,0,0,1) ]
NOALIAS __forceinline const __m128 sse_multiply_vector3_matrix43( const __m128& v, const __m128& rowX, const __m128& rowY, const __m128& rowZ )
{
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v ), rowX ),
                                   _mm_mul_ps( sse_splat<VY>( v ), rowY ) ),
                                   _mm_mul_ps( sse_splat<VZ>( v ), rowZ ) );
}

/// vector4 <-- vector3 [Implicit W=0] * matrix44
NOALIAS __forceinline const __m128 sse_multiply_vector3_matrix44( const __m128& v, const __m128& rowX, const __m128& rowY, const __m128& rowZ )
{
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v ), rowX ),
                                   _mm_mul_ps( sse_splat<VY>( v ), rowY ) ),
                                   _mm_mul_ps( sse_splat<VZ>( v ), rowZ ) );
}



/// vector3 <-- matrix33 * vector3 [Implicit W=0]
NOALIAS __forceinline const __m128 sse_multiply_matrix33_vector3( const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& v )
{
    __m128 colX, colY, colZ;
    sse_transpose_matrix33( rowX, rowY, rowZ, &colX, &colY, &colZ );
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v ), colX ),
                                   _mm_mul_ps( sse_splat<VY>( v ), colY ) ),
                                   _mm_mul_ps( sse_splat<VZ>( v ), colZ ) );
}

/// vector4 <-- matrix43 [ Implicit column W=(0,0,0,1) ] * vector3 [Implicit W=0]
NOALIAS __forceinline const __m128 sse_multiply_matrix43_vector3( const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW, const __m128& v )
{
    __m128 colX, colY, colZ;
    sse_transpose_matrix43( rowX, rowY, rowZ, rowW, &colX, &colY, &colZ );
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v ), colX ),
                                   _mm_mul_ps( sse_splat<VY>( v ), colY ) ),
                                   _mm_mul_ps( sse_splat<VZ>( v ), colZ ) );
}

/// vector4 <-- matrix44 * vector3 [Implicit W=0]
NOALIAS __forceinline const __m128 sse_multiply_matrix44_vector3( const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW, const __m128& v )
{
    __m128 colX, colY, colZ, colW;
    sse_transpose_matrix44( rowX, rowY, rowZ, rowW, &colX, &colY, &colZ, &colW );
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v ), colX ),
                                   _mm_mul_ps( sse_splat<VY>( v ), colY ) ),
                                   _mm_mul_ps( sse_splat<VZ>( v ), colZ ) );
}


/// vector4 <-- vector4 * matrix33 [ Implicit row W = column W = (0,0,0,1) ]
NOALIAS __forceinline const __m128 sse_multiply_vector4_matrix33( const __m128& v, const __m128& rowX, const __m128& rowY, const __m128& rowZ )
{
    const __m128 result = _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v ), rowX ),
                          _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( v ), rowY ),
                                      _mm_mul_ps( sse_splat<VW>( v ), rowZ ) ) );
    return sse_select<0,0,0,1>( result, v );
}

/// vector4 <-- vector4 * matrix43 [ Implicit column W=(0,0,0,1) ]
NOALIAS __forceinline const __m128 sse_multiply_vector4_matrix43( const __m128& v, const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW )
{
    const __m128 result = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v ), rowX ),
                                                  _mm_mul_ps( sse_splat<VY>( v ), rowY ) ),
                                      _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( v ), rowZ ),
                                                  _mm_mul_ps( sse_splat<VW>( v ), rowW ) ) );
    return sse_select<0,0,0,1>( result, v );
}

/// vector4 <-- vector4 * matrix44
NOALIAS __forceinline const __m128 sse_multiply_vector4_matrix44( const __m128& v, const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW )
{
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v ), rowX ),
                                   _mm_mul_ps( sse_splat<VY>( v ), rowY ) ),
                       _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( v ), rowZ ),
                                   _mm_mul_ps( sse_splat<VW>( v ), rowW ) ) );
}



/// point3 <-- point3 [Implicit W=1] * matrix33
NOALIAS __forceinline const __m128 sse_multiply_point3_matrix33( const __m128& p, const __m128& rowX, const __m128& rowY, const __m128& rowZ )
{
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( p ), rowX ),
                                   _mm_mul_ps( sse_splat<VY>( p ), rowY ) ),
                                   _mm_mul_ps( sse_splat<VZ>( p ), rowZ ) );
}

/// point3 <-- point3 [Implicit W=1] * matrix43 [ Implicit column W=(0,0,0,1) ]
NOALIAS __forceinline const __m128 sse_multiply_point3_matrix43( const __m128& p, const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW )
{
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( p ), rowX ),
                                   _mm_mul_ps( sse_splat<VY>( p ), rowY ) ),
                       _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( p ), rowZ ),
                                                                   rowW ) );
}

/// vector4 <-- point3 [Implicit W=1] * matrix44
NOALIAS __forceinline const __m128 sse_multiply_point3_matrix44( const __m128& p, const __m128& rowX, const __m128& rowY, const __m128& rowZ, const __m128& rowW )
{
    return _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( p ), rowX ),
                                   _mm_mul_ps( sse_splat<VY>( p ), rowY ) ),
                       _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( p ), rowZ ),
                                                                   rowW ) );
}



/// SSE pack S16's to S8's with signed saturation
NOALIAS __forceinline const __m128i sse_pack_sat_s16_to_s8( const __m128i a, const __m128i b )
{
    return _mm_packs_epi16( a, b );
}

/// SSE pack S16's to U8's with unsigned saturation
NOALIAS __forceinline const __m128i sse_pack_sat_s16_to_u8( const __m128i a, const __m128i b )
{
    return _mm_packus_epi16( a, b );
}

/// SSE pack U16's to S8's with signed saturation
NOALIAS __forceinline const __m128i sse_pack_sat_u16_to_s8( const __m128i a, const __m128i b )
{
    // NOTE: the _mm_packus_epi16 intrinsic packs S16's to U8's so we need additional code
    // to handle the case where the source U16's have the msb (sign) bit set...
    const __m128i zero( Const<__m128i>::Zero() );
    const __m128i v127( _mm_set1_epi16( 127 ) );
    return _mm_packs_epi16( sse_select( a, v127, _mm_cmplt_epi16( a, zero ) ),
                            sse_select( b, v127, _mm_cmplt_epi16( b, zero ) ) );
}

/// SSE pack U16's to U8's with unsigned saturation
NOALIAS __forceinline const __m128i sse_pack_sat_u16_to_u8( const __m128i a, const __m128i b )
{
    // NOTE: the _mm_packus_epi16 intrinsic packs S16's to U8's so we need additional code
    // to handle the case where the source U16's have the msb (sign) bit set...
    const __m128i zero( Const<__m128i>::Zero() );
    const __m128i v255( _mm_set1_epi16( 255 ) );
    return _mm_packus_epi16( sse_select( a, v255, _mm_cmplt_epi16( a, zero ) ),
                             sse_select( b, v255, _mm_cmplt_epi16( b, zero ) ) );
}


/// SSE pack S16's/U16's to S8's/U8's (modulo)
//NOALIAS __forceinline const __m128i sse_pack_mod_u16_to_u8( const __m128i a, const __m128i b )
//{
//    // NOTE: the _mm_packus_epi16 intrinsic packs S16's to U8's with saturation so we need
//    // additional code to perform the modulo arithmetic first
//    const __m128i v255( _mm_set1_epi16( 255 ) );
//    return _mm_packus_epi16( _mm_and_si128( a, v255 ),
//                             _mm_and_si128( b, v255 ) );
//}






NOALIAS __forceinline const V128 SIMD_Scalar_Finite( const V128& x )
{
    const V128 abs_mask( (__m128&)c_VectorAbsMask );
    const V128 flt_max( c_VectorFltMax );
    const V128 abs_x( _mm_and_ps( x, abs_mask ) );
    return _mm_and_ps( _mm_cmpord_ss( x, x ), _mm_cmple_ss( abs_x, flt_max ) );
}

NOALIAS __forceinline const V128 SIMD_Scalar_IsNan( const V128& x )
{
    return _mm_cmpunord_ss( x, x );
}

NOALIAS __forceinline const V128 SIMD_Vector_Finite( const V128& x )
{
    const V128 abs_mask( (__m128&)c_VectorAbsMask );
    const V128 flt_max( c_VectorFltMax );
    const V128 abs_x( _mm_and_ps( x, abs_mask ) );
    return _mm_and_ps( _mm_cmpord_ps( x, x ), _mm_cmple_ps( abs_x, flt_max ) );
}

NOALIAS __forceinline const V128 SIMD_Vector_IsNan( const V128& x )
{
    return _mm_cmpunord_ps( x, x );
}



/// SSE Vector Reciprocal : 1 / B : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __m128 SIMD_Rcp( const __m128& B )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 rcp_B0( _mm_rcp_ps( B ) );
    //const V128 rcp_B( _mm_and_ps( rcp_B0, SIMD_Vector_Finite( rcp_B0 ) ) );
    //const V128 result( _mm_sub_ps( _mm_add_ps( rcp_B, rcp_B ), _mm_mul_ps( B, _mm_mul_ps( rcp_B, rcp_B ) ) ) );
    //return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#else
    const V128 rcp_B( _mm_rcp_ps( B ) );
    const V128 result( _mm_sub_ps( _mm_add_ps( rcp_B, rcp_B ), _mm_mul_ps( B, _mm_mul_ps( rcp_B, rcp_B ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// SSE Vector Divide : A / B : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __m128 SIMD_Div( const __m128& A, const __m128& B )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 rcp_B0( _mm_rcp_ps( B ) );
    //const V128 rcp_B( _mm_and_ps( rcp_B0, SIMD_Vector_Finite( rcp_B0 ) ) );
    //const V128 result( _mm_sub_ps( _mm_add_ps( rcp_B, rcp_B ), _mm_mul_ps( B, _mm_mul_ps( rcp_B, rcp_B ) ) ) );
    //return _mm_mul_ps( A, _mm_and_ps( result, SIMD_Vector_Finite( result ) ) );
    //#else
    const V128 rcp_B( _mm_rcp_ps( B ) );
    const V128 result( _mm_sub_ps( _mm_add_ps( rcp_B, rcp_B ), _mm_mul_ps( B, _mm_mul_ps( rcp_B, rcp_B ) ) ) );
    return _mm_mul_ps( A, _mm_and_ps( result, SIMD_Vector_Finite( result ) ) );
    //#endif
}

/// SSE Vector Reciprocal Square Root : 1 / sqrt( B ) : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __m128 SIMD_RcpSqrt( const __m128& B )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 finite_positive_B( _mm_max_ps( _mm_and_ps( B, SIMD_Vector_Finite( B ) ), _mm_setzero_ps() ) );
    //const V128 rcp_sqrt_B0( _mm_rsqrt_ps( finite_positive_B ) );
    //const V128 rcp_sqrt_B( _mm_and_ps( rcp_sqrt_B0, SIMD_Vector_Finite( rcp_sqrt_B0 ) ) );
    //const V128 result( _mm_mul_ps( _mm_mul_ps( rcp_sqrt_B, Const<__m128>::Half() ), _mm_sub_ps( Const<__m128>::Three(), _mm_mul_ps( B, _mm_mul_ps( rcp_sqrt_B, rcp_sqrt_B ) ) ) ) );
    //return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#else
    const V128 rcp_sqrt_B( _mm_rsqrt_ps( B ) );
    const V128 result( _mm_mul_ps( _mm_mul_ps( rcp_sqrt_B, Const<__m128>::Half() ), _mm_sub_ps( Const<__m128>::Three(), _mm_mul_ps( B, _mm_mul_ps( rcp_sqrt_B, rcp_sqrt_B ) ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// SSE Vector Reciprocal Square Root : 1 / sqrt( B ) : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __m128 SIMD_RcpSqrt( const __m128& B, const __m128& errorResult )
{
    const V128 rcp_sqrt_B( _mm_rsqrt_ps( B ) );
    const V128 result( _mm_mul_ps( _mm_mul_ps( rcp_sqrt_B, Const<__m128>::Half() ), _mm_sub_ps( Const<__m128>::Three(), _mm_mul_ps( B, _mm_mul_ps( rcp_sqrt_B, rcp_sqrt_B ) ) ) ) );
    return sse_select( errorResult, result, SIMD_Vector_Finite( result ) );
}

/// SSE Vector Square Root : sqrt( A ) : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __m128 SIMD_Sqrt( const __m128& A )
{
    return _mm_mul_ps( A, SIMD_RcpSqrt< T_PrecisionBits >( A ) );
}



/// SSE Vector Divide : A / B : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __m128 SIMD_UnsafeDivide( const __m128& A, const __m128& B )
{
    return _mm_div_ps( A, B );
}








/// Vector Mask Generator Template : Default
template < bool x, bool y, bool z, bool w >
FORCE_INLINE
const V128 SIMD_Mask()
{
    const V128 _1000( _mm_cmpeq_ss( _mm_setzero_ps(), _mm_setzero_ps() ) );
    return SIMD_Permute<!x,!y,!z,!w>( _1000 );
}

/// Vector Mask Generator Template : Specialisation for 0000
template <>
FORCE_INLINE
const V128 SIMD_Mask< 0, 0, 0, 0 >()
{
    const V128 _0000( _mm_setzero_ps() );
    return _0000;
}

/// Vector Mask Generator Template : Specialisation for 1000
template <>
FORCE_INLINE
const V128 SIMD_Mask< 1, 0, 0, 0 >()
{
    const V128 _1000( _mm_cmpeq_ss( _mm_setzero_ps(), _mm_setzero_ps() ) );
    return _1000;
}

/// Vector Mask Generator Template : Specialisation for 1111
template <>
FORCE_INLINE
const V128 SIMD_Mask< 1, 1, 1, 1 >()
{
    const V128 _1111( _mm_cmpeq_ps( _mm_setzero_ps(), _mm_setzero_ps() ) );
    return _1111;
}





/// Vector Masking Function Template
template < bool x, bool y, bool z, bool w >
__forceinline
const V128 SIMD_Mask( const V128& src )
{
    return SIMD_AND( src, SIMD_Mask<x,y,z,w>() );
}






/// SIMD Scalar/Vector Zero
FORCE_INLINE
const V128 SIMD_Zero()
{
    return _mm_setzero_ps();
}

/// SIMD Scalar F32 Constructor
FORCE_INLINE
const V128 SIMD_Scalar( const F32& f )
{
    return _mm_set_ss( f );
}

/// SIMD Scalar F32 Constructor
NOALIAS __forceinline const V128 SIMD_Scalar( F32 const * const __restrict ptr )
{
    return _mm_load_ss( ptr );
}

/// SIMD Scalar S32 Constructor
FORCE_INLINE
const V128 SIMD_Scalar( const S32& i )
{
    return _mm_cvtsi32_ss( _mm_setzero_ps(), i );
}

/// SIMD Scalar U32 Constructor
FORCE_INLINE
const V128 SIMD_Scalar( const U32& i )
{
    return _mm_cvtsi32_ss( _mm_setzero_ps(), i );
}





/// SIMD Scalar Vector2 X Constructor
FORCE_INLINE
const V128 SIMD_Scalar_Vector2X( const V128& x )
{
    return SIMD_Permute<VX,3,3,3>( x );
}

/// SIMD Scalar Vector2 Y Constructor
FORCE_INLINE
const V128 SIMD_Scalar_Vector2Y( const V128& y )
{
    return SIMD_Permute<VY,3,3,3>( y );
}

/// SIMD Scalar Vector3 X Constructor
FORCE_INLINE
const V128 SIMD_Scalar_Vector3X( const V128& x )
{
    return SIMD_Permute<VX,3,3,3>( x );
}

/// SIMD Scalar Vector3 Y Constructor
FORCE_INLINE
const V128 SIMD_Scalar_Vector3Y( const V128& y )
{
    return SIMD_Permute<VY,3,3,3>( y );
}

/// SIMD Scalar Vector3 Z Constructor
FORCE_INLINE
const V128 SIMD_Scalar_Vector3Z( const V128& z )
{
    return SIMD_Permute<VZ,3,3,3>( z );
}

/// SIMD Scalar Vector4 X Constructor
FORCE_INLINE
const V128 SIMD_Scalar_Vector4X( const V128& x )
{
    return SIMD_AND( x, SIMD_Mask<1,0,0,0>() );
}

/// SIMD Scalar Vector4 Y Constructor
FORCE_INLINE
const V128 SIMD_Scalar_Vector4Y( const V128& y )
{
    return SIMD_AND( SIMD_Splat4<VY>( y ), SIMD_Mask<1,0,0,0>() );
}

/// SIMD Scalar Vector4 Z Constructor
FORCE_INLINE
const V128 SIMD_Scalar_Vector4Z( const V128& z )
{
    return SIMD_AND( SIMD_Splat4<VZ>( z ), SIMD_Mask<1,0,0,0>() );
}

/// SIMD Scalar Vector4 W Constructor
FORCE_INLINE
const V128 SIMD_Scalar_Vector4W( const V128& w )
{
    return SIMD_AND( SIMD_Splat4<VW>( w ), SIMD_Mask<1,0,0,0>() );
}





/// SIMD Scalar VectorX Constructor
FORCE_INLINE
const V128 SIMD_Scalar_VectorX( const V128& x )
{
    return x;
}

/// SIMD Scalar VectorY Constructor
FORCE_INLINE
const V128 SIMD_Scalar_VectorY( const V128& y )
{
    return SIMD_Permute<VY,0,0,0>( y );
}

/// SIMD Scalar VectorZ Constructor
FORCE_INLINE
const V128 SIMD_Scalar_VectorZ( const V128& z )
{
    return SIMD_Permute<VZ,0,0,0>( z );
}

/// SIMD Scalar VectorW Constructor
FORCE_INLINE
const V128 SIMD_Scalar_VectorW( const V128& w )
{
    return SIMD_Permute<VW,0,0,0>( w );
}

/// SIMD Scalar Broadcast to XYZW
FORCE_INLINE
const V128 SIMD_ScalarBroadcastToXYZW( const V128& src )
{
    return SIMD_Splat4<VX>( src );
}

/// SIMD Scalar Cast to F32
FORCE_INLINE
const F32 SIMD_ScalarToF32( const V128& src )
{
    F32 f;
    _mm_store_ss( &f, src );
    return f;
}

/// SIMD Scalar Cast to F32
FORCE_INLINE
const S32 SIMD_ScalarToS32( const V128& src )
{
    //return _mm_cvtss_si32( src );
    return _mm_cvttss_si32( src );
}

/// SIMD Scalar Abs
FORCE_INLINE
const V128 SIMD_ScalarAbs( const V128& src )
{
    return _mm_and_ps( src, (__m128&)c_VectorAbsMask );
}

/// SIMD Scalar Sqrt
FORCE_INLINE
const V128 SIMD_ScalarSqrt( const V128& src )
{
    //return _mm_sqrt_ss( src );
    //return SIMD_Sqrt1( src );
    return _mm_mul_ps( src, SSE_RcpSqrtSS( src ) );
}

/// SIMD Scalar Reciprocal
FORCE_INLINE
const V128 SIMD_ScalarReciprocal( const V128& src )
{
    return SSE_RcpSS( src );
}

/// SIMD Scalar Reciprocal Sqrt
FORCE_INLINE
const V128 SIMD_ScalarReciprocalSqrt( const V128& src )
{
    return SSE_RcpSqrtSS( src );
}

/// SIMD Scalar Negate
FORCE_INLINE
const V128 SIMD_ScalarNegate( const V128& src )
{
    return _mm_xor_ps( src, (__m128&)c_VectorSignMask );
}

/// SIMD Scalar Add
FORCE_INLINE
const V128 SIMD_ScalarAdd( const V128& a, const V128& b )
{
    return _mm_add_ss( a, b );
}

/// SIMD Scalar Subtract
FORCE_INLINE
const V128 SIMD_ScalarSubtract( const V128& a, const V128& b )
{
    return _mm_sub_ss( a, b );
}

/// SIMD Scalar Multiply
FORCE_INLINE
const V128 SIMD_ScalarMultiply( const V128& a, const V128& b )
{
    return _mm_mul_ss( a, b );
}

/// SIMD Scalar Divide
FORCE_INLINE
const V128 SIMD_ScalarDivide( const V128& a, const V128& b )
{
    return _mm_mul_ss( a, SIMD_ScalarReciprocal( b ) );
}

/// SIMD Scalar Unsafe Divide
NOALIAS __forceinline const V128 SIMD_Scalar_UnsafeDivide( const V128& a, const V128& b )
{
    const V128 approx_rcp_b( _mm_rcp_ss( b ) );
    const V128 rcp_b( _mm_sub_ss( _mm_add_ss( approx_rcp_b, approx_rcp_b ), _mm_mul_ss( b, _mm_mul_ss( approx_rcp_b, approx_rcp_b ) ) ) );
    return _mm_mul_ss( a, rcp_b );
}

/// SIMD Scalar Min
FORCE_INLINE
const V128 SIMD_ScalarMin( const V128& a, const V128& b )
{
    return _mm_min_ss( a, b );
}

/// SIMD Scalar Max
FORCE_INLINE
const V128 SIMD_ScalarMax( const V128& a, const V128& b )
{
    return _mm_max_ss( a, b );
}

/// SIMD Scalar Compare EQ
FORCE_INLINE
const S32 SIMD_ScalarCompareEQ( const V128& a, const V128& b )
{
    return _mm_ucomieq_ss( a, b );
}

/// SIMD Scalar Compare NE
FORCE_INLINE
const S32 SIMD_ScalarCompareNE( const V128& a, const V128& b )
{
    return _mm_ucomineq_ss( a, b );
}

/// SIMD Scalar Compare LE
FORCE_INLINE
const S32 SIMD_ScalarCompareLE( const V128& a, const V128& b )
{
    return _mm_ucomile_ss( a, b );
}

/// SIMD Scalar Compare LT
FORCE_INLINE
const S32 SIMD_ScalarCompareLT( const V128& a, const V128& b )
{
    return _mm_ucomilt_ss( a, b );
}

/// SIMD Scalar Compare GE
FORCE_INLINE
const S32 SIMD_ScalarCompareGE( const V128& a, const V128& b )
{
    return _mm_ucomige_ss( a, b );
}

/// SIMD Scalar Compare GT
FORCE_INLINE
const S32 SIMD_ScalarCompareGT( const V128& a, const V128& b )
{
    return _mm_ucomigt_ss( a, b );
}

/// SIMD Scalar Comparison Selector EQ
FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorEQ( const V128& a, const V128& b )
{
    return C_SelectorScalar( SIMD_Splat4<VX>( _mm_cmpeq_ss( a, b ) ) );
}

/// SIMD Scalar Comparison Selector NE
FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorNE( const V128& a, const V128& b )
{
    return C_SelectorScalar( SIMD_Splat4<VX>( _mm_cmpneq_ss( a, b ) ) );
}

/// SIMD Scalar Comparison Selector LE
FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorLE( const V128& a, const V128& b )
{
    return C_SelectorScalar( SIMD_Splat4<VX>( _mm_cmple_ss( a, b ) ) );
}

/// SIMD Scalar Comparison Selector LT
FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorLT( const V128& a, const V128& b )
{
    return C_SelectorScalar( SIMD_Splat4<VX>( _mm_cmplt_ss( a, b ) ) );
}

/// SIMD Scalar Comparison Selector GE
FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorGE( const V128& a, const V128& b )
{
    return C_SelectorScalar( SIMD_Splat4<VX>( _mm_cmpge_ss( a, b ) ) );
}

/// SIMD Scalar Comparison Selector GT
FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorGT( const V128& a, const V128& b )
{
    return C_SelectorScalar( SIMD_Splat4<VX>( _mm_cmpgt_ss( a, b ) ) );
}

/// SIMD Scalar Selector Test : if ( selector == 0xFFFFFFFF ) returns 1 else returns 0
FORCE_INLINE
const U32 SIMD_TestScalarSelector( const V128& src )
{
	return _mm_movemask_ps( src ) & E_SSE_MaskBits_X;
}

/// SIMD Scalar FP Round To Integer : Towards minus infinity
//NOALIAS __forceinline const V128 SIMD_Scalar_RoundToIntMinusInf( const V128& src )
//{
//    #if defined( _SSE4_ )
//    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC );
//    #else
//    
//    const __m128 zero = _mm_setzero_ps();
//    const __m128 one = c_VectorOne;
//    const __m128 src_is_pos = _mm_cmpge_ps( src, zero );
//    
//    #if defined( _SSE2_ )
//    const __m128 src_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src ) );
//    const __m128 one_minus_src_trunc = _mm_sub_ps( one, src_trunc ); // If src -ve : Minimum integer value to add to src such that the sum is positive.
//    const __m128 src_neg_to_pos = _mm_add_ps( src, one_minus_src_trunc );
//	const __m128 src_neg_to_pos_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src_neg_to_pos ) );
//    const __m128 result_pos = src_trunc;
//    const __m128 result_neg = _mm_sub_ps( src_neg_to_pos_trunc, one_minus_src_trunc );
//    #else
//	const __m128 src_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src ),
//	                                           _mm_cvttps_pi32( _mm_movehl_ps( src, src ) ) );
//    const __m128 one_minus_src_trunc = _mm_sub_ps( one, src_trunc ); // If src -ve : Minimum integer value to add to src such that the sum is positive.
//    const __m128 src_neg_to_pos = _mm_add_ps( src, one_minus_src_trunc );
//	const __m128 src_neg_to_pos_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src_neg_to_pos ),
//	                                                      _mm_cvttps_pi32( _mm_movehl_ps( src_neg_to_pos, src_neg_to_pos ) ) );
//    const __m128 result_pos = src_trunc;
//    const __m128 result_neg = _mm_sub_ps( src_neg_to_pos_trunc, one_minus_src_trunc );
//	_m_empty();
//    #endif // #if defined( _SSE2_ )
//    
//    const __m128 result = _mm_or_ps( _mm_and_ps( src_is_pos, result_pos ), _mm_andnot_ps( src_is_pos, result_neg ) );
//    
//    #endif // #if defined( _SSE4_ )
//    return result;
//}

/// SIMD Scalar FP Round To Integer : Towards plus infinity
//NOALIAS __forceinline const V128 SIMD_Scalar_RoundToIntPlusInf( const V128& src )
//{
//    #if defined( _SSE4_ )
//    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC );
//    #else
//    
//    const __m128 zero = _mm_setzero_ps();
//    const __m128 one = c_VectorOne;
//    const __m128 src_is_pos = _mm_cmpge_ps( src, zero );
//    
//    #if defined( _SSE2_ )
//    const __m128 src_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src ) );
//    const __m128 one_plus_src_trunc = _mm_add_ps( one, src_trunc ); // If src +ve : Minimum integer value to subtract from src such that the sum is negative.
//    const __m128 src_pos_to_neg = _mm_sub_ps( src, one_plus_src_trunc );
//	const __m128 src_pos_to_neg_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src_pos_to_neg ) );
//    const __m128 result_pos = _mm_add_ps( src_pos_to_neg_trunc, one_plus_src_trunc );
//    const __m128 result_neg = src_trunc;
//    #else
//	const __m128 src_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src ),
//	                                           _mm_cvttps_pi32( _mm_movehl_ps( src, src ) ) );
//    const __m128 one_plus_src_trunc = _mm_add_ps( one, src_trunc ); // If src +ve : Minimum integer value to subtract from src such that the sum is negative.
//    const __m128 src_pos_to_neg = _mm_sub_ps( src, one_plus_src_trunc );
//	const __m128 src_pos_to_neg_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src_pos_to_neg ),
//	                                                      _mm_cvttps_pi32( _mm_movehl_ps( src_pos_to_neg, src_pos_to_neg ) ) );
//    const __m128 result_pos = _mm_add_ps( src_pos_to_neg_trunc, one_plus_src_trunc );
//    const __m128 result_neg = src_trunc;
//	_m_empty();
//    #endif // #if defined( _SSE2_ )
//    
//    const __m128 result = _mm_or_ps( _mm_and_ps( src_is_pos, result_pos ), _mm_andnot_ps( src_is_pos, result_neg ) );
//    
//    #endif // #if defined( _SSE4_ )
//    return result;
//}

/// SIMD Scalar FP Round To Integer : Towards nearest
//NOALIAS __forceinline const V128 SIMD_Scalar_RoundToIntNearest( const V128& src )
//{
//    #if defined( _SSE4_ )
//    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
//    #else
//    
//    const __m128 half = c_VectorHalf;
//    const __m128 abs_mask = c_VectorAbsMask;
//    const __m128 abs_src = _mm_and_ps( abs_mask, src );
//    const __m128 sign_result = _mm_andnot_ps( abs_mask, src );
//    const __m128 abs_src_plus_half = _mm_add_ps( abs_src, half );
//    
//    #if defined( _SSE2_ )
//    const __m128 abs_result = _mm_cvtepi32_ps( _mm_cvttps_epi32( abs_src_plus_half ) );
//    #else
//	const __m128 abs_result = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( abs_src_plus_half ),
//	                                            _mm_cvttps_pi32( _mm_movehl_ps( abs_src_plus_half, abs_src_plus_half ) ) );
//	_m_empty();
//    #endif // #if defined( _SSE2_ )
//    
//    const __m128 result = _mm_or_ps( sign_result, abs_result );
//    #endif // #if defined( _SSE4_ )
//    return result;
//}

/// SIMD Scalar FP Round To Integer : Towards zero
//NOALIAS __forceinline const V128 SIMD_Scalar_RoundToIntZero( const V128& src )
//{
//    #if defined( _SSE4_ )
//    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC );
//    #elif defined( _SSE2_ )
//    const __m128 result = _mm_cvtepi32_ps( _mm_cvttps_epi32( src ) );
//    #else
//    const __m128 result = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src ),
//                                            _mm_cvttps_pi32( _mm_movehl_ps( src, src ) ) );
//	_m_empty();
//    #endif
//    return result;
//}
























/// SIMD VectorX Constructor from float
FORCE_INLINE
const V128 SIMD_VectorX_Float( const F32& f )
{
    return _mm_setr_ps( f, 0.0f, 0.0f, 0.0f );
}

/// SIMD VectorX Constructor from Scalar
FORCE_INLINE
const V128 SIMD_VectorX_Scalar( const V128& f )
{
    return f;
}

/// SIMD VectorX Constructor from VectorY
FORCE_INLINE
const V128 SIMD_VectorX_VectorY( const V128& y )
{
    return SIMD_Permute<VY,3,3,3>( y );
}

/// SIMD VectorX Constructor from VectorZ
FORCE_INLINE
const V128 SIMD_VectorX_VectorZ( const V128& z )
{
    return SIMD_Permute<VZ,3,3,3>( z );
}

/// SIMD VectorX Constructor from VectorW
FORCE_INLINE
const V128 SIMD_VectorX_VectorW( const V128& w )
{
    return SIMD_Permute<VW,0,0,0>( w );
}

/// SIMD VectorX Constructor from Vector2
FORCE_INLINE
const V128 SIMD_VectorX_Vector2( const V128& src )
{
    return SIMD_AND( src, SIMD_Mask<1,0,0,0>() );
}

/// SIMD VectorX Constructor from Vector3
FORCE_INLINE
const V128 SIMD_VectorX_Vector3( const V128& src )
{
    return SIMD_AND( src, SIMD_Mask<1,0,0,0>() );
}



/// SIMD VectorY Constructor from float
FORCE_INLINE
const V128 SIMD_VectorY_Float( const F32& f )
{
    return _mm_setr_ps( 0.0f, f, 0.0f, 0.0f );
}

/// SIMD VectorY Constructor from Scalar
FORCE_INLINE
const V128 SIMD_VectorY_Scalar( const V128& f )
{
    return SIMD_Permute<3,VX,3,3>( f );
}

/// SIMD VectorY Constructor from VectorX
FORCE_INLINE
const V128 SIMD_VectorY_VectorX( const V128& x )
{
    return SIMD_Permute<3,VX,3,3>( x );
}

/// SIMD VectorY Constructor from VectorZ
FORCE_INLINE
const V128 SIMD_VectorY_VectorZ( const V128& z )
{
    return SIMD_Permute<3,VZ,3,3>( z );
}

/// SIMD VectorY Constructor from VectorW
FORCE_INLINE
const V128 SIMD_VectorY_VectorW( const V128& w )
{
    return SIMD_Permute<0,VW,0,0>( w );
}

/// SIMD VectorY Constructor from Vector2
FORCE_INLINE
const V128 SIMD_VectorY_Vector2( const V128& src )
{
    return SIMD_AND( src, SIMD_Mask<0,1,0,0>() );
}

/// SIMD VectorY Constructor from Vector3
FORCE_INLINE
const V128 SIMD_VectorY_Vector3( const V128& src )
{
    return SIMD_AND( src, SIMD_Mask<0,1,0,0>() );
}



/// SIMD VectorZ Constructor from float
FORCE_INLINE
const V128 SIMD_VectorZ_Float( const F32& f )
{
    return _mm_setr_ps( 0.0f, 0.0f, f, 0.0f );
}

/// SIMD VectorZ Constructor from Scalar
FORCE_INLINE
const V128 SIMD_VectorZ_Scalar( const V128& f )
{
    return SIMD_Permute<3,3,VX,3>( f );
}

/// SIMD VectorZ Constructor from VectorX
FORCE_INLINE
const V128 SIMD_VectorZ_VectorX( const V128& x )
{
    return SIMD_Permute<3,3,VX,3>( x );
}

/// SIMD VectorZ Constructor from VectorY
FORCE_INLINE
const V128 SIMD_VectorZ_VectorY( const V128& y )
{
    return SIMD_Permute<3,3,VY,3>( y );
}

/// SIMD VectorZ Constructor from VectorW
FORCE_INLINE
const V128 SIMD_VectorZ_VectorW( const V128& w )
{
    return SIMD_Permute<0,0,VW,0>( w );
}

/// SIMD VectorZ Constructor from Vector3
FORCE_INLINE
const V128 SIMD_VectorZ_Vector3( const V128& src )
{
    return SIMD_AND( src, SIMD_Mask<0,0,1,0>() );
}



/// SIMD VectorW Constructor from float
FORCE_INLINE
const V128 SIMD_VectorW_Float( const F32& f )
{
    return _mm_setr_ps( 0.0f, 0.0f, 0.0f, f );
}

/// SIMD VectorW Constructor from Scalar
FORCE_INLINE
const V128 SIMD_VectorW_Scalar( const V128& f )
{
    //return SIMD_Permute<3,3,3,VX>( f );
    return SIMD_AND( SIMD_Permute<VX,VX,VX,VX>( f ), SIMD_Mask<0,0,0,1>() );
}

/// SIMD VectorW Constructor from VectorX
FORCE_INLINE
const V128 SIMD_VectorW_VectorX( const V128& x )
{
    return SIMD_Permute<3,3,3,VX>( x );
}

/// SIMD VectorW Constructor from VectorY
FORCE_INLINE
const V128 SIMD_VectorW_VectorY( const V128& y )
{
    return SIMD_Permute<3,3,3,VY>( y );
}

/// SIMD VectorW Constructor from VectorZ
FORCE_INLINE
const V128 SIMD_VectorW_VectorZ( const V128& z )
{
    return SIMD_Permute<3,3,3,VZ>( z );
}

/// SIMD VectorW Constructor from Vector4
FORCE_INLINE
const V128 SIMD_VectorW_Vector4( const V128& src )
{
    return SIMD_AND( src, SIMD_Mask<0,0,0,1>() );
}












/// SIMD Vector Constructor
FORCE_INLINE
const V128 SIMD_Vector2( const F32& x, const F32& y )
{
    return _mm_setr_ps( x, y, 0.0f, 0.0f );
}

/// SIMD Vector Constructor
FORCE_INLINE
const V128 SIMD_Vector3( const F32& x, const F32& y, const F32& z )
{
    return _mm_setr_ps( x, y, z, 0.0f );
}

/// SIMD Vector Constructor
FORCE_INLINE
const V128 SIMD_Vector4( const F32& x, const F32& y, const F32& z, const F32& w )
{
    return _mm_setr_ps( x, y, z, w );
}

/// SIMD Vector2 Constructor: Source data all in X components!
FORCE_INLINE
const V128 SIMD_Vector2( const V128& x, const V128& y )
{
    return SIMD_MergeHigh( SIMD_MergeHigh( x, SIMD_Zero() ), SIMD_MergeHigh( y, SIMD_Zero() ) );
}

/// SIMD Vector3 Constructor: Source data all in X components!
FORCE_INLINE
const V128 SIMD_Vector3( const V128& x, const V128& y, const V128& z )
{
    return SIMD_MergeHigh( SIMD_MergeHigh( x, z ), SIMD_MergeHigh( y, SIMD_Zero() ) );
}

/// SIMD Vector4 Constructor: Source data all in X components!
FORCE_INLINE
const V128 SIMD_Vector4( const V128& x, const V128& y, const V128& z, const V128& w )
{
    return SIMD_MergeHigh( SIMD_MergeHigh( x, z ), SIMD_MergeHigh( y, w ) );
}

/// SIMD Vector2 Scalar Constructor: Source data in HF32 format [ SSE - X component ]
FORCE_INLINE
const V128 SIMD_Vector2_Scalar( const V128& xy )
{
    return SIMD_Splat2<VX>( xy );
}

/// SIMD Vector3 Scalar Constructor: Source data in HF32 format [ SSE - X component ]
FORCE_INLINE
const V128 SIMD_Vector3_Scalar( const V128& xyz )
{
    return SIMD_Splat3<VX>( xyz );
}

/// SIMD Vector4 Scalar Constructor: Source data in HF32 format [ SSE - X component ]
FORCE_INLINE
const V128 SIMD_Vector4_Scalar( const V128& xyzw )
{
    return SIMD_Splat4<VX>( xyzw );
}

/// SIMD Vector4 Constructor from Vector3 & VectorW
__forceinline
const V128 SIMD_Vector4_Vector3W( const V128& vec3, const V128& vecW )
{
    return SIMD_Select( SIMD_Mask<1,1,1,0>(), vec3, vecW );
}

/// SIMD Vector4 Constructor from VectorW
__forceinline
const V128 SIMD_Vector4_VectorW( const V128& vecW )
{
    return SIMD_Select( SIMD_Mask<1,1,1,0>(), SIMD_Zero(), vecW );
}

/// SIMD Vector3 Constructor from Vector4
FORCE_INLINE
const V128 SIMD_Vector3_Vector4( const V128& vec4 )
{
    return _mm_and_ps( vec4, SIMD_Mask<1,1,1,0>() );
}

/// SIMD Vector Truncate To Int
//FORCE_INLINE
//const V4_S32 SIMD_TruncateToInt( const V128& a )
//{
//    V4_S32 result;
//    result.m_XY = _mm_cvttps_pi32( a ); // Truncate
//    result.m_ZW = _mm_cvttps_pi32( _mm_movehl_ps( a, a ) );
//    _m_empty();
//    return result;
//}

/// SIMD Vector Unaligned Load
FORCE_INLINE
const V128 SIMD_LoadUnaligned( const F32* ptr )
{
    return _mm_loadu_ps( ptr );
}

/// SIMD Vector Unaligned Store
FORCE_INLINE
void SIMD_StoreUnaligned( const V128& a, F32* ptr )
{
    _mm_storeu_ps( ptr, a );
}

/// SIMD Select: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
FORCE_INLINE
const V128 SIMD_Select( const V128& selector, const V128& a, const V128& b )
{
    return SIMD_OR( SIMD_AND( a, selector ), SIMD_ANDNOT( b, selector ) );
    
    //#if !defined( _SIMD_ )
    //m_F = ( ((U32&)selector.m_V) & ((U32&)a.m_F) ) | ( ~((U32&)selector.m_V) & ((U32&)b.m_F) );
    //#endif
}

/// SIMD Vector Permute
template < int ax, int ay, int az, int aw >
FORCE_INLINE
const V128 SIMD_Permute( const V128& a )
{
    //return _mm_shuffle_ps( a, a, _MM_SHUFFLE( aw, az, ay, ax ) );
    return (__m128&)_mm_shuffle_epi32( (__m128i&)a, _MM_SHUFFLE( aw, az, ay, ax ) );
}

/// SIMD Vector Permute : 2 Sources
template < int ax, int ay, int bz, int bw >
FORCE_INLINE
const V128 SIMD_Permute( const V128& a, const V128& b )
{
    return _mm_shuffle_ps( a, b, _MM_SHUFFLE( bw, bz, ay, ax ) );
}

/// SIMD Vector Permute : 3 Sources
template < int ax, int by, int cz >
FORCE_INLINE
const V128 SIMD_Permute( const V128& a, const V128& b, const V128& c )
{
    return SIMD_Permute< 0, 2, 0, 2 >( SIMD_Permute< ax, ax, by, by >( a, b ), SIMD_Permute< cz, cz, 0, 0 >( c, _mm_setzero_ps() ) );
}

/// SIMD Vector Permute : 3 Sources
template < int ax, int by, int cz, int cw >
FORCE_INLINE
const V128 SIMD_Permute( const V128& a, const V128& b, const V128& c )
{
    return SIMD_Permute< 0, 2, cz, cw >( SIMD_Permute< ax, ax, by, by >( a, b ), c );
}

/// SIMD Vector Permute : 4 Sources
template < int ax, int by, int cz, int dw >
FORCE_INLINE
const V128 SIMD_Permute( const V128& a, const V128& b, const V128& c, const V128& d )
{
    return SIMD_Permute< 0, 2, 0, 2 >( SIMD_Permute< ax, ax, by, by >( a, b ), SIMD_Permute< cz, cz, dw, dw >( c, d ) );
}

/// SIMD Vector Splat2: Replicate component I to XY - Z&W unchanged
template < int I >
FORCE_INLINE
const V128 SIMD_Splat2( const V128& a )
{
    return SIMD_Permute<I,I,VZ,VW>( a );
}

/// SIMD Vector Splat3: Replicate component I to XYZ - W unchanged
template < int I >
FORCE_INLINE
const V128 SIMD_Splat3( const V128& a )
{
    return SIMD_Permute<I,I,I,VW>( a );
}

/// SIMD Vector Splat4: Replicate component I to XYZW
template < int I >
FORCE_INLINE
const V128 SIMD_Splat4( const V128& a )
{
    return SIMD_Permute<I,I,I,I>( a );
}

/// SIMD Vector Merge High Words
FORCE_INLINE
const V128 SIMD_MergeHigh( const V128& a, const V128& b )
{
    return _mm_unpacklo_ps( a, b );
}

/// SIMD Vector Merge Low Words
FORCE_INLINE
const V128 SIMD_MergeLow( const V128& a, const V128& b )
{
    return _mm_unpackhi_ps( a, b );
}

/// SIMD Vector Set X : Returns src with X set to scalar HF32
FORCE_INLINE
const V128 SIMD_SetX( const V128& src, const V128& scalar )
{
    return SIMD_Permute<0,2,2,3>( SIMD_Permute<0,0,1,1>( scalar, src ), src );
}

/// SIMD Vector Set Y : Returns src with Y set to scalar HF32
FORCE_INLINE
const V128 SIMD_SetY( const V128& src, const V128& scalar )
{
    return SIMD_Permute<0,2,2,3>( SIMD_Permute<0,0,0,0>( src, scalar ), src );
}

/// SIMD Vector Set Z : Returns src with Z set to scalar HF32
FORCE_INLINE
const V128 SIMD_SetZ( const V128& src, const V128& scalar )
{
    return SIMD_Permute<0,1,0,2>( src, SIMD_Permute<0,0,3,3>( scalar, src ) );
}

/// SIMD Vector Set W : Returns src with W set to scalar HF32
FORCE_INLINE
const V128 SIMD_SetW( const V128& src, const V128& scalar )
{
    return SIMD_Permute<0,1,0,2>( src, SIMD_Permute<2,2,0,0>( src, scalar ) );
}

/// SIMD Vector Bitwise NOT
FORCE_INLINE
const V128 SIMD_NOT( const V128& a )
{
    return _mm_xor_ps( a, SIMD_Mask<1,1,1,1>() );
}

/// SIMD Vector Bitwise AND
FORCE_INLINE
const V128 SIMD_AND( const V128& a, const V128& b )
{
    return _mm_and_ps( a, b );
}

/// SIMD Vector Bitwise ANDNOT: a & ~b
FORCE_INLINE
const V128 SIMD_ANDNOT( const V128& a, const V128& b )
{
    return _mm_andnot_ps( b, a );
}

/// SIMD Vector Bitwise OR
FORCE_INLINE
const V128 SIMD_OR( const V128& a, const V128& b )
{
    return _mm_or_ps( a, b );
}

/// SIMD Vector Bitwise XOR
FORCE_INLINE
const V128 SIMD_XOR( const V128& a, const V128& b )
{
    return _mm_xor_ps( a, b );
}

/// SIMD Vector FP Abs
NOALIAS __forceinline const V128 SIMD_Abs( const V128& src )
{
    return _mm_and_ps( src, (__m128&)c_VectorAbsMask );
}

/// SIMD Vector FP Add
NOALIAS __forceinline const V128 SIMD_Add( const V128& a, const V128& b )
{
    return _mm_add_ps( a, b );
}

/// SIMD Vector FP Subtract
NOALIAS __forceinline const V128 SIMD_Subtract( const V128& a, const V128& b )
{
    return _mm_sub_ps( a, b );
}

/// SIMD Vector FP Multiply
NOALIAS __forceinline const V128 SIMD_Multiply( const V128& a, const V128& b )
{
    return _mm_mul_ps( a, b );
}

/// SIMD Vector FP Multiply Add : A * B + C
NOALIAS __forceinline const V128 SIMD_MultiplyAdd( const V128& a, const V128& b, const V128& c )
{
    return _mm_add_ps( _mm_mul_ps( a, b ), c );
}

/// SIMD Vector FP Subtract Multiply : A - B * C
NOALIAS __forceinline const V128 SIMD_SubtractMultiply( const V128& a, const V128& b, const V128& c )
{
    return _mm_sub_ps( a, _mm_mul_ps( b, c ) );
}

/// SIMD Vector FP Dot Product 2 - Result is HF32 [ SSE : X component ]
NOALIAS __forceinline const V128 SIMD_DotProduct2( const V128& a, const V128& b )
{
    const V128 ab = _mm_mul_ps( a, b );
    //return _mm_and_ps( _mm_add_ss( ab, SIMD_Splat4<VY>( ab ) ), SIMD_Mask<1,0,0,0>() );
    return _mm_add_ps( SIMD_Splat4<VX>( ab ), SIMD_Splat4<VY>( ab ) );
}

/// SIMD Vector FP Dot Product 3 - Result is HF32 [ SSE : X component ]
NOALIAS __forceinline const V128 SIMD_DotProduct3( const V128& a, const V128& b )
{
    const V128 ab = _mm_mul_ps( a, b );
    //return _mm_and_ps( _mm_add_ss( _mm_add_ss( ab, SIMD_Splat4<VY>( ab ) ), SIMD_Splat4<VZ>( ab ) ), SIMD_Mask<1,0,0,0>() );
    return _mm_add_ps( _mm_add_ps( SIMD_Splat4<VX>( ab ), SIMD_Splat4<VY>( ab ) ), SIMD_Splat4<VZ>( ab ) );
}

/// SIMD Vector FP Dot Product 4 - Result is HF32 [ SSE : X component ]
NOALIAS __forceinline const V128 SIMD_DotProduct4( const V128& a, const V128& b )
{
    const V128 ab = _mm_mul_ps( a, b );
    //return _mm_and_ps( _mm_add_ss( _mm_add_ss( ab, SIMD_Splat4<VY>( ab ) ), _mm_add_ss( SIMD_Splat4<VZ>( ab ), SIMD_Splat4<VW>( ab ) ) ), SIMD_Mask<1,0,0,0>() );
    return _mm_add_ps( _mm_add_ps( SIMD_Splat4<VX>( ab ), SIMD_Splat4<VY>( ab ) ), _mm_add_ps( SIMD_Splat4<VZ>( ab ), SIMD_Splat4<VW>( ab ) ) );
}

/// SIMD Vector FP Cross Product
NOALIAS __forceinline const V128 SIMD_CrossProduct( const V128& a, const V128& b )
{
    static const unsigned __ = VW;
    const V128 a_yzx = SIMD_Permute<VY,VZ,VX,__>( a );
    const V128 a_zxy = SIMD_Permute<VZ,VX,VY,__>( a );
    const V128 b_yzx = SIMD_Permute<VY,VZ,VX,__>( b );
    const V128 b_zxy = SIMD_Permute<VZ,VX,VY,__>( b );
    return SIMD_SubtractMultiply( SIMD_Multiply( a_yzx, b_zxy ), a_zxy, b_yzx );
}

/// SIMD Vector FP Sum of Elements 3 - Result is HF32 [ SSE : X component ]
NOALIAS __forceinline const V128 SIMD_Sum2( const V128& a )
{
    return _mm_add_ss( a, SIMD_Splat4<VY>( a ) );
}

/// SIMD Vector FP Sum of Elements 3 - Result is HF32 [ SSE : X component ]
NOALIAS __forceinline const V128 SIMD_Sum3( const V128& a )
{
    return _mm_add_ss( _mm_add_ss( a, SIMD_Splat4<VY>( a ) ), SIMD_Splat4<VZ>( a ) );
}

/// SIMD Vector FP Sum of Elements 4 - Result is HF32 [ SSE : X component ]
NOALIAS __forceinline const V128 SIMD_Sum4( const V128& a )
{
    return _mm_add_ss( _mm_add_ss( a, SIMD_Splat4<VY>( a ) ), _mm_add_ss( SIMD_Splat4<VZ>( a ), SIMD_Splat4<VW>( a ) ) );
}

/// SIMD Vector FP Min
NOALIAS __forceinline const V128 SIMD_Min( const V128& a, const V128& b )
{
    return _mm_min_ps( a, b );
}

/// SIMD Vector FP Max
NOALIAS __forceinline const V128 SIMD_Max( const V128& a, const V128& b )
{
    return _mm_max_ps( a, b );
}

/// SIMD Vector Reciprocal X
NOALIAS __forceinline const V128 SIMD_Reciprocal1( const V128& a )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 rcp_a0( _mm_rcp_ss( a ) );
    //const V128 rcp_a( _mm_and_ps( rcp_a0, SIMD_Vector_Finite( rcp_a0 ) ) );
    //const V128 result( _mm_sub_ss( _mm_add_ss( rcp_a, rcp_a ), _mm_mul_ss( a, _mm_mul_ss( rcp_a, rcp_a ) ) ) );
    //return _mm_and_ps( result, SIMD_Scalar_Finite( result ) );
    //#else
    const V128 rcp_a( _mm_rcp_ss( a ) );
    const V128 result( _mm_sub_ss( _mm_add_ss( rcp_a, rcp_a ), _mm_mul_ss( a, _mm_mul_ss( rcp_a, rcp_a ) ) ) );
    return _mm_and_ps( result, SIMD_Scalar_Finite( result ) );
    //#endif
}

/// SIMD Vector Reciprocal XY
NOALIAS __forceinline const V128 SIMD_Reciprocal2( const V128& a )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 rcp_a0( _mm_rcp_ps( a ) );
    //const V128 rcp_a( _mm_and_ps( rcp_a0, SIMD_Vector_Finite( rcp_a0 ) ) );
    //const V128 result( _mm_sub_ps( _mm_add_ps( rcp_a, rcp_a ), _mm_mul_ps( a, _mm_mul_ps( rcp_a, rcp_a ) ) ) );
    //return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#else
    const V128 rcp_a( _mm_rcp_ps( a ) );
    const V128 result( _mm_sub_ps( _mm_add_ps( rcp_a, rcp_a ), _mm_mul_ps( a, _mm_mul_ps( rcp_a, rcp_a ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// SIMD Vector Reciprocal XYZ
NOALIAS __forceinline const V128 SIMD_Reciprocal3( const V128& a )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 rcp_a0( _mm_rcp_ps( a ) );
    //const V128 rcp_a( _mm_and_ps( rcp_a0, SIMD_Vector_Finite( rcp_a0 ) ) );
    //const V128 result( _mm_sub_ps( _mm_add_ps( rcp_a, rcp_a ), _mm_mul_ps( a, _mm_mul_ps( rcp_a, rcp_a ) ) ) );
    //return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#else
    const V128 rcp_a( _mm_rcp_ps( a ) );
    const V128 result( _mm_sub_ps( _mm_add_ps( rcp_a, rcp_a ), _mm_mul_ps( a, _mm_mul_ps( rcp_a, rcp_a ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// SIMD Vector Reciprocal XYZW
NOALIAS __forceinline const V128 SIMD_Reciprocal4( const V128& a )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 rcp_a0( _mm_rcp_ps( a ) );
    //const V128 rcp_a( _mm_and_ps( rcp_a0, SIMD_Vector_Finite( rcp_a0 ) ) );
    //const V128 result( _mm_sub_ps( _mm_add_ps( rcp_a, rcp_a ), _mm_mul_ps( a, _mm_mul_ps( rcp_a, rcp_a ) ) ) );
    //return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#else
    const V128 rcp_a( _mm_rcp_ps( a ) );
    const V128 result( _mm_sub_ps( _mm_add_ps( rcp_a, rcp_a ), _mm_mul_ps( a, _mm_mul_ps( rcp_a, rcp_a ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// SIMD Vector Reciprocal Square Root X
NOALIAS __forceinline const V128 SIMD_ReciprocalSqrt1( const V128& a )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 finite_positive_a( _mm_max_ps( _mm_and_ps( a, SIMD_Vector_Finite( a ) ), _mm_setzero_ps() ) );
    //const V128 rcp_sqrt_a0( _mm_rsqrt_ss( finite_positive_a ) );
    //const V128 rcp_sqrt_a( _mm_and_ps( rcp_sqrt_a0, SIMD_Vector_Finite( rcp_sqrt_a0 ) ) );
    //return _mm_mul_ss( _mm_mul_ss( rcp_sqrt_a, c_VectorHalf ), _mm_sub_ss( c_VectorThree, _mm_mul_ss( a, _mm_mul_ss( rcp_sqrt_a, rcp_sqrt_a ) ) ) );
    //#else
    const V128 rcp_sqrt_a( _mm_rsqrt_ss( a ) );
    const V128 result( _mm_mul_ss( _mm_mul_ss( rcp_sqrt_a, c_VectorHalf ), _mm_sub_ss( c_VectorThree, _mm_mul_ss( a, _mm_mul_ss( rcp_sqrt_a, rcp_sqrt_a ) ) ) ) );
    return _mm_and_ps( result, SIMD_Scalar_Finite( result ) );
    //#endif
}

/// SIMD Vector Reciprocal Square Root XY
NOALIAS __forceinline const V128 SIMD_ReciprocalSqrt2( const V128& a )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 finite_positive_a( _mm_max_ps( _mm_and_ps( a, SIMD_Vector_Finite( a ) ), _mm_setzero_ps() ) );
    //const V128 rcp_sqrt_a0( _mm_rsqrt_ps( finite_positive_a ) );
    //const V128 rcp_sqrt_a( _mm_and_ps( rcp_sqrt_a0, SIMD_Vector_Finite( rcp_sqrt_a0 ) ) );
    //return _mm_mul_ps( _mm_mul_ps( rcp_sqrt_a, c_VectorHalf ), _mm_sub_ps( c_VectorThree, _mm_mul_ps( a, _mm_mul_ps( rcp_sqrt_a, rcp_sqrt_a ) ) ) );
    //#else
    const V128 rsa( _mm_rsqrt_ps( a ) );
    const V128 result( _mm_mul_ps( _mm_mul_ps( rsa, c_VectorHalf ), _mm_sub_ps( c_VectorThree, _mm_mul_ps( a, _mm_mul_ps( rsa, rsa ) ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// SIMD Vector Reciprocal Square Root XYZ
NOALIAS __forceinline const V128 SIMD_ReciprocalSqrt3( const V128& a )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 finite_positive_a( _mm_max_ps( _mm_and_ps( a, SIMD_Vector_Finite( a ) ), _mm_setzero_ps() ) );
    //const V128 rcp_sqrt_a0( _mm_rsqrt_ps( finite_positive_a ) );
    //const V128 rcp_sqrt_a( _mm_and_ps( rcp_sqrt_a0, SIMD_Vector_Finite( rcp_sqrt_a0 ) ) );
    //return _mm_mul_ps( _mm_mul_ps( rcp_sqrt_a, c_VectorHalf ), _mm_sub_ps( c_VectorThree, _mm_mul_ps( a, _mm_mul_ps( rcp_sqrt_a, rcp_sqrt_a ) ) ) );
    //#else
    const V128 rsa( _mm_rsqrt_ps( a ) );
    const V128 result( _mm_mul_ps( _mm_mul_ps( rsa, c_VectorHalf ), _mm_sub_ps( c_VectorThree, _mm_mul_ps( a, _mm_mul_ps( rsa, rsa ) ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// SIMD Vector Reciprocal Square Root XYZW
NOALIAS __forceinline const V128 SIMD_ReciprocalSqrt4( const V128& a )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 finite_positive_a( _mm_max_ps( _mm_and_ps( a, SIMD_Vector_Finite( a ) ), _mm_setzero_ps() ) );
    //const V128 rcp_sqrt_a0( _mm_rsqrt_ps( finite_positive_a ) );
    //const V128 rcp_sqrt_a( _mm_and_ps( rcp_sqrt_a0, SIMD_Vector_Finite( rcp_sqrt_a0 ) ) );
    //return _mm_mul_ps( _mm_mul_ps( rcp_sqrt_a, c_VectorHalf ), _mm_sub_ps( c_VectorThree, _mm_mul_ps( a, _mm_mul_ps( rcp_sqrt_a, rcp_sqrt_a ) ) ) );
    //#else
    const V128 rsa( _mm_rsqrt_ps( a ) );
    const V128 result( _mm_mul_ps( _mm_mul_ps( rsa, c_VectorHalf ), _mm_sub_ps( c_VectorThree, _mm_mul_ps( a, _mm_mul_ps( rsa, rsa ) ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// SIMD Vector Square Root X
NOALIAS __forceinline const V128 SIMD_Sqrt1( const V128& a )
{
    return _mm_mul_ss( a, SIMD_ReciprocalSqrt1( a ) );
}

/// SIMD Vector Square Root XY
NOALIAS __forceinline const V128 SIMD_Sqrt2( const V128& a )
{
    return _mm_mul_ps( a, SIMD_ReciprocalSqrt2( a ) );
}

/// SIMD Vector Square Root XYZ
NOALIAS __forceinline const V128 SIMD_Sqrt3( const V128& a )
{
    return _mm_mul_ps( a, SIMD_ReciprocalSqrt3( a ) );
}

/// SIMD Vector Square Root XYZW
NOALIAS __forceinline const V128 SIMD_Sqrt4( const V128& a )
{
    return _mm_mul_ps( a, SIMD_ReciprocalSqrt4( a ) );
}

/// SIMD Vector Normalise XY
NOALIAS __forceinline const V128 SIMD_Normalise2( const V128& a )
{
    return SIMD_Multiply( a, SIMD_ReciprocalSqrt2( SIMD_DotProduct2( a, a ) ) );
}

/// SIMD Vector Normalise XYZ
NOALIAS __forceinline const V128 SIMD_Normalise3( const V128& a )
{
    return SIMD_Multiply( a, SIMD_ReciprocalSqrt3( SIMD_DotProduct3( a, a ) ) );
}

/// SIMD Vector Normalise XYZW
NOALIAS __forceinline const V128 SIMD_Normalise4( const V128& a )
{
    return SIMD_Multiply( a, SIMD_ReciprocalSqrt4( SIMD_DotProduct4( a, a ) ) );
}

/// SIMD Vector Compare ==
NOALIAS __forceinline const V128 SIMD_Vector_Selector_EQ( const V128& a, const V128& b )
{
    return _mm_cmpeq_ps( a, b );
}

/// SIMD Vector Compare !=
NOALIAS __forceinline const V128 SIMD_Vector_Selector_NE( const V128& a, const V128& b )
{
    return _mm_cmpneq_ps( a, b );
}

/// SIMD Vector Compare >=
NOALIAS __forceinline const V128 SIMD_Vector_Selector_GE( const V128& a, const V128& b )
{
    return _mm_cmpge_ps( a, b );
}

/// SIMD Vector Compare >
NOALIAS __forceinline const V128 SIMD_Vector_Selector_GT( const V128& a, const V128& b )
{
    return _mm_cmpgt_ps( a, b );
}

/// SIMD Vector Compare <=
NOALIAS __forceinline const V128 SIMD_Vector_Selector_LE( const V128& a, const V128& b )
{
    return _mm_cmple_ps( a, b );
}

/// SIMD Vector Compare <
NOALIAS __forceinline const V128 SIMD_Vector_Selector_LT( const V128& a, const V128& b )
{
    return _mm_cmplt_ps( a, b );
}






/// SIMD Vector3 FP Round To Integer : Towards minus infinity
//NOALIAS __forceinline const V128 SIMD_Vector3_RoundToIntMinusInf( const V128& src )
//{
//    #if defined( _SSE4_ )
//    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC );
//    #else
//    
//    const __m128 zero = _mm_setzero_ps();
//    const __m128 one = c_VectorOne;
//    const __m128 src_is_pos = _mm_cmpge_ps( src, zero );
//    
//    #if defined( _SSE2_ )
//    const __m128 src_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src ) );
//    const __m128 one_minus_src_trunc = _mm_sub_ps( one, src_trunc ); // If src -ve : Minimum integer value to add to src such that the sum is positive.
//    const __m128 src_neg_to_pos = _mm_add_ps( src, one_minus_src_trunc );
//    const __m128 src_neg_to_pos_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src_neg_to_pos ) );
//    const __m128 result_pos = src_trunc;
//    const __m128 result_neg = _mm_sub_ps( src_neg_to_pos_trunc, one_minus_src_trunc );
//    #else
//    const __m128 src_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src ),
//                                               _mm_cvttps_pi32( _mm_movehl_ps( src, src ) ) );
//    const __m128 one_minus_src_trunc = _mm_sub_ps( one, src_trunc ); // If src -ve : Minimum integer value to add to src such that the sum is positive.
//    const __m128 src_neg_to_pos = _mm_add_ps( src, one_minus_src_trunc );
//    const __m128 src_neg_to_pos_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src_neg_to_pos ),
//                                                          _mm_cvttps_pi32( _mm_movehl_ps( src_neg_to_pos, src_neg_to_pos ) ) );
//    const __m128 result_pos = src_trunc;
//    const __m128 result_neg = _mm_sub_ps( src_neg_to_pos_trunc, one_minus_src_trunc );
//    _m_empty();
//    #endif // #if defined( _SSE2_ )
//    
//    const __m128 result = _mm_or_ps( _mm_and_ps( src_is_pos, result_pos ), _mm_andnot_ps( src_is_pos, result_neg ) );
//    
//    #endif // #if defined( _SSE4_ )
//    return result;
//}

/// SIMD Vector3 FP Round To Integer : Towards plus infinity
//NOALIAS __forceinline const V128 SIMD_Vector3_RoundToIntPlusInf( const V128& src )
//{
//    #if defined( _SSE4_ )
//    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC );
//    #else
//    
//    const __m128 zero = _mm_setzero_ps();
//    const __m128 one = c_VectorOne;
//    const __m128 src_is_pos = _mm_cmpge_ps( src, zero );
//    
//    #if defined( _SSE2_ )
//    const __m128 src_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src ) );
//    const __m128 one_plus_src_trunc = _mm_add_ps( one, src_trunc ); // If src +ve : Minimum integer value to subtract from src such that the sum is negative.
//    const __m128 src_pos_to_neg = _mm_sub_ps( src, one_plus_src_trunc );
//    const __m128 src_pos_to_neg_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src_pos_to_neg ) );
//    const __m128 result_pos = _mm_add_ps( src_pos_to_neg_trunc, one_plus_src_trunc );
//    const __m128 result_neg = src_trunc;
//    #else
//    const __m128 src_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src ),
//                                               _mm_cvttps_pi32( _mm_movehl_ps( src, src ) ) );
//    const __m128 one_plus_src_trunc = _mm_add_ps( one, src_trunc ); // If src +ve : Minimum integer value to subtract from src such that the sum is negative.
//    const __m128 src_pos_to_neg = _mm_sub_ps( src, one_plus_src_trunc );
//    const __m128 src_pos_to_neg_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src_pos_to_neg ),
//                                                          _mm_cvttps_pi32( _mm_movehl_ps( src_pos_to_neg, src_pos_to_neg ) ) );
//    const __m128 result_pos = _mm_add_ps( src_pos_to_neg_trunc, one_plus_src_trunc );
//    const __m128 result_neg = src_trunc;
//    _m_empty();
//    #endif // #if defined( _SSE2_ )
//    
//    const __m128 result = _mm_or_ps( _mm_and_ps( src_is_pos, result_pos ), _mm_andnot_ps( src_is_pos, result_neg ) );
//    
//    #endif // #if defined( _SSE4_ )
//    return result;
//}

/// SIMD Vector3 FP Round To Integer : Towards nearest
//NOALIAS __forceinline const V128 SIMD_Vector3_RoundToIntNearest( const V128& src )
//{
//    #if defined( _SSE4_ )
//    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
//    #else
//    
//    const __m128 half = c_VectorHalf;
//    const __m128 abs_mask = c_VectorAbsMask;
//    const __m128 abs_src = _mm_and_ps( abs_mask, src );
//    const __m128 sign_result = _mm_andnot_ps( abs_mask, src );
//    const __m128 abs_src_plus_half = _mm_add_ps( abs_src, half );
//    
//    #if defined( _SSE2_ )
//    const __m128 abs_result = _mm_cvtepi32_ps( _mm_cvttps_epi32( abs_src_plus_half ) );
//    #else
//    const __m128 abs_result = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( abs_src_plus_half ),
//                                                _mm_cvttps_pi32( _mm_movehl_ps( abs_src_plus_half, abs_src_plus_half ) ) );
//    _m_empty();
//    #endif // #if defined( _SSE2_ )
//    
//    const __m128 result = _mm_or_ps( sign_result, abs_result );
//    #endif // #if defined( _SSE4_ )
//    return result;
//}

/// SIMD Vector3 FP Round To Integer : Towards zero
//NOALIAS __forceinline const V128 SIMD_Vector3_RoundToIntZero( const V128& src )
//{
//    #if defined( _SSE4_ )
//    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC );
//    #elif defined( _SSE2_ )
//    const __m128 result = _mm_cvtepi32_ps( _mm_cvttps_epi32( src ) );
//    #else
//    const __m128 result = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src ),
//                                            _mm_cvttps_pi32( _mm_movehl_ps( src, src ) ) );
//    _m_empty();
//    #endif
//    return result;
//}



/// SIMD Vector3 FP Compare == ( true iff true for ALL 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_EQ( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpeq_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) == E_SSE_MaskBits_XYZ;
}

/// SIMD Vector3 FP Compare !=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_NE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpneq_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) == E_SSE_MaskBits_XYZ;
}

/// SIMD Vector3 FP Compare >=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_GE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpge_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) == E_SSE_MaskBits_XYZ;
}

/// SIMD Vector3 FP Compare <=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_LE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmple_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) == E_SSE_MaskBits_XYZ;
}

/// SIMD Vector3 FP Compare >
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_GT( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpgt_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) == E_SSE_MaskBits_XYZ;
}

/// SIMD Vector3 FP Compare <
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_LT( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmplt_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) == E_SSE_MaskBits_XYZ;
}



/// SIMD Vector3 FP Compare == ( true if true for ANY of 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_EQ( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpeq_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) != 0;
}

/// SIMD Vector3 FP Compare !=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_NE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpneq_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) != 0;
}

/// SIMD Vector3 FP Compare >=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_GE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpge_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) != 0;
}

/// SIMD Vector3 FP Compare <=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_LE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmple_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) != 0;
}

/// SIMD Vector3 FP Compare >
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_GT( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpgt_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) != 0;
}

/// SIMD Vector3 FP Compare <
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_LT( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmplt_ps( a, b ) ) & E_SSE_MaskBits_XYZ ) != 0;
}






/// SIMD Vector4 FP Round To Integer : Towards minus infinity
NOALIAS __forceinline const V128 SSE_RoundToIntMinusInf( const V128& src )
{
    #if defined( _SSE4_ )
    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC );
    #else
    
    const __m128 zero = _mm_setzero_ps();
    const __m128 one = c_VectorOne;
    const __m128 src_is_pos = _mm_cmpge_ps( src, zero );
    
    #if defined( _SSE2_ )
    const __m128 src_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src ) );
    const __m128 one_minus_src_trunc = _mm_sub_ps( one, src_trunc ); // If src -ve : Minimum integer value to add to src such that the sum is positive.
    const __m128 src_neg_to_pos = _mm_add_ps( src, one_minus_src_trunc );
	const __m128 src_neg_to_pos_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src_neg_to_pos ) );
    const __m128 result_pos = src_trunc;
    const __m128 result_neg = _mm_sub_ps( src_neg_to_pos_trunc, one_minus_src_trunc );
    #else
	const __m128 src_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src ),
	                                           _mm_cvttps_pi32( _mm_movehl_ps( src, src ) ) );
    const __m128 one_minus_src_trunc = _mm_sub_ps( one, src_trunc ); // If src -ve : Minimum integer value to add to src such that the sum is positive.
    const __m128 src_neg_to_pos = _mm_add_ps( src, one_minus_src_trunc );
	const __m128 src_neg_to_pos_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src_neg_to_pos ),
	                                                      _mm_cvttps_pi32( _mm_movehl_ps( src_neg_to_pos, src_neg_to_pos ) ) );
    const __m128 result_pos = src_trunc;
    const __m128 result_neg = _mm_sub_ps( src_neg_to_pos_trunc, one_minus_src_trunc );
	_m_empty();
    #endif // #if defined( _SSE2_ )
    
    const __m128 result = _mm_or_ps( _mm_and_ps( src_is_pos, result_pos ), _mm_andnot_ps( src_is_pos, result_neg ) );
    
    #endif // #if defined( _SSE4_ )
    return result;
}

/// SIMD Vector4 FP Round To Integer : Towards plus infinity
NOALIAS __forceinline const V128 SSE_RoundToIntPlusInf( const V128& src )
{
    #if defined( _SSE4_ )
    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC );
    #else
    
    const __m128 zero = _mm_setzero_ps();
    const __m128 one = c_VectorOne;
    const __m128 src_is_pos = _mm_cmpge_ps( src, zero );
    
    #if defined( _SSE2_ )
    const __m128 src_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src ) );
    const __m128 one_plus_src_trunc = _mm_add_ps( one, src_trunc ); // If src +ve : Minimum integer value to subtract from src such that the sum is negative.
    const __m128 src_pos_to_neg = _mm_sub_ps( src, one_plus_src_trunc );
	const __m128 src_pos_to_neg_trunc = _mm_cvtepi32_ps( _mm_cvttps_epi32( src_pos_to_neg ) );
    const __m128 result_pos = _mm_add_ps( src_pos_to_neg_trunc, one_plus_src_trunc );
    const __m128 result_neg = src_trunc;
    #else
	const __m128 src_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src ),
	                                           _mm_cvttps_pi32( _mm_movehl_ps( src, src ) ) );
    const __m128 one_plus_src_trunc = _mm_add_ps( one, src_trunc ); // If src +ve : Minimum integer value to subtract from src such that the sum is negative.
    const __m128 src_pos_to_neg = _mm_sub_ps( src, one_plus_src_trunc );
	const __m128 src_pos_to_neg_trunc = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src_pos_to_neg ),
	                                                      _mm_cvttps_pi32( _mm_movehl_ps( src_pos_to_neg, src_pos_to_neg ) ) );
    const __m128 result_pos = _mm_add_ps( src_pos_to_neg_trunc, one_plus_src_trunc );
    const __m128 result_neg = src_trunc;
	_m_empty();
    #endif // #if defined( _SSE2_ )
    
    const __m128 result = _mm_or_ps( _mm_and_ps( src_is_pos, result_pos ), _mm_andnot_ps( src_is_pos, result_neg ) );
    
    #endif // #if defined( _SSE4_ )
    return result;
}

/// SIMD Vector4 FP Round To Integer : Towards nearest
NOALIAS __forceinline const V128 SSE_RoundToIntNearest( const V128& src )
{
    #if defined( _SSE4_ )
    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
    #else
    
    const __m128 half = c_VectorHalf;
    const __m128 abs_mask = c_VectorAbsMask;
    const __m128 abs_src = _mm_and_ps( abs_mask, src );
    const __m128 sign_result = _mm_andnot_ps( abs_mask, src );
    const __m128 abs_src_plus_half = _mm_add_ps( abs_src, half );
    
    #if defined( _SSE2_ )
    const __m128 abs_result = _mm_cvtepi32_ps( _mm_cvttps_epi32( abs_src_plus_half ) );
    #else
	const __m128 abs_result = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( abs_src_plus_half ),
	                                            _mm_cvttps_pi32( _mm_movehl_ps( abs_src_plus_half, abs_src_plus_half ) ) );
	_m_empty();
    #endif // #if defined( _SSE2_ )
    
    const __m128 result = _mm_or_ps( sign_result, abs_result );
    #endif // #if defined( _SSE4_ )
    return result;
}

/// SIMD Vector4 FP Round To Integer : Towards zero
NOALIAS __forceinline const V128 SSE_RoundToIntZero( const V128& src )
{
    #if defined( _SSE4_ )
    const __m128 result = _mm_round_ps( src, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC );
    #elif defined( _SSE2_ )
    const __m128 result = _mm_cvtepi32_ps( _mm_cvttps_epi32( src ) );
    #else
    const __m128 result = _mm_cvtpi32x2_ps( _mm_cvttps_pi32( src ),
                                            _mm_cvttps_pi32( _mm_movehl_ps( src, src ) ) );
	_m_empty();
    #endif
    return result;
}



/// SIMD Vector4 FP Compare == ( true iff true for ALL 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_EQ( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpeq_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) == E_SSE_MaskBits_XYZW;
}

/// SIMD Vector4 FP Compare !=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_NE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpneq_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) == E_SSE_MaskBits_XYZW;
}

/// SIMD Vector4 FP Compare >=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_GE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpge_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) == E_SSE_MaskBits_XYZW;
}

/// SIMD Vector4 FP Compare <=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_LE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmple_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) == E_SSE_MaskBits_XYZW;
}

/// SIMD Vector4 FP Compare >
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_GT( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpgt_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) == E_SSE_MaskBits_XYZW;
}

/// SIMD Vector4 FP Compare <
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_LT( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmplt_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) == E_SSE_MaskBits_XYZW;
}




/// SIMD Vector4 FP Compare == ( true if true for ANY of 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_EQ( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpeq_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) != 0;
}

/// SIMD Vector4 FP Compare !=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_NE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpneq_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) != 0;
}

/// SIMD Vector4 FP Compare >=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_GE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpge_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) != 0;
}

/// SIMD Vector4 FP Compare <=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_LE( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmple_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) != 0;
}

/// SIMD Vector4 FP Compare >
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_GT( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmpgt_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) != 0;
}

/// SIMD Vector4 FP Compare <
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_LT( const V128& a, const V128& b )
{
    return ( _mm_movemask_ps( _mm_cmplt_ps( a, b ) ) /* & E_SSE_MaskBits_XYZW */ ) != 0;
}

















/// Replicate X into XYZW
FORCE_INLINE
const V128 SSE_ReplicateX( const V128& src )
{
    return SIMD_Permute<VX,VX,VX,VX>( src );
}

/// Replicate Y into XYZW
FORCE_INLINE
const V128 SSE_ReplicateY( const V128& src )
{
    return SIMD_Permute<VY,VY,VY,VY>( src );
}

/// Replicate Z into XYZW
FORCE_INLINE
const V128 SSE_ReplicateZ( const V128& src )
{
    return SIMD_Permute<VZ,VZ,VZ,VZ>( src );
}

/// Replicate W into XYZW
FORCE_INLINE
const V128 SSE_ReplicateW( const V128& src )
{
    return SIMD_Permute<VW,VW,VW,VW>( src );
}

/// Replicate X into XYZ, Preserve W
FORCE_INLINE
const V128 SSE_ReplicateX_XYZ( const V128& src )
{
    return SIMD_Permute<VX,VX,VX,VW>( src );
}

/// Replicate Y into XYZ, Preserve W
FORCE_INLINE
const V128 SSE_ReplicateY_XYZ( const V128& src )
{
    return SIMD_Permute<VY,VY,VY,VW>( src );
}

/// Replicate Z into XYZ, Preserve W
FORCE_INLINE
const V128 SSE_ReplicateZ_XYZ( const V128& src )
{
    return SIMD_Permute<VZ,VZ,VZ,VW>( src );
}

/// Replicate X into XY, Preserve ZW
FORCE_INLINE
const V128 SSE_ReplicateX_XY( const V128& src )
{
    return SIMD_Permute<VX,VX,VZ,VW>( src );
}

/// Replicate Y into XY, Preserve ZW
FORCE_INLINE
const V128 SSE_ReplicateY_XY( const V128& src )
{
    return SIMD_Permute<VY,VY,VZ,VW>( src );
}

/// Approx Reciprocal : 11 bit accuracy : Scalar - X Only
FORCE_INLINE
const V128 SSE_ApproxRcpSS( const V128& x )
{
    const V128 result( _mm_rcp_ss( x ) );
    return _mm_and_ps( result, SIMD_Scalar_Finite( result ) );
}

/// Approx Reciprocal : 11 bit accuracy : Packed - XYZW
FORCE_INLINE
const V128 SSE_ApproxRcpPS( const V128& xyzw )
{
    const V128 result( _mm_rcp_ps( xyzw ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
}

/// Approx Reciprocal + Newton-Raphson Iteration : 22 bit accuracy : Scalar - X Only
FORCE_INLINE
const V128 SSE_RcpSS( const V128& x )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 rx0( _mm_rcp_ss( x ) );
    //const V128 rx( _mm_and_ps( rx0, SIMD_Scalar_Finite( rx0 ) ) );
    //const V128 result( _mm_sub_ss( _mm_add_ss( rx, rx ), _mm_mul_ss( x, _mm_mul_ss( rx, rx ) ) ) );
    //return result;
    //#else
    const V128 rx( _mm_rcp_ss( x ) );
    const V128 result( _mm_sub_ss( _mm_add_ss( rx, rx ), _mm_mul_ss( x, _mm_mul_ss( rx, rx ) ) ) );
    return _mm_and_ps( result, SIMD_Scalar_Finite( result ) );
    //#endif
}

/// Approx Reciprocal + Newton-Raphson Iteration : 22 bit accuracy : Packed - XY
FORCE_INLINE
const V128 SSE_RcpPS_XY( const V128& xy )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 r_xy0( _mm_and_ps( SIMD_Mask<1,1,0,0>(), _mm_rcp_ps( xy ) ) );
    //const V128 r_xy( _mm_and_ps( r_xy0, SIMD_Vector_Finite( r_xy0 ) ) );
    //const V128 result( _mm_sub_ps( _mm_add_ps( r_xy, r_xy ), _mm_mul_ps( xy, _mm_mul_ps( r_xy, r_xy ) ) ) );
    //return result;
    //#else
    const V128 r_xy( _mm_and_ps( SIMD_Mask<1,1,0,0>(), _mm_rcp_ps( xy ) ) );
    const V128 result( _mm_sub_ps( _mm_add_ps( r_xy, r_xy ), _mm_mul_ps( xy, _mm_mul_ps( r_xy, r_xy ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// Approx Reciprocal + Newton-Raphson Iteration : 22 bit accuracy : Packed - XYZ
FORCE_INLINE
const V128 SSE_RcpPS_XYZ( const V128& xyz )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 r_xyz0( _mm_and_ps( SIMD_Mask<1,1,1,0>(), _mm_rcp_ps( xyz ) ) );
    //const V128 r_xyz( _mm_and_ps( r_xyz0, SIMD_Vector_Finite( r_xyz0 ) ) );
    //const V128 result( _mm_sub_ps( _mm_add_ps( r_xyz, r_xyz ), _mm_mul_ps( xyz, _mm_mul_ps( r_xyz, r_xyz ) ) ) );
    //return result;
    //#else
    const V128 r_xyz( _mm_and_ps( SIMD_Mask<1,1,1,0>(), _mm_rcp_ps( xyz ) ) );
    const V128 result( _mm_sub_ps( _mm_add_ps( r_xyz, r_xyz ), _mm_mul_ps( xyz, _mm_mul_ps( r_xyz, r_xyz ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// Approx Reciprocal + Newton-Raphson Iteration : 22 bit accuracy : Packed - XYZW
FORCE_INLINE
const V128 SSE_RcpPS_XYZW( const V128& xyzw )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 r_xyzw0( _mm_rcp_ps( xyzw ) );
    //const V128 r_xyzw( _mm_and_ps( r_xyzw0, SIMD_Vector_Finite( r_xyzw0 ) ) );
    //const V128 result( _mm_sub_ps( _mm_add_ps( r_xyzw, r_xyzw ), _mm_mul_ps( xyzw, _mm_mul_ps( r_xyzw, r_xyzw ) ) ) );
    //return result;
    //#else
    const V128 r_xyzw( _mm_rcp_ps( xyzw ) );
    const V128 result( _mm_sub_ps( _mm_add_ps( r_xyzw, r_xyzw ), _mm_mul_ps( xyzw, _mm_mul_ps( r_xyzw, r_xyzw ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}

/// Approx Reciprocal Sqrt : 11 bit accuracy : Scalar - X Only
FORCE_INLINE
const V128 SSE_ApproxRcpSqrtSS( const V128& x )
{
    const V128 result( _mm_rsqrt_ss( x ) );
    return _mm_and_ps( result, SIMD_Scalar_Finite( result ) );
}

/// Approx Reciprocal Sqrt : 11 bit accuracy : Packed - XYZW
FORCE_INLINE
const V128 SSE_ApproxRcpSqrtPS( const V128& xyzw )
{
    const V128 result( _mm_rsqrt_ps( xyzw ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
}

/// Approx Reciprocal Sqrt + Newton-Raphson Iteration : 22 bit accuracy : Scalar - X Only
FORCE_INLINE
const V128 SSE_RcpSqrtSS( const V128& x )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 rsqrt_x0 = _mm_rsqrt_ss( _mm_max_ps( x, _mm_setzero_ps() ) );
    //const V128 rsqrt_x = _mm_and_ps( rsqrt_x0, SIMD_Scalar_Finite( rsqrt_x0 ) );
    //const V128 result( _mm_mul_ss( _mm_mul_ss( rsqrt_x, c_VectorHalf ), _mm_sub_ss( c_VectorThree, _mm_mul_ss( x, _mm_mul_ss( rsqrt_x, rsqrt_x ) ) ) ) );
    //return result;
    //#else
    const V128 rsqrt_x = _mm_rsqrt_ss( x );
    const V128 result( _mm_mul_ss( _mm_mul_ss( rsqrt_x, c_VectorHalf ), _mm_sub_ss( c_VectorThree, _mm_mul_ss( x, _mm_mul_ss( rsqrt_x, rsqrt_x ) ) ) ) );
    return _mm_and_ps( result, SIMD_Scalar_Finite( result ) );
    //#endif
}

/// Approx Reciprocal Sqrt + Newton-Raphson Iteration : 22 bit accuracy : Packed - XYZW
FORCE_INLINE
const V128 SSE_RcpSqrtPS( const V128& xyzw )
{
    //#if defined( MATHS_UNMASK_FLOAT_EXCEPTIONS )
    //const V128 rsqrt_xyzw0 = _mm_rsqrt_ps( _mm_max_ps( xyzw, _mm_setzero_ps() ) );
    //const V128 rsqrt_xyzw = _mm_and_ps( rsqrt_xyzw0, SIMD_Vector_Finite( rsqrt_xyzw0 ) );
    //const V128 result( _mm_mul_ps( _mm_mul_ps( rsqrt_xyzw, c_VectorHalf ), _mm_sub_ps( c_VectorThree, _mm_mul_ps( xyzw, _mm_mul_ps( rsqrt_xyzw, rsqrt_xyzw ) ) ) ) );
    //return result;
    //#else
    const V128 rsqrt_xyzw = _mm_rsqrt_ps( xyzw );
    const V128 result( _mm_mul_ps( _mm_mul_ps( rsqrt_xyzw, c_VectorHalf ), _mm_sub_ps( c_VectorThree, _mm_mul_ps( xyzw, _mm_mul_ps( rsqrt_xyzw, rsqrt_xyzw ) ) ) ) );
    return _mm_and_ps( result, SIMD_Vector_Finite( result ) );
    //#endif
}



#endif // #if defined( _SSE_ )

}; // namespace Maths



#endif // #ifndef SIMD_SSE_INL




