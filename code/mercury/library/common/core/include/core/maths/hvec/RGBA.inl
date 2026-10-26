#ifndef RGBA_INL
#define RGBA_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

/// Default Constructor
__forceinline
C_RGBA_U1555::C_RGBA_U1555()
    : m_ARGB( 0 )
{
}

/// Copy Constructor
__forceinline
C_RGBA_U1555::C_RGBA_U1555( const C_RGBA_U1555& copy )
    : m_ARGB( copy.m_ARGB )
{
}

/// C_RGBA_U8 Constructor
__forceinline
C_RGBA_U1555::C_RGBA_U1555( const C_RGBA_U8& src )
    #if !defined( _VMX128_ )
    : m_ARGB( ( ( ( src.A() >> ( C_RGBA_U8::BitsA - BitsA ) ) << ShiftA ) & MaskA ) |
              ( ( ( src.R() >> ( C_RGBA_U8::BitsR - BitsR ) ) << ShiftR ) & MaskR ) |
              ( ( ( src.G() >> ( C_RGBA_U8::BitsG - BitsG ) ) << ShiftG ) & MaskG ) |
              ( ( ( src.B() >> ( C_RGBA_U8::BitsB - BitsB ) ) << ShiftB ) & MaskB ) )
    #endif
{
    #if defined( _VMX128_ )
    const __vector4 src_argb = __vpermwi( __lvlx( &src.m_ARGB, 0 ), 0 );
    const __vector4 dst_argb = __vpkpx( src_argb, src_argb );
    __stvehx( dst_argb, &m_ARGB, 0 );
    #endif
}

/// U8 RGBA Constructor
__forceinline
C_RGBA_U1555::C_RGBA_U1555( const U8 r, const U8 g, const U8 b, const U8 a )
    : m_ARGB( ( ( a << ShiftA ) & MaskA ) |
              ( ( r << ShiftR ) & MaskR ) |
              ( ( g << ShiftG ) & MaskG ) |
              ( ( b << ShiftB ) & MaskB ) )
{
}

/// S32 RGBA Constructor
__forceinline
C_RGBA_U1555::C_RGBA_U1555( const S32 r, const S32 g, const S32 b, const S32 a )
    : m_ARGB( (U16)( ( ( a << ShiftA ) & MaskA ) |
                     ( ( r << ShiftR ) & MaskR ) |
                     ( ( g << ShiftG ) & MaskG ) |
                     ( ( b << ShiftB ) & MaskB ) ) )
{
}

/// F32 RGBA Constructor
__forceinline
C_RGBA_U1555::C_RGBA_U1555( const F32 r, const F32 g, const F32 b, const F32 a )
	: m_ARGB( (U16)( ( ( ( (U32)( a * 255.0f ) ) << ShiftA ) & MaskA ) |
                     ( ( ( (U32)( r * 255.0f ) ) << ShiftR ) & MaskR ) |
                     ( ( ( (U32)( g * 255.0f ) ) << ShiftG ) & MaskG ) |
                     ( ( ( (U32)( b * 255.0f ) ) << ShiftB ) & MaskB ) ) )
{
}

/// F32 RGB Constructor
__forceinline
C_RGBA_U1555::C_RGBA_U1555( const F32 r, const F32 g, const F32 b )
	: m_ARGB( (U16)( ( 0x1 << ShiftA ) |
                     ( ( ( (U32)( r * 255.0f ) ) << ShiftR ) & MaskR ) |
                     ( ( ( (U32)( g * 255.0f ) ) << ShiftG ) & MaskG ) |
                     ( ( ( (U32)( b * 255.0f ) ) << ShiftB ) & MaskB ) ) )
{
}

/// Explicit Packed U32 Colour Constructor
__forceinline
C_RGBA_U1555::C_RGBA_U1555( U16 rgba )
    : m_ARGB( rgba )
{
}

/// Cast to Packed U16 Colour
__forceinline
C_RGBA_U1555::operator U16 () const
{
    return m_ARGB;
}

/// Copy Assignment
__forceinline
const C_RGBA_U1555& C_RGBA_U1555::operator = ( const C_RGBA_U1555& copy )
{
    m_ARGB = copy.m_ARGB;
    return *this;
}

/// C_RGBA_U8 Assignment
__forceinline
const C_RGBA_U1555& C_RGBA_U1555::operator = ( const C_RGBA_U8& src )
{
    #if defined( _VMX128_ )
    const __vector4 src_argb = __vpermwi( __lvlx( &src.m_ARGB, 0 ), 0 );
    const __vector4 dst_argb = __vpkpx( src_argb, src_argb );
    __stvehx( dst_argb, &m_ARGB, 0 );
    #else
    m_ARGB = ( ( ( src.A() >> ( C_RGBA_U8::BitsA - BitsA ) ) << ShiftA ) & MaskA ) |
             ( ( ( src.R() >> ( C_RGBA_U8::BitsR - BitsR ) ) << ShiftR ) & MaskR ) |
             ( ( ( src.G() >> ( C_RGBA_U8::BitsG - BitsG ) ) << ShiftG ) & MaskG ) |
             ( ( ( src.B() >> ( C_RGBA_U8::BitsB - BitsB ) ) << ShiftB ) & MaskB );
    #endif
    return *this;
}

/// Accessor: Get Alpha
__forceinline
U8 C_RGBA_U1555::A() const
{
    return (U8)( ( m_ARGB & MaskA ) >> ShiftA );
}

/// Accessor: Get Red
__forceinline
U8 C_RGBA_U1555::R() const
{
    return (U8)( ( m_ARGB & MaskR ) >> ShiftR );
}

/// Accessor: Get Green
__forceinline
U8 C_RGBA_U1555::G() const
{
    return (U8)( ( m_ARGB & MaskG ) >> ShiftG );
}

/// Accessor: Get Blue
__forceinline
U8 C_RGBA_U1555::B() const
{
    return (U8)( ( m_ARGB & MaskB ) >> ShiftB );
}

/// Accessor: Set Alpha
__forceinline
void C_RGBA_U1555::A( const U8 a )
{
    // TODO!
    //assert( 0 && "Not implemented yet!\n" );
    m_ARGB = ( m_ARGB & ~MaskA ) | ( ( a << ShiftA ) & MaskA );
}

/// Accessor: Set Red
__forceinline
void C_RGBA_U1555::R( const U8 r )
{
    // TODO!
    //assert( 0 && "Not implemented yet!\n" );
    m_ARGB = ( m_ARGB & ~MaskR ) | ( ( r << ShiftR ) & MaskR );
}

/// Accessor: Set Green
__forceinline
void C_RGBA_U1555::G( const U8 g )
{
    // TODO!
    //assert( 0 && "Not implemented yet!\n" );
    m_ARGB = ( m_ARGB & ~MaskG ) | ( ( g << ShiftG ) & MaskG );
}

/// Accessor: Set Blue
__forceinline
void C_RGBA_U1555::B( const U8 b )
{
    // TODO!
    //assert( 0 && "Not implemented yet!\n" );
    m_ARGB = ( m_ARGB & ~MaskB ) | ( ( b << ShiftB ) & MaskB );
}

/// Get Packed U16 Colour
__forceinline
U16 C_RGBA_U1555::Colour() const
{
    return m_ARGB;
}














/// Default Constructor
__forceinline
C_RGBA_U8::C_RGBA_U8() :
    m_ARGB( 0 )
{
}

/// Copy Constructor
__forceinline
C_RGBA_U8::C_RGBA_U8( const C_RGBA_U8& copy ) :
    m_ARGB( copy.m_ARGB )
{
}

/// C_RGBA_U1555 Constructor
__forceinline
C_RGBA_U8::C_RGBA_U8( const C_RGBA_U1555& /*src*/ )
{
    // TODO!
    assert( 0 && "Not implemented yet!\n" );
}

/// C_RGBA_F32 Constructor
__forceinline
C_RGBA_U8::C_RGBA_U8( const C_RGBA_F32& src )
    #if !defined( _VMX128_ )
	: m_ARGB( ( ( (U8)( src.A() * 255.0f ) ) << ShiftA ) |
              ( ( (U8)( src.R() * 255.0f ) ) << ShiftR ) |
              ( ( (U8)( src.G() * 255.0f ) ) << ShiftG ) |
              ( ( (U8)( src.B() * 255.0f ) ) << ShiftB ) )
    #endif
    // TODO: SSE Version!
{
    #if defined( _VMX128_ )
    __stvewx( __vspltw( __vpkd3d( __vzero(), __vnmsubfp( src.m_V.m_V, PackRGBA_Scale, c_VectorThree ), VPACK_D3DCOLOR, VPACK_32, 3 ), 0 ), &m_ARGB, 0 );
    #endif
}

/// U8 RGBA Constructor
__forceinline
C_RGBA_U8::C_RGBA_U8( const U8 r, const U8 g, const U8 b, const U8 a ) :
    m_ARGB( ( a << ShiftA ) |
            ( r << ShiftR ) |
            ( g << ShiftG ) |
            ( b << ShiftB ) )
{
}

/// S32 RGBA Constructor
__forceinline
C_RGBA_U8::C_RGBA_U8( const S32 r, const S32 g, const S32 b, const S32 a ) :
    m_ARGB( ( ( a << ShiftA ) & MaskA ) |
            ( ( r << ShiftR ) & MaskR ) |
            ( ( g << ShiftG ) & MaskG ) |
            ( ( b << ShiftB ) & MaskB ) )
{
}

/// F32 RGBA Constructor
__forceinline
C_RGBA_U8::C_RGBA_U8( const F32 r, const F32 g, const F32 b, const F32 a ) : 
	m_ARGB( ( ( (U8)( a * 255.0f ) ) << ShiftA ) |
		    ( ( (U8)( r * 255.0f ) ) << ShiftR ) |
		    ( ( (U8)( g * 255.0f ) ) << ShiftG ) |
		    ( ( (U8)( b * 255.0f ) ) << ShiftB ) )
{
}

/// F32 RGB Constructor
__forceinline
C_RGBA_U8::C_RGBA_U8( const F32 r, const F32 g, const F32 b ) : 
	m_ARGB( ( 0xFF                   << ShiftA ) |
		    ( ( (U8)( r * 255.0f ) ) << ShiftR ) |
		    ( ( (U8)( g * 255.0f ) ) << ShiftG ) |
		    ( ( (U8)( b * 255.0f ) ) << ShiftB ) )
{
}

/// Explicit Packed U32 Colour Constructor
__forceinline
C_RGBA_U8::C_RGBA_U8( U32 rgba ) :
    m_ARGB( rgba )
{
}

/// Cast to Packed U32 Colour
__forceinline
C_RGBA_U8::operator U32 () const
{
    return m_ARGB;
}

/// Copy Assignment
__forceinline
const C_RGBA_U8& C_RGBA_U8::operator = ( const C_RGBA_U8& copy )
{
    m_ARGB = copy.m_ARGB;
    return *this;
}

/// C_RGBA_U1555 Assignment
__forceinline
const C_RGBA_U8& C_RGBA_U8::operator = ( const C_RGBA_U1555& /*src*/ )
{
    // TODO!
    assert( 0 && "Not implemented yet!\n" );
    #if defined( PLATFORM_PS3 ) || defined( PLATFORM_SPU )
	return *this;// Suppress 1000 Warnings!
    #endif
}

/// C_RGBA_F32 Assignment
__forceinline
const C_RGBA_U8& C_RGBA_U8::operator = ( const C_RGBA_F32& src )
{
    #if defined( _VMX128_ )
    __stvewx( __vspltw( __vpkd3d( __vzero(), __vnmsubfp( src.m_V.m_V, PackRGBA_Scale, c_VectorThree ), VPACK_D3DCOLOR, VPACK_32, 3 ), 0 ), &m_ARGB, 0 );
    #else
    // TODO: SSE Version!
	m_ARGB = ( ( (U8)( src.A() * 255.0f ) ) << ShiftA ) |
             ( ( (U8)( src.R() * 255.0f ) ) << ShiftR ) |
             ( ( (U8)( src.G() * 255.0f ) ) << ShiftG ) |
             ( ( (U8)( src.B() * 255.0f ) ) << ShiftB );
    #endif
    return *this;
}

/// Accessor: Get Alpha
__forceinline
U8 C_RGBA_U8::A() const
{
    return (U8)( ( m_ARGB & MaskA ) >> ShiftA );
}

/// Accessor: Get Red
__forceinline
U8 C_RGBA_U8::R() const
{
    return (U8)( ( m_ARGB & MaskR ) >> ShiftR );
}

/// Accessor: Get Green
__forceinline
U8 C_RGBA_U8::G() const
{
    return (U8)( ( m_ARGB & MaskG ) >> ShiftG );
}

/// Accessor: Get Blue
__forceinline
U8 C_RGBA_U8::B() const
{
    return (U8)( ( m_ARGB & MaskB ) >> ShiftB );
}

/// Accessor: Set Alpha
__forceinline
void C_RGBA_U8::A( const U8 a )
{
    m_ARGB = ( m_ARGB & ~MaskA ) | ( a << ShiftA );
}

/// Accessor: Set Red
__forceinline
void C_RGBA_U8::R( const U8 r )
{
    m_ARGB = ( m_ARGB & ~MaskR ) | ( r << ShiftR );
}

/// Accessor: Set Green
__forceinline
void C_RGBA_U8::G( const U8 g )
{
    m_ARGB = ( m_ARGB & ~MaskG ) | ( g << ShiftG );
}

/// Accessor: Set Blue
__forceinline
void C_RGBA_U8::B( const U8 b )
{
    m_ARGB = ( m_ARGB & ~MaskB ) | ( b << ShiftB );
}

/// Get Packed U32 Colour
__forceinline
U32 C_RGBA_U8::Colour() const
{
    return m_ARGB;
}










/// Default Constructor
__forceinline
C_RGBA_F32::C_RGBA_F32()
{
}

/// Copy Constructor
__forceinline
C_RGBA_F32::C_RGBA_F32( const C_RGBA_F32& copy ) :
    m_V( copy.m_V )
{
}

/// C_RGBA_U8 Constructor
__forceinline
C_RGBA_F32::C_RGBA_F32( const C_RGBA_U8& src )
    #if defined( _VMX128_ )
    // NOTE: The Xenon D3D Unpack swizzles source ARGB to RGBA!!!
    : m_V( __vnmsubfp( __vupkd3d( __vspltw( __lvlx( &src.m_ARGB, 0 ), 0 ), VPACK_D3DCOLOR ), UnpackRGBA_ScaleOffset, UnpackRGBA_ScaleOffset ) )
    #else
    // TODO: SSE Version!
    : m_V( (F32)src.R() / 255.0f,
           (F32)src.G() / 255.0f,
           (F32)src.B() / 255.0f,
           (F32)src.A() / 255.0f )
    #endif
{
}

/// HVec3 Constructor
__forceinline
C_RGBA_F32::C_RGBA_F32( const HVec3 PCREF src )
    : m_V( src || HVecW( /*(HVec4)*/c_UnitVectorW ) ) // HVecW now has a constructor from 4xF32 arrays for non-SIMD version!
{
}

/// HVec4 Constructor
__forceinline
C_RGBA_F32::C_RGBA_F32( const HVec4 PCREF src )
    : m_V( src )
{
}

/// HF32 Constructor
__forceinline
C_RGBA_F32::C_RGBA_F32( const HF32 PCREF r, const HF32 PCREF g, const HF32 PCREF b, const HF32 PCREF a )
    : m_V( r, g, b, a )
{
}

/// Copy Assignment
__forceinline
const C_RGBA_F32& C_RGBA_F32::operator = ( const C_RGBA_F32& copy )
{
    m_V = copy.m_V;
    return *this;
}

/// C_RGBA_U8 Assignment
__forceinline
const C_RGBA_F32& C_RGBA_F32::operator = ( const C_RGBA_U8 src )
{
    #if defined( _VMX128_ )
    // NOTE: The Xenon D3D Unpack swizzles source ARGB to RGBA!!!
    m_V.m_V = __vnmsubfp( __vupkd3d( __vspltw( __lvlx( &src.m_ARGB, 0 ), 0 ), VPACK_D3DCOLOR ), UnpackRGBA_ScaleOffset, UnpackRGBA_ScaleOffset );
    #else
    // TODO: SSE Version!
    m_V = HVec4( (F32)src.R() / 255.0f,
                 (F32)src.G() / 255.0f,
                 (F32)src.B() / 255.0f,
                 (F32)src.A() / 255.0f );
    #endif
    return *this;
}

/// Accessor: Get R
__forceinline
HF32 C_RGBA_F32::R() const
{
    return m_V.X();
}

/// Accessor: Get G
__forceinline
HF32 C_RGBA_F32::G() const
{
    return m_V.Y();
}

/// Accessor: Get B
__forceinline
HF32 C_RGBA_F32::B() const
{
    return m_V.Z();
}

/// Accessor: Get A
__forceinline
HF32 C_RGBA_F32::A() const
{
    return m_V.W();
}

/// Accessor: Set R
__forceinline
void C_RGBA_F32::R( const HF32 PCREF r )
{
    m_V.X( r );
}

/// Accessor: Set G
__forceinline
void C_RGBA_F32::G( const HF32 PCREF g )
{
    m_V.Y( g );
}

/// Accessor: Set B
__forceinline
void C_RGBA_F32::B( const HF32 PCREF b )
{
    m_V.Z( b );
}

/// Accessor: Set A
__forceinline
void C_RGBA_F32::A( const HF32 PCREF a )
{
    m_V.W( a );
}

/// Negation operator
__forceinline
C_RGBA_F32 C_RGBA_F32::operator - () const
{
    return C_RGBA_F32( -m_V );
}

/// Addition operator
__forceinline
C_RGBA_F32 C_RGBA_F32::operator + ( const C_RGBA_F32 PCREF src ) const
{
    return C_RGBA_F32( m_V + src.m_V );
}

/// Subtraction operator
__forceinline
C_RGBA_F32 C_RGBA_F32::operator - ( const C_RGBA_F32 PCREF src ) const
{
    return C_RGBA_F32( m_V - src.m_V );
}

/// Multiplication operator
__forceinline
C_RGBA_F32 C_RGBA_F32::operator * ( const C_RGBA_F32 PCREF src ) const
{
    return C_RGBA_F32( m_V * src.m_V );
}

/// Division operator
__forceinline
C_RGBA_F32 C_RGBA_F32::operator / ( const C_RGBA_F32 PCREF src ) const
{
    return C_RGBA_F32( m_V / src.m_V );
}

/// Scalar Multiplication operator
__forceinline
C_RGBA_F32 C_RGBA_F32::operator * ( const HF32 PCREF src ) const
{
    return C_RGBA_F32( m_V * src );
}

/// Scalar Division operator
__forceinline
C_RGBA_F32 C_RGBA_F32::operator / ( const HF32 PCREF src ) const
{
    return C_RGBA_F32( m_V / src );
}

/// Min
__forceinline
C_RGBA_F32 C_RGBA_F32::Min( const C_RGBA_F32 PCREF src ) const
{
    return C_RGBA_F32( m_V.Min( src.m_V ) );
}

/// Min
__forceinline
C_RGBA_F32 C_RGBA_F32::Max( const C_RGBA_F32 PCREF src ) const
{
    return C_RGBA_F32( m_V.Max( src.m_V ) );
}

/// Saturate
__forceinline
C_RGBA_F32 C_RGBA_F32::Saturate() const
{
    const HVec4 v_zero;
    const HVec4 v_one( c_VectorOne );
    return C_RGBA_F32( m_V.Max( v_zero ).Min( v_one ) );
}

/// Clamp
__forceinline
C_RGBA_F32 C_RGBA_F32::Clamp( const C_RGBA_F32 PCREF lo, const C_RGBA_F32 PCREF hi ) const
{
    return C_RGBA_F32( m_V.Max( lo.m_V ).Min( hi.m_V ) );
}





}; // namespace Maths



#endif
