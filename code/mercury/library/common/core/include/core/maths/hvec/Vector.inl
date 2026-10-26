#ifndef MATHS_HVEC_VECTOR_INL
#define MATHS_HVEC_VECTOR_INL

#ifdef _MSC_VER
#pragma once
#endif

#include "simd.h"

namespace Maths
{
//
// ---------------------------------------------------
// |-- HVec2 : Hardware 2 Element F32 Vector Class --|
// ---------------------------------------------------
//

#if defined( MATHS_DONT_INITIALISE_TO_ZERO )
/// Default Constructor
NOALIAS __forceinline HVec2::HVec2()
{
}
#else
/// Default Constructor
NOALIAS __forceinline HVec2::HVec2()
    #if defined( _SPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Zero() )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    #endif
}
#endif

/// Zero Init Constructor
NOALIAS __forceinline HVec2::HVec2( const _ZERO_INIT /*zeroinit*/ )
    #if defined( _SPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Zero() )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    #endif
}

/// Undefined Constructor
NOALIAS __forceinline HVec2::HVec2( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline HVec2::HVec2( const HVec2& copy )
    #if defined( _SIMD_ )
    : m_V( copy.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = copy.m_V[VX];
    m_V[VY] = copy.m_V[VY];
    #endif
}

/// F32 Array Constructor
NOALIAS __forceinline HVec2::HVec2( F32 const * const __restrict ptr )
    #if defined( _SPU_ )
    //: m_V( (vector float) { ptr[0], ptr[1], 0.0f, 0.0f } )
    //: m_V( spu_and( spu_load_unaligned( ptr ), (vector float)Const< vector unsigned int >::Mask(1,1,0,0) ) )
    : m_V( spu_load_unaligned( ptr ) )
    #elif defined( _PPU_ )
    //: m_V( vec_load_unaligned( ptr ) )
    //: m_V( vec_and( vec_load_unaligned( ptr ), (vector float)Const< vector unsigned int >::Mask(1,1,0,0) ) )
    : m_V( vec_load_unaligned( ptr ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,1,0,0>( SIMD_LoadUnaligned( ptr ) ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = ptr[VX];
    m_V[VY] = ptr[VY];
    #endif
}

/// HF32 Pair Constructor
NOALIAS __forceinline HVec2::HVec2( const HF32 PCREF x, const HF32 PCREF y )
    #if defined( _SPU_ )
    : m_V( spu_permute<V0X,V1X,V1W,V1W>( x.m_F, y.m_F ) )
    #elif defined( _PPU_ )
    : m_V( vec_mergeh( x.m_F, y.m_F ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Vector2( x.m_F, y.m_F ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = x;
    m_V[VY] = y;
    #endif
}

/// Explicit HF32 Broadcast Constructor
NOALIAS __forceinline HVec2::HVec2( const HF32 PCREF xy )
    #if defined( _SPU_ )
    : m_V( xy.m_F )
    #elif defined( _PPU_ )
    : m_V( xy.m_F )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Vector2_Scalar( xy.m_F ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = xy;
    m_V[VY] = xy;
    #endif
}

/// HVecX Constructor
NOALIAS __forceinline HVec2::HVec2( const HVecX PCREF x )
    #if defined( _SPU_ )
    : m_V( spu_permute<V0X,V1W,V1W,V1W>( x.m_X, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_mask<1,0,0,0>( x.m_X ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,0,0,0>( x.m_X ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = x.m_X;
    m_V[VY] = 0.0f;
    #endif
}

/// HVecY Constructor
NOALIAS __forceinline HVec2::HVec2( const HVecY PCREF y )
    #if defined( _SPU_ )
    : m_V( spu_permute<V1W,V0Y,V1W,V1W>( y.m_Y, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_mask<0,1,0,0>( y.m_Y ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<0,1,0,0>( y.m_Y ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = y.m_Y;
    #endif
}

/// HVec3 Constructor: Explicit construction due to data loss!
NOALIAS __forceinline HVec2::HVec2( const HVec3 PCREF src )
    #if defined( _SIMD_ )
    : m_V( src.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V[VX];
    m_V[VY] = src.m_V[VY];
    #endif
}

/// HVec4 Constructor: Explicit construction due to data loss!
NOALIAS __forceinline HVec2::HVec2( const HVec4 PCREF src )
    #if defined( _SIMD_ )
    : m_V( src.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V[VX];
    m_V[VY] = src.m_V[VY];
    #endif
}

/// C_Vertex Constructor: Explicit construction due to data loss!
NOALIAS __forceinline HVec2::HVec2( const C_Vertex PCREF src )
    #if defined( _SIMD_ )
    : m_V( src.m_V.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V.m_V[VX];
    m_V[VY] = src.m_V.m_V[VY];
    #endif
}

/// C_Vector Constructor: Explicit construction due to data loss!
NOALIAS __forceinline HVec2::HVec2( const C_Vector PCREF src )
    #if defined( _SIMD_ )
    : m_V( src.m_V.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V.m_V[VX];
    m_V[VY] = src.m_V.m_V[VY];
    #endif
}

/// C_Normal Constructor: Explicit construction due to data loss!
NOALIAS __forceinline HVec2::HVec2( const C_Normal PCREF src )
    #if defined( _SIMD_ )
    : m_V( src.m_V.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V.m_V[VX];
    m_V[VY] = src.m_V.m_V[VY];
    #endif
}

#if defined( _SIMD_ )

/// V128 Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVec2::HVec2( const V128 PCREF src ) :
    m_V( src )
{
}

/// V128 Cast
NOALIAS __forceinline HVec2::operator const V128 () const __restrict
{
    return m_V;
}

#else

/// V128 Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVec2::HVec2( const V128 PCREF src )
{
    m_V[0] = src.m_X;
    m_V[1] = src.m_Y;
}

/// V128 Cast
NOALIAS __forceinline HVec2::operator const V128 () const __restrict
{
    V128 v;
    v.m_X = m_V[0];
    v.m_Y = m_V[1];
    return v;
}

#endif

/// HVecX Cast
NOALIAS __forceinline HVec2::operator const HVecX () const
{
    #if defined( _SPU_ )
    HVecX result( UNDEFINED );
    result.m_X = m_V;
    return result;
    #elif defined( _PPU_ )
    HVecX result( UNDEFINED );
    result.m_X = m_V;
    return result;
    #elif defined( _SIMD_ )
    //return HVecX( SIMD_Mask<1,0,0,0>( m_V ) );
    return HVecX( m_V );
    #else
    return HVecX( m_V[VX] );
    #endif
}

/// HVecY Cast
NOALIAS __forceinline HVec2::operator const HVecY () const
{
    #if defined( _SPU_ )
    HVecY result( UNDEFINED );
    result.m_Y = m_V;
    return result;
    #elif defined( _PPU_ )
    HVecY result( UNDEFINED );
    result.m_Y = m_V;
    return result;
    #elif defined( _SIMD_ )
    //return HVecY( SIMD_Mask<0,1,0,0>( m_V ) );
    return HVecY( m_V );
    #else
    return HVecY( m_V[VY] );
    #endif
}

/// Copy Assignment
NOALIAS __forceinline const HVec2& HVec2::operator = ( const HVec2& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_V[VX] = copy.m_V[VX];
    m_V[VY] = copy.m_V[VY];
    #endif
    
    return copy;
}

/// HVecX Assignment
NOALIAS __forceinline const HVec2& HVec2::operator = ( const HVecX PCREF x )
{
    #if defined( _SPU_ )
    m_V = spu_permute<V0X,V1W,V1W,V1W>( x.m_X, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    m_V = vec_mask<1,0,0,0>( x.m_X );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<1,0,0,0>( x.m_X );
    #else
    m_V[VX] = x.m_X;
    m_V[VY] = 0.0f;
    #endif
    
    return *this;
}

/// HVecY Assignment
NOALIAS __forceinline const HVec2& HVec2::operator = ( const HVecY PCREF y )
{
    #if defined( _SPU_ )
    m_V = spu_permute<V1W,V0Y,V1W,V1W>( y.m_Y, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    m_V = vec_mask<0,1,0,0>( y.m_Y );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<0,1,0,0>( y.m_Y );
    #else
    m_V[VX] = 0.0f;
    m_V[VY] = y.m_Y;
    #endif
    return *this;
}

/// Accessor: Get X
NOALIAS __forceinline const HF32 HVec2::X() const
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VX>( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_V, VX );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_Vector2X( m_V ) );
    #else
    return HF32( m_V[VX] );
    #endif
}

/// Accessor: Get Y
NOALIAS __forceinline const HF32 HVec2::Y() const
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VY>( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_V, VY );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_Vector2Y( m_V ) );
    #else
    return HF32( m_V[VY] );
    #endif
}

/// Accessor: Get Z - Required for templated accessors - should never be used!
NOALIAS __forceinline const HF32 HVec2::Z() const
{
    assert( false );
    return HF32( 0.0f );
}

/// Accessor: Get W - Required for templated accessors - should never be used!
NOALIAS __forceinline const HF32 HVec2::W() const
{
    assert( false );
    return HF32( 0.0f );
}

/// Accessor: Set X
NOALIAS __forceinline void HVec2::X( const HF32 PCREF x )
{
    #if defined( _SPU_ )
    //m_V = spu_insert( spu_extract( x.m_F, VX ), m_V, VX );
    //m_V = spu_permute<V1X,V0Y,V0Z,V0W>( m_V, x.m_F );
    m_V = spu_select<1,0,0,0>( m_V, x.m_F );
    #elif defined( _PPU_ )
    m_V = vec_sel( m_V, x.m_F, vec_u32_mask<1,0,0,0>() );
    #elif defined( _SIMD_ )
    m_V = SIMD_SetX( m_V, x.m_F );
    #else
    m_V[VX] = x.m_F;
    #endif
}

/// Accessor: Set Y
NOALIAS __forceinline void HVec2::Y( const HF32 PCREF y )
{
    #if defined( _SPU_ )
    //m_V = spu_permute<V0X,V1Y,V0Z,V0W>( m_V, y.m_F );
    m_V = spu_select<0,1,0,0>( m_V, y.m_F );
    #elif defined( _PPU_ )
    m_V = vec_sel( m_V, y.m_F, vec_u32_mask<0,1,0,0>() );
    #elif defined( _SIMD_ )
    m_V = SIMD_SetY( m_V, y.m_F );
    #else
    m_V[VY] = y.m_F;
    #endif
}

/// Accessor: Set Z - Required for templated accessors - should never be used!
NOALIAS __forceinline void HVec2::Z( const HF32 PCREF /*scalar*/ )
{
    assert( false );
}

/// Accessor: Set W - Required for templated accessors - should never be used!
NOALIAS __forceinline void HVec2::W( const HF32 PCREF /*scalar*/ )
{
    assert( false );
}

/// Unary Negation Operator
NOALIAS __forceinline const HVec2 HVec2::operator - () const
{
    #if defined( _SPU_ )
    //const vector float sign_mask = (vector float)spu_splats( 0x80000000 );
    HVec2 result( UNDEFINED );
    //result.m_V = spu_xor( m_V, sign_mask );
    result.m_V = spu_negate( m_V );
    return result;
    #elif defined( _PPU_ )
    //const vector float sign_mask = (vector float)(vector unsigned int)0x80000000;
    //const vector float sign_mask = (vector float)(vector unsigned int)( 0x80000000, 0x80000000, 0x80000000, 0x80000000 );
    const vector float sign_mask = Const< vector float >::SignMask();
    HVec2 result( UNDEFINED );
    result.m_V = vec_vxor( m_V, sign_mask );
    return result;
    #elif defined( _VMX128_ )
    return HVec2( SIMD_XOR( m_V, c_VectorSignMask ) );
    #elif defined( _SIMD_ )
    return HVec2( SIMD_XOR( m_V, (__m128&)c_VectorSignMask ) );
    #else
    return HVec2( -m_V[VX],
                  -m_V[VY] );
    #endif
}

/// Binary Addition Operator
NOALIAS __forceinline const HVec2 HVec2::operator + ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = spu_add( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = vec_add( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Add( m_V, src.m_V ) );
    #else
    return HVec2( m_V[VX] + src.m_V[VX],
                  m_V[VY] + src.m_V[VY] );
    #endif
}

/// Binary Subtraction Operator
NOALIAS __forceinline const HVec2 HVec2::operator - ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = spu_sub( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = vec_vsubfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Subtract( m_V, src.m_V ) );
    #else
    return HVec2( m_V[VX] - src.m_V[VX],
                  m_V[VY] - src.m_V[VY] );
    #endif
}

/// Binary Multiplication Operator
NOALIAS __forceinline const HVec2 HVec2::operator * ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = spu_mul( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = vec_mul( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Multiply( m_V, src.m_V ) );
    #else
    return HVec2( m_V[VX] * src.m_V[VX],
                  m_V[VY] * src.m_V[VY] );
    #endif
}

/// Binary Division Operator
NOALIAS __forceinline const HVec2 HVec2::operator / ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  A = m_V;
    const vector float  B = src.m_V;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec2 result( UNDEFINED );
    result.m_V = spu_mul( A, rcp_B );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float A = m_V;
    const vector float B = src.m_V;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec2 result( UNDEFINED );
    result.m_V = vec_mul( A, rcp_B );
    return result;
    
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Multiply( m_V, SIMD_Reciprocal2( src.m_V ) ) );
    #else
    //return HVec2( m_V[VX] / src.m_V[VX],
    //              m_V[VY] / src.m_V[VY] );
    return HVec2( m_V[VX] * Maths::Reciprocal( src.m_V[VX] ),
                  m_V[VY] * Maths::Reciprocal( src.m_V[VY] ) );
    #endif
}

/// Add Update Operator
NOALIAS __forceinline void HVec2::operator += ( const HVec2 PCREF src )
{
    #if defined( _SPU_ )
    m_V = spu_add( m_V, src.m_V );
    #elif defined( _PPU_ )
    m_V = vec_add( m_V, src.m_V );
    #elif defined( _SIMD_ )
    m_V = SIMD_Add( m_V, src.m_V );
    #else
    m_V[VX] += src.m_V[VX];
    m_V[VY] += src.m_V[VY];
    #endif
}

/// Subtract Update Operator
NOALIAS __forceinline void HVec2::operator -= ( const HVec2 PCREF src )
{
    #if defined( _SPU_ )
    m_V = spu_sub( m_V, src.m_V );
    #elif defined( _PPU_ )
    m_V = vec_vsubfp( m_V, src.m_V );
    #elif defined( _SIMD_ )
    m_V = SIMD_Subtract( m_V, src.m_V );
    #else
    m_V[VX] -= src.m_V[VX];
    m_V[VY] -= src.m_V[VY];
    #endif
}

/// Multiply Update Operator
NOALIAS __forceinline void HVec2::operator *= ( const HVec2 PCREF src )
{
    #if defined( _SPU_ )
    m_V = spu_mul( m_V, src.m_V );
    #elif defined( _PPU_ )
    m_V = vec_mul( m_V, src.m_V );
    #elif defined( _SIMD_ )
    m_V = SIMD_Multiply( m_V, src.m_V );
    #else
    m_V[VX] *= src.m_V[VX];
    m_V[VY] *= src.m_V[VY];
    #endif
}

/// Divide Update Operator
NOALIAS __forceinline void HVec2::operator /= ( const HVec2 PCREF src )
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  A = m_V;
    const vector float  B = src.m_V;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    m_V = spu_mul( A, rcp_B );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float A = m_V;
    const vector float B = src.m_V;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    m_V = vec_mul( A, rcp_B );
    
    #elif defined( _SIMD_ )
    m_V = SIMD_Multiply( m_V, SIMD_Reciprocal2( src.m_V ) );
    #else
    //m_V[VX] /= src.m_V[VX];
    //m_V[VY] /= src.m_V[VY];
    m_V[VX] *= Maths::Reciprocal( src.m_V[VX] );
    m_V[VY] *= Maths::Reciprocal( src.m_V[VY] );
    #endif
}

/// Returns copy with negated Y component
NOALIAS __forceinline const HVec2 HVec2::NegatedY() const
{
    #if defined( _SPU_ )
    //const vector float sign_mask_y = (vector float)(vector unsigned int){ (unsigned int)0, (unsigned int)0x80000000, (unsigned int)0, (unsigned int)0 };
    const vector float sign_mask_y = (vector float)(vector unsigned int)( (unsigned int)0, (unsigned int)0x80000000, (unsigned int)0, (unsigned int)0 );
    HVec2 result( UNDEFINED );
    result.m_V = spu_xor( m_V, sign_mask_y );
    return result;
    #elif defined( _PPU_ )
    //const vector float sign_mask_y = (vector float)(vector unsigned int){ (unsigned int)0, (unsigned int)0x80000000, (unsigned int)0, (unsigned int)0 };
    const vector float sign_mask_y = (vector float)(vector unsigned int)( (unsigned int)0, (unsigned int)0x80000000, (unsigned int)0, (unsigned int)0 );
    HVec2 result( UNDEFINED );
    result.m_V = vec_vxor( m_V, sign_mask_y );
    return result;
    #elif defined( _VMX128_ )
    return HVec2( SIMD_XOR( m_V, c_VectorY_SignMask ) );
    #elif defined( _SIMD_ )
    return HVec2( SIMD_XOR( m_V, (__m128&)c_VectorY_SignMask ) );
    #else
    return HVec2( m_V[VX], -m_V[VY] );
    #endif
}

/// HF32 Multiply Operator
NOALIAS __forceinline const HVec2 HVec2::operator * ( const HF32 PCREF src ) const
{
    #if defined( _SPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = spu_mul( m_V, src.m_F );
    return result;
    #elif defined( _PPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = vec_mul( m_V, src.m_F );
    return result;
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Multiply( m_V, SIMD_ScalarBroadcastToXYZW( src.m_F ) ) );
    #else
    return HVec2( m_V[VX] * src,
                  m_V[VY] * src );
    #endif
}

/// HF32 Divide Operator
NOALIAS __forceinline const HVec2 HVec2::operator / ( const HF32 PCREF src ) const
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  A = m_V;
    const vector float  B = src.m_F;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec2 result( UNDEFINED );
    result.m_V = spu_mul( A, rcp_B );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float A = m_V;
    const vector float B = src.m_F;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec2 result( UNDEFINED );
    result.m_V = vec_mul( A, rcp_B );
    return result;
    
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Multiply( m_V, SIMD_ScalarBroadcastToXYZW( src.Reciprocal().m_F ) ) );
    #else
    //return HVec2( m_V[VX] / src,
    //              m_V[VY] / src );
    const F32 rcp_src = Maths::Reciprocal( src );
    return HVec2( m_V[VX] * rcp_src,
                  m_V[VY] * rcp_src );
    #endif
}

/// HF32 Multiply Update Operator
NOALIAS __forceinline void HVec2::operator *= ( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    m_V = spu_mul( m_V, src.m_F );
    #elif defined( _PPU_ )
    m_V = vec_mul( m_V, src.m_F );
    #elif defined( _SIMD_ )
    m_V = SIMD_Multiply( m_V, SIMD_ScalarBroadcastToXYZW( src.m_F ) );
    #else
    m_V[VX] *= src;
    m_V[VY] *= src;
    #endif
}

/// HF32 Divide Update Operator
NOALIAS __forceinline void HVec2::operator /= ( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  A = m_V;
    const vector float  B = src.m_F;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    m_V = spu_mul( A, rcp_B );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float A = m_V;
    const vector float B = src.m_F;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    m_V = vec_mul( A, rcp_B );
    
    #elif defined( _SIMD_ )
    m_V = SIMD_Multiply( m_V, SIMD_ScalarBroadcastToXYZW( src.Reciprocal().m_F ) );
    #else
    //m_V[VX] /= src;
    //m_V[VY] /= src;
    const F32 rcp_src = Maths::Reciprocal( src );
    m_V[VX] *= rcp_src;
    m_V[VY] *= rcp_src;
    #endif
}

/// Min
NOALIAS __forceinline const HVec2 HVec2::Min( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = spu_min( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = vec_vminfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Min( m_V, src.m_V ) );
    #else
    return HVec2( Maths::Min( m_V[VX], src.m_V[VX] ),
                  Maths::Min( m_V[VY], src.m_V[VY] ) );
    #endif
}

/// Max
NOALIAS __forceinline const HVec2 HVec2::Max( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = spu_max( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = vec_vmaxfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Max( m_V, src.m_V ) );
    #else
    return HVec2( Maths::Max( m_V[VX], src.m_V[VX] ),
                  Maths::Max( m_V[VY], src.m_V[VY] ) );
    #endif
}

/// Min Update
NOALIAS __forceinline void HVec2::MinEq( const HVec2 PCREF src )
{
    #if defined( _SPU_ )
    m_V = spu_min( m_V, src.m_V );
    #elif defined( _PPU_ )
    m_V = vec_vminfp( m_V, src.m_V );
    #elif defined( _SIMD_ )
    m_V = SIMD_Min( m_V, src.m_V );
    #else
    m_V[VX] = Maths::Min( m_V[VX], src.m_V[VX] );
    m_V[VY] = Maths::Min( m_V[VY], src.m_V[VY] );
    #endif
}

/// Max Update
NOALIAS __forceinline void HVec2::MaxEq( const HVec2 PCREF src )
{
    #if defined( _SPU_ )
    m_V = spu_max( m_V, src.m_V );
    #elif defined( _PPU_ )
    m_V = vec_vmaxfp( m_V, src.m_V );
    #elif defined( _SIMD_ )
    m_V = SIMD_Max( m_V, src.m_V );
    #else
    m_V[VX] = Maths::Max( m_V[VX], src.m_V[VX] );
    m_V[VY] = Maths::Max( m_V[VY], src.m_V[VY] );
    #endif
}

/// Lerp
NOALIAS __forceinline const HVec2 HVec2::Lerp( const HVec2 PCREF dst, const HF32 PCREF f ) const
{
    #if defined( _SPU_ )
    const vector float src = m_V;
    const vector float delta = spu_sub( dst.m_V, src );
    HVec2 result( UNDEFINED );
    result.m_V = spu_madd( delta, f.m_F, src );
    return result;
    #elif defined( _PPU_ )
    const vector float src = m_V;
    const vector float delta = vec_vsubfp( dst.m_V, src );
    HVec2 result( UNDEFINED );
    result.m_V = vec_madd( delta, f.m_F, src );
    return result;
    #else
    const HVec2 src = *this;
    return src + ( ( dst - src ) * f );
    #endif
}

/// Horizontal sum of all elements
NOALIAS __forceinline const HF32 HVec2::Sum() const
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_sum2( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_sum2( m_V );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Sum2( m_V ) );
    #else
    return HF32( m_V[VX] + m_V[VY] );
    #endif
}

/// Dot product
NOALIAS __forceinline const HF32 HVec2::operator | ( const HVec2 PCREF rhs ) const
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_dot_product2( m_V, rhs.m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_dot_product2( m_V, rhs.m_V );
    return result;
    #else
    const HVec2 lhs = *this;
    return ( lhs * rhs ).Sum();
    #endif
}

/// Cross product
NOALIAS __forceinline const HF32 HVec2::operator ^ ( const HVec2 PCREF rhs ) const
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_cross_product2( m_V, rhs.m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_cross_product2( m_V, rhs.m_V );
    return result;
    #else
    // TODO: Optimise this for VMX & SSE
    const HVec2 lhs = *this;
    return lhs.X() * rhs.Y() - lhs.Y() * rhs.X();
    #endif
}

/// Vector Length Squared
NOALIAS __forceinline const HF32 HVec2::Length2() const
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_length_squared2( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_length_squared2( m_V );
    return result;
    #else
    const HVec2 src = *this;
    return ( src * src ).Sum();
    #endif
}    

/// Vector Length Squared
NOALIAS __forceinline const HF32 HVec2::LengthSquared() const
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_length_squared2( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_length_squared2( m_V );
    return result;
    #else
    return Length2();
    #endif
}    

/// Vector Length
NOALIAS __forceinline const HF32 HVec2::Length() const
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = spu_length_squared2( m_V ); // length squared
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = spu_mul( B, rcp_sqrt_B ); // length squared / length
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = vec_length_squared2( m_V ); // length squared
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = vec_mul( B, rcp_sqrt_B ); // length squared / length
    return result;
    
    #else
    return Length2().Sqrt();
    #endif
}    

/// Vector Reciprocal of Length
NOALIAS __forceinline const HF32 HVec2::RcpLength() const
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = spu_length_squared2( m_V ); // length squared
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_sqrt_B;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = vec_length_squared2( m_V ); // length squared
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_sqrt_B;
    return result;
    
    #else
    return Length2().RcpSqrt();
    #endif
}    

/// Vector Reciprocal of Length
NOALIAS __forceinline const HF32 HVec2::ReciprocalLength() const
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = spu_length_squared2( m_V ); // length squared
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_sqrt_B;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = vec_length_squared2( m_V ); // length squared
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_sqrt_B;
    return result;
    
    #else
    return RcpLength();
    #endif
}    

/// Vector Normalised
NOALIAS __forceinline const HVec2 HVec2::Normalised() const
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = spu_length_squared2( m_V ); // length squared
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HVec2 result( UNDEFINED );
    result.m_V = spu_mul( m_V, rcp_sqrt_B );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = vec_length_squared2( m_V ); // length squared
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HVec2 result( UNDEFINED );
    result.m_V = vec_mul( m_V, rcp_sqrt_B );
    return result;
    
    #else
    
    const HF32  nearlyZero( c_ScalarFltMin );
    const HF32  length2( Length2() );
    const HVec2 vZero;
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec2 result( *this * ( Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() ) );
    //#else
    const HVec2 result( *this * ( length2.ReciprocalSqrt() ) );
    //#endif
    
    #if defined( MATHS_DEBUG_BREAK_ON_ZERO_LENGTH_NORMALISE )
    if ( length2 <= nearlyZero )
    {
        DebugBreak();
    }
    #endif
    
    return Select( length2.GT( nearlyZero ), result, vZero );
    
    #endif
}

/// Vector Normalise
NOALIAS __forceinline void HVec2::Normalise()
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = spu_length_squared2( m_V ); // length squared
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    m_V = spu_mul( m_V, rcp_sqrt_B );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = vec_length_squared2( m_V ); // length squared
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    m_V = vec_mul( m_V, rcp_sqrt_B );
    
    #else
    
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Length2() );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec2 result( *this * ( Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() ) );
    //#else
    const HVec2 result( *this * ( length2.ReciprocalSqrt() ) );
    //#endif
    
    #if defined( MATHS_DEBUG_BREAK_ON_ZERO_LENGTH_NORMALISE )
    if ( length2 <= nearlyZero )
    {
        DebugBreak();
    }
    #endif
    
    #if defined( _SIMD_ )
    m_V = Select( length2.GT( nearlyZero ), result, HVec2() ).m_V;
    #else
    *this = ( Select( length2.GT( nearlyZero ), result, HVec2() ) );
    #endif
    
    #endif
}

/// Vector Normalised - safe version with additional parameter to use for zero length vectors!
NOALIAS __forceinline const HVec2 HVec2::SafeNormalised( const HVec2 PCREF zeroLengthResult ) const
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = spu_length_squared2( m_V ); // length squared
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HVec2 result( UNDEFINED );
    result.m_V = spu_sel( zeroLengthResult.m_V, spu_mul( m_V, rcp_sqrt_B ), valid );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = vec_length_squared2( m_V ); // length squared
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HVec2 result( UNDEFINED );
    result.m_V = vec_sel( zeroLengthResult.m_V, vec_mul( m_V, rcp_sqrt_B ), valid );
    return result;
    
    #else
    
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Length2() );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec2 result( *this * ( Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() ) );
    //#else
    const HVec2 result( *this * ( length2.ReciprocalSqrt() ) );
    //#endif
    
    return Select( length2.GT( nearlyZero ), result, zeroLengthResult );
    
    #endif
}

/// Vector Normalise - safe version with additional parameter to use for zero length vectors!
NOALIAS __forceinline void HVec2::SafeNormalise( const HVec2 PCREF zeroLengthResult )
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = spu_length_squared2( m_V ); // length squared
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    m_V = spu_sel( zeroLengthResult.m_V, spu_mul( m_V, rcp_sqrt_B ), valid );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = vec_length_squared2( m_V ); // length squared
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    m_V = vec_sel( zeroLengthResult.m_V, vec_mul( m_V, rcp_sqrt_B ), valid );
    
    #else
    
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Length2() );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec2 result( *this * ( Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() ) );
    //#else
    const HVec2 result( *this * ( length2.ReciprocalSqrt() ) );
    //#endif
    
    #if defined( _SIMD_ )
    m_V = Select( length2.GT( nearlyZero ), result, zeroLengthResult ).m_V;
    #else
    *this = ( Select( length2.GT( nearlyZero ), result, zeroLengthResult ) );
    #endif
    
    #endif
}

/// Comparison Operator ==
NOALIAS __forceinline const Vector2_Bool HVec2::operator == ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_cmpeq( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = (vector bool int)vec_vcmpeqfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector2_Bool( SIMD_Vector_Selector_EQ( m_V, src.m_V ) );
    #else
    return Vector2_Bool( m_V[VX] == src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] == src.m_V[VY] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator !=
NOALIAS __forceinline const Vector2_Bool HVec2::operator != ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_cmpne( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    const vector bool int eq = (vector bool int)vec_vcmpeqfp( m_V, src.m_V );
    Vector2_Bool result( UNDEFINED );
    result.m_V = vec_vnor( eq, eq );
    return result;
    #elif defined( _SIMD_ )
    return Vector2_Bool( SIMD_Vector_Selector_NE( m_V, src.m_V ) );
    #else
    return Vector2_Bool( m_V[VX] != src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] != src.m_V[VY] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator <=
NOALIAS __forceinline const Vector2_Bool HVec2::operator <= ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_cmple( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = vec_vcmpgefp( src.m_V, m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector2_Bool( SIMD_Vector_Selector_LE( m_V, src.m_V ) );
    #else
    return Vector2_Bool( m_V[VX] <= src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] <= src.m_V[VY] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator <
NOALIAS __forceinline const Vector2_Bool HVec2::operator < ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_cmplt( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = vec_cmpgt( src.m_V, m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector2_Bool( SIMD_Vector_Selector_LT( m_V, src.m_V ) );
    #else
    return Vector2_Bool( m_V[VX] < src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] < src.m_V[VY] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator >=
NOALIAS __forceinline const Vector2_Bool HVec2::operator >= ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_cmpge( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = vec_vcmpgefp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector2_Bool( SIMD_Vector_Selector_GE( m_V, src.m_V ) );
    #else
    return Vector2_Bool( m_V[VX] >= src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] >= src.m_V[VY] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator >
NOALIAS __forceinline const Vector2_Bool HVec2::operator > ( const HVec2 PCREF src ) const
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_cmpgt( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = vec_cmpgt( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector2_Bool( SIMD_Vector_Selector_GT( m_V, src.m_V ) );
    #else
    return Vector2_Bool( m_V[VX] > src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] > src.m_V[VY] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Swizzle HVec2:XY into a HVec3:XZ with zero Y
NOALIAS __forceinline const HVec3 HVec2::ShuffleXYtoXZ() const
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_shuffle( m_V, Const< vector float >::Zero(), spu_shuffle_pattern_word<V0X,V1W,V0Y,V1W>() );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_mergeh( m_V, Const< vector float >::Zero() );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vmrghw( m_V, __vzero() );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Permute<VX,3,VY,3>( m_V ) );
    #else
    return HVec3( m_V[VX], 0.0f, m_V[VY] );
    #endif
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void HVec2::Deserialise( T_Stream& stream ) __restrict
{
    #if defined( _SPU_ )
    F32 x, y;
    stream >> x;
    stream >> y;
    //m_V = (vector float){ x, y, 0.0f, 0.0f };
    m_V = spu_insert( x, spu_insert( y, spu_splats( 0.0f ), VY ), VX );
    #elif defined( _PPU_ )
    F32 x, y;
    stream >> x;
    stream >> y;
    m_V = (vector float){ x, y, 0.0f, 0.0f };
    #elif defined( _SIMD_ )
    F32 x, y;
    stream >> x;
    stream >> y;
    m_V = SIMD_Vector2( x, y );
    #else
    stream >> m_V[VX];
    stream >> m_V[VY];
    #endif
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void HVec2::Serialise( T_Stream& stream ) __restrict
{
    stream << (F32)X();
    stream << (F32)Y();
}



#if defined( _SIMD_ )

/// Array Index Operator
NOALIAS __forceinline const VectorAccessor<HVec2> HVec2::operator [] ( const U32 index )
{
    assert( index < 2 );
    return VectorAccessor<HVec2>( *this, index );
}

/// Const Array Index Operator
NOALIAS __forceinline const ConstVecAccessor<HVec2> HVec2::operator [] ( const U32 index ) const
{
    assert( index < 2 );
    return ConstVecAccessor<HVec2>( *this, index );
}

#else

/// Array Index Operator
NOALIAS __forceinline F32& HVec2::operator [] ( const U32 index )
{
    assert( index < 2 );
    return m_V[ index ];
}

/// Const Array Index Operator
NOALIAS __forceinline const F32 HVec2::operator [] ( const U32 index ) const
{
    assert( index < 2 );
    return m_V[ index ];
}

#endif


NOALIAS __forceinline HVec2 operator * ( const HVec2 PCREF v, const F32& f ) { return v * HF32( f ); }
NOALIAS __forceinline HVec2 operator / ( const HVec2 PCREF v, const F32& f ) { return v / HF32( f ); }

NOALIAS __forceinline void operator *= ( HVec2& v, const F32& f ) { v *= HF32( f ); }
NOALIAS __forceinline void operator /= ( HVec2& v, const F32& f ) { v /= HF32( f ); }




// TODO: Other platform versions!
template < int DividePower2 >
NOALIAS __forceinline const HVec2 UnpackSignedHalfWordXY( U32 const * const __restrict pXY )
{
    #if defined( _SPU_ )
    
    //const vector signed int xy_int32( (vector signed int){ (S32)((S16*)pXY)[0], (S32)((S16*)pXY)[1], (S32)0, (S32)0 } );
    const unsigned int         xy( *pXY );
    //const vector unsigned char pattern( (vector unsigned char)( 0x80, 0x80, 0x00, 0x01,
    //                                                            0x80, 0x80, 0x02, 0x03,
    //                                                            0x80, 0x80, 0x80, 0x80,
    //                                                            0x80, 0x80, 0x80, 0x80 ) );
    const vector unsigned char pattern( Const< vector unsigned char >::Pattern_UnpackS16_XY() );
    const vector signed short  xy_int16( (vector signed short)spu_promote( xy, VX ) );
    const vector signed int    xy_int32( spu_extend( spu_shuffle( xy_int16, xy_int16, pattern ) ) );
    const vector float         xy_float32( spu_convtf( xy_int32, DividePower2 ) );
    HVec2 result( UNDEFINED );
    result.m_V = xy_float32;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector signed short xy_int16( vec_load_left_aligned( (S16*)pXY ) );
    const vector signed int   xy_int32( vec_vupkhsh( xy_int16 ) );
    const vector float        xy_float32( vec_vcfsx( xy_int32, DividePower2 ) );
    HVec2 result( UNDEFINED );
    result.m_V = xy_float32;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 xy_int16( __vspltw( __lvlx( pXY, 0x0 ), 0 ) );
    const __vector4 xy_int32( __vupkhsh( xy_int16 ) );
    const __vector4 xy_float32( __vcsxwfp( xy_int32, DividePower2 ) );
    
    HVec2 result( UNDEFINED );
    result.m_V = __vrlimi( xy_float32, __vzero(), 0, VMX_MASK_INSERT_ZW );
    return result;
    
    #else
    
    S16 const * const __restrict ptr = (S16*)pXY;
    const F32 x( (F32)ptr[0] / (F32)( 1 << DividePower2 ) );
    const F32 y( (F32)ptr[1] / (F32)( 1 << DividePower2 ) );
    return HVec2( x, y );
    
    #endif
}






//
// ---------------------------------------------------
// |-- HVec3 : Hardware 3 Element F32 Vector Class --|
// ---------------------------------------------------
//

#if defined( MATHS_DONT_INITIALISE_TO_ZERO )
/// Default Constructor
NOALIAS __forceinline HVec3::HVec3()
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
}

#else
/// Default Constructor
NOALIAS __forceinline HVec3::HVec3()
    #if defined( _SPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _VMX128_ )
    : m_V( __vzero() )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Zero() )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    #endif
}
#endif

/// Copy Constructor
NOALIAS __forceinline HVec3::HVec3( const HVec3& copy )
    #if defined( _SIMD_ )
    : m_V( copy.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = copy.m_V[VX];
    m_V[VY] = copy.m_V[VY];
    m_V[VZ] = copy.m_V[VZ];
    #endif
}

/// Zero Init Constructor
NOALIAS __forceinline HVec3::HVec3( const _ZERO_INIT /*zero_init*/ )
    #if defined( _SPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _VMX128_ )
    : m_V( __vzero() )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Zero() )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    #endif
}

/// Undefined Constructor
NOALIAS __forceinline HVec3::HVec3( const _UNDEFINED /*undefined*/ )
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
}

/// HVecX + HVecY + HVecZ Triple Constructor
NOALIAS __forceinline HVec3::HVec3( const HVecX PCREF x, const HVecY PCREF y, const HVecZ PCREF z )
    #if defined( _SPU_ )
    //: m_V( (vector float){ spu_extract( x.m_X, VX ), 
    //                       spu_extract( y.m_Y, VY ),
    //                       spu_extract( z.m_Z, VZ ),
    //                       0.0f } )
    : m_V( spu_insert( spu_extract( x.m_X, VX ),
           spu_insert( spu_extract( y.m_Y, VY ),
           spu_insert( spu_extract( z.m_Z, VZ ),
           spu_promote( 0.0f, VW ), VZ ), VY ), VX ) )
    #elif defined( _PPU_ )
    : m_V( vec_mergeh( vec_mergeh( vec_splat( x.m_X, VX ), vec_splat( z.m_Z, VZ ) ),
                       vec_mergeh( vec_splat( y.m_Y, VY ), Const< vector float >::Zero() ) ) )
    #elif defined( _VMX128_ )
        #if defined MATHS_VMX_ZERO_PAD_ELEMENTS
        : m_V( __vrlimi( __vrlimi( x.m_X, y.m_Y, VMX_MASK_INSERT_Y, 0 ), __vrlimi( z.m_Z, __vzero(), VMX_MASK_INSERT_W, 0 ), VMX_MASK_INSERT_ZW, 0 ) )
        #else
        : m_V( __vrlimi( __vrlimi( x.m_X, y.m_Y, VMX_MASK_INSERT_Y, 0 ), z.m_Z, VMX_MASK_INSERT_Z, 0 ) )
        #endif
    #elif defined( _SSE_ )
    : m_V( _mm_shuffle_ps( _mm_shuffle_ps( x.m_X, y.m_Y, _MM_SHUFFLE( 1, 1, 0, 0 ) ), z.m_Z, _MM_SHUFFLE( 3, 2, 2, 0 ) ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_OR( SIMD_OR( SIMD_Mask<1,0,0,0>( x.m_X ), SIMD_Mask<0,1,0,0>( y.m_Y ) ), SIMD_Mask<0,0,1,0>( z.m_Z ) ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = x.m_X;
    m_V[VY] = y.m_Y;
    m_V[VZ] = z.m_Z;
    #endif
}

/// HVecX Constructor
NOALIAS __forceinline HVec3::HVec3( const HVecX PCREF x )
    #if defined( _SPU_ )
    //: m_V( (vector float){ spu_extract( x.m_X, VX ), 0.0f, 0.0f, 0.0f } )
    : m_V( spu_insert( spu_extract( x.m_X, VX ), Const< vector float >::Zero(), VX ) )
    #elif defined( _PPU_ )
    : m_V( vec_mask<1,0,0,0>( x.m_X ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,0,0,0>( x.m_X ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = x.m_X;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    #endif
}

/// HVecY Constructor
NOALIAS __forceinline HVec3::HVec3( const HVecY PCREF y )
    #if defined( _SPU_ )
    //: m_V( (vector float){ 0.0f, spu_extract( y.m_Y, VY ), 0.0f, 0.0f } )
    : m_V( spu_insert( spu_extract( y.m_Y, VY ), Const< vector float >::Zero(), VY ) )
    #elif defined( _PPU_ )
    : m_V( vec_mask<0,1,0,0>( y.m_Y ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<0,1,0,0>( y.m_Y ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = y.m_Y;
    m_V[VZ] = 0.0f;
    #endif
}

/// HVecZ Constructor
NOALIAS __forceinline HVec3::HVec3( const HVecZ PCREF z )
    #if defined( _SPU_ )
    //: m_V( (vector float){ 0.0f, 0.0f, spu_extract( z.m_Z, VZ ), 0.0f } )
    : m_V( spu_insert( spu_extract( z.m_Z, VZ ), Const< vector float >::Zero(), VZ ) )
    #elif defined( _PPU_ )
    : m_V( vec_mask<0,0,1,0>( z.m_Z ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<0,0,1,0>( z.m_Z ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = z.m_Z;
    #endif
}

/// HVec2 Constructor
NOALIAS __forceinline HVec3::HVec3( const HVec2 PCREF src )
    #if defined( _SPU_ )
    : m_V( spu_insert( 0.0f, src.m_V, VZ ) )
    #elif defined( _PPU_ )
    : m_V( vec_mask<1,1,0,0>( src.m_V ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,1,0,0>( src.m_V ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V[VX];
    m_V[VY] = src.m_V[VY];
    m_V[VZ] = 0.0f;
    #endif
}

/// HVec4 Constructor: Explicit construction due to data loss!
NOALIAS __forceinline HVec3::HVec3( const HVec4 PCREF src )
    #if defined( _SPU_ )
    : m_V( src.m_V )
    #elif defined( _PPU_ )
    : m_V( src.m_V )
    #elif defined( _VMX128_ )
        #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
        : m_V( __vrlimi( src.m_V, __vzero(), VMX_MASK_INSERT_W, 0 ) )
        #else
        : m_V( src.m_V )
        #endif
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,1,1,0>( src.m_V ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V[VX];
    m_V[VY] = src.m_V[VY];
    m_V[VZ] = src.m_V[VZ];
    #endif
}

/// C_Vertex Constructor
NOALIAS __forceinline HVec3::HVec3( const C_Vertex PCREF src )
    #if defined( _SIMD_ )
    : m_V( src.m_V.m_V )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V[VX];
    m_V[VY] = src.m_V[VY];
    m_V[VZ] = src.m_V[VZ];
    #endif
}

/// C_Vector Constructor
NOALIAS __forceinline HVec3::HVec3( const C_Vector PCREF src )
    #if defined( _SIMD_ )
    : m_V( src.m_V.m_V )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V[VX];
    m_V[VY] = src.m_V[VY];
    m_V[VZ] = src.m_V[VZ];
    #endif
}

/// C_Normal Constructor
NOALIAS __forceinline HVec3::HVec3( const C_Normal PCREF src )
    #if defined( _SIMD_ )
    : m_V( src.m_V.m_V )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_V[VX];
    m_V[VY] = src.m_V[VY];
    m_V[VZ] = src.m_V[VZ];
    #endif
}

/// Explicit HF32 Broadcast Constructor
NOALIAS __forceinline HVec3::HVec3( const HF32 PCREF xyz )
    #if defined( _SPU_ )
    : m_V( xyz.m_F )
    #elif defined( _PPU_ )
    : m_V( xyz.m_F )
    #elif defined( _VMX128_ )
        #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
        : m_V( __vrlimi( xyz.m_F, __vzero(), VMX_MASK_INSERT_W, 0 ) )
        #else
        : m_V( xyz.m_F )
        #endif
    #elif defined( _SIMD_ )
    : m_V( SIMD_Vector3_Scalar( xyz.m_F ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = xyz.m_F;
    m_V[VY] = xyz.m_F;
    m_V[VZ] = xyz.m_F;
    #endif
}

/// HF32 Triple Constructor
NOALIAS __forceinline HVec3::HVec3( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z )
    #if defined( _SPU_ )
    //: m_V( (vector float){ spu_extract( x.m_F, VX ), spu_extract( y.m_F, VY ), spu_extract( z.m_F, VZ ), 0.0f } ) // Which is best?
    : m_V( spu_insert( 0.0f, spu_insert( spu_extract( z.m_F, VZ ), spu_insert( spu_extract( y.m_F, VY ), x.m_F, VY ), VZ ), VW ) )
    #elif defined( _PPU_ )
    : m_V( vec_mergeh( vec_mergeh( x.m_F, z.m_F ),
                       vec_mergeh( y.m_F, Const< vector float >::Zero() ) ) )
    #elif defined( _VMX128_ )
        #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
        : m_V( __vrlimi( __vrlimi( x.m_F, y.m_F, VMX_MASK_INSERT_Y, 0 ), __vrlimi( z.m_F, __vzero(), VMX_MASK_INSERT_W, 0 ), VMX_MASK_INSERT_ZW, 0 ) )
        #else
        : m_V( __vrlimi( __vrlimi( x.m_F, y.m_F, VMX_MASK_INSERT_Y, 0 ), z.m_F, VMX_MASK_INSERT_Z, 0 ) )
        #endif
    #elif defined( _SIMD_ )
    : m_V( SIMD_Vector3( x.m_F, y.m_F, z.m_F ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = x;
    m_V[VY] = y;
    m_V[VZ] = z;
    #endif
}

/// Explicit F32 Array Constructor
NOALIAS __forceinline HVec3::HVec3( F32 const * const __restrict pXYZ )
    #if defined( _SPU_ )
    //: m_V( (vector float){ pXYZ[0], pXYZ[1], pXYZ[2], 0.0f } )
    : m_V( spu_insert( 0.0f, spu_load_unaligned( pXYZ ), VW ) )
    #elif defined( _PPU_ )
    : m_V( vec_load_unaligned( pXYZ ) )
    #elif defined( _VMX128_ )
        #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
        : m_V( __vrlimi( __loadunalignedvector( pXYZ ), __vzero(), VMX_MASK_INSERT_W, 0 ) )
        #else
        : m_V( __loadunalignedvector( pXYZ ) )
        #endif
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,1,1,0>( SIMD_LoadUnaligned( pXYZ ) ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = pXYZ[VX];
    m_V[VY] = pXYZ[VY];
    m_V[VZ] = pXYZ[VZ];
    #endif
}

/// Explicit Constructor from Float3 ptr
NOALIAS __forceinline HVec3::HVec3( MathsStorage::Float3 const * const __restrict pStorage )
    #if defined( _SPU_ )
    //: m_V( (vector float){ pStorage->x, pStorage->y, pStorage->z, 0.0f } )
    : m_V( spu_insert( 0.0f, spu_load_unaligned( &pStorage->x ), VW ) )
    #elif defined( _PPU_ )
    : m_V( vec_load_unaligned( pStorage->v ) )
    #elif defined( _VMX128_ )
        #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
        : m_V( __vrlimi( __loadunalignedvector( pStorage ), __vzero(), VMX_MASK_INSERT_W, 0 ) )
        #else
        : m_V( __loadunalignedvector( pStorage ) )
        #endif
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,1,1,0>( SIMD_LoadUnaligned( (F32*)pStorage ) ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = pStorage->x;
    m_V[VY] = pStorage->y;
    m_V[VZ] = pStorage->z;
    #endif
}

/// Cast to Float3
NOALIAS __forceinline HVec3::operator const MathsStorage::Float3 () const __restrict
{
    MathsStorage::Float3 result;
    #if defined( _SPU_ )
    result.x = spu_extract( m_V, VX );
    result.y = spu_extract( m_V, VY );
    result.z = spu_extract( m_V, VZ );
    #elif defined( _PPU_ )
    vec_ste( vec_splat( m_V, VX ), 0, &result.x );
    vec_ste( vec_splat( m_V, VY ), 0, &result.y );
    vec_ste( vec_splat( m_V, VZ ), 0, &result.z );
    #elif defined( _VMX128_ )
    __stvewx( __vspltw( m_V, VX ), &result.x, 0 );
    __stvewx( __vspltw( m_V, VY ), &result.y, 0 );
    __stvewx( __vspltw( m_V, VZ ), &result.z, 0 );
    #elif defined( _SSE_ )
    _mm_store_ss( &result.x, m_V );
    _mm_store_ss( &result.y, _mm_shuffle_ps( m_V, m_V, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &result.z, _mm_shuffle_ps( m_V, m_V, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    #else
    result.x = m_V[VX];
    result.y = m_V[VY];
    result.z = m_V[VZ];
    #endif
    return result;
}

/// Explicit Constructor from U16 x, y, z
NOALIAS __forceinline HVec3::HVec3( U16 const * const __restrict pXYZ )
    #if defined( _SPU_ )
	: m_V( spu_convtf( ((vector unsigned int){ (unsigned int)pXYZ[0],
                                               (unsigned int)pXYZ[1],
                                               (unsigned int)pXYZ[2],
											   (unsigned int)0 } ), 0 ) )
    #elif defined( _PPU_ )
    //: m_V( vec_vcfux( vec_vmrghh(( (vector unsigned short)0, vec_load_unaligned( (const S16*)pXYZ ) )), 0 ) )
    #if defined( vec_splat_u16 )
    : m_V( vec_vcfux( (vector unsigned int)vec_vmrghh( vec_splat_u16( 0 ), (vector unsigned short)vec_load_unaligned( (const S16*)pXYZ ) ), 0 ) )
    #else
	: m_V( vec_vcfux( ((vector unsigned int)vec_vmrghh( (vector unsigned short)0, (vector unsigned short)vec_load_unaligned( (const S16*)pXYZ ) )), 0 ) )
    #endif
    #elif defined( _VMX128_ )
        #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
        : m_V( __vrlimi( __vcfux( __vmrghh( __vzero(), __loadunalignedvector( pXYZ ) ), 0 ), __vzero(), VMX_MASK_INSERT_W, 0 ) )
        #else
        : m_V( __vcfux( __vmrghh( __vzero(), __loadunalignedvector( pXYZ ) ), 0 ) )
        #endif
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if defined (_SSE_)
    *this = HVec3( (F32)pXYZ[0],
                   (F32)pXYZ[1],
                   (F32)pXYZ[2] );
    #elif !defined( _SIMD_ )
    m_V[VX] = (F32)pXYZ[0];
    m_V[VY] = (F32)pXYZ[1];
    m_V[VZ] = (F32)pXYZ[2];
    #endif
}

/// Explicit Constructor from Fixed3_S16 ptr
template < int T_Precision > NOALIAS __forceinline HVec3::HVec3( MathsStorage::Fixed3_S16<T_Precision> const * pStorage )
    #if defined( _SPU_ )
	: m_V( spu_convtf( ((vector signed int){ (signed int)pStorage->x,
                                             (signed int)pStorage->y,
                                             (signed int)pStorage->z,
											 (signed int)0 } ), T_Precision ) )
    #elif defined( _PPU_ )
    : m_V( vec_vcfsx( (vector signed int)vec_vupkhsh( (vector signed short)vec_load_unaligned( pStorage->v ) ), T_Precision ) )
    #elif defined( _VMX128_ )
        #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
        : m_V( __vrlimi( __vcfsx( __vupkhsh( __loadunalignedvector( pStorage ) ), T_Precision ), __vzero(), VMX_MASK_INSERT_W, 0 ) )
        #else
        : m_V( __vcfsx( __vupkhsh( __loadunalignedvector( pStorage ) ), T_Precision ) )
        #endif
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if defined (_SSE_)
    const F32 multiplier( 1.0f / (F32)( 1 << T_Precision ) );
    *this = HVec3( (F32)pStorage->x * multiplier,
                   (F32)pStorage->y * multiplier,
                   (F32)pStorage->z * multiplier);
    #elif !defined( _SIMD_ )
    const F32 multiplier( 1.0f / (F32)( 1 << T_Precision ) );
    m_V[VX] = (F32)pStorage->x * multiplier;
    m_V[VY] = (F32)pStorage->y * multiplier;
    m_V[VZ] = (F32)pStorage->z * multiplier;
    #endif
}

/// Cast to Fixed3_S16
template < int T_Precision > NOALIAS __forceinline HVec3::operator const MathsStorage::Fixed3_S16<T_Precision> () const __restrict
{
    MathsStorage::Fixed3_S16<T_Precision> result;
    #if defined( _SPU_ )
    const vector signed int temp = spu_convts( m_V, T_Precision );
    result.x = (S16)spu_extract( temp, VX );
    result.y = (S16)spu_extract( temp, VY );
    result.z = (S16)spu_extract( temp, VZ );
    #elif defined( _PPU_ )
    const vector signed short packed( vec_vpkswss( vec_vctsxs( m_V, T_Precision ), (vector signed int)vec_vspltisw( 0 ) ) );
    vec_stvehx( vec_vsplth( packed, 0 ), 0, &result.x );
    vec_stvehx( vec_vsplth( packed, 1 ), 0, &result.y );
    vec_stvehx( vec_vsplth( packed, 2 ), 0, &result.z );
    #elif defined( _VMX128_ )
    const __vector4 packed( __vpkswss( __vctsxs( m_V, T_Precision ), __vzero() ) );
    __stvehx( __vsplth( packed, 0 ), &result.x, 0 );
    __stvehx( __vsplth( packed, 1 ), &result.y, 0 );
    __stvehx( __vsplth( packed, 2 ), &result.z, 0 );
    #elif defined (_SSE_)
    const F32 multiplier( 1 << T_Precision );
    result.x = (S16)( X() * multiplier );
    result.y = (S16)( Y() * multiplier );
    result.z = (S16)( Z() * multiplier );   
    #else
    const F32 multiplier( 1 << T_Precision );
    result.x = (S16)( m_V[VX] * multiplier );
    result.y = (S16)( m_V[VY] * multiplier );
    result.z = (S16)( m_V[VZ] * multiplier );
    #endif
    return result;
}

/// Selector Constructor: Copies selector masks
NOALIAS __forceinline HVec3::HVec3( const Vector3_Bool PCREF selector )
    #if defined( _SPU_ )
    : m_V( (vector float)selector.m_V )
    #elif defined( _PPU_ )
    : m_V( (vector float)selector.m_V )
    #elif defined( _SIMD_ )
    : m_V( selector.m_V )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = (F32&)selector.m_X;
    m_V[VY] = (F32&)selector.m_Y;
    m_V[VZ] = (F32&)selector.m_Z;
    #endif
}

/// Selector Constructor: Selects fields from the inputs based on the selector mask: result = ( ( selector & a ) | ( ~selector & b ) )
NOALIAS __forceinline HVec3::HVec3( const Vector3_Bool PCREF selector, const HVec3 PCREF a, const HVec3 PCREF b )
    #if defined( _SPU_ )
    : m_V( spu_sel( b.m_V, a.m_V, selector.m_V ) )
    #elif defined( _PPU_ )
    : m_V( vec_sel( b.m_V, a.m_V, selector.m_V ) )
    #elif defined( _VMX128_ )
    : m_V( __vsel( b, a, selector.m_V ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Select( selector.m_V, a.m_V, b.m_V ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = selector.m_X != 0x00000000 ? a.m_V[VX] : b.m_V[VX];
    m_V[VY] = selector.m_Y != 0x00000000 ? a.m_V[VY] : b.m_V[VY];
    m_V[VZ] = selector.m_Z != 0x00000000 ? a.m_V[VZ] : b.m_V[VZ];
    #endif
}

/// HVec3_Multiply Constructor: Collapses the delayed multiply
NOALIAS __forceinline HVec3::HVec3( const HVec3_Multiply& src )
    #if defined( _SPU_ )
    : m_V( spu_mul( src.m_A, src.m_B ) )
    #elif defined( _PPU_ )
    : m_V( vec_mul( src.m_A, src.m_B ) )
    #elif defined( _VMX128_ )
    : m_V( __vmulfp( src.m_A, src.m_B ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Multiply( src.m_A.m_V, src.m_B.m_V ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_A.m_V[VX] * src.m_B.m_V[VX];
    m_V[VY] = src.m_A.m_V[VY] * src.m_B.m_V[VY];
    m_V[VZ] = src.m_A.m_V[VZ] * src.m_B.m_V[VZ];
    #endif
}

/// HVec3_Add Constructor: Collapses the delayed add
NOALIAS __forceinline HVec3::HVec3( const HVec3_Add& src )
    #if defined( _SPU_ )
    : m_V( spu_add( src.m_A, src.m_B ) )
    #elif defined( _PPU_ )
    : m_V( vec_add( src.m_A, src.m_B ) )
    #elif defined( _VMX128_ )
    : m_V( __vaddfp( src.m_A, src.m_B ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Add( src.m_A.m_V, src.m_B.m_V ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_A.m_V[VX] + src.m_B.m_V[VX];
    m_V[VY] = src.m_A.m_V[VY] + src.m_B.m_V[VY];
    m_V[VZ] = src.m_A.m_V[VZ] + src.m_B.m_V[VZ];
    #endif
}

/// HVec3_Subtract Constructor: Collapses the delayed subtract
NOALIAS __forceinline HVec3::HVec3( const HVec3_Subtract& src )
    #if defined( _SPU_ )
    : m_V( spu_sub( src.m_A, src.m_B ) )
    #elif defined( _PPU_ )
    : m_V( vec_vsubfp( src.m_A, src.m_B ) )
    #elif defined( _VMX128_ )
    : m_V( __vsubfp( src.m_A, src.m_B ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Subtract( src.m_A.m_V, src.m_B.m_V ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    #if !defined( _SIMD_ )
    m_V[VX] = src.m_A.m_V[VX] - src.m_B.m_V[VX];
    m_V[VY] = src.m_A.m_V[VY] - src.m_B.m_V[VY];
    m_V[VZ] = src.m_A.m_V[VZ] - src.m_B.m_V[VZ];
    #endif
}


#if defined( _SIMD_ )

/// V128 Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVec3::HVec3( const V128 src )
    : m_V( src )
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
}

/// V128 Cast
NOALIAS __forceinline HVec3::operator const V128 () const __restrict
{
    return m_V;
}

#else

/// V128 Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVec3::HVec3( const V128 src )
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif
    
    m_V[0] = src.m_X;
    m_V[1] = src.m_Y;
    m_V[2] = src.m_Z;
}

/// V128 Cast
NOALIAS __forceinline HVec3::operator const V128 () const __restrict
{
    V128 v;
    v.m_X = m_V[0];
    v.m_Y = m_V[1];
    v.m_Z = m_V[2];
    return v;
}

#endif

/// Validate that the elements are all finite and that the W element is zero
NOALIAS __forceinline void HVec3::DebugValidate() const __restrict
{
    #if defined( MATHS_DEBUG_MAX )
        #if defined( _VMX128_ )
            U32 CR1, CR2, CR3, CR4;
            __vcmpequwR( __vcmpbfp( __vspltw( m_V, VX ), c_VectorFltMax ), __vzero(), &CR1 );
            __vcmpequwR( __vcmpbfp( __vspltw( m_V, VY ), c_VectorFltMax ), __vzero(), &CR2 );
            __vcmpequwR( __vcmpbfp( __vspltw( m_V, VZ ), c_VectorFltMax ), __vzero(), &CR3 );
            __vcmpeqfpR( __vspltw( m_V, VW ), __vzero(), &CR4 );
            if ( !( CR1 & CR2 & CR3 & CR4 & VMX_CR6_ALL_TRUE ) )
            {
                DebugBreak();
            }
        #else
            if ( !_finite( X() ) || !_finite( Y() ) || !_finite( Z() ) )
            {
                //#ifdef SHOOTER_VECMAT_DEBUG_PRINT
                //DbgPrint( "HVec3::DebugValidate(): Non-Finite HVec3!" );
                //#endif
            }
            
            #if defined( _SIMD_ )
            if ( HVec4( m_V ).W() != 0.0f )
            {
                //#ifdef SHOOTER_VECMAT_DEBUG_PRINT
                //DbgPrint( "HVec3::DebugValidate(): Invalid HVec3!" );
                //#endif
            }
            #endif
        #endif
    #endif
}

/// HVecX Cast
NOALIAS __forceinline HVec3::operator const HVecX () const __restrict
{
    #if defined( _SPU_ )
    HVecX result( UNDEFINED );
    result.m_X = m_V;
    return result;
    #elif defined( _PPU_ )
    HVecX result( UNDEFINED );
    result.m_X = m_V;
    return result;
    #elif defined( _VMX128_ )
    HVecX result( UNDEFINED );
    result.m_X = __vrlimi( __vzero(), m_V, VMX_MASK_INSERT_X, 0 );
    return result;
    #elif defined( _SIMD_ )
    return HVecX( SIMD_Mask<1,0,0,0>( m_V ) );
    #else
    return HVecX( m_V[VX] );
    #endif
}

/// HVecY Cast
NOALIAS __forceinline HVec3::operator const HVecY () const __restrict
{
    #if defined( _SPU_ )
    HVecY result( UNDEFINED );
    result.m_Y = m_V;
    return result;
    #elif defined( _PPU_ )
    HVecY result( UNDEFINED );
    result.m_Y = m_V;
    return result;
    #elif defined( _VMX128_ )
    HVecY result( UNDEFINED );
    result.m_Y = __vrlimi( __vzero(), m_V, VMX_MASK_INSERT_Y, 0 );
    return result;
    #elif defined( _SIMD_ )
    return HVecY( SIMD_Mask<0,1,0,0>( m_V ) );
    #else
    return HVecY( m_V[VY] );
    #endif
}

/// HVecZ Cast
NOALIAS __forceinline HVec3::operator const HVecZ () const __restrict
{
    #if defined( _SPU_ )
    HVecZ result( UNDEFINED );
    result.m_Z = m_V;
    return result;
    #elif defined( _PPU_ )
    HVecZ result( UNDEFINED );
    result.m_Z = m_V;
    return result;
    #elif defined( _VMX128_ )
    HVecZ result( UNDEFINED );
    result.m_Z = __vrlimi( __vzero(), m_V, VMX_MASK_INSERT_Z, 0 );
    return result;
    #elif defined( _SIMD_ )
    return HVecZ( SIMD_Mask<0,0,1,0>( m_V ) );
    #else
    return HVecZ( m_V[VZ] );
    #endif
}

/// Copy Assignment
NOALIAS __forceinline const HVec3& HVec3::operator = ( const HVec3& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_V[VX] = copy[VX];
    m_V[VY] = copy[VY];
    m_V[VZ] = copy[VZ];
    #endif
    
    //return *this;
    return copy;
}

/// HVecX Assignment
NOALIAS __forceinline const HVec3& HVec3::operator = ( const HVecX PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_insert( spu_extract( src.m_X, VX ), Const< vector float >::Zero(), VX );
    #elif defined( _PPU_ )
    m_V = vec_mask<1,0,0,0>( src.m_X );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<1,0,0,0>( src.m_X );
    #else
    m_V[VX] = src.m_X;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    #endif
    return *this;
}

/// HVecY Assignment
NOALIAS __forceinline const HVec3& HVec3::operator = ( const HVecY PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_insert( spu_extract( src.m_Y, VY ), Const< vector float >::Zero(), VY );
    #elif defined( _PPU_ )
    m_V = vec_mask<0,1,0,0>( src.m_Y );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<0,1,0,0>( src.m_Y );
    #else
    m_V[VX] = 0.0f;
    m_V[VY] = src.m_Y;
    m_V[VZ] = 0.0f;
    #endif
    return *this;
}

/// HVecZ Assignment
NOALIAS __forceinline const HVec3& HVec3::operator = ( const HVecZ PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_insert( spu_extract( src.m_Z, VZ ), Const< vector float >::Zero(), VZ );
    #elif defined( _PPU_ )
    m_V = vec_mask<0,0,1,0>( src.m_Z );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<0,0,1,0>( src.m_Z );
    #else
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = src.m_Z;
    #endif
    return *this;
}

/// HVec2 Assignment
NOALIAS __forceinline const HVec3& HVec3::operator = ( const HVec2 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_insert( 0.0f, src.m_V, VZ );
    #elif defined( _PPU_ )
    m_V = vec_mask<1,1,0,0>( src.m_V );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( __vzero(), src.m_V, VMX_MASK_INSERT_XY, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<1,1,0,0>( src.m_V );
    #else
    m_V[VX] = src.m_V[VX];
    m_V[VY] = src.m_V[VY];
    m_V[VZ] = 0.0f;
    #endif
    
    return *this;
}

/// C_Vector Assignment
NOALIAS __forceinline const HVec3& HVec3::operator = ( const C_Vector PCREF src ) __restrict
{
    #if defined( _SIMD_ )
    m_V = src.m_V.m_V;
    #else
    m_V[VX] = src.m_V.m_V[VX];
    m_V[VY] = src.m_V.m_V[VY];
    m_V[VZ] = src.m_V.m_V[VZ];
    #endif
    
    return *this;
}

/// C_Vertex Assignment
NOALIAS __forceinline const HVec3& HVec3::operator = ( const C_Vertex PCREF src ) __restrict
{
    #if defined( _SIMD_ )
    m_V = src.m_V.m_V;
    #else
    m_V[VX] = src.m_V.m_V[VX];
    m_V[VY] = src.m_V.m_V[VY];
    m_V[VZ] = src.m_V.m_V[VZ];
    #endif
    
    return *this;
}

/// C_Normal Assignment
NOALIAS __forceinline const HVec3& HVec3::operator = ( const C_Normal PCREF src ) __restrict
{
    #if defined( _SIMD_ )
    m_V = src.m_V.m_V;
    #else
    m_V[VX] = src.m_V.m_V[VX];
    m_V[VY] = src.m_V.m_V[VY];
    m_V[VZ] = src.m_V.m_V[VZ];
    #endif
    
    return *this;
}

/// Accessor: Get X
NOALIAS __forceinline const HF32 HVec3::X() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VX>( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_V, VX );
    return result;
    #elif defined( _VMX128_ )
    HF32 result( UNDEFINED );
    result.m_F = __vspltw( m_V, VX );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_Vector3X( m_V ) );
    #else
    return HF32( m_V[VX] );
    #endif
}

/// Accessor: Get Y
NOALIAS __forceinline const HF32 HVec3::Y() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VY>( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_V, VY );
    return result;
    #elif defined( _VMX128_ )
    HF32 result( UNDEFINED );
    result.m_F = __vspltw( m_V, VY );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_Vector3Y( m_V ) );
    #else
    return HF32( m_V[VY] );
    #endif
}

/// Accessor: Get Z
NOALIAS __forceinline const HF32 HVec3::Z() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VZ>( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_V, VZ );
    return result;
    #elif defined( _VMX128_ )
    HF32 result( UNDEFINED );
    result.m_F = __vspltw( m_V, VZ );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_Vector3Z( m_V ) );
    #else
    return HF32( m_V[VZ] );
    #endif
}

/// Accessor: Get W - Required for templated accessors - should never be used!
NOALIAS __forceinline const HF32 HVec3::W() const __restrict
{
    assert( false );
    const HF32 zero(Constants<HF32>::Zero());
    return zero;
}

/// Accessor: Set X
NOALIAS __forceinline void HVec3::X( const HF32 PCREF scalar ) __restrict
{
    #if defined( _SPU_ )
    //m_V = spu_permute<V1X,V0Y,V0Z,V0W>( m_V, scalar.m_F );
    m_V = spu_select<1,0,0,0>( m_V, scalar.m_F );
    #elif defined( _PPU_ )
    m_V = vec_select<1,0,0,0>( m_V, scalar.m_F );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( m_V, scalar.m_F, VMX_MASK_INSERT_X, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_SetX( m_V, scalar.m_F );
    #else
    m_V[VX] = scalar.m_F;
    #endif
}

/// Accessor: Set Y
NOALIAS __forceinline void HVec3::Y( const HF32 PCREF scalar ) __restrict
{
    #if defined( _SPU_ )
    //m_V = spu_permute<V0X,V1Y,V0Z,V0W>( m_V, scalar.m_F );
    m_V = spu_select<0,1,0,0>( m_V, scalar.m_F );
    #elif defined( _PPU_ )
    m_V = vec_select<0,1,0,0>( m_V, scalar.m_F );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( m_V, scalar.m_F, VMX_MASK_INSERT_Y, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_SetY( m_V, scalar.m_F );
    #else
    m_V[VY] = scalar.m_F;
    #endif
}

/// Accessor: Set Z
NOALIAS __forceinline void HVec3::Z( const HF32 PCREF scalar ) __restrict
{
    #if defined( _SPU_ )
    //m_V = spu_permute<V0X,V0Y,V1Z,V0W>( m_V, scalar.m_F );
    m_V = spu_select<0,0,1,0>( m_V, scalar.m_F );
    #elif defined( _PPU_ )
    m_V = vec_select<0,0,1,0>( m_V, scalar.m_F );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( m_V, scalar.m_F, VMX_MASK_INSERT_Z, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_SetZ( m_V, scalar.m_F );
    #else
    m_V[VZ] = scalar.m_F;
    #endif
}

/// Accessor: Set W - Required for templated accessors - should never be used!
NOALIAS __forceinline void HVec3::W( const HF32 PCREF /*scalar*/ ) __restrict
{
    assert( false );
}

//#if defined( _SIMD_ )
///// Accessor: Get W
//NOALIAS __forceinline const HF32 HVec3::TestW() const __restrict
//{
//    #if defined( _SPU_ )
//    return HF32( spu_splat<VW>( m_V ) );
//    #elif defined( _PPU_ )
//    return HF32( vec_splat( m_V, VW ) );
//    #else
//    return HF32( SIMD_Scalar_Vector4W( m_V ) );
//    #endif
//}
//#endif

/// Zeroes element X
NOALIAS __forceinline void HVec3::ZeroX() __restrict
{
    #if defined( _SPU_ )
    m_V = spu_insert( 0.0f, m_V, VX );
    #elif defined( _PPU_ )
    m_V = vec_mask<0,1,1,0>( m_V );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_X, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<0,1,1,0>( m_V );
    #else
    m_V[VX] = 0.0f;
    #endif
}

/// Zeroes element Y
NOALIAS __forceinline void HVec3::ZeroY() __restrict
{
    #if defined( _SPU_ )
    m_V = spu_insert( 0.0f, m_V, VY );
    #elif defined( _PPU_ )
    m_V = vec_mask<1,0,1,0>( m_V );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_Y, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<1,0,1,0>( m_V );
    #else
    m_V[VY] = 0.0f;
    #endif
}

/// Zeroes element Z
NOALIAS __forceinline void HVec3::ZeroZ() __restrict
{
    #if defined( _SPU_ )
    m_V = spu_insert( 0.0f, m_V, VZ );
    #elif defined( _PPU_ )
    m_V = vec_mask<1,1,0,0>( m_V );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_Z, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<1,1,0,0>( m_V );
    #else
    m_V[VZ] = 0.0f;
    #endif
}

/// Zeroes elements XY
NOALIAS __forceinline void HVec3::ZeroXY() __restrict
{
    #if defined( _SPU_ )
    m_V = spu_select<1,1,0,1>( m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    m_V = vec_mask<0,0,1,0>( m_V );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_XY, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<0,0,1,0>( m_V );
    #else
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    #endif
}

/// Zeroes elements XZ
NOALIAS __forceinline void HVec3::ZeroXZ() __restrict
{
    #if defined( _SPU_ )
    m_V = spu_select<1,0,1,1>( m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    m_V = vec_mask<0,1,0,0>( m_V );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_XZ, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<0,1,0,0>( m_V );
    #else
    m_V[VX] = 0.0f;
    m_V[VZ] = 0.0f;
    #endif
}

/// Zeroes elements YZ
NOALIAS __forceinline void HVec3::ZeroYZ() __restrict
{
    #if defined( _SPU_ )
    m_V = spu_select<0,1,1,1>( m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    m_V = vec_mask<1,0,0,0>( m_V );
    #elif defined( _VMX128_ )
    m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_YZ, 0 );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<1,0,0,0>( m_V );
    #else
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    #endif
}

/// Zero the vector if any element exceeds +/- bounds
NOALIAS __forceinline void HVec3::ZeroIfOutOfBounds( const Maths::HF32 PCREF bounds ) __restrict
{
    #if defined( _SPU_ )
    const vector float absV = spu_abs( m_V );
    const vector unsigned int inBoundsXYZ = spu_cmplt( absV, bounds.m_F );
    const vector unsigned int inBounds = spu_and( spu_and( spu_splat<VX>( inBoundsXYZ ),
                                                           spu_splat<VY>( inBoundsXYZ ) ),
                                                           spu_splat<VZ>( inBoundsXYZ ) );
    m_V = spu_and( m_V, (vector float)inBounds );
    #elif defined( _PPU_ )
    const vector float absV = vec_abs( m_V );
    const vector bool int inBoundsXYZ = vec_cmpgt( bounds.m_F, absV );
    const vector bool int inBounds = vec_and( vec_and( vec_splat( inBoundsXYZ, VX ),
                                                         vec_splat( inBoundsXYZ, VY ) ),
                                                         vec_splat( inBoundsXYZ, VZ ) );
    m_V = vec_and( m_V, (vector float)inBounds );
    #elif defined( _VMX128_ )
    const __vector4 inBoundsXYZ( __vcmpequw( __vcmpbfp( m_V, bounds.m_F ), __vzero() ) );
    const __vector4 inBounds = __vand( __vand( __vspltw( inBoundsXYZ, VX ),
                                               __vspltw( inBoundsXYZ, VY ) ),
                                               __vspltw( inBoundsXYZ, VZ ) );
    m_V = __vsel( __vzero(), m_V, inBounds );
    #elif defined( _SIMD_ )
    if ( ( Abs( X() ) >= bounds ) |
         ( Abs( Y() ) >= bounds ) |
         ( Abs( Z() ) >= bounds ) )
    {
        m_V = SIMD_Zero();
    }
    #else
    if ( ( Abs( m_V[VX] ) >= bounds ) |
         ( Abs( m_V[VY] ) >= bounds ) |
         ( Abs( m_V[VZ] ) >= bounds ) )
    {
        m_V[VX] = 0.0f;
        m_V[VY] = 0.0f;
        m_V[VZ] = 0.0f;
    }
    #endif
}

/// Returns copy with zeroed element X
NOALIAS __forceinline const HVec3 HVec3::ZeroedX() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_select<1,0,0,1>( m_V, Const< vector float >::Zero() );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_mask<0,1,1,0>( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_X, 0 );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Mask<0,1,1,0>( m_V ) );
    #else
    return HVec3( 0.0f, m_V[VY], m_V[VZ] );
    #endif
}

/// Returns copy with zeroed element Y
NOALIAS __forceinline const HVec3 HVec3::ZeroedY() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_select<0,1,0,1>( m_V, Const< vector float >::Zero() );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_mask<1,0,1,0>( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_Y, 0 );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Mask<1,0,1,0>( m_V ) );
    #else
    return HVec3( m_V[VX], 0.0f, m_V[VZ] );
    #endif
}

/// Returns copy with zeroed element Z
NOALIAS __forceinline const HVec3 HVec3::ZeroedZ() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_select<0,0,1,1>( m_V, Const< vector float >::Zero() );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_mask<1,1,0,0>( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_Z, 0 );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Mask<1,1,0,0>( m_V ) );
    #else
    return HVec3( m_V[VX], m_V[VY], 0.0f );
    #endif
}

/// Returns copy with zeroed element XY
NOALIAS __forceinline const HVec3 HVec3::ZeroedXY() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_select<1,1,0,1>( m_V, Const< vector float >::Zero() );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_mask<0,0,1,0>( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_XY, 0 );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Mask<0,0,1,0>( m_V ) );
    #else
    return HVec3( 0.0f, 0.0f, m_V[VZ] );
    #endif
}

/// Returns copy with zeroed element XZ
NOALIAS __forceinline const HVec3 HVec3::ZeroedXZ() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_select<1,0,1,1>( m_V, Const< vector float >::Zero() );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_mask<0,1,0,0>( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_XZ, 0 );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Mask<0,1,0,0>( m_V ) );
    #else
    return HVec3( 0.0f, m_V[VY], 0.0f );
    #endif
}

/// Returns copy with zeroed element YZ
NOALIAS __forceinline const HVec3 HVec3::ZeroedYZ() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_select<0,1,1,1>( m_V, Const< vector float >::Zero() );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_mask<1,0,0,0>( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_YZ, 0 );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Mask<1,0,0,0>( m_V ) );
    #else
    return HVec3( m_V[VX], 0.0f, 0.0f );
    #endif
}

/// Shuffle
template < unsigned IX, unsigned IY, unsigned IZ >
NOALIAS __forceinline const HVec3 HVec3::Shuffle() const __restrict
{
    StaticAssert( IX < 3, IX_MUST_BE_LESS_THAN_3 );
    StaticAssert( IY < 3, IY_MUST_BE_LESS_THAN_3 );
    StaticAssert( IZ < 3, IZ_MUST_BE_LESS_THAN_3 );
    
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_shuffle( m_V, m_V, spu_shuffle_pattern_word<IX,IY,IZ,VW>() );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_permute<IX,IY,IZ,VW>( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vpermwi( m_V, VPERMWI_CONST( IX, IY, IZ, VW ) );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Permute<IX,IY,IZ,3>( m_V ) );
    #else
    return HVec3( m_V[IX], m_V[IY], m_V[IZ] );
    #endif
}

/// Min
SYS_DEPRECATED NOALIAS __forceinline const HVec3 HVec3::Min( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_min( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_vminfp( m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vminfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Min( m_V, src.m_V ) );
    #else
    return HVec3( Maths::Min( m_V[VX], src.m_V[VX] ),
                  Maths::Min( m_V[VY], src.m_V[VY] ),
                  Maths::Min( m_V[VZ], src.m_V[VZ] ) );
    #endif
}

/// Max
SYS_DEPRECATED NOALIAS __forceinline const HVec3 HVec3::Max( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_max( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_vmaxfp( m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vmaxfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Max( m_V, src.m_V ) );
    #else
    return HVec3( Maths::Max( m_V[VX], src.m_V[VX] ),
                  Maths::Max( m_V[VY], src.m_V[VY] ),
                  Maths::Max( m_V[VZ], src.m_V[VZ] ) );
    #endif
}

/// Min Update
SYS_DEPRECATED NOALIAS __forceinline void HVec3::MinEq( const HVec3 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_min( m_V, src.m_V );
    #elif defined( _PPU_ )
    m_V = vec_vminfp( m_V, src.m_V );
    #elif defined( _VMX128_ )
    m_V = __vminfp( m_V, src.m_V );
    #elif defined( _SIMD_ )
    m_V = SIMD_Min( m_V, src.m_V );
    #else
    m_V[VX] = Maths::Min( m_V[VX], src.m_V[VX] );
    m_V[VY] = Maths::Min( m_V[VY], src.m_V[VY] );
    m_V[VZ] = Maths::Min( m_V[VZ], src.m_V[VZ] );
    #endif
}

/// Max Update
SYS_DEPRECATED NOALIAS __forceinline void HVec3::MaxEq( const HVec3 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_max( m_V, src.m_V );
    #elif defined( _PPU_ )
    m_V = vec_vmaxfp( m_V, src.m_V );
    #elif defined( _VMX128_ )
    m_V = __vmaxfp( m_V, src.m_V );
    #elif defined( _SIMD_ )
    m_V = SIMD_Max( m_V, src.m_V );
    #else
    m_V[VX] = Maths::Max( m_V[VX], src.m_V[VX] );
    m_V[VY] = Maths::Max( m_V[VY], src.m_V[VY] );
    m_V[VZ] = Maths::Max( m_V[VZ], src.m_V[VZ] );
    #endif
}

/// Reciprocal of elements
SYS_DEPRECATED NOALIAS __forceinline const HVec3 HVec3::Reciprocal() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float B( m_V );
    const vector float zero = Const< vector float >::Zero();
    vector float rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec3 result;
    result.m_V = rcp_B;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float B( m_V );
    const vector float zero = Const< vector float >::Zero();
    vector float rcp_B;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec3 result;
    result.m_V = rcp_B;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 B( m_V );
    __vector4 rcp_B;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL( B, __vzero(), rcp_B, valid );
    
    HVec3 result;
    #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
    result.m_V = __vrlimi( rcp_B, __vzero(), VMX_MASK_INSERT_W, 0 );
    #else
    result.m_V = rcp_B;
    #endif
    return result;
    
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Reciprocal3( m_V ) );
    #else
    return HVec3( Maths::Reciprocal( m_V[VX] ),
                  Maths::Reciprocal( m_V[VY] ),
                  Maths::Reciprocal( m_V[VZ] ) );
    #endif
}

/// Horizontal sum of all elements
SYS_DEPRECATED NOALIAS __forceinline const HF32 HVec3::Sum() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_sum3( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_sum3( m_V );
    return result;
    #elif defined( _VMX128_ )
    const __vector4 one( c_VectorOne );
    HF32 result( UNDEFINED );
    result.m_F = __vmsum3fp( m_V, one );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Sum3( m_V ) );
    #else
    return HF32( m_V[VX] + m_V[VY] + m_V[VZ] );
    #endif
}

/// Vector Length Squared
SYS_DEPRECATED NOALIAS __forceinline const HF32 HVec3::Length2() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_length_squared3( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_length_squared3( m_V );
    return result;
    #elif defined( _VMX128_ )
    HF32 result( UNDEFINED );
    result.m_F = __vmsum3fp( m_V, m_V );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_DotProduct3( m_V, m_V ) );
    #else
    const HVec3 src = *this;
    return ( src * src ).Sum();
    #endif
}    

/// Vector Length Squared
SYS_DEPRECATED NOALIAS __forceinline const HF32 HVec3::LengthSquared() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_length_squared3( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_length_squared3( m_V );
    return result;
    #elif defined( _VMX128_ )
    HF32 result( UNDEFINED );
    result.m_F = __vmsum3fp( m_V, m_V );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_DotProduct3( m_V, m_V ) );
    #else
    const HVec3 src = *this;
    return ( src * src ).Sum();
    #endif
}    

/// Vector Length
SYS_DEPRECATED NOALIAS __forceinline const HF32 HVec3::Length() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float vec_len_squared = spu_length_squared3( m_V );
    const vector float zero = Const< vector float >::Zero();
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HF32 result( UNDEFINED );
    result.m_F = spu_mul( vec_len_squared, vec_rcp_len );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float vec_len_squared = vec_length_squared3( m_V );
    const vector float zero = Const< vector float >::Zero();
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HF32 result( UNDEFINED );
    result.m_F = vec_mul( vec_len_squared, vec_rcp_len );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_rcp_length ) );
    
    HF32 result( UNDEFINED );
    result.m_F = vector_length;
    return result;
    
    #else
    return Maths::Sqrt( Maths::LengthSquared( *this ) );
    #endif
}    

/// Vector Reciprocal of Length
SYS_DEPRECATED NOALIAS __forceinline const HF32 HVec3::RcpLength() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float vec_len_squared = spu_length_squared3( m_V );
    const vector float zero = Const< vector float >::Zero();
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HF32 result( UNDEFINED );
    result.m_F = vec_rcp_len;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float vec_len_squared = vec_length_squared3( m_V );
    const vector float zero = Const< vector float >::Zero();
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HF32 result( UNDEFINED );
    result.m_F = vec_rcp_len;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    
    HF32 result( UNDEFINED );
    result.m_F = vector_rcp_length;
    return result;
    
    #else
    return Maths::ReciprocalSqrt( Maths::LengthSquared( *this ) );
    #endif
}    

/// Vector Reciprocal of Length
SYS_DEPRECATED NOALIAS __forceinline const HF32 HVec3::ReciprocalLength() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float vec_len_squared = spu_length_squared3( m_V );
    const vector float zero = Const< vector float >::Zero();
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HF32 result( UNDEFINED );
    result.m_F = vec_rcp_len;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float vec_len_squared = vec_length_squared3( m_V );
    const vector float zero = Const< vector float >::Zero();
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HF32 result( UNDEFINED );
    result.m_F = vec_rcp_len;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    
    HF32 result( UNDEFINED );
    result.m_F = vector_rcp_length;
    return result;
    
    #else
    return Maths::ReciprocalSqrt( Maths::LengthSquared( *this ) );
    #endif
}    

/// Star function = Cross Product Matrix ( a* Matrix )
SYS_DEPRECATED NOALIAS __forceinline const HMat33 HVec3::Star() const __restrict
{
    #if defined( _SPU_ )
    
	const vector float pos_V( m_V );
	const vector float neg_V( spu_negate( m_V ) );
    const float posX = spu_extract( pos_V, VX );
    const float posY = spu_extract( pos_V, VY );
    const float posZ = spu_extract( pos_V, VZ );
    const float negX = spu_extract( neg_V, VX );
    const float negY = spu_extract( neg_V, VY );
    const float negZ = spu_extract( neg_V, VZ );
    //return HMat33( HVec3(  0.0f,    -m_V[VZ],   m_V[VY] ),
    //               HVec3(  m_V[VZ],  0.0f,     -m_V[VX] ),
    //               HVec3( -m_V[VY],  m_V[VX],   0.0f    ) );
    HMat33 result( UNDEFINED );
    //result.m_AX = (vector float)( 0.0f, negZ, posY, 0.0f );
    //result.m_AY = (vector float)( posZ, 0.0f, negX, 0.0f );
    //result.m_AZ = (vector float)( negY, posX, 0.0f, 0.0f );
    result.m_AX = spu_insert( 0.0f, spu_insert( negZ, spu_insert( posY, spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    result.m_AY = spu_insert( posZ, spu_insert( 0.0f, spu_insert( negX, spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    result.m_AZ = spu_insert( negY, spu_insert( posX, spu_insert( 0.0f, spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float posX = vec_splat( m_V, VX );
    const vector float posY = vec_splat( m_V, VY );
    const vector float posZ = vec_splat( m_V, VZ );
    const vector float negX = vec_negate( posX );
    const vector float negY = vec_negate( posY );
    const vector float negZ = vec_negate( posZ );
    const vector float ax = vec_mergeh( vec_mergeh( zero, posY ), vec_mergeh( negZ, zero ) );
    const vector float ay = vec_mergeh( vec_mergeh( posZ, negX ), zero );
    const vector float az = vec_mergeh( vec_mergeh( negY, zero ), vec_mergeh( posX, zero ) );
    HMat33 result( UNDEFINED );
    result.m_AX = ax;
    result.m_AY = ay;
    result.m_AZ = az;
    return result;
    
    #elif defined( _VMX128_ )
    
    static const unsigned __ = VW;
    const __vector4 signMask( c_VectorSignMask );
    const __vector4 xyz0( __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_W, 0 ) );
    const __vector4 xyz_neg( __vxor( m_V, signMask ) );
    const __vector4 cx( __vrlimi( __vpermwi( xyz0, VPERMWI_CONST(__,VZ,__,__) ), __vpermwi( xyz_neg, VPERMWI_CONST(__,__,VY,__) ), VMX_MASK_INSERT_Z, 0 ) );
    const __vector4 cy( __vrlimi( __vpermwi( xyz0, VPERMWI_CONST(__,__,VX,__) ), __vpermwi( xyz_neg, VPERMWI_CONST(VZ,__,__,__) ), VMX_MASK_INSERT_X, 0 ) );
    const __vector4 cz( __vrlimi( __vpermwi( xyz0, VPERMWI_CONST(VY,__,__,__) ), __vpermwi( xyz_neg, VPERMWI_CONST(__,VX,__,__) ), VMX_MASK_INSERT_Y, 0 ) );
    
    HMat33 result( UNDEFINED );
    result.m_CX = cx;
    result.m_CY = cy;
    result.m_CZ = cz;
    return result;
    
    #elif defined( _SIMD_ )
    
    static const unsigned __ = VW;
    const HVec4 xyz0( *this );
    const HVec3 ax( SIMD_Subtract( SIMD_Permute<__,__,VY,__>( xyz0 ), SIMD_Permute<__,VZ,__,__>( xyz0 ) ) );
    const HVec3 ay( SIMD_Subtract( SIMD_Permute<VZ,__,__,__>( xyz0 ), SIMD_Permute<__,__,VX,__>( xyz0 ) ) );
    const HVec3 az( SIMD_Subtract( SIMD_Permute<__,VX,__,__>( xyz0 ), SIMD_Permute<VY,__,__,__>( xyz0 ) ) );
    return HMat33( ax, ay, az );
    
    #else
    return HMat33( HVec3(  0.0f,    -m_V[VZ],   m_V[VY] ),
                   HVec3(  m_V[VZ],  0.0f,     -m_V[VX] ),
                   HVec3( -m_V[VY],  m_V[VX],   0.0f    ) );
    #endif
}

SYS_DEPRECATED NOALIAS __forceinline const HVec3 HVec3::Transformed( const C_Quaternion PCREF rotation, const HVec3 PCREF translation ) const __restrict
{	
	return *this * rotation + translation;
}

/// Vertex Distance
SYS_DEPRECATED NOALIAS __forceinline const HF32 HVec3::Distance( const HVec3 PCREF v ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float point_A( m_V );
    const vector float point_B( v.m_V );
    const vector float vec( spu_sub( point_B, point_A ) );
    const vector float vec_len_squared( spu_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HF32 result( UNDEFINED );
    result.m_F = spu_mul( vec_len_squared, vec_rcp_len );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float point_A( m_V );
    const vector float point_B( v.m_V );
    const vector float vec( vec_vsubfp( point_B, point_A ) );
    const vector float vec_len_squared( vec_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HF32 result( UNDEFINED );
    result.m_F = vec_mul( vec_len_squared, vec_rcp_len );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 point_A( m_V );
    const __vector4 point_B( v.m_V );
    const __vector4 vector( __vsubfp( point_B, point_A ) );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_rcp_length ) );
    HF32 result( UNDEFINED );
    result.m_F = vector_length;
    return result;
    
    #else
    return Maths::Length( *this - v );
    #endif
}

/// Vertex Distance Squared
SYS_DEPRECATED NOALIAS __forceinline const HF32 HVec3::DistanceSquared( const HVec3 PCREF v ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float point_A( m_V );
    const vector float point_B( v.m_V );
    const vector float vec( spu_sub( point_B, point_A ) );
    const vector float distance2( spu_length_squared3( vec ) );
    HF32 result( UNDEFINED );
    result.m_F = distance2;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float point_A( m_V );
    const vector float point_B( v.m_V );
    const vector float vec( vec_sub( point_B, point_A ) );
    const vector float distance2( vec_length_squared3( vec ) );
    HF32 result( UNDEFINED );
    result.m_F = distance2;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 point_A( m_V );
    const __vector4 point_B( v.m_V );
    const __vector4 vector( __vsubfp( point_B, point_A ) );
    const __vector4 distance2( __vmsum3fp( vector, vector ) );
    HF32 result( UNDEFINED );
    result.m_F = distance2;
    return result;
    
    #else
    return Maths::LengthSquared( *this - v );
    #endif
}

/// Lerp
SYS_DEPRECATED NOALIAS __forceinline const HVec3 HVec3::Lerp( const HVec3 PCREF dst, const HF32 PCREF f ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float v_src( m_V );
    const vector float v_dst( dst.m_V );
    const vector float v_f( f.m_F );
    HVec3 result( UNDEFINED );
    result.m_V = spu_madd( spu_sub( v_dst, v_src ), v_f, v_src );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float v_src( m_V );
    const vector float v_dst( dst.m_V );
    const vector float v_f( f.m_F );
    HVec3 result( UNDEFINED );
    result.m_V = vec_madd( vec_sub( v_dst, v_src ), v_f, v_src );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 v_src( m_V );
    const __vector4 v_dst( dst.m_V );
    const __vector4 v_src_to_dst( __vsubfp( v_dst, v_src ) );
    const __vector4 v_f( f.m_F );
    const __vector4 x( __vmaddfp( v_src_to_dst, v_f, v_src ) );
    HVec3 result( UNDEFINED );
    result.m_V = x;
    return result;
    
    #else
    const HVec3 src = *this;
    return src + ( ( dst - src ) * f );
    #endif
}

/// Vector Normalised
SYS_DEPRECATED NOALIAS __forceinline const C_Normalised HVec3::Normalised() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float vec( m_V );
    const vector float vec_len_squared( spu_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    C_Normalised result( UNDEFINED );
    result.m_Normal.m_V.m_V = spu_mul( vec, vec_rcp_len );
    result.m_Length.m_F     = spu_mul( vec_len_squared, vec_rcp_len );
    result.m_Length2.m_F    = vec_len_squared;
    result.m_RcpLength.m_F  = vec_rcp_len;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float vec( m_V );
    const vector float vec_len_squared( vec_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    C_Normalised result( UNDEFINED );
    result.m_Normal.m_V.m_V = vec_mul( vec, vec_rcp_len );
    result.m_Length.m_F     = vec_mul( vec_len_squared, vec_rcp_len );
    result.m_Length2.m_F    = vec_len_squared;
    result.m_RcpLength.m_F  = vec_rcp_len;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_rcp_length ) );
    const __vector4 normal( __vmulfp( vector, vector_rcp_length ) );
    
    C_Normalised result( UNDEFINED );
    result.m_Normal.m_V.m_V = normal;
    result.m_Length.m_F     = vector_length;
    result.m_Length2.m_F    = vector_length_squared;
    result.m_RcpLength.m_F  = vector_rcp_length;
    return result;
    
    #else
    
    const HVec3 vZero;
    const HF32  nearlyZero( c_ScalarFltMin );
    const HF32  length2( Maths::LengthSquared( *this ) );
    
    //#if defined( _DEBUG )
    //const HF32 zero( Constants<HF32>::Zero() );
    //const HF32 reciprocalLength( Maths::Select( length2.GT( nearlyZero ), length2, zero ).ReciprocalSqrt() );
    //#else
    const HF32 reciprocalLength( length2.ReciprocalSqrt() );
    //#endif
    
    const HVec3 normalisedVector( *this * reciprocalLength );
    
    #if defined( MATHS_DEBUG_BREAK_ON_ZERO_LENGTH_NORMALISE )
    if ( length2 <= nearlyZero )
    {
        DebugBreak();
    }
    #endif
    
    const C_Normal normal( Maths::Select( length2.GT( nearlyZero ), normalisedVector, vZero ), C_Normal::UNIT_LENGTH_VECTOR );
    return C_Normalised( normal, length2 * reciprocalLength, length2, reciprocalLength );
    
    #endif
}

/// Vector Normalise
SYS_DEPRECATED NOALIAS __forceinline void HVec3::Normalise() __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero( Const< vector float >::Zero() );
    const vector float vec( m_V );
    const vector float vec_len_squared( spu_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    m_V = spu_mul( vec, vec_rcp_len );
    
    #elif defined( _PPU_ )
    
    const vector float zero( Const< vector float >::Zero() );
    const vector float vec( m_V );
    const vector float vec_len_squared( vec_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    m_V = vec_mul( vec, vec_rcp_len );
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 normal( __vmulfp( vector, vector_rcp_length ) );
    m_V = normal;
    
    #else
    
    const HVec3 vZero;
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Maths::LengthSquared( *this ) );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec3 result( *this * Maths::Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() );
    //#else
    const HVec3 result( *this * length2.ReciprocalSqrt() );
    //#endif
    
    #if defined( MATHS_DEBUG_BREAK_ON_ZERO_LENGTH_NORMALISE )
    if ( length2 <= nearlyZero )
    {
        DebugBreak();
    }
    #endif
    
    #if defined( _SIMD_ )
    m_V = Maths::Select( length2.GT( nearlyZero ), result, vZero ).m_V;
    #else
    *this = (  Maths::Select( length2.GT( nearlyZero ), result, HVec3() ) );
    #endif
    
    #endif
}

// If the vector's length is longer the specified value, reduces it length to that value. 
NOALIAS __forceinline void HVec3::LimitLength( const HF32 PCREF limit ) __restrict
{
    *this = VectorOfMagnitude( Maths::Length( *this ).Min( limit ) );
}

/// Vector Normalise - safe version with additional parameter to use for zero length vectors!
NOALIAS __forceinline const C_Normalised HVec3::SafeNormalised( const HVec3 PCREF zeroLengthResult ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float vec( m_V );
    const vector float invalid_result( zeroLengthResult.m_V );
    const vector float vec_len_squared = spu_length_squared3( vec );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, invalid_result, vec_rcp_len, valid );
    C_Normalised result( UNDEFINED );
    result.m_Normal.m_V.m_V = spu_mul( vec, vec_rcp_len );
    result.m_Length.m_F     = spu_mul( vec_len_squared, vec_rcp_len );
    result.m_Length2.m_F    = vec_len_squared;
    result.m_RcpLength.m_F  = vec_rcp_len;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float vec( m_V );
    const vector float invalid_result( zeroLengthResult.m_V );
    const vector float vec_len_squared = vec_length_squared3( vec );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, invalid_result, vec_rcp_len, valid );
    C_Normalised result( UNDEFINED );
    result.m_Normal.m_V.m_V = vec_mul( vec, vec_rcp_len );
    result.m_Length.m_F     = vec_mul( vec_len_squared, vec_rcp_len );
    result.m_Length2.m_F    = vec_len_squared;
    result.m_RcpLength.m_F  = vec_rcp_len;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 invalid_result( zeroLengthResult.m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, invalid_result, vector_rcp_length, valid );
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_rcp_length ) );
    const __vector4 normal( __vmulfp( vector, vector_rcp_length ) );
    
    C_Normalised result( UNDEFINED );
    result.m_Normal.m_V.m_V = normal;
    result.m_Length.m_F     = vector_length;
    result.m_Length2.m_F    = vector_length_squared;
    result.m_RcpLength.m_F  = vector_rcp_length;
    return result;
    
    #else
    
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Maths::LengthSquared( *this ) );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HF32 reciprocalLength( Maths::Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() );
    //#else
    const HF32 reciprocalLength( length2.ReciprocalSqrt() );
    //#endif
    
    const HVec3 normalisedVector( *this * reciprocalLength );
    
    const C_Normal normal( Maths::Select( length2.GT( nearlyZero ), normalisedVector, zeroLengthResult ), C_Normal::UNIT_LENGTH_VECTOR );
    return C_Normalised( normal, length2 * reciprocalLength, length2, reciprocalLength );
    
    #endif
}

/// Vector Normalise - safe version with additional parameter to use for zero length vectors!
NOALIAS __forceinline void HVec3::SafeNormalise( const HVec3 PCREF zeroLengthResult ) __restrict
{
    #if defined( _SPU_ )
    
    const vector float vec( m_V );
    const vector float invalid_result( zeroLengthResult.m_V );
    const vector float vec_len_squared = spu_length_squared3( vec );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, invalid_result, vec_rcp_len, valid );
    m_V = spu_mul( vec, vec_rcp_len );
    
    #elif defined( _PPU_ )
    
    const vector float vec( m_V );
    const vector float invalid_result( zeroLengthResult.m_V );
    const vector float vec_len_squared = vec_length_squared3( vec );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, invalid_result, vec_rcp_len, valid );
    m_V = vec_mul( vec, vec_rcp_len );
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 invalid_result( zeroLengthResult.m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, invalid_result, vector_rcp_length, valid );
    const __vector4 normal( __vmulfp( vector, vector_rcp_length ) );
    
    m_V = normal;
    
    #else
    
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Maths::LengthSquared( *this ) );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec3 result( *this * Maths::Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() );
    //#else
    const HVec3 result( *this * length2.ReciprocalSqrt() );
    //#endif
    
    #if defined( _SIMD_ )
    m_V = Maths::Select( length2.GT( nearlyZero ), result, zeroLengthResult ).m_V;
    #else
    *this = ( Maths::Select( length2.GT( nearlyZero ), result, zeroLengthResult ) );
    #endif
    
    #endif
}

/// Get Vector of Magnitude
NOALIAS __forceinline const HVec3 HVec3::VectorOfMagnitude( const HF32 PCREF f ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float vec( m_V );
    const vector float mag( f.m_F );
    const vector float zero = Const< vector float >::Zero();
    const vector float vec_x_mag( spu_mul( vec, mag ) );
    const vector float vec_len_squared = spu_length_squared3( vec );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HVec3 result( UNDEFINED );
    result.m_V = spu_mul( vec_x_mag, vec_rcp_len );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float vec( m_V );
    const vector float mag( f.m_F );
    const vector float zero = Const< vector float >::Zero();
    const vector float vec_x_mag( vec_mul( vec, mag ) );
    const vector float vec_len_squared = vec_length_squared3( vec );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, zero, vec_rcp_len, valid );
    HVec3 result( UNDEFINED );
    result.m_V = vec_mul( vec_x_mag, vec_rcp_len );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 magnitude( f.m_F );
    const __vector4 vector_length2( __vmsum3fp( vector, vector ) );
    const __vector4 vector_x_magnitude( __vmulfp( vector, magnitude ) );
    
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length2, __vzero(), vector_rcp_length, valid );
    const __vector4 new_vector( __vmulfp( vector_x_magnitude, vector_rcp_length ) );
    
    HVec3 result( UNDEFINED );
    result.m_V = new_vector;
    return result;
    
    #else
    return *this * ( f * Maths::ReciprocalLength( *this ) );
    #endif
}

/// Get component of this vector parallel to input normal
NOALIAS __forceinline const HVec3 HVec3::Parallel( const C_Normal PCREF normal ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float v( m_V );
    const vector float n( normal.m_V );
    const vector float dp = spu_dot_product3( v, n );
    HVec3 result( UNDEFINED );
    result.m_V = spu_mul( n, dp );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float v( m_V );
    const vector float n( normal.m_V );
    const vector float dp = vec_dot_product3( v, n );
    HVec3 result( UNDEFINED );
    result.m_V = vec_mul( n, dp );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 v( m_V );
    const __vector4 n( normal.m_V );
    const __vector4 dp( __vmsum3fp( v, n ) );
    const __vector4 v_para( __vmulfp( n, dp ) );
    HVec3 result( UNDEFINED );
    result.m_V = v_para;
    return result;
    
    #else
    const HF32 dp( *this | normal.m_V );
    return normal.m_V * dp;
    #endif
}

/// Get component of this vector perpendicular to input normal
NOALIAS __forceinline const HVec3 HVec3::Perpendicular( const C_Normal PCREF normal ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float v( m_V );
    const vector float n( normal.m_V );
    const vector float dp = spu_dot_product3( v, n );
    HVec3 result( UNDEFINED );
    result.m_V = spu_nmsub( n, dp, v );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float v( m_V );
    const vector float n( normal.m_V );
    const vector float dp = vec_dot_product3( v, n );
    HVec3 result( UNDEFINED );
    result.m_V = vec_nmsub( n, dp, v );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 v( m_V );
    const __vector4 n( normal.m_V );
    const __vector4 dp( __vmsum3fp( v, n ) );
    const __vector4 v_perp( __vnmsubfp( n, dp, v ) );
    
    HVec3 result( UNDEFINED );
    result.m_V = v_perp;
    return result;
    
    #else
    return *this - ( Parallel( normal ) );
    #endif
}

/// Get components of this vector parallel & perpendicular to input normal
NOALIAS __forceinline void HVec3::ParaPerp( const C_Normal PCREF normal, HVec3& result_para, HVec3& result_perp ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float v( m_V );
    const vector float n( normal.m_V );
    const vector float dp = spu_dot_product3( v, n );
    result_para.m_V = spu_mul( n, dp );
    result_perp.m_V = spu_nmsub( n, dp, v );
    
    #elif defined( _PPU_ )
    
    const vector float v( m_V );
    const vector float n( normal.m_V );
    const vector float dp = vec_dot_product3( v, n );
    result_para.m_V = vec_mul( n, dp );
    result_perp.m_V = vec_nmsub( n, dp, v );
    
    #elif defined( _VMX128_ )
    
    const __vector4 v( m_V );
    const __vector4 n( normal.m_V );
    const __vector4 dp( __vmsum3fp( v, n ) );
    result_para.m_V = __vmulfp( n, dp );
    result_perp.m_V = __vnmsubfp( n, dp, v );
    
    #else
    const HVec3 para( Parallel( normal ) );
    result_para = para;
    result_perp = *this - ( para );
    #endif
}

/// Get axes perpendicular to vector.
NOALIAS __forceinline void HVec3::PerpendicularAxes( HVec3& u, HVec3& v ) const __restrict
{
    // TODO: VMX OPTIMISED VERSION?
    
	const Maths::AxisZ axisZ( Maths::Normalise( *this ) );
    const HMat33 orientation( axisZ );
    u = orientation.AX();
    v = orientation.AY();
}

/// Shuffle HVec3:XZ into a HVec2:XY
NOALIAS __forceinline const HVec2 HVec3::ShuffleXZtoXY() const __restrict
{
    #if defined( _SPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = spu_permute<VX,VZ,VW,VW>( m_V );
    return result;
    #elif defined( _PPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = vec_permute<VX,VZ,VW,VW>( m_V );
    return result;
    #elif defined( _VMX128_ )
    const __vector4 xyz( m_V );
    const __vector4 xz( __vpermwi( xyz, VPERMWI_CONST( VX, VZ, VW, VW ) ) );
    HVec2 result( UNDEFINED );
    result.m_V = xz;
    return result;
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Permute<VX,VZ,3,3>( m_V ) );
    #else
    return HVec2( m_V[VX], m_V[VZ] );
    #endif
}

/// Round to integer towards minus infinity
NOALIAS __forceinline const HVec3 HVec3::RoundToIntMinusInf() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_rfim( m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_vrfim( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrfim( m_V );
    return result;
    #elif defined( _SSE_ )
    return HVec3( SSE_RoundToIntMinusInf( m_V ) );
    #else
    return HVec3( floorf( m_V[VX] ),
                  floorf( m_V[VY] ),
                  floorf( m_V[VZ] ) );
    #endif
}

/// Round to integer towards plus infinity
NOALIAS __forceinline const HVec3 HVec3::RoundToIntPlusInf() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_rfip( m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_vrfip( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrfip( m_V );
    return result;
    #elif defined( _SSE_ )
    return HVec3( SSE_RoundToIntPlusInf( m_V ) );
    #else
    return HVec3( ceilf( m_V[VX] ),
                  ceilf( m_V[VY] ),
                  ceilf( m_V[VZ] ) );
    #endif
}

/// Round to integer towards nearest
NOALIAS __forceinline const HVec3 HVec3::RoundToIntNearest() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_rfin( m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_vrfin( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrfin( m_V );
    return result;
    #elif defined( _SSE_ )
    return HVec3( SSE_RoundToIntNearest( m_V ) );
    #else
    return HVec3( _copysign( floorf( fabsf( m_V[VX] ) + 0.5f ), m_V[VX] ),
                  _copysign( floorf( fabsf( m_V[VY] ) + 0.5f ), m_V[VY] ),
                  _copysign( floorf( fabsf( m_V[VZ] ) + 0.5f ), m_V[VZ] ) );
    #endif
}

/// Round to integer towards zero
NOALIAS __forceinline const HVec3 HVec3::RoundToIntZero() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_rfiz( m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_vrfiz( m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrfiz( m_V );
    return result;
    #elif defined( _SSE_ )
    return HVec3( SSE_RoundToIntZero( m_V ) );
    #else
    return HVec3( (F32)(S32)m_V[VX],
                  (F32)(S32)m_V[VY],
                  (F32)(S32)m_V[VZ] );
    #endif
}

/// Comparison Operator ==
NOALIAS __forceinline const Vector3_Bool HVec3::operator == ( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_cmpeq( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    //result.m_V = vec_vcmpeqfp( m_V, src.m_V );
    result.m_V = vec_cmpeq( m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = __vcmpeqfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_Vector_Selector_EQ( m_V, src.m_V ) );
    #else
    return Vector3_Bool( m_V[VX] == src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] == src.m_V[VY] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VZ] == src.m_V[VZ] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator !=
NOALIAS __forceinline const Vector3_Bool HVec3::operator != ( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_cmpne( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    const vector bool int eq( vec_cmpeq( m_V, src.m_V ) );
    Vector3_Bool result( UNDEFINED );
    result.m_V = vec_vnor( eq, eq );
    return result;
    #elif defined( _VMX128_ )
    const __vector4 eq( __vcmpeqfp( m_V, src.m_V ) );
    Vector3_Bool result( UNDEFINED );
    result.m_V = __vnor( eq, eq );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_Vector_Selector_NE( m_V, src.m_V ) );
    #else
    return Vector3_Bool( m_V[VX] != src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] != src.m_V[VY] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VZ] != src.m_V[VZ] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator <=
NOALIAS __forceinline const Vector3_Bool HVec3::operator <= ( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_cmple( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = vec_vcmpgefp( src.m_V, m_V );
    return result;
    #elif defined( _VMX128_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = __vcmpgefp( src.m_V, m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_Vector_Selector_LE( m_V, src.m_V ) );
    #else
    return Vector3_Bool( m_V[VX] <= src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] <= src.m_V[VY] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VZ] <= src.m_V[VZ] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator <
NOALIAS __forceinline const Vector3_Bool HVec3::operator < ( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_cmplt( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = vec_cmpgt( src.m_V, m_V );
    return result;
    #elif defined( _VMX128_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = __vcmpgtfp( src.m_V, m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_Vector_Selector_LT( m_V, src.m_V ) );
    #else
    return Vector3_Bool( m_V[VX] < src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] < src.m_V[VY] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VZ] < src.m_V[VZ] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator >=
NOALIAS __forceinline const Vector3_Bool HVec3::operator >= ( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_cmpge( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = vec_vcmpgefp( m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = __vcmpgefp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_Vector_Selector_GE( m_V, src.m_V ) );
    #else
    return Vector3_Bool( m_V[VX] >= src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] >= src.m_V[VY] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VZ] >= src.m_V[VZ] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Comparison Operator >
NOALIAS __forceinline const Vector3_Bool HVec3::operator > ( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_cmpgt( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = vec_cmpgt( m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = __vcmpgtfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_Vector_Selector_GT( m_V, src.m_V ) );
    #else
    return Vector3_Bool( m_V[VX] > src.m_V[VX] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VY] > src.m_V[VY] ? 0xFFFFFFFF : 0x00000000,
                         m_V[VZ] > src.m_V[VZ] ? 0xFFFFFFFF : 0x00000000 );
    #endif
}


/// Comparison Function ==
NOALIAS __forceinline const U32 HVec3::All_EQ( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int eq = spu_cmpeq( m_V, src.m_V );
    return spu_extract( eq, VX ) &
           spu_extract( eq, VY ) &
           spu_extract( eq, VZ ) & 1;
    #elif defined( _PPU_ )
	const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_all_eq( a , b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a_xyzz, b_xyzz, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_All_EQ( m_V, src.m_V );
    #else
    return ( m_V[VX] == src.m_V[VX] ) &&
           ( m_V[VY] == src.m_V[VY] ) &&
           ( m_V[VZ] == src.m_V[VZ] );
    #endif
}

/// Comparison Function !=
NOALIAS __forceinline const U32 HVec3::All_NE( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int ne = spu_cmpne( m_V, src.m_V );
    return spu_extract( ne, VX ) &
           spu_extract( ne, VY ) &
           spu_extract( ne, VZ ) & 1;
    #elif defined( _PPU_ )
	const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_all_ne( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a_xyzz, b_xyzz, &CR );
    return ( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_All_NE( m_V, src.m_V );
    #else
    return ( m_V[VX] != src.m_V[VX] ) &&
           ( m_V[VY] != src.m_V[VY] ) &&
           ( m_V[VZ] != src.m_V[VZ] );
    #endif
}

/// Comparison Function <=
NOALIAS __forceinline const U32 HVec3::All_LE( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int le = spu_cmple( m_V, src.m_V );
    return spu_extract( le, VX ) &
           spu_extract( le, VY ) &
           spu_extract( le, VZ ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_all_le( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgefpR( b_xyzz, a_xyzz, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_All_LE( m_V, src.m_V );
    #else
    return ( m_V[VX] <= src.m_V[VX] ) &&
           ( m_V[VY] <= src.m_V[VY] ) &&
           ( m_V[VZ] <= src.m_V[VZ] );
    #endif
}

/// Comparison Function <
NOALIAS __forceinline const U32 HVec3::All_LT( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int lt = spu_cmplt( m_V, src.m_V );
    return spu_extract( lt, VX ) &
           spu_extract( lt, VY ) &
           spu_extract( lt, VZ ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_all_lt( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgtfpR( b_xyzz, a_xyzz, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_All_LT( m_V, src.m_V );
    #else
    return ( m_V[VX] < src.m_V[VX] ) &&
           ( m_V[VY] < src.m_V[VY] ) &&
           ( m_V[VZ] < src.m_V[VZ] );
    #endif
}

/// Comparison Function >=
NOALIAS __forceinline const U32 HVec3::All_GE( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int ge = spu_cmpge( m_V, src.m_V );
    return spu_extract( ge, VX ) &
           spu_extract( ge, VY ) &
           spu_extract( ge, VZ ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_all_ge( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgefpR( a_xyzz, b_xyzz, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_All_GE( m_V, src.m_V );
    #else
    return ( m_V[VX] >= src.m_V[VX] ) &&
           ( m_V[VY] >= src.m_V[VY] ) &&
           ( m_V[VZ] >= src.m_V[VZ] );
    #endif
}

/// Comparison Function >
NOALIAS __forceinline const U32 HVec3::All_GT( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int gt = spu_cmpgt( m_V, src.m_V );
    return spu_extract( gt, VX ) &
           spu_extract( gt, VY ) &
           spu_extract( gt, VZ ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_all_gt( a , b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgtfpR( a_xyzz, b_xyzz, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_All_GT( m_V, src.m_V );
    #else
    return ( m_V[VX] > src.m_V[VX] ) &&
           ( m_V[VY] > src.m_V[VY] ) &&
           ( m_V[VZ] > src.m_V[VZ] );
    #endif
}


/// Comparison Function ==
NOALIAS __forceinline const U32 HVec3::Any_EQ( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int eq = spu_cmpeq( m_V, src.m_V );
    return ( spu_extract( eq, VX ) |
             spu_extract( eq, VY ) |
             spu_extract( eq, VZ ) ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_any_eq( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a_xyzz, b_xyzz, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_Any_EQ( m_V, src.m_V );
    #else
    return ( m_V[VX] == src.m_V[VX] ) ||
           ( m_V[VY] == src.m_V[VY] ) ||
           ( m_V[VZ] == src.m_V[VZ] );
    #endif
}

/// Comparison Function !=
NOALIAS __forceinline const U32 HVec3::Any_NE( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int ne = spu_cmpne( m_V, src.m_V );
    return ( spu_extract( ne, VX ) |
             spu_extract( ne, VY ) |
             spu_extract( ne, VZ ) ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_any_ne( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a_xyzz, b_xyzz, &CR );
    return ~( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_Any_NE( m_V, src.m_V );
    #else
    return ( m_V[VX] != src.m_V[VX] ) ||
           ( m_V[VY] != src.m_V[VY] ) ||
           ( m_V[VZ] != src.m_V[VZ] );
    #endif
}

/// Comparison Function <=
NOALIAS __forceinline const U32 HVec3::Any_LE( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int le = spu_cmple( m_V, src.m_V );
    return ( spu_extract( le, VX ) |
             spu_extract( le, VY ) |
             spu_extract( le, VZ ) ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_any_le( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgefpR( b_xyzz, a_xyzz, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_Any_LE( m_V, src.m_V );
    #else
    return ( m_V[VX] <= src.m_V[VX] ) ||
           ( m_V[VY] <= src.m_V[VY] ) ||
           ( m_V[VZ] <= src.m_V[VZ] );
    #endif
}

/// Comparison Function <
NOALIAS __forceinline const U32 HVec3::Any_LT( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int lt = spu_cmplt( m_V, src.m_V );
    return ( spu_extract( lt, VX ) |
             spu_extract( lt, VY ) |
             spu_extract( lt, VZ ) ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_any_lt( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgtfpR( b_xyzz, a_xyzz, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_Any_LT( m_V, src.m_V );
    #else
    return ( m_V[VX] < src.m_V[VX] ) ||
           ( m_V[VY] < src.m_V[VY] ) ||
           ( m_V[VZ] < src.m_V[VZ] );
    #endif
}

/// Comparison Function >=
NOALIAS __forceinline const U32 HVec3::Any_GE( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int ge = spu_cmpge( m_V, src.m_V );
    return ( spu_extract( ge, VX ) |
             spu_extract( ge, VY ) |
             spu_extract( ge, VZ ) ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_any_ge( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgefpR( a_xyzz, b_xyzz, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_Any_GE( m_V, src.m_V );
    #else
    return ( m_V[VX] >= src.m_V[VX] ) ||
           ( m_V[VY] >= src.m_V[VY] ) ||
           ( m_V[VZ] >= src.m_V[VZ] );
    #endif
}

/// Comparison Function >
NOALIAS __forceinline const U32 HVec3::Any_GT( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    const vector unsigned int gt = spu_cmpgt( m_V, src.m_V );
    return ( spu_extract( gt, VX ) |
             spu_extract( gt, VY ) |
             spu_extract( gt, VZ ) ) & 1;
    #elif defined( _PPU_ )
    const vector float a = vec_permute<VX,VY,VZ,VZ>( m_V );
	const vector float b = vec_permute<VX,VY,VZ,VZ>( src.m_V );
    return vec_any_gt( a, b );
    #elif defined( _VMX128_ )
    U32 CR;
    const __vector4 a_xyzz( __vpermwi( m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b_xyzz( __vpermwi( src.m_V, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgtfpR( a_xyzz, b_xyzz, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
    #elif defined( _SIMD_ )
    return SIMD_Vector3_Compare_Any_GT( m_V, src.m_V );
    #else
    return ( m_V[VX] > src.m_V[VX] ) ||
           ( m_V[VY] > src.m_V[VY] ) ||
           ( m_V[VZ] > src.m_V[VZ] );
    #endif
}

/// Bitwise AND: THIS & SRC
NOALIAS __forceinline const HVec3 HVec3::AND( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_and( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_and( m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vand( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_AND( m_V, src.m_V ) );
    #else
    const U32 x = (U32&)m_V[VX] & (U32&)src.m_V[VX];
    const U32 y = (U32&)m_V[VY] & (U32&)src.m_V[VY];
    const U32 z = (U32&)m_V[VZ] & (U32&)src.m_V[VZ];
    return HVec3( (F32&)x, (F32&)y, (F32&)z );
    #endif
}

/// Bitwise AND-NOT: THIS & ~SRC
NOALIAS __forceinline const HVec3 HVec3::ANDNOT( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_andc( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_vandc( m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vandc( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_ANDNOT( m_V, src.m_V ) );
    #else
    const U32 x = (U32&)m_V[VX] & ~(U32&)src.m_V[VX];
    const U32 y = (U32&)m_V[VY] & ~(U32&)src.m_V[VY];
    const U32 z = (U32&)m_V[VZ] & ~(U32&)src.m_V[VZ];
    return HVec3( (F32&)x, (F32&)y, (F32&)z );
    #endif
}

/// Bitwise OR: THIS | SRC
NOALIAS __forceinline const HVec3 HVec3::OR( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_or( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_vor( m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vor( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_OR( m_V, src.m_V ) );
    #else
    const U32 x = (U32&)m_V[VX] | (U32&)src.m_V[VX];
    const U32 y = (U32&)m_V[VY] | (U32&)src.m_V[VY];
    const U32 z = (U32&)m_V[VZ] | (U32&)src.m_V[VZ];
    return HVec3( (F32&)x, (F32&)y, (F32&)z );
    #endif
}

/// Bitwise XOR: THIS ^ SRC
NOALIAS __forceinline const HVec3 HVec3::XOR( const HVec3 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_xor( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_vxor( m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vxor( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_XOR( m_V, src.m_V ) );
    #else
    const U32 x = (U32&)m_V[VX] ^ (U32&)src.m_V[VX];
    const U32 y = (U32&)m_V[VY] ^ (U32&)src.m_V[VY];
    const U32 z = (U32&)m_V[VZ] ^ (U32&)src.m_V[VZ];
    return HVec3( (F32&)x, (F32&)y, (F32&)z );
    #endif
}

/// Finite Test
NOALIAS __forceinline const Scalar_Bool HVec3::Finite() const __restrict
{
    // TODO: SPU!
    
    #if defined( _PPU_ )
    
    const vector float abs_mask = Const< vector float >::AbsMask();
    const vector float flt_max = Const< vector float >::FltMax();
    const vector float xyzz = vec_permute<VX,VY,VZ,VZ>( m_V );
    const vector float abs_xyzz = vec_and( xyzz, abs_mask );
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_horizontal_and( vec_and( vec_cmpge( flt_max, abs_xyzz ), vec_cmpeq( xyzz, xyzz ) ) );
    return result;
    
    #elif defined( _VMX128_ )
    
    //const __vector4 v_zero( __vzero() );
    //const __vector4 xyz0( __vrlimi( m_V, v_zero, VMX_MASK_INSERT_W, 0 ) );
	const __vector4 xyzz( __vrlimi( m_V, m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W ) );
    const __vector4 fltMax( c_VectorFltMax );
    
    Scalar_Bool result( UNDEFINED );
    //result.m_V = vmx_horizontal_and_int32( __vand( __vcmpequw( __vcmpbfp( xyzz, fltMax ), v_zero ), __vcmpeqfp( xyzz, xyzz ) ) );
    result.m_V = vmx_horizontal_and_int32( __vand( __vcmpgefp( fltMax, vmx_abs( xyzz ) ), __vcmpeqfp( xyzz, xyzz ) ) );
    return result;
    
    #else
    return Scalar_Bool( _finite( X() ) && ( Abs( X() ) < FLT_MAX ) &&
                        _finite( Y() ) && ( Abs( Y() ) < FLT_MAX ) && 
                        _finite( Z() ) && ( Abs( Z() ) < FLT_MAX ) );
    #endif
}

/// Read from Stream
template < typename Archive >
NOALIAS __forceinline void HVec3::Deserialise( Archive& ar ) __restrict
{
    #if defined( _SIMD_ )
    MathsStorage::Float3 xyz;
    ar >> xyz.x;
    ar >> xyz.y;
    ar >> xyz.z;
    m_V = Load<HVec3>( &xyz ).m_V;
    #else
    ar >> m_V[VX];
    ar >> m_V[VY];
    ar >> m_V[VZ];
    #endif
}

/// Write to Stream
template < typename Archive >
NOALIAS __forceinline void HVec3::Serialise( Archive& ar ) __restrict
{
    #if defined( _SIMD_ )
    MathsStorage::Float3 xyz;
    Store( *this, &xyz );
    ar << xyz.x;
    ar << xyz.y;
    ar << xyz.z;
    #else
    ar << m_V[VX];
    ar << m_V[VY];
    ar << m_V[VZ];
    #endif
}



#if defined( _SIMD_ )

/// Array Index Operator
NOALIAS __forceinline const VectorAccessor<HVec3> HVec3::operator [] ( const U32 index ) __restrict
{
    assert( index < 3 );
    return VectorAccessor<HVec3>( *this, index );
}

/// Const Array Index Operator
NOALIAS __forceinline const ConstVecAccessor<HVec3> HVec3::operator [] ( const U32 index ) const __restrict
{
    assert( index < 3 );
    return ConstVecAccessor<HVec3>( *this, index );
}

#else

/// Array Index Operator
NOALIAS __forceinline F32& HVec3::operator [] ( const U32 index ) __restrict
{
    assert( index < 3 );
    return m_V[ index ];
}

/// Const Array Index Operator
NOALIAS __forceinline const F32 HVec3::operator [] ( const U32 index ) const __restrict
{
    assert( index < 3 );
    return m_V[ index ];
}

#endif




//
// ---------------------------------------------------
// |-- HVec4 : Hardware 4 Element F32 Vector Class --|
// ---------------------------------------------------
//

#if defined( MATHS_DONT_INITIALISE_TO_ZERO )
/// Default Constructor
NOALIAS __forceinline HVec4::HVec4()
{
}
#else
/// Default Constructor
NOALIAS __forceinline HVec4::HVec4()
    #if defined( _SPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Zero() )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    m_V[VW] = 0.0f;
    #endif
}
#endif

/// Zero Init Constructor
NOALIAS __forceinline HVec4::HVec4( const _ZERO_INIT /*zeroinit*/ )
    #if defined( _SPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_V( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Zero() )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    m_V[VW] = 0.0f;
    #endif
}

/// Undefined Constructor
NOALIAS __forceinline HVec4::HVec4( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline HVec4::HVec4( const HVec4& copy )
    #if defined( _SIMD_ )
    : m_V( copy.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = copy.m_V[VX];
    m_V[VY] = copy.m_V[VY];
    m_V[VZ] = copy.m_V[VZ];
    m_V[VW] = copy.m_V[VW];
    #endif
}

/// HVecX Constructor
NOALIAS __forceinline HVec4::HVec4( const HVecX PCREF x )
    #if defined( _SPU_ )
    : m_V( spu_select<0,1,1,1>( x.m_X, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_and( x.m_X, Const< vector float >::Mask(1,0,0,0) ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,0,0,0>( x.m_X ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = x.m_X;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    m_V[VW] = 0.0f;
    #endif
}

/// HVecY Constructor
NOALIAS __forceinline HVec4::HVec4( const HVecY PCREF y )
    #if defined( _SPU_ )
    : m_V( spu_select<1,0,1,1>( y.m_Y, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_and( y.m_Y, Const< vector float >::Mask(0,1,0,0) ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<0,1,0,0>( y.m_Y ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = y.m_Y;
    m_V[VZ] = 0.0f;
    m_V[VW] = 0.0f;
    #endif
}

/// HVecZ Constructor
NOALIAS __forceinline HVec4::HVec4( const HVecZ PCREF z )
    #if defined( _SPU_ )
    : m_V( spu_select<1,1,0,1>( z.m_Z, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_and( z.m_Z, Const< vector float >::Mask(0,0,1,0) ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<0,0,1,0>( z.m_Z ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = z.m_Z;
    m_V[VW] = 0.0f;
    #endif
}

/// HVecW Constructor
NOALIAS __forceinline HVec4::HVec4( const HVecW PCREF w )
    #if defined( _SPU_ )
    : m_V( spu_select<1,1,1,0>( w.m_W, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_and( w.m_W, Const< vector float >::Mask(0,0,0,1) ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<0,0,0,1>( w.m_W ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    m_V[VW] = w.m_W;
    #endif
}

/// HVec2 Constructor
NOALIAS __forceinline HVec4::HVec4( const HVec2 PCREF xy )
    #if defined( _SPU_ )
    : m_V( spu_select<0,0,1,1>( xy.m_V, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_and( xy.m_V, Const< vector float >::Mask(1,1,0,0) ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,1,0,0>( xy.m_V ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = xy.m_V[VX];
    m_V[VY] = xy.m_V[VY];
    m_V[VZ] = 0.0f;
    m_V[VW] = 0.0f;
    #endif
}

/// HVec3 Constructor
NOALIAS __forceinline HVec4::HVec4( const HVec3 PCREF xyz )
    #if defined( _SPU_ )
    : m_V( spu_select<0,0,0,1>( xyz.m_V, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_and( xyz.m_V, Const< vector float >::Mask(1,1,1,0) ) )
    #elif defined( _VMX128_ )
    : m_V( __vrlimi( xyz.m_V, __vzero(), VMX_MASK_INSERT_W, 0 ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,1,1,0>( xyz.m_V ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = xyz.m_V[VX];
    m_V[VY] = xyz.m_V[VY];
    m_V[VZ] = xyz.m_V[VZ];
    m_V[VW] = 0.0f;
    #endif
}

/// C_Vertex Constructor
NOALIAS __forceinline HVec4::HVec4( const C_Vertex PCREF xyz )
    #if defined( _SPU_ )
    //: m_V( spu_select<0,0,0,1>( xyz.m_V.m_V, Const< vector float >::Zero() ) )
    : m_V( spu_select<0,0,0,1>( xyz.m_V.m_V, Const< vector float >::One() ) )
    #elif defined( _PPU_ )
    //: m_V( vec_and( xyz.m_V.m_V, Const< vector float >::Mask(1,1,1,0) ) )
    : m_V( vec_select<0,0,0,1>( xyz.m_V.m_V, Const< vector float >::One() ) )
    #elif defined( _VMX128_ )
    //: m_V( __vrlimi( xyz.m_V.m_V, __vzero(), VMX_MASK_INSERT_W, 0 ) )
    : m_V( __vrlimi( xyz.m_V.m_V, Const< __vector4 >::One(), VMX_MASK_INSERT_W, 0 ) )
    #elif defined( _SIMD_ )
    //: m_V( SIMD_Mask<1,1,1,0>( xyz.m_V.m_V ) )
    : m_V( sse_select<0,0,0,1>( xyz.m_V.m_V, Const< __m128 >::One() ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = xyz.m_V.m_V[VX];
    m_V[VY] = xyz.m_V.m_V[VY];
    m_V[VZ] = xyz.m_V.m_V[VZ];
    m_V[VW] = 1.0f; // 0.0f;
    #endif
}

/// C_Vector Constructor
NOALIAS __forceinline HVec4::HVec4( const C_Vector PCREF xyz )
    #if defined( _SPU_ )
    : m_V( spu_select<0,0,0,1>( xyz.m_V.m_V, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_and( xyz.m_V.m_V, Const< vector float >::Mask(1,1,1,0) ) )
    #elif defined( _VMX128_ )
    : m_V( __vrlimi( xyz.m_V.m_V, __vzero(), VMX_MASK_INSERT_W, 0 ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,1,1,0>( xyz.m_V.m_V ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = xyz.m_V.m_V[VX];
    m_V[VY] = xyz.m_V.m_V[VY];
    m_V[VZ] = xyz.m_V.m_V[VZ];
    m_V[VW] = 0.0f;
    #endif
}

/// C_Normal Constructor
NOALIAS __forceinline HVec4::HVec4( const C_Normal PCREF xyz )
    #if defined( _SPU_ )
    : m_V( spu_select<0,0,0,1>( xyz.m_V.m_V, Const< vector float >::Zero() ) )
    #elif defined( _PPU_ )
    : m_V( vec_and( xyz.m_V.m_V, Const< vector float >::Mask(1,1,1,0) ) )
    #elif defined( _VMX128_ )
    : m_V( __vrlimi( xyz.m_V.m_V, __vzero(), VMX_MASK_INSERT_W, 0 ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Mask<1,1,1,0>( xyz.m_V.m_V ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = xyz.m_V.m_V[VX];
    m_V[VY] = xyz.m_V.m_V[VY];
    m_V[VZ] = xyz.m_V.m_V[VZ];
    m_V[VW] = 0.0f;
    #endif
}

/// HF32 Quad Constructor
NOALIAS __forceinline HVec4::HVec4( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z, const HF32 PCREF w )
    #if defined( _SPU_ )
    //: m_V( (vector float)( spu_extract( x.m_F, VX ),
    //                       spu_extract( y.m_F, VY ),
    //                       spu_extract( z.m_F, VZ ),
    //                       spu_extract( w.m_F, VW ) ) )
    : m_V( spu_insert( spu_extract( x.m_F, VX ),
           spu_insert( spu_extract( y.m_F, VY ),
           spu_insert( spu_extract( z.m_F, VZ ),
           spu_promote( spu_extract( w.m_F, VW ), VW ), VZ ), VY ), VX ) )
    #elif defined( _PPU_ )
    : m_V( vec_mergeh( vec_mergeh( x.m_F, z.m_F ), vec_mergeh( y.m_F, w.m_F ) ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_MergeHigh( SIMD_MergeHigh( x.m_F, z.m_F ), SIMD_MergeHigh( y.m_F, w.m_F ) ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = x;
    m_V[VY] = y;
    m_V[VZ] = z;
    m_V[VW] = w;
    #endif
}

/// HF32 Broadcast Constructor
NOALIAS __forceinline HVec4::HVec4( const HF32 PCREF xyzw )
    #if defined( _SPU_ )
    : m_V( xyzw.m_F )
    #elif defined( _PPU_ )
    : m_V( xyzw.m_F )
    #elif defined( _SIMD_ )
    : m_V( SIMD_Vector4_Scalar( xyzw.m_F ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = xyzw;
    m_V[VY] = xyzw;
    m_V[VZ] = xyzw;
    m_V[VW] = xyzw;
    #endif
}

/// F32 Array Constructor
NOALIAS __forceinline HVec4::HVec4( const F32* ptr )
    #if defined( _SPU_ )
    //: m_V( (vector float){ ptr[0], ptr[1], ptr[2], ptr[3] } )
    : m_V( spu_load_unaligned( ptr ) )
    #elif defined( _PPU_ )
    : m_V( vec_load_unaligned( ptr ) )
    #elif defined( _SIMD_ )
    : m_V( SIMD_LoadUnaligned( ptr ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[VX] = ptr[VX];
    m_V[VY] = ptr[VY];
    m_V[VZ] = ptr[VZ];
    m_V[VW] = ptr[VW];
    #endif
}

#if defined( _SIMD_ )

/// Explicit V128 Constructor
NOALIAS __forceinline HVec4::HVec4( const V128 PCREF src ) :
    m_V( src )
{
}

/// V128 Cast
NOALIAS __forceinline HVec4::operator const V128 () const __restrict
{
    return m_V;
}

#else

/// Explicit V128 Constructor
NOALIAS __forceinline HVec4::HVec4( const V128 PCREF src )
{
    m_V[0] = src.m_X;
    m_V[1] = src.m_Y;
    m_V[2] = src.m_Z;
    m_V[3] = src.m_W;
}

/// V128 Cast
NOALIAS __forceinline HVec4::operator const V128 () const __restrict
{
    V128 v;
    v.m_X = m_V[0];
    v.m_Y = m_V[1];
    v.m_Z = m_V[2];
    v.m_W = m_V[3];
    return v;
}
#endif

/// Validate that elements are all finite
NOALIAS __forceinline void HVec4::DebugValidate() const __restrict
{
    #if defined( MATHS_DEBUG_MAX )
    #if defined( _VMX128_ )
    U32 CR1, CR2, CR3, CR4;
    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VX ), c_VectorFltMax ), __vzero(), &CR1 );
    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VY ), c_VectorFltMax ), __vzero(), &CR2 );
    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VZ ), c_VectorFltMax ), __vzero(), &CR3 );
    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VW ), c_VectorFltMax ), __vzero(), &CR4 );
    if ( !( CR1 & CR2 & CR3 & CR4 & VMX_CR6_ALL_TRUE ) )
    {
        DebugBreak();
    }
    #else
    if ( !_finite( X() ) || !_finite( Y() ) || !_finite( Z() ) || !_finite( W() ) )
    {
        //#ifdef SHOOTER_VECMAT_DEBUG_PRINT
        //DbgPrint( "HVec4::DebugValidate(): Non-Finite HVec4!" );
        //#endif
    }
    #endif
    #endif
}

/// HVecX Cast
NOALIAS __forceinline HVec4::operator const HVecX () const __restrict
{
    #if defined( _SPU_ )
    HVecX result( UNDEFINED );
    result.m_X = m_V;
    return result;
    #elif defined( _PPU_ )
    HVecX result( UNDEFINED );
    result.m_X = m_V;
    return result;
    #elif defined( _SIMD_ )
    return HVecX( SIMD_Mask<1,0,0,0>( m_V ) );
    #else
    return HVecX( m_V[VX] );
    #endif
}

/// HVecY Cast
NOALIAS __forceinline HVec4::operator const HVecY () const __restrict
{
    #if defined( _SPU_ )
    HVecY result( UNDEFINED );
    result.m_Y = m_V;
    return result;
    #elif defined( _PPU_ )
    HVecY result( UNDEFINED );
    result.m_Y = m_V;
    return result;
    #elif defined( _SIMD_ )
    return HVecY( SIMD_Mask<0,1,0,0>( m_V ) );
    #else
    return HVecY( m_V[VY] );
    #endif
}

/// HVecZ Cast
NOALIAS __forceinline HVec4::operator const HVecZ () const __restrict
{
    #if defined( _SPU_ )
    HVecZ result( UNDEFINED );
    result.m_Z = m_V;
    return result;
    #elif defined( _PPU_ )
    HVecZ result( UNDEFINED );
    result.m_Z = m_V;
    return result;
    #elif defined( _SIMD_ )
    return HVecZ( SIMD_Mask<0,0,1,0>( m_V ) );
    #else
    return HVecZ( m_V[VZ] );
    #endif
}

/// HVecW Cast
NOALIAS __forceinline HVec4::operator const HVecW () const __restrict
{
    #if defined( _SPU_ )
    HVecW result( UNDEFINED );
    result.m_W = m_V;
    return result;
    #elif defined( _PPU_ )
    HVecW result( UNDEFINED );
    result.m_W = m_V;
    return result;
    #elif defined( _SIMD_ )
    return HVecW( SIMD_Mask<0,0,0,1>( m_V ) );
    #else
    return HVecW( m_V[VW] );
    #endif
}

/// Copy Assignment
NOALIAS __forceinline const HVec4& HVec4::operator = ( const HVec4& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_V[VX] = copy.m_V[VX];
    m_V[VY] = copy.m_V[VY];
    m_V[VZ] = copy.m_V[VZ];
    m_V[VW] = copy.m_V[VW];
    #endif
    return copy;
}

/// HVecX Assignment
NOALIAS __forceinline const HVec4& HVec4::operator = ( const HVecX PCREF x ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_select<0,1,1,1>( x.m_X, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    //m_V = vec_and( x.m_X, (vector float)(vector unsigned int){ (unsigned int)-1, (unsigned int)0, (unsigned int)0, (unsigned int)0 } );
    m_V = vec_and( x.m_X, Const< vector float >::Mask(1,0,0,0) );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<1,0,0,0>( x.m_X );
    #else
    m_V[VX] = x.m_X;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    m_V[VW] = 0.0f;
    #endif
    return *this;
}

/// HVecY Assignment
NOALIAS __forceinline const HVec4& HVec4::operator = ( const HVecY PCREF y ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_select<1,0,1,1>( y.m_Y, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    //m_V = vec_and( y.m_Y, (vector float)(vector unsigned int){ (unsigned int)0, (unsigned int)-1, (unsigned int)0, (unsigned int)0 } );
    m_V = vec_and( y.m_Y, Const< vector float >::Mask(0,1,0,0) );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<0,1,0,0>( m_V = y.m_Y );
    #else
    m_V[VX] = 0.0f;
    m_V[VY] = y.m_Y;
    m_V[VZ] = 0.0f;
    m_V[VW] = 0.0f;
    #endif
    return *this;
}

/// HVecZ Assignment
NOALIAS __forceinline const HVec4& HVec4::operator = ( const HVecZ PCREF z ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_select<1,1,0,1>( z.m_Z, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    //m_V = vec_and( z.m_Z, (vector float)(vector unsigned int){ (unsigned int)0, (unsigned int)0, (unsigned int)-1, (unsigned int)0 } );
    m_V = vec_and( z.m_Z, Const< vector float >::Mask(0,0,1,0) );
    #elif defined( _SIMD_ )
    m_V = m_V = SIMD_Mask<0,0,1,0>( z.m_Z );
    #else
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = z.m_Z;
    m_V[VW] = 0.0f;
    #endif
    return *this;
}

/// HVecW Assignment
NOALIAS __forceinline const HVec4& HVec4::operator = ( const HVecW PCREF w ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_select<1,1,1,0>( w.m_W, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    //m_V = vec_and( w.m_W, (vector float)(vector unsigned int){ (unsigned int)0, (unsigned int)0, (unsigned int)0, (unsigned int)-1 } );
    m_V = vec_and( w.m_W, Const< vector float >::Mask(0,0,0,1) );
    #elif defined( _SIMD_ )
    m_V = m_V = SIMD_Mask<0,0,0,1>( w.m_W );
    #else
    m_V[VX] = 0.0f;
    m_V[VY] = 0.0f;
    m_V[VZ] = 0.0f;
    m_V[VW] = w.m_W;
    #endif
    return *this;
}

/// HVec2 Assignment
NOALIAS __forceinline const HVec4& HVec4::operator = ( const HVec2 PCREF xy ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_select<0,0,1,1>( xy.m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    //m_V = vec_and( xy.m_V, (vector float)(vector unsigned int){ (unsigned int)-1, (unsigned int)-1, (unsigned int)0, (unsigned int)0 } );
    m_V = vec_and( xy.m_V, Const< vector float >::Mask(1,1,0,0) );
    #elif defined( _SIMD_ )
    m_V = SIMD_Mask<1,1,0,0>( xy.m_V );
    #else
    m_V[VX] = xy.m_V[VX];
    m_V[VY] = xy.m_V[VY];
    m_V[VZ] = 0.0f;
    m_V[VW] = 0.0f;
    #endif
    return *this;
}

/// HVec3 Assignment
NOALIAS __forceinline const HVec4& HVec4::operator = ( const HVec3 PCREF xyz ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_select<0,0,0,1>( xyz.m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    //m_V = vec_and( xyz.m_V, (vector float)(vector unsigned int){ (unsigned int)-1, (unsigned int)-1, (unsigned int)-1, (unsigned int)0 } );
    m_V = vec_and( xyz.m_V, Const< vector float >::Mask(1,1,1,0) );
    #elif defined( _SIMD_ )
    m_V = xyz.m_V;
    #else
    m_V[VX] = xyz.m_V[VX];
    m_V[VY] = xyz.m_V[VY];
    m_V[VZ] = xyz.m_V[VZ];
    m_V[VW] = 0.0f;
    #endif
    return *this;
}

/// Accessor: Get X
NOALIAS __forceinline const HF32 HVec4::X() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VX>( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_V, VX );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_Vector4X( m_V ) );
    #else
    return m_V[VX];
    #endif
}

/// Accessor: Get Y
NOALIAS __forceinline const HF32 HVec4::Y() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VY>( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_V, VY );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_Vector4Y( m_V ) );
    #else
    return m_V[VY];
    #endif
}

/// Accessor: Get Z
NOALIAS __forceinline const HF32 HVec4::Z() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VZ>( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_V, VZ );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_Vector4Z( m_V ) );
    #else
    return m_V[VZ];
    #endif
}

/// Accessor: Get W
NOALIAS __forceinline const HF32 HVec4::W() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VW>( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_V, VW );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_Vector4W( m_V ) );
    #else
    return m_V[VW];
    #endif
}

/// Accessor: Get XYZ
NOALIAS __forceinline const HVec3 HVec4::XYZ() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = m_V;
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = m_V;
    return result;
    #elif defined( _VMX128_ )
    return HVec3( __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_W, 0 ) );
    #elif defined( _SIMD_ )
    return HVec3( X(), Y(), Z() );
    #else
    return HVec3( m_V[VX], m_V[VY], m_V[VZ] );
    #endif
}

/// Accessor: Get YZW
NOALIAS __forceinline const HVec3 HVec4::YZW() const __restrict
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_permute<V0Y,V0Z,V0W,V1W>( m_V, Const< vector float >::Zero() );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_permute<V0Y,V0Z,V0W,V1W>( m_V, Const< vector float >::Zero() );
    return result;
    #elif defined( _VMX128_ )
    return HVec3( __vrlimi( __vzero(), m_V, VMX_MASK_INSERT_XYZ, 1 ) );
    #elif defined( _SIMD_ )
    return HVec3( Y(), Z(), W() );
    #else
    return HVec3( m_V[VY], m_V[VZ], m_V[VW] );
    #endif
}

/// Accessor: Set X
NOALIAS __forceinline void HVec4::X( const HF32 PCREF scalar ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_permute<V1X,V0Y,V0Z,V0W>( m_V, scalar.m_F );
    #elif defined( _PPU_ )
    m_V = vec_permute<V1X,V0Y,V0Z,V0W>( m_V, scalar.m_F );
    #elif defined( _SIMD_ )
    m_V = SIMD_SetX( m_V, scalar.m_F );
    #else
    m_V[VX] = scalar.m_F;
    #endif
}

/// Accessor: Set Y
NOALIAS __forceinline void HVec4::Y( const HF32 PCREF scalar ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_permute<V0X,V1Y,V0Z,V0W>( m_V, scalar.m_F );
    #elif defined( _PPU_ )
    m_V = vec_permute<V0X,V1Y,V0Z,V0W>( m_V, scalar.m_F );
    #elif defined( _SIMD_ )
    m_V = SIMD_SetY( m_V, scalar.m_F );
    #else
    m_V[VY] = scalar.m_F;
    #endif
}

/// Accessor: Set Z
NOALIAS __forceinline void HVec4::Z( const HF32 PCREF scalar ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_permute<V0X,V0Y,V1Z,V0W>( m_V, scalar.m_F );
    #elif defined( _PPU_ )
    m_V = vec_permute<V0X,V0Y,V1Z,V0W>( m_V, scalar.m_F );
    #elif defined( _SIMD_ )
    m_V = SIMD_SetZ( m_V, scalar.m_F );
    #else
    m_V[VZ] = scalar.m_F;
    #endif
}

/// Accessor: Set W
NOALIAS __forceinline void HVec4::W( const HF32 PCREF scalar ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_permute<V0X,V0Y,V0Z,V1W>( m_V, scalar.m_F );
    #elif defined( _PPU_ )
    m_V = vec_permute<V0X,V0Y,V0Z,V1W>( m_V, scalar.m_F );
    #elif defined( _SIMD_ )
    m_V = SIMD_SetW( m_V, scalar.m_F );
    #else
    m_V[VW] = scalar.m_F;
    #endif
}

/// Unary Negation Operator
NOALIAS __forceinline const HVec4 HVec4::operator - () const __restrict
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_negate( m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    //result.m_V = vec_vxor( m_V, (vector float)(vector unsigned int)0x80000000 );
    //result.m_V = vec_vxor( m_V, (vector float)(vector unsigned int)( 0x80000000, 0x80000000, 0x80000000, 0x80000000 ) );
    result.m_V = vec_vxor( m_V, Const< vector float >::SignMask() );
    return result;
    #elif defined( _VMX128_ )
    return HVec4( SIMD_XOR( m_V, c_Vector4_SignMask ) );
    #elif defined( _SIMD_ )
    return HVec4( SIMD_XOR( m_V, (__m128&)c_Vector4_SignMask ) );
    #else
    return HVec4( -m_V[VX],
                  -m_V[VY],
                  -m_V[VZ],
                  -m_V[VW] );
    #endif
}

/// Binary Addition Operator
NOALIAS __forceinline const HVec4 HVec4::operator + ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_add( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_add( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Add( m_V, src.m_V ) );
    #else
    return HVec4( m_V[VX] + src.m_V[VX],
                  m_V[VY] + src.m_V[VY],
                  m_V[VZ] + src.m_V[VZ],
                  m_V[VW] + src.m_V[VW] );
    #endif
}

///  Addition Operator
NOALIAS __forceinline void HVec4::operator += ( const HVec4 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_add( m_V, src.m_V );
    #elif defined( _PPU_ )
    m_V = vec_add( m_V, src.m_V );
    #elif defined( _SIMD_ )
    m_V = SIMD_Add( m_V, src.m_V );
    #else
    m_V[VX] += src.m_V[VX];
    m_V[VY] += src.m_V[VY];
	m_V[VZ] += src.m_V[VZ];
	m_V[VW] += src.m_V[VW];
    #endif
}

/// Binary Subtraction Operator
NOALIAS __forceinline const HVec4 HVec4::operator - ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_sub( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_vsubfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Subtract( m_V, src.m_V ) );
    #else
    return HVec4( m_V[VX] - src.m_V[VX],
                  m_V[VY] - src.m_V[VY],
                  m_V[VZ] - src.m_V[VZ],
                  m_V[VW] - src.m_V[VW] );
    #endif
}

/// Subtraction Operator
NOALIAS __forceinline void HVec4::operator -= ( const HVec4 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_sub( m_V, src.m_V );
    #elif defined( _PPU_ )
    m_V = vec_vsubfp( m_V, src.m_V );
    #elif defined( _SIMD_ )
    m_V = SIMD_Subtract( m_V, src.m_V );
    #else
    m_V[VX] -= src.m_V[VX];
    m_V[VY] -= src.m_V[VY];
    m_V[VZ] -= src.m_V[VZ];
    m_V[VW] -= src.m_V[VW];
    #endif
}

/// Binary Multiplication Operator
NOALIAS __forceinline const HVec4 HVec4::operator * ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_mul( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_mul( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Multiply( m_V, src.m_V ) );
    #else
    return HVec4( m_V[VX] * src.m_V[VX],
                  m_V[VY] * src.m_V[VY],
                  m_V[VZ] * src.m_V[VZ],
                  m_V[VW] * src.m_V[VW] );
    #endif
}

/// Binary Division Operator
NOALIAS __forceinline const HVec4 HVec4::operator / ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  A = m_V;
    const vector float  B = src.m_V;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = spu_mul( A, rcp_B );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float A = m_V;
    const vector float B = src.m_V;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = vec_mul( A, rcp_B );
    return result;
    
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Multiply( m_V, SIMD_Reciprocal4( src.m_V ) ) );
    #else
    //return HVec4( m_V[VX] / src.m_V[VX],
    //              m_V[VY] / src.m_V[VY],
    //              m_V[VZ] / src.m_V[VZ],
    //              m_V[VW] / src.m_V[VW] );
    return HVec4( m_V[VX] * Maths::Reciprocal( src.m_V[VX] ),
                  m_V[VY] * Maths::Reciprocal( src.m_V[VY] ),
                  m_V[VZ] * Maths::Reciprocal( src.m_V[VZ] ),
                  m_V[VW] * Maths::Reciprocal( src.m_V[VW] ) );
    #endif
}

/// HF32 Multiplication Operator
NOALIAS __forceinline const HVec4 HVec4::operator * ( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_mul( m_V, src.m_F );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_mul( m_V, src.m_F );
    return result;
    #elif defined( _VMX128_ )
    HVec4 result( UNDEFINED );
    result.m_V = __vmulfp( m_V, src.m_F );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Multiply( m_V, SIMD_Splat4<VX>( src.m_F ) ) );
    #else
    return HVec4( m_V[VX] * src,
                  m_V[VY] * src,
                  m_V[VZ] * src,
                  m_V[VW] * src );
    #endif
}

/// HF32 Multiply Update Operator
NOALIAS __forceinline void HVec4::operator *= ( const HF32 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_V = spu_mul( m_V, src.m_F );
    #elif defined( _PPU_ )
    m_V = vec_mul( m_V, src.m_F );
    #elif defined( _VMX128_ )
    m_V = __vmulfp( m_V, src.m_F );
    #elif defined( _SIMD_ )
    m_V = SIMD_Multiply( m_V, SIMD_Splat4<VX>( src.m_F ) );
    #else
    m_V[VX] *= src;
    m_V[VY] *= src;
    m_V[VZ] *= src;
    m_V[VW] *= src;
    #endif
}

/// HF32 Division Operator
NOALIAS __forceinline const HVec4 HVec4::operator / ( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  A = m_V;
    const vector float  B = src.m_F;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = spu_mul( A, rcp_B );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float A = m_V;
    const vector float B = src.m_F;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = vec_mul( A, rcp_B );
    return result;
    
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Multiply( m_V, SIMD_Vector4_Scalar( SIMD_ScalarReciprocal( src.m_F ) ) ) );
    #else
    //return *this * ( HF32( 1.0f ) / src );
    return *this * Maths::Reciprocal( src );
    #endif
}

/// HF32 Divide Update Operator
NOALIAS __forceinline void HVec4::operator /= ( const HF32 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  A = m_V;
    const vector float  B = src.m_F;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    m_V = spu_mul( A, rcp_B );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float A = m_V;
    const vector float B = src.m_F;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    m_V = vec_mul( A, rcp_B );
    
    #elif defined( _SIMD_ )
    m_V = SIMD_Multiply( m_V, SIMD_Vector4_Scalar( SIMD_ScalarReciprocal( src.m_F ) ) );
    #else
    //*this *= ( HF32( 1.0f ) / src );
    *this *= Maths::Reciprocal( src );
    #endif
}

/// Min
NOALIAS __forceinline const HVec4 HVec4::Min( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_min( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_vminfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Min( m_V, src.m_V ) );
    #else
    return HVec4( Maths::Min( m_V[VX], src.m_V[VX] ),
                  Maths::Min( m_V[VY], src.m_V[VY] ),
                  Maths::Min( m_V[VZ], src.m_V[VZ] ),
                  Maths::Min( m_V[VW], src.m_V[VW] ) );
    #endif
}

/// Max
NOALIAS __forceinline const HVec4 HVec4::Max( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_max( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_vmaxfp( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Max( m_V, src.m_V ) );
    #else
    return HVec4( Maths::Max( m_V[VX], src.m_V[VX] ),
                  Maths::Max( m_V[VY], src.m_V[VY] ),
                  Maths::Max( m_V[VZ], src.m_V[VZ] ),
                  Maths::Max( m_V[VW], src.m_V[VW] ) );
    #endif
}

/// Horizontal sum of all elements
NOALIAS __forceinline const HF32 HVec4::Sum() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_sum4( m_V );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    //result.m_F = vec_add( vec_add( vec_splat( m_V, VX ), vec_splat( m_V, VY ) ),
    //                         vec_add( vec_splat( m_V, VZ ), vec_splat( m_V, VW ) ) );
    result.m_F = vec_sum4( m_V );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Sum4( m_V ) );
    #else
    return HF32( m_V[VX] + m_V[VY] + m_V[VZ] + m_V[VW] );
    #endif
}

/// Vector Length Squared
NOALIAS __forceinline const HF32 HVec4::Length2() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_length_squared4( m_V );
    return result;
    #elif defined( _PPU_ )
    //const vector float x = vec_splat( m_V, VX );
    //const vector float y = vec_splat( m_V, VY );
    //const vector float z = vec_splat( m_V, VZ );
    //const vector float w = vec_splat( m_V, VW );
    //const vector float length_squared = vec_add( vec_madd( x, x, vec_mul( y, y ) ),
    //                                                vec_madd( z, z, vec_mul( w, w ) ) );
    HF32 result( UNDEFINED );
    //result.m_F = length_squared;
    result.m_F = vec_length_squared4( m_V );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_DotProduct4( m_V, m_V ) );
    #else
    const HVec4 src = *this;
    return ( src * src ).Sum();
    #endif
}

/// Vector Length Squared
NOALIAS __forceinline const HF32 HVec4::LengthSquared() const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_length_squared4( m_V );
    return result;
    #elif defined( _PPU_ )
    //const vector float x = vec_splat( m_V, VX );
    //const vector float y = vec_splat( m_V, VY );
    //const vector float z = vec_splat( m_V, VZ );
    //const vector float w = vec_splat( m_V, VW );
    //const vector float length_squared = vec_add( vec_madd( x, x, vec_mul( y, y ) ),
    //                                                vec_madd( z, z, vec_mul( w, w ) ) );
    HF32 result( UNDEFINED );
    //result.m_F = length_squared;
    result.m_F = vec_length_squared4( m_V );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_DotProduct4( m_V, m_V ) );
    #else
    const HVec4 src = *this;
    return ( src * src ).Sum();
    #endif
}

/// Vector Length
NOALIAS __forceinline const HF32 HVec4::Length() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float length_squared = spu_length_squared4( m_V );
    vector float rcp_length;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    const vector float length = spu_mul( length_squared, rcp_length );
    HF32 result( UNDEFINED );
    result.m_F = length;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    //const vector float x = vec_splat( m_V, VX );
    //const vector float y = vec_splat( m_V, VY );
    //const vector float z = vec_splat( m_V, VZ );
    //const vector float w = vec_splat( m_V, VW );
    //const vector float length_squared = vec_add( vec_madd( x, x, vec_mul( y, y ) ),
    //                                                vec_madd( z, z, vec_mul( w, w ) ) );
    const vector float length_squared = vec_length_squared4( m_V );
    vector float rcp_length;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    const vector float length = vec_mul( length_squared, rcp_length );
    HF32 result( UNDEFINED );
    result.m_F = length;
    return result;
    
    #else
    return Length2().Sqrt();
    #endif
}

/// Vector Reciprocal of Length
NOALIAS __forceinline const HF32 HVec4::RcpLength() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float length_squared = spu_length_squared4( m_V );
    vector float rcp_length;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_length;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    //const vector float x = vec_splat( m_V, VX );
    //const vector float y = vec_splat( m_V, VY );
    //const vector float z = vec_splat( m_V, VZ );
    //const vector float w = vec_splat( m_V, VW );
    //const vector float length_squared = vec_add( vec_madd( x, x, vec_mul( y, y ) ),
    //                                                vec_madd( z, z, vec_mul( w, w ) ) );
    const vector float length_squared = vec_length_squared4( m_V );
    vector float rcp_length;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_length;
    return result;
    
    #else
    return Length2().RcpSqrt();
    #endif
}    

/// Vector Reciprocal of Length
NOALIAS __forceinline const HF32 HVec4::ReciprocalLength() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float length_squared = spu_length_squared4( m_V );
    vector float rcp_length;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_length;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    //const vector float x = vec_splat( m_V, VX );
    //const vector float y = vec_splat( m_V, VY );
    //const vector float z = vec_splat( m_V, VZ );
    //const vector float w = vec_splat( m_V, VW );
    //const vector float length_squared = vec_add( vec_madd( x, x, vec_mul( y, y ) ),
    //                                                vec_madd( z, z, vec_mul( w, w ) ) );
    const vector float length_squared = vec_length_squared4( m_V );
    vector float rcp_length;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_length;
    return result;
    
    #else
    return Length2().RcpSqrt();
    #endif
}    

/// Vector Normalised
NOALIAS __forceinline const HVec4 HVec4::Normalised() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float length_squared = spu_length_squared4( m_V );
    vector float rcp_length;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    HVec4 result( UNDEFINED );
    result.m_V = spu_mul( m_V, rcp_length );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    //const vector float x = vec_splat( m_V, VX );
    //const vector float y = vec_splat( m_V, VY );
    //const vector float z = vec_splat( m_V, VZ );
    //const vector float w = vec_splat( m_V, VW );
    //const vector float length_squared = vec_add( vec_madd( x, x, vec_mul( y, y ) ),
    //                                                vec_madd( z, z, vec_mul( w, w ) ) );
    const vector float length_squared = vec_length_squared4( m_V );
    vector float rcp_length;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    HVec4 result( UNDEFINED );
    result.m_V = vec_mul( m_V, rcp_length );
    return result;
    
    #else
    
    const HVec4 vZero;
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Length2() );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec4 result( *this * ( Maths::Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() ) );
    //#else
    const HVec4 result( *this * ( length2.ReciprocalSqrt() ) );
    //#endif
    
    #if defined( MATHS_DEBUG_BREAK_ON_ZERO_LENGTH_NORMALISE )
    if ( length2 <= nearlyZero )
    {
        DebugBreak();
    }
    #endif
    
    return Maths::Select( length2.GT( nearlyZero ), result, vZero );
    
    #endif
}

/// Vector Normalise
NOALIAS __forceinline void HVec4::Normalise() __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float length_squared = spu_length_squared4( m_V );
    vector float rcp_length;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    m_V = spu_mul( m_V, rcp_length );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    //const vector float x = vec_splat( m_V, VX );
    //const vector float y = vec_splat( m_V, VY );
    //const vector float z = vec_splat( m_V, VZ );
    //const vector float w = vec_splat( m_V, VW );
    //const vector float length_squared = vec_add( vec_madd( x, x, vec_mul( y, y ) ),
    //                                                vec_madd( z, z, vec_mul( w, w ) ) );
    const vector float length_squared = vec_length_squared4( m_V );
    vector float rcp_length;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    m_V = vec_mul( m_V, rcp_length );
    
    #else
    
    const HVec4 vZero;
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Length2() );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec4 result( *this * ( Maths::Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() ) );
    //#else
    const HVec4 result( *this * ( length2.ReciprocalSqrt() ) );
    //#endif
    
    #if defined( MATHS_DEBUG_BREAK_ON_ZERO_LENGTH_NORMALISE )
    if ( length2 <= nearlyZero )
    {
        DebugBreak();
    }
    #endif
    
    #if defined( _SIMD_ )
    m_V = Maths::Select( length2.GT( nearlyZero ), result, vZero ).m_V;
    #else
    *this = ( Maths::Select( length2.GT( nearlyZero ), result, HVec4() ) );
    #endif
    
    #endif
}

/// Vector Normalised - safe version with additional parameter to use for zero length vectors!
NOALIAS __forceinline const HVec4 HVec4::SafeNormalised( const HVec4 PCREF zeroLengthResult ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float length_squared = spu_length_squared4( m_V );
    vector float rcp_length;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    HVec4 result( UNDEFINED );
    result.m_V = spu_sel( zeroLengthResult.m_V, spu_mul( m_V, rcp_length ), valid );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    //const vector float x = vec_splat( m_V, VX );
    //const vector float y = vec_splat( m_V, VY );
    //const vector float z = vec_splat( m_V, VZ );
    //const vector float w = vec_splat( m_V, VW );
    //const vector float length_squared = vec_add( vec_madd( x, x, vec_mul( y, y ) ),
    //                                                vec_madd( z, z, vec_mul( w, w ) ) );
    const vector float length_squared = vec_length_squared4( m_V );
    vector float rcp_length;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    HVec4 result( UNDEFINED );
    result.m_V = vec_sel( zeroLengthResult.m_V, vec_mul( m_V, rcp_length ), valid );
    return result;
    
    #else
    
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Length2() );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec4 result( *this * ( Maths::Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() ) );
    //#else
    const HVec4 result( *this * ( length2.ReciprocalSqrt() ) );
    //#endif
    
    return Maths::Select( length2.GT( nearlyZero ), result, zeroLengthResult );
    
    #endif
}

/// Vector Normalise - safe version with additional parameter to use for zero length vectors!
NOALIAS __forceinline void HVec4::SafeNormalise( const HVec4 PCREF zeroLengthResult ) __restrict
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float length_squared = spu_length_squared4( m_V );
    vector float rcp_length;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    m_V = spu_sel( zeroLengthResult.m_V, spu_mul( m_V, rcp_length ), valid );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    //const vector float x = vec_splat( m_V, VX );
    //const vector float y = vec_splat( m_V, VY );
    //const vector float z = vec_splat( m_V, VZ );
    //const vector float w = vec_splat( m_V, VW );
    //const vector float length_squared = vec_add( vec_madd( x, x, vec_mul( y, y ) ),
    //                                                vec_madd( z, z, vec_mul( w, w ) ) );
    const vector float length_squared = vec_length_squared4( m_V );
    vector float rcp_length;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( length_squared, zero, rcp_length, valid );
    m_V = vec_sel( zeroLengthResult.m_V, vec_mul( m_V, rcp_length ), valid );
    
    #else
    
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 length2( Length2() );
    
    //#if defined( _DEBUG )
    //const HF32 one( c_ScalarOne );
    //const HVec4 result( *this * ( Maths::Select( length2.GT( nearlyZero ), length2, one ).ReciprocalSqrt() ) );
    //#else
    const HVec4 result( *this * ( length2.ReciprocalSqrt() ) );
    //#endif
    
    #if defined( _SIMD_ )
    m_V = Maths::Select( length2.GT( nearlyZero ), result, zeroLengthResult ).m_V;
    #else
    const bool length2_GT_nearlyZero( length2.GT( nearlyZero ) );
    m_V[VX] = length2_GT_nearlyZero ? result.m_V[VX] : zeroLengthResult.m_V[VX];
    m_V[VY] = length2_GT_nearlyZero ? result.m_V[VY] : zeroLengthResult.m_V[VY];
    m_V[VZ] = length2_GT_nearlyZero ? result.m_V[VZ] : zeroLengthResult.m_V[VZ];
    m_V[VW] = length2_GT_nearlyZero ? result.m_V[VW] : zeroLengthResult.m_V[VW];
    #endif
    
    #endif
}

/// Dot product
NOALIAS __forceinline const HF32 HVec4::operator | ( const HVec4 PCREF r ) const __restrict
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_dot_product4( m_V, r.m_V );
    return result;
    #elif defined( _PPU_ )
    //const vector float xyzw2 = vec_mul( m_V, r.m_V );
    //const vector float x2 = vec_splat( xyzw2, VX );
    //const vector float y2 = vec_splat( xyzw2, VY );
    //const vector float z2 = vec_splat( xyzw2, VZ );
    //const vector float w2 = vec_splat( xyzw2, VW );
    //const vector float dp = vec_add( vec_add( x2, y2 ),
    //                                    vec_add( z2, w2 ) );
    HF32 result( UNDEFINED );
    //result.m_F = dp;
    result.m_F = vec_dot_product4( m_V, r.m_V );
    return result;
    #elif defined( _VMX128_ )
    const HVec4 l = *this;
    return HF32( __vmsum4fp( l.m_V, r.m_V ) );
    #else
    const HVec4 l = *this;
    return ( l * r ).Sum();
    #endif
}

/// Vector * Matrix Multiplication
NOALIAS __forceinline const HVec4 HVec4::operator * ( const HMat44& m ) const __restrict
{
    #if defined( _SPU_ )
    
    HVec4 result( UNDEFINED );
    result.m_V = spu_add( spu_madd( spu_splat<VX>( m_V ), m.m_AX,
                          spu_mul ( spu_splat<VY>( m_V ), m.m_AY ) ),
                          spu_madd( spu_splat<VZ>( m_V ), m.m_AZ,
                          spu_mul ( spu_splat<VW>( m_V ), m.m_AW ) ) );
    return result;
    
    #elif defined( _PPU_ )
    
    HVec4 result( UNDEFINED );
    result.m_V = vec_add( vec_madd( vec_splat( m_V, VX ), m.m_AX,
                             vec_mul ( vec_splat( m_V, VY ), m.m_AY ) ),
                             vec_madd( vec_splat( m_V, VZ ), m.m_AZ,
                             vec_mul ( vec_splat( m_V, VW ), m.m_AW ) ) );
    return result;
    
    #elif defined( _VMX128_ )
    
    const V128 x( __vmsum4fp( m_V, m.m_CX ) );
    const V128 y( __vmsum4fp( m_V, m.m_CY ) );
    const V128 z( __vmsum4fp( m_V, m.m_CZ ) );
    const V128 w( __vmsum4fp( m_V, m.m_CW ) );
    return HVec4( __vmrghw( __vmrghw( x, z ), __vmrghw( y, w ) ) );
    
    #elif defined( _SSE_ )
    
    return HVec4( SIMD_Add( SIMD_MultiplyAdd( SIMD_Splat4<VX>( m_V ), m.m_AX.m_V,
                            SIMD_Multiply   ( SIMD_Splat4<VY>( m_V ), m.m_AY.m_V ) ),
                            SIMD_MultiplyAdd( SIMD_Splat4<VZ>( m_V ), m.m_AZ.m_V,
                            SIMD_Multiply   ( SIMD_Splat4<VW>( m_V ), m.m_AW.m_V ) ) ) );
    
    #else
    
    const HVec4 v = *this;
    const F32 x = v[VX] * m[MX][VX] + v[VY] * m[MY][VX] + v[VZ] * m[MZ][VX] + v[VW] * m[MW][VX];
    const F32 y = v[VX] * m[MX][VY] + v[VY] * m[MY][VY] + v[VZ] * m[MZ][VY] + v[VW] * m[MW][VY];
    const F32 z = v[VX] * m[MX][VZ] + v[VY] * m[MY][VZ] + v[VZ] * m[MZ][VZ] + v[VW] * m[MW][VZ];
    const F32 w = v[VX] * m[MX][VW] + v[VY] * m[MY][VW] + v[VZ] * m[MZ][VW] + v[VW] * m[MW][VW];
    return HVec4( x, y, z, w );
    
    #endif
}

/// Comparison Operator ==
NOALIAS __forceinline const Vector4_Bool HVec4::operator == ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_cmpeq( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_cmpeq( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_Vector_Selector_EQ( m_V, src.m_V ) );
    #else
    return Vector4_Bool( *(const U32*)(m_V[VX] == src.m_V[VX] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VY] == src.m_V[VY] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VZ] == src.m_V[VZ] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VW] == src.m_V[VW] ? &c_fSet : &c_fClear) );
    #endif
}

/// Comparison Operator !=
NOALIAS __forceinline const Vector4_Bool HVec4::operator != ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_cmpne( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    const vector bool int eq = vec_cmpeq( m_V, src.m_V );
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_vnor( eq, eq );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_Vector_Selector_NE( m_V, src.m_V ) );
    #else
    return Vector4_Bool( *(const U32*)(m_V[VX] != src.m_V[VX] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VY] != src.m_V[VY] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VZ] != src.m_V[VZ] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VW] != src.m_V[VW] ? &c_fSet : &c_fClear) );
    #endif
}

/// Comparison Operator <=
NOALIAS __forceinline const Vector4_Bool HVec4::operator <= ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_cmple( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_cmpge( src.m_V, m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_Vector_Selector_LE( m_V, src.m_V ) );
    #else
    return Vector4_Bool( *(const U32*)(m_V[VX] <= src.m_V[VX] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VY] <= src.m_V[VY] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VZ] <= src.m_V[VZ] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VW] <= src.m_V[VW] ? &c_fSet : &c_fClear) );
    #endif
}

/// Comparison Operator <
NOALIAS __forceinline const Vector4_Bool HVec4::operator < ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_cmplt( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_cmpgt( src.m_V, m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_Vector_Selector_LT( m_V, src.m_V ) );
    #else
    return Vector4_Bool( *(const U32*)(m_V[VX] < src.m_V[VX] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VY] < src.m_V[VY] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VZ] < src.m_V[VZ] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VW] < src.m_V[VW] ? &c_fSet : &c_fClear) );
    #endif
}

/// Comparison Operator >=
NOALIAS __forceinline const Vector4_Bool HVec4::operator >= ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_cmpge( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_cmpge( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_Vector_Selector_GE( m_V, src.m_V ) );
    #else
    return Vector4_Bool( *(const U32*)(m_V[VX] >= src.m_V[VX] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VY] >= src.m_V[VY] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VZ] >= src.m_V[VZ] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VW] >= src.m_V[VW] ? &c_fSet : &c_fClear) );
    #endif
}

/// Comparison Operator >
NOALIAS __forceinline const Vector4_Bool HVec4::operator > ( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_cmpgt( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_cmpgt( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_Vector_Selector_GT( m_V, src.m_V ) );
    #else
    return Vector4_Bool( *(const U32*)(m_V[VX] > src.m_V[VX] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VY] > src.m_V[VY] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VZ] > src.m_V[VZ] ? &c_fSet : &c_fClear),
                         *(const U32*)(m_V[VW] > src.m_V[VW] ? &c_fSet : &c_fClear) );
    #endif
}

/// Bitwise AND: THIS & SRC
NOALIAS __forceinline const HVec4 HVec4::AND( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result;
    result.m_V = spu_and( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result;
    result.m_V = vec_and( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_AND( m_V, src.m_V ) );
    #else
    const U32 x = (U32&)m_V[VX] & (U32&)src.m_V[VX];
    const U32 y = (U32&)m_V[VY] & (U32&)src.m_V[VY];
    const U32 z = (U32&)m_V[VZ] & (U32&)src.m_V[VZ];
    const U32 w = (U32&)m_V[VW] & (U32&)src.m_V[VW];
    return HVec4( (F32&)x, (F32&)y, (F32&)z, (F32&)w );
    #endif
}

/// Bitwise AND-NOT: THIS & ~SRC
NOALIAS __forceinline const HVec4 HVec4::ANDNOT( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result;
    result.m_V = spu_andc( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result;
    result.m_V = vec_vandc( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_ANDNOT( m_V, src.m_V ) );
    #else
    const U32 x = (U32&)m_V[VX] & ~(U32&)src.m_V[VX];
    const U32 y = (U32&)m_V[VY] & ~(U32&)src.m_V[VY];
    const U32 z = (U32&)m_V[VZ] & ~(U32&)src.m_V[VZ];
    const U32 w = (U32&)m_V[VW] & ~(U32&)src.m_V[VW];
    return HVec4( (F32&)x, (F32&)y, (F32&)z, (F32&)w );
    #endif
}

/// Bitwise OR: THIS | SRC
NOALIAS __forceinline const HVec4 HVec4::OR( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result;
    result.m_V = spu_or( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result;
    result.m_V = vec_vor( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_OR( m_V, src.m_V ) );
    #else
    const U32 x = (U32&)m_V[VX] | (U32&)src.m_V[VX];
    const U32 y = (U32&)m_V[VY] | (U32&)src.m_V[VY];
    const U32 z = (U32&)m_V[VZ] | (U32&)src.m_V[VZ];
    const U32 w = (U32&)m_V[VW] | (U32&)src.m_V[VW];
    return HVec4( (F32&)x, (F32&)y, (F32&)z, (F32&)w );
    #endif
}

/// Bitwise XOR: THIS ^ SRC
NOALIAS __forceinline const HVec4 HVec4::XOR( const HVec4 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    HVec4 result;
    result.m_V = spu_xor( m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result;
    result.m_V = vec_vxor( m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_XOR( m_V, src.m_V ) );
    #else
    const U32 x = (U32&)m_V[VX] ^ (U32&)src.m_V[VX];
    const U32 y = (U32&)m_V[VY] ^ (U32&)src.m_V[VY];
    const U32 z = (U32&)m_V[VZ] ^ (U32&)src.m_V[VZ];
    const U32 w = (U32&)m_V[VW] ^ (U32&)src.m_V[VW];
    return HVec4( (F32&)x, (F32&)y, (F32&)z, (F32&)w );
    #endif
}

/// Finite Test
NOALIAS __forceinline const Scalar_Bool HVec4::Finite() const __restrict
{
    // TODO: SPU!
    
    #if defined( _PPU_ )
    
    const vector float abs_mask = Const< vector float >::AbsMask();
    const vector float flt_max = Const< vector float >::FltMax();
    const vector float xyzw = m_V;
    const vector float abs_xyzw = vec_and( xyzw, abs_mask );
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_horizontal_and( vec_and( vec_cmpge( flt_max, abs_xyzw ), vec_cmpeq( xyzw, xyzw ) ) );
    return result;
    
    #elif defined( _VMX128_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = vmx_horizontal_and_int32( __vand( __vcmpequw( __vcmpbfp( m_V, c_VectorFltMax ), __vzero() ), __vcmpeqfp( m_V, m_V ) ) );
    return result;
    
    #else
    
    //return Scalar_Bool( _finite( X() ) && _finite( Y() ) && _finite( Z() ) && _finite( W() ) );
    return Scalar_Bool( _finite( X() ) && ( Abs( X() ) < FLT_MAX ) &&
                        _finite( Y() ) && ( Abs( Y() ) < FLT_MAX ) && 
                        _finite( Z() ) && ( Abs( Z() ) < FLT_MAX ) && 
                        _finite( W() ) && ( Abs( W() ) < FLT_MAX ) );
    
    #endif
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void HVec4::Deserialise( T_Stream& stream ) __restrict
{
    #if defined( _SPU_ )
    F32 x, y, z, w;
    stream >> x;
    stream >> y;
    stream >> z;
    stream >> w;
    //m_V = (vector float){ x, y, z, w };
    m_V = spu_insert( x, spu_insert( y, spu_insert( z, spu_promote( w, VW ), VZ ), VY ), VX );
    #elif defined( _PPU_ )
    F32 x, y, z, w;
    stream >> x;
    stream >> y;
    stream >> z;
    stream >> w;
    m_V = (vector float){ x, y, z, w };
    #elif defined( _SIMD_ )
    F32 x, y, z, w;
    stream >> x;
    stream >> y;
    stream >> z;
    stream >> w;
    m_V = SIMD_Vector4( x, y, z, w );
    #else
    stream >> m_V[VX];
    stream >> m_V[VY];
    stream >> m_V[VZ];
    stream >> m_V[VW];
    #endif
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void HVec4::Serialise( T_Stream& stream ) __restrict
{
    stream << (F32)X();
    stream << (F32)Y();
    stream << (F32)Z();
    stream << (F32)W();
}




#if defined( _SIMD_ )

/// Array Index Operator
NOALIAS __forceinline const VectorAccessor<HVec4> HVec4::operator [] ( const U32 index ) __restrict
{
    assert( index < 4 );
    return VectorAccessor<HVec4>( *this, index );
}

/// Const Array Index Operator
NOALIAS __forceinline const ConstVecAccessor<HVec4> HVec4::operator [] ( const U32 index ) const __restrict
{
    assert( index < 4 );
    return ConstVecAccessor<HVec4>( *this, index );
}

#else

/// Array Index Operator
NOALIAS __forceinline F32& HVec4::operator [] ( const U32 index ) __restrict
{
    assert( index < 4 );
    return m_V[ index ];
}

/// Const Array Index Operator
NOALIAS __forceinline const F32 HVec4::operator [] ( const U32 index ) const __restrict
{
    assert( index < 4 );
    return m_V[ index ];
}

#endif


NOALIAS __forceinline const HVec4 operator * ( const HF32 PCREF f, const HVec4 PCREF v )
{
    return v * f;
}

NOALIAS __forceinline const HVec4 operator / ( const HF32 PCREF f, const HVec4 PCREF v )
{
    return HVec4( f ) / v;
}


NOALIAS __forceinline const HVec4 operator * ( const HVec4 PCREF v, const F32& f )
{
    return v * HF32( f );
}

NOALIAS __forceinline const HVec4 operator / ( const HVec4 PCREF v, const F32& f )
{
    return v / HF32( f );
}


NOALIAS __forceinline void operator *= ( HVec4& v, const F32& f )
{
    v *= HF32( f );
}

NOALIAS __forceinline void operator /= ( HVec4& v, const F32& f )
{
    v /= HF32( f );
}


/// Round to integer towards minus infinity
NOALIAS __forceinline const HVec4 RoundToIntMinusInf( const HVec4 PCREF src )
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED);
    result.m_V = spu_rfim( src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_vrfim( src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec4 result( UNDEFINED );
    result.m_V = __vrfim( src.m_V );
    return result;
    #elif defined( _SSE_ )
    return HVec4( SSE_RoundToIntMinusInf( src.m_V ) );
    #else
    return HVec4( floorf( src.m_V[VX] ),
                  floorf( src.m_V[VY] ),
                  floorf( src.m_V[VZ] ),
                  floorf( src.m_V[VW] ) );
    #endif
}

/// Round to integer towards plus infinity
NOALIAS __forceinline const HVec4 RoundToIntPlusInf( const HVec4 PCREF src )
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_rfip( src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED);
    result.m_V = vec_vrfip( src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec4 result( UNDEFINED);
    result.m_V = __vrfip( src.m_V );
    return result;
    #elif defined( _SSE_ )
    return HVec4( SSE_RoundToIntPlusInf( src.m_V ) );
    #else
    return HVec4( ceilf( src.m_V[VX] ),
                  ceilf( src.m_V[VY] ),
                  ceilf( src.m_V[VZ] ),
                  ceilf( src.m_V[VW] ) );
    #endif
}

/// Round to integer towards nearest
NOALIAS __forceinline const HVec4 RoundToIntNearest( const HVec4 PCREF src )
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_rfin( src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_vrfin( src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec4 result( UNDEFINED );
    result.m_V = __vrfin( src.m_V );
    return result;
    #elif defined( _SSE_ )
    return HVec4( SSE_RoundToIntNearest( src.m_V ) );
    #else
    return HVec4( _copysign( floorf( fabsf( src.m_V[VX] ) + 0.5f ), src.m_V[VX] ),
                  _copysign( floorf( fabsf( src.m_V[VY] ) + 0.5f ), src.m_V[VY] ),
                  _copysign( floorf( fabsf( src.m_V[VZ] ) + 0.5f ), src.m_V[VZ] ),
                  _copysign( floorf( fabsf( src.m_V[VW] ) + 0.5f ), src.m_V[VW] ) );
    #endif
}

/// Round to integer towards zero
NOALIAS __forceinline const HVec4 RoundToIntZero( const HVec4 PCREF src )
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_rfiz( src.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_vrfiz( src.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec4 result( UNDEFINED );
    result.m_V = __vrfiz( src.m_V );
    return result;
    #elif defined( _SSE_ )
    return HVec4( SSE_RoundToIntZero( src.m_V ) );
    #else
    return HVec4( (F32)(S32)src.m_V[VX],
                  (F32)(S32)src.m_V[VY],
                  (F32)(S32)src.m_V[VZ],
                  (F32)(S32)src.m_V[VW] );
    #endif
}


/// Sum of Products: ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z )
NOALIAS __forceinline const HF32 MultiplySum3( const HVec4 PCREF a, const HVec4 PCREF b )
{
    #if defined( _SPU_ )
    const vector float ab = spu_mul( a.m_V, b.m_V );
    const vector float x = spu_splat<VX>( ab );
    const vector float y = spu_splat<VY>( ab );
    const vector float z = spu_splat<VZ>( ab );
    HF32 result( UNDEFINED );
    result.m_F = spu_add( spu_add( x, y ), z );
    return result;
    #elif defined( _PPU_ )
    const vector float ab = vec_mul( a.m_V, b.m_V );
    const vector float x = vec_splat( ab, VX );
    const vector float y = vec_splat( ab, VY );
    const vector float z = vec_splat( ab, VZ );
    HF32 result( UNDEFINED );
    result.m_F = vec_add( vec_add( x, y ), z );
    return result;
    #elif defined( _VMX128_ )
    return HF32( __vmsum3fp( a.m_V, b.m_V ) );
    #elif defined( _SSE_ )
    return HF32( SIMD_DotProduct3( a.m_V, b.m_V ) );
    #else
    return HF32( a.m_V[VX] * b.m_V[VX] +
                 a.m_V[VY] * b.m_V[VY] +
                 a.m_V[VZ] * b.m_V[VZ] );
    #endif
}

/// Sum of Products: ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z ) + ( a.w * b.w )
NOALIAS __forceinline const HF32 MultiplySum4( const HVec4 PCREF a, const HVec4 PCREF b )
{
    #if defined( _SPU_ )
    const vector float ab = spu_mul( a.m_V, b.m_V );
    const vector float x = spu_splat<VX>( ab );
    const vector float y = spu_splat<VY>( ab );
    const vector float z = spu_splat<VZ>( ab );
    const vector float w = spu_splat<VW>( ab );
    HF32 result( UNDEFINED );
    result.m_F = spu_add( spu_add( x, y ), spu_add( z, w ) );
    return result;
    #elif defined( _PPU_ )
    const vector float ab = vec_mul( a.m_V, b.m_V );
    const vector float x = vec_splat( ab, VX );
    const vector float y = vec_splat( ab, VY );
    const vector float z = vec_splat( ab, VZ );
    const vector float w = vec_splat( ab, VW );
    HF32 result( UNDEFINED );
    result.m_F = vec_add( vec_add( x, y ), vec_add( z, w ) );
    return result;
    #elif defined( _VMX128_ )
    return HF32( __vmsum4fp( a.m_V, b.m_V ) );
    #elif defined( _SSE_ )
    return HF32( SIMD_DotProduct4( a.m_V, b.m_V ) );
    #else
    return HF32( a.m_V[VX] * b.m_V[VX] +
                 a.m_V[VY] * b.m_V[VY] +
                 a.m_V[VZ] * b.m_V[VZ] +
                 a.m_V[VW] * b.m_V[VW] );
    #endif
}


/// Returns a HVec4 with each element having it's magnitude taken from the respective elements in 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const HVec4 CopySign( const HVec4 PCREF magnitude, const HVec4 PCREF sign )
{
    #if defined( _SPU_ )
    //const vector unsigned int sign_mask = (vector unsigned int)0x80000000;
    const vector unsigned int sign_mask = Const< vector unsigned int >::SignMask();
    HVec4 result( UNDEFINED );
    result.m_V = spu_sel( magnitude.m_V, sign.m_V, sign_mask );
    return result;
    #elif defined( _PPU_ )
    //const vector unsigned int sign_mask = (vector unsigned int)0x80000000;
    const vector unsigned int sign_mask = Const< vector unsigned int >::SignMask();
    HVec4 result( UNDEFINED );
    result.m_V = vec_sel( magnitude.m_V, sign.m_V, sign_mask );
    return result;
    #elif defined( _VMX128_ )
    const __vector4 sign_mask( c_VectorSignMask );
    HVec4 result( UNDEFINED );
    result.m_V = __vsel( magnitude.m_V, sign.m_V, sign_mask );
    return result;
    #elif defined( _SIMD_ )
    const HVec4 sign_mask( c_VectorSignMask );
    return HVec4( SIMD_Select( sign_mask.m_V, sign.m_V, magnitude.m_V ) );
    #else
    return HVec4( (F32)_copysign( magnitude.m_V[VX], sign.m_V[VX] ),
                  (F32)_copysign( magnitude.m_V[VY], sign.m_V[VY] ),
                  (F32)_copysign( magnitude.m_V[VZ], sign.m_V[VZ] ),
                  (F32)_copysign( magnitude.m_V[VW], sign.m_V[VW] ) );
    #endif
}

/// HVec4 Reciprocal
NOALIAS __forceinline const HVec4 Reciprocal( const HVec4 PCREF src )
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = src.m_V;
    vector float       rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = rcp_B;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = src.m_V;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = rcp_B;
    return result;
    
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Reciprocal4( src.m_V ) );
    #else
    const F32 zero( 0.0f );
    return HVec4( Reciprocal( Max( zero, src.m_V[VX] ) ),
                  Reciprocal( Max( zero, src.m_V[VY] ) ),
                  Reciprocal( Max( zero, src.m_V[VZ] ) ),
                  Reciprocal( Max( zero, src.m_V[VW] ) ) );
    #endif
}


/// HVec4 ReciprocalSqrt
NOALIAS __forceinline const HVec4 ReciprocalSqrt( const HVec4 PCREF src )
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = src.m_V;
    vector float       rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = rcp_sqrt_B;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = src.m_V;
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = rcp_sqrt_B;
    return result;
    
    #elif defined( _SIMD_ )
    return HVec4( SIMD_ReciprocalSqrt4( src.m_V ) );
    #else
    const F32 zero( 0.0f );
    return HVec4( ReciprocalSqrt( Max( zero, src.m_V[VX] ) ),
                  ReciprocalSqrt( Max( zero, src.m_V[VY] ) ),
                  ReciprocalSqrt( Max( zero, src.m_V[VZ] ) ),
                  ReciprocalSqrt( Max( zero, src.m_V[VW] ) ) );
    #endif
}


/// HVec4 Sqrt
NOALIAS __forceinline const HVec4 Sqrt( const HVec4 PCREF src )
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = src.m_V;
    vector float       rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = spu_mul( B, rcp_sqrt_B );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = src.m_V;
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HVec4 result( UNDEFINED );
    result.m_V = vec_mul( B, rcp_sqrt_B );
    return result;
    
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Sqrt4( src.m_V ) );
    #else
    const F32 zero( 0.0f );
    return HVec4( Sqrt( Max( zero, src.m_V[VX] ) ),
                  Sqrt( Max( zero, src.m_V[VY] ) ),
                  Sqrt( Max( zero, src.m_V[VZ] ) ),
                  Sqrt( Max( zero, src.m_V[VW] ) ) );
    #endif
}









//
// --------------------------------------------------------------------
// |-- C_Origin : Origin for conversion between C_Vertex & C_Vector --|
// --------------------------------------------------------------------
//

/// Addition: Origin + Vector --> Vertex
NOALIAS __forceinline const C_Vertex C_Origin::operator + ( const C_Vector PCREF src ) const
{
    return C_Vertex( src.m_V );
}

/// Subtraction: Origin - Vertex --> Vector
NOALIAS __forceinline const C_Vector C_Origin::operator - ( const C_Vertex PCREF src ) const
{
    return C_Vector( src.m_V );
}






//
// ----------------------------------------------
// |-- C_Vertex : HVec3 with vertex semantics --|
// ----------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_Vertex::C_Vertex()
{
}

/// Zero Init Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const _ZERO_INIT /*undefined*/ ) :
    m_V( ZERO_INIT )
{
}

/// Undefined Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const _UNDEFINED /*zeroinit*/ ) :
    m_V( UNDEFINED )
{
}

/// Copy Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const C_Vertex& copy ) :
    m_V( copy.m_V )
{
}

/// Origin Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const C_Origin PCREF /*src*/ ) :
    m_V( c_VectorZero )
{
}

/// Explicit Vector Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const C_Vector PCREF src ) :
    m_V( src.m_V )
{
}

/// HVec3 Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const HVec3 PCREF src ) :
    m_V( src )
{
}

/// HVec4 Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const HVec4 PCREF src ) :
    m_V( HVec3( src ) )
{
}

/// HF32 XYZ Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z ) :
    m_V( x, y, z )
{
}

/// Explicit F32 Array Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const F32* xyz ) :
    m_V( xyz )
{
}

/// Explicit HF32 XYZ Broadcast Constructor
NOALIAS __forceinline C_Vertex::C_Vertex( const HF32 PCREF xyz ) :
    m_V( xyz )
{
}

/// Explicit Constructor from Float3 ptr
NOALIAS __forceinline C_Vertex::C_Vertex( MathsStorage::Float3 const * const __restrict pStorage ) :
    m_V( pStorage )
{
}

/// Cast to Float3
NOALIAS __forceinline C_Vertex::operator const MathsStorage::Float3 () const __restrict
{
    return (MathsStorage::Float3)m_V;
}

/// Explicit Constructor from Fixed3_S16 ptr
template < int T_Precision >
NOALIAS __forceinline C_Vertex::C_Vertex( MathsStorage::Fixed3_S16<T_Precision> const * pStorage ) :
    m_V( pStorage )
{
}

/// Cast to Fixed3_S16
template < int T_Precision >
NOALIAS __forceinline C_Vertex::operator const MathsStorage::Fixed3_S16<T_Precision> () const __restrict
{
    return (MathsStorage::Fixed3_S16<T_Precision>)m_V;
}

/// HVec3_Multiply Constructor
//FORCE_INLINE
//C_Vertex::C_Vertex( const HVec3_Multiply& src )
//    : m_V( HVec3( src ) )
//{
//}

/// HVec3_Add Constructor
//FORCE_INLINE
//C_Vertex::C_Vertex( const HVec3_Add& src )
//    : m_V( HVec3( src ) )
//{
//}

/// HVec3_Subtract Constructor
//FORCE_INLINE
//C_Vertex::C_Vertex( const HVec3_Subtract& src )
//    : m_V( HVec3( src ) )
//{
//}

/// HVec3 Cast
//FORCE_INLINE
//C_Vertex::operator const HVec3 () const
//{
//    return m_V;
//}

/// Copy Assignment
NOALIAS __forceinline const C_Vertex C_Vertex::operator = ( const C_Vertex& copy )
{
    m_V = copy.m_V;
    return copy;
}

/// Origin Assignment
NOALIAS __forceinline const C_Vertex C_Vertex::operator = ( const C_Origin PCREF /*src*/ ) __restrict
{
    m_V = HVec3( c_VectorZero );
    return *this;
}

/// HVec3_Multiply Assignment
NOALIAS __forceinline const C_Vertex C_Vertex::operator = ( const HVec3_Multiply& src )
{
    m_V = HVec3( src );
    return *this;
}

/// HVec3_Add Assignment
NOALIAS __forceinline const C_Vertex C_Vertex::operator = ( const HVec3_Add& src )
{
    m_V = HVec3( src );
    return *this;
}

/// HVec3_Subtract Assignment
//FORCE_INLINE
//const C_Vertex C_Vertex::operator = ( const HVec3_Subtract& src )
//{
//    m_V = HVec3( src );
//    return *this;
//}

/// C_Vertex_Multiply Assignment
/*NOALIAS __forceinline const C_Vertex C_Vertex::operator = ( const C_Vertex_Multiply& src ) __restrict
{
    m_V = C_Vertex( src );
    return *this;
}

/// C_Vertex_Add Assignment
NOALIAS __forceinline const C_Vertex C_Vertex::operator = ( const C_Vertex_Add& src ) __restrict
{
    m_V = C_Vertex( src );
    return *this;
}*/

/// C_Vertex_Subtract Assignment
NOALIAS __forceinline const C_Vertex C_Vertex::operator = ( const C_Vertex_Subtract& src ) __restrict
{
    m_V = C_Vertex( src );
    return *this;
}

/// Accessor: X
NOALIAS __forceinline const HF32 C_Vertex::X() const __restrict
{
    return m_V.X();
}

/// Accessor: Y
NOALIAS __forceinline const HF32 C_Vertex::Y() const __restrict
{
    return m_V.Y();
}

/// Accessor: Z
NOALIAS __forceinline const HF32 C_Vertex::Z() const __restrict
{
    return m_V.Z();
}

/// Element Zero X
NOALIAS __forceinline void C_Vertex::ZeroX() __restrict
{
    //m_V.ZeroX();
    m_V = Maths::ZeroX( m_V );
}

/// Element Zero Y
NOALIAS __forceinline void C_Vertex::ZeroY() __restrict
{
    //m_V.ZeroY();
    m_V = Maths::ZeroY( m_V );
}

/// Element Zero Z
NOALIAS __forceinline void C_Vertex::ZeroZ() __restrict
{
    //m_V.ZeroZ();
    m_V = Maths::ZeroZ( m_V );
}

/// Element Zero XY
NOALIAS __forceinline void C_Vertex::ZeroXY() __restrict
{
    //m_V.ZeroXY();
    m_V = Maths::ZeroXY( m_V );
}

/// Element Zero XZ
NOALIAS __forceinline void C_Vertex::ZeroXZ() __restrict
{
    //m_V.ZeroXZ();
    m_V = Maths::ZeroXZ( m_V );
}

/// Element Zero YZ
NOALIAS __forceinline void C_Vertex::ZeroYZ() __restrict
{
    //m_V.ZeroYZ();
    m_V = Maths::ZeroYZ( m_V );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vertex C_Vertex::ZeroedX() const __restrict
{
    //return C_Vertex( m_V.ZeroedX() );
    return C_Vertex( Maths::ZeroX( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vertex C_Vertex::ZeroedY() const __restrict
{
    //return C_Vertex( m_V.ZeroedY() );
    return C_Vertex( Maths::ZeroY( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vertex C_Vertex::ZeroedZ() const __restrict
{
    //return C_Vertex( m_V.ZeroedZ() );
    return C_Vertex( Maths::ZeroZ( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vertex C_Vertex::ZeroedXY() const __restrict
{
    //return C_Vertex( m_V.ZeroedXY() );
    return C_Vertex( Maths::ZeroXY( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vertex C_Vertex::ZeroedXZ() const __restrict
{
    //return C_Vertex( m_V.ZeroedXZ() );
    return C_Vertex( Maths::ZeroXZ( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vertex C_Vertex::ZeroedYZ() const __restrict
{
    //return C_Vertex( m_V.ZeroedYZ() );
    return C_Vertex( Maths::ZeroYZ( m_V ) );
}

/// Addition: Vertex + Vertex --> Vertex
NOALIAS __forceinline const C_Vertex C_Vertex::operator + ( const C_Vertex PCREF src ) const __restrict
{
    return C_Vertex( m_V + src.m_V );
}

/// Addition: Vertex + Vector --> Vertex
NOALIAS __forceinline const C_Vertex C_Vertex::operator + ( const C_Vector PCREF src ) const __restrict
{
    return C_Vertex( m_V + src.m_V );
}

/// Addition: Vertex + Normal --> Vertex
NOALIAS __forceinline const C_Vertex C_Vertex::operator + ( const C_Normal PCREF src ) const __restrict
{
    return C_Vertex( m_V + src.m_V );
}

/// Addition: Vertex += Vertex
/*NOALIAS __forceinline void C_Vertex::operator += ( const C_Vertex PCREF src ) __restrict
{
    m_V += src.m_V;
}*/

/// Addition: Vertex += Vector
NOALIAS __forceinline void C_Vertex::operator += ( const C_Vector PCREF src ) __restrict
{
    m_V += src.m_V;
}

/// Subtraction: Vertex - Vertex --> Vector
NOALIAS __forceinline const C_Vector C_Vertex::operator - ( const C_Vertex PCREF src ) const __restrict
{
    return C_Vector( m_V - src.m_V );
}

/// Subtraction: Vertex - Vector --> Vertex
NOALIAS __forceinline const C_Vertex C_Vertex::operator - ( const C_Vector PCREF src ) const __restrict
{
    return C_Vertex( m_V - src.m_V );
}

/// Subtraction: Vertex -= Vector
NOALIAS __forceinline void C_Vertex::operator -= ( const C_Vector PCREF src ) __restrict
{
    m_V -= src.m_V;
}

/// Subtraction: Vertex - Normal --> Vertex
NOALIAS __forceinline const C_Vertex C_Vertex::operator - ( const C_Normal PCREF src ) const __restrict
{
    return C_Vertex( m_V - src.m_V );
}

/// Subtraction: Vertex - Origin --> Vector
NOALIAS __forceinline const C_Vector C_Vertex::operator - ( const C_Origin PCREF /*src*/ ) const __restrict
{
    return C_Vector( m_V );
}

/// Multiplication: Vertex * HF32 --> Vertex
//NOALIAS __forceinline const C_Vertex C_Vertex::operator * ( const HF32 PCREF scalar ) const __restrict
//{
//    return C_Vertex( m_V * scalar );
//}

/// Multiplication: Vertex * HMat43 --> Vertex
NOALIAS __forceinline const C_Vertex C_Vertex::operator * ( const HMat43& matrix ) const __restrict
{
    return C_Vertex( m_V * matrix );
}

/// Multiplication: Vertex * C_QuatTrans --> Vertex
//NOALIAS __forceinline const C_Vertex C_Vertex::operator * ( const C_QuatTrans& qt ) const __restrict
//{
//    return C_Vertex( ( m_V * qt.m_Quaternion ) + qt.m_Translation );
//}

NOALIAS __forceinline const C_Vertex C_Vertex::Transformed( const C_Quaternion PCREF rotation, const C_Vector PCREF translation ) const __restrict
{
	//return m_V.Transformed( rotation, translation.m_V );
	return Transform( m_V, rotation, translation.m_V );
}

/// Multiplication: Vertex *= HF32
NOALIAS __forceinline void C_Vertex::operator *= ( const HF32 PCREF scalar ) __restrict
{
    m_V *= scalar;
}

/// Minimum: Vertex min Vertex --> Vertex
NOALIAS __forceinline const C_Vertex C_Vertex::Min( const C_Vertex PCREF src ) const __restrict
{
    return C_Vertex( Maths::Min( m_V, src.m_V ) );
}

/// Minimum: Vertex max Vertex --> Vertex
NOALIAS __forceinline const C_Vertex C_Vertex::Max( const C_Vertex PCREF src ) const __restrict
{
    return C_Vertex( Maths::Max( m_V, src.m_V ) );
}

/// Distance
NOALIAS __forceinline const HF32 C_Vertex::Distance( const C_Vertex PCREF src ) const __restrict
{
    //return m_V.Distance( src.m_V );
    return Maths::Distance( m_V, src.m_V );
}

/// DistanceSquared
NOALIAS __forceinline const HF32 C_Vertex::DistanceSquared( const C_Vertex PCREF src ) const __restrict
{
    //return m_V.DistanceSquared( src.m_V );
    return Maths::DistanceSquared( m_V, src.m_V );
}

/// Lerp
NOALIAS __forceinline const C_Vertex C_Vertex::Lerp( const C_Vertex PCREF src, const HF32 PCREF t ) const __restrict
{
    //return C_Vertex( m_V.Lerp( src.m_V, t ) );
    return C_Vertex( Maths::Lerp( m_V, src.m_V, t ) );
}

/// Round to integer towards minus infinity
NOALIAS __forceinline const C_Vertex C_Vertex::RoundToIntMinusInf() const __restrict
{
    return C_Vertex( m_V.RoundToIntMinusInf() );
}

/// Round to integer towards plus infinity
NOALIAS __forceinline const C_Vertex C_Vertex::RoundToIntPlusInf() const __restrict
{
    return C_Vertex( m_V.RoundToIntPlusInf() );
}

/// Round to integer towards nearest
NOALIAS __forceinline const C_Vertex C_Vertex::RoundToIntNearest() const __restrict
{
    return C_Vertex( m_V.RoundToIntNearest() );
}

/// Round to integer towards zero
NOALIAS __forceinline const C_Vertex C_Vertex::RoundToIntZero() const __restrict
{
    return C_Vertex( m_V.RoundToIntZero() );
}

/// Comparison - Less Than
/*NOALIAS __forceinline const Vector3_Bool C_Vertex::operator < ( const C_Vertex PCREF src ) const __restrict
{
    return m_V < src.m_V;
}

/// Comparison - Less Equal
NOALIAS __forceinline const Vector3_Bool C_Vertex::operator <= ( const C_Vertex PCREF src ) const __restrict
{
    Vector3_Bool result( UNDEFINED );
    result = m_V <= src.m_V;
    return result;
}

/// Comparison - Greater Than
NOALIAS __forceinline const Vector3_Bool C_Vertex::operator > ( const C_Vertex PCREF src ) const __restrict
{
    return m_V > src.m_V;
}

/// Comparison - Greater Equal
NOALIAS __forceinline const Vector3_Bool C_Vertex::operator >= ( const C_Vertex PCREF src ) const __restrict
{
    return m_V >= src.m_V;
}

/// Comparison - Equal
NOALIAS __forceinline const Vector3_Bool C_Vertex::operator == ( const C_Vertex PCREF src ) const __restrict
{
    return m_V == src.m_V;
}

/// Comparison - Not Equal
NOALIAS __forceinline const Vector3_Bool C_Vertex::operator != ( const C_Vertex PCREF src ) const __restrict
{
    return m_V != src.m_V;
}*/

/// Comparison Function ==
NOALIAS __forceinline const U32 C_Vertex::All_EQ( const C_Vertex PCREF src ) const __restrict
{
    return m_V.All_EQ( src.m_V );
}

/// Comparison Function !=
NOALIAS __forceinline const U32 C_Vertex::All_NE( const C_Vertex PCREF src ) const __restrict
{
    return m_V.All_NE( src.m_V );
}

/// Comparison Function <=
NOALIAS __forceinline const U32 C_Vertex::All_LE( const C_Vertex PCREF src ) const __restrict
{
    return m_V.All_LE( src.m_V );
}

/// Comparison Function <
NOALIAS __forceinline const U32 C_Vertex::All_LT( const C_Vertex PCREF src ) const __restrict
{
    return m_V.All_LT( src.m_V );
}

/// Comparison Function >=
NOALIAS __forceinline const U32 C_Vertex::All_GE( const C_Vertex PCREF src ) const __restrict
{
    return m_V.All_GE( src.m_V );
}

/// Comparison Function >
NOALIAS __forceinline const U32 C_Vertex::All_GT( const C_Vertex PCREF src ) const __restrict
{
    return m_V.All_GT( src.m_V );
}

/// Comparison Function ==
NOALIAS __forceinline const U32 C_Vertex::Any_EQ( const C_Vertex PCREF src ) const __restrict
{
    return m_V.Any_EQ( src.m_V );
}

/// Comparison Function !=
NOALIAS __forceinline const U32 C_Vertex::Any_NE( const C_Vertex PCREF src ) const __restrict
{
    return m_V.Any_NE( src.m_V );
}

/// Comparison Function <=
NOALIAS __forceinline const U32 C_Vertex::Any_LE( const C_Vertex PCREF src ) const __restrict
{
    return m_V.Any_LE( src.m_V );
}

/// Comparison Function <
NOALIAS __forceinline const U32 C_Vertex::Any_LT( const C_Vertex PCREF src ) const __restrict
{
    return m_V.Any_LT( src.m_V );
}

/// Comparison Function >=
NOALIAS __forceinline const U32 C_Vertex::Any_GE( const C_Vertex PCREF src ) const __restrict
{
    return m_V.Any_GE( src.m_V );
}

/// Comparison Function >
NOALIAS __forceinline const U32 C_Vertex::Any_GT( const C_Vertex PCREF src ) const __restrict
{
    return m_V.Any_GT( src.m_V );
}

/// Finite test
NOALIAS __forceinline const Scalar_Bool C_Vertex::Finite() const __restrict
{
    return m_V.Finite();
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Vertex::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_V;
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Vertex::Serialise( T_Stream& stream ) __restrict
{
    m_V.Serialise(stream);
}

NOALIAS __forceinline void operator *= ( C_Vertex& v, const F32& f ) { v *= Load<HF32>( &f ); }
NOALIAS __forceinline void operator /= ( C_Vertex& v, const F32& f ) { v *= Reciprocal( Load<HF32>( &f ) ); }




//
// ----------------------------------------------
// |-- C_Vector : HVec3 with vector semantics --|
// ----------------------------------------------
//

/// Zero Init Constructor
NOALIAS __forceinline C_Vector::C_Vector( const _ZERO_INIT /*zeroinit*/ ) :
	m_V( ZERO_INIT )
{
}

/// Default Constructor
NOALIAS __forceinline C_Vector::C_Vector()
{
}

/// Undefined Constructor
NOALIAS __forceinline C_Vector::C_Vector( const _UNDEFINED /*undefined*/ ) :
    m_V( UNDEFINED )
{
}

/// Copy Constructor
NOALIAS __forceinline C_Vector::C_Vector( const C_Vector& copy ) :
    m_V( copy.m_V )
{
}

/// Explicit Vertex Constructor
NOALIAS __forceinline C_Vector::C_Vector( const C_Vertex PCREF src ) :
    m_V( src.m_V )
{
}

/// Explicit Normal Constructor
NOALIAS __forceinline C_Vector::C_Vector( const C_Normal PCREF src ) :
    m_V( src.m_V )
{
}

/// HVec3 Constructor
NOALIAS __forceinline C_Vector::C_Vector( const HVec3 PCREF src ) :
    m_V( src )
{
}

/// Explicit HVec4 Constructor
NOALIAS __forceinline C_Vector::C_Vector( const HVec4 PCREF src ) :
    m_V( HVec3( src ) )
{
}

/// HF32 XYZ Constructor
NOALIAS __forceinline C_Vector::C_Vector( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z ) :
    m_V( x, y, z )
{
}

/// Explicit F32 Array Constructor
NOALIAS __forceinline C_Vector::C_Vector( const F32* xyz ) :
    m_V( xyz )
{
}

/// HF32 XYZ Broadcast Constructor
NOALIAS __forceinline C_Vector::C_Vector( const HF32 PCREF xyz ) :
    m_V( xyz )
{
}

/// Explicit Constructor from Float3 ptr
NOALIAS __forceinline C_Vector::C_Vector( MathsStorage::Float3 const * const __restrict pStorage ) :
    m_V( pStorage )
{
}

/// Cast to Float3
NOALIAS __forceinline C_Vector::operator const MathsStorage::Float3 () const __restrict
{
    return (MathsStorage::Float3)m_V;
}

/// Explicit Constructor from Fixed3_S16 ptr
template < int T_Precision >
NOALIAS __forceinline C_Vector::C_Vector( MathsStorage::Fixed3_S16<T_Precision> const * pStorage ) :
    m_V( pStorage )
{
}

/// Cast to Fixed3_S16
template < int T_Precision >
NOALIAS __forceinline C_Vector::operator const MathsStorage::Fixed3_S16<T_Precision> () const __restrict
{
    return (MathsStorage::Fixed3_S16<T_Precision>)m_V;
}

/// HVec3_Multiply Constructor
//FORCE_INLINE
//C_Vector::C_Vector( const HVec3_Multiply& src )
//    : m_V( HVec3( src ) )
//{
//}

/// HVec3_Add Constructor
//FORCE_INLINE
//C_Vector::C_Vector( const HVec3_Add& src )
//    : m_V( HVec3( src ) )
//{
//}

/// HVec3_Subtract Constructor
//FORCE_INLINE
//C_Vector::C_Vector( const HVec3_Subtract& src )
//    : m_V( HVec3( src ) )
//{
//}

/// HVec3 Cast
//FORCE_INLINE
//C_Vector::operator const HVec3 () const
//{
//    return m_V;
//}

/// Copy Assignment
NOALIAS __forceinline const C_Vector C_Vector::operator = ( const C_Vector& copy )
{
    m_V = copy.m_V;
    return copy;
}

/// HVec3 Assignment
NOALIAS __forceinline const C_Vector C_Vector::operator = ( const HVec3 PCREF src ) __restrict
{
    m_V = src;
    return *this;
}

/// C_Normal Assignment
NOALIAS __forceinline const C_Vector C_Vector::operator = ( const C_Normal PCREF src ) __restrict
{
    m_V = src.m_V;
    return *this;
}

/// HVec3_Multiply Assignment
//FORCE_INLINE
//const C_Vector C_Vector::operator = ( const HVec3_Multiply& src )
//{
//    m_V = HVec3( src );
//    return *this;
//}

/// HVec3_Add Assignment
//FORCE_INLINE
//const C_Vector C_Vector::operator = ( const HVec3_Add& src )
//{
//    m_V = HVec3( src );
//    return *this;
//}

/// HVec3_Subtract Assignment
//FORCE_INLINE
//const C_Vector C_Vector::operator = ( const HVec3_Subtract& src )
//{
//    m_V = HVec3( src );
//    return *this;
//}

/// Accessor: X
NOALIAS __forceinline const HF32 C_Vector::X() const __restrict
{
    return m_V.X();
}

/// Accessor: Y
NOALIAS __forceinline const HF32 C_Vector::Y() const __restrict
{
    return m_V.Y();
}

/// Accessor: Z
NOALIAS __forceinline const HF32 C_Vector::Z() const __restrict
{
    return m_V.Z();
}

/// Accessor: Set X
NOALIAS __forceinline void C_Vector::X( const HF32 PCREF x ) __restrict
{
    m_V.X( x );
}

/// Accessor: Set Y
NOALIAS __forceinline void C_Vector::Y( const HF32 PCREF y ) __restrict
{
    m_V.Y( y );
}

/// Accessor: Set Z
NOALIAS __forceinline void C_Vector::Z( const HF32 PCREF z ) __restrict
{
    m_V.Z( z );
}

/// Element Zero X
NOALIAS __forceinline void C_Vector::ZeroX() __restrict
{
    //m_V.ZeroX();
    m_V = Maths::ZeroX( m_V );
}

/// Element Zero Y
NOALIAS __forceinline void C_Vector::ZeroY() __restrict
{
    //m_V.ZeroY();
    m_V = Maths::ZeroY( m_V );
}

/// Element Zero Z
NOALIAS __forceinline void C_Vector::ZeroZ() __restrict
{
    //m_V.ZeroZ();
    m_V = Maths::ZeroZ( m_V );
}

/// Element Zero XY
NOALIAS __forceinline void C_Vector::ZeroXY() __restrict
{
    //m_V.ZeroXY();
    m_V = Maths::ZeroXY( m_V );
}

/// Element Zero XZ
NOALIAS __forceinline void C_Vector::ZeroXZ() __restrict
{
    //m_V.ZeroXZ();
    m_V = Maths::ZeroXZ( m_V );
}

/// Element Zero YZ
NOALIAS __forceinline void C_Vector::ZeroYZ() __restrict
{
    //m_V.ZeroYZ();
    m_V = Maths::ZeroYZ( m_V );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vector C_Vector::ZeroedX() const __restrict
{
    //return C_Vector( m_V.ZeroedX() );
    return C_Vector( Maths::ZeroX( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vector C_Vector::ZeroedY() const __restrict
{
    //return C_Vector( m_V.ZeroedY() );
    return C_Vector( Maths::ZeroY( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vector C_Vector::ZeroedZ() const __restrict
{
    //return C_Vector( m_V.ZeroedZ() );
    return C_Vector( Maths::ZeroZ( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vector C_Vector::ZeroedXY() const __restrict
{
    //return C_Vector( m_V.ZeroedXY() );
    return C_Vector( Maths::ZeroXY( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vector C_Vector::ZeroedXZ() const __restrict
{
    //return C_Vector( m_V.ZeroedXZ() );
    return C_Vector( Maths::ZeroXZ( m_V ) );
}

/// Return copy with elements zeroed
NOALIAS __forceinline const C_Vector C_Vector::ZeroedYZ() const __restrict
{
    //return C_Vector( m_V.ZeroedYZ() );
    return C_Vector( Maths::ZeroYZ( m_V ) );
}

/// Negation: Vector --> Vector
NOALIAS __forceinline const C_Vector C_Vector::operator - () const __restrict
{
    return C_Vector( -m_V );
}

/// Addition: Vector + Vector --> Vector
NOALIAS __forceinline const C_Vector C_Vector::operator + ( const C_Vector PCREF src ) const __restrict
{
    return C_Vector( m_V + src.m_V );
}

/// Addition: Vector + Normal --> Vector
NOALIAS __forceinline const C_Vector C_Vector::operator + ( const C_Normal PCREF src ) const __restrict
{
    return C_Vector( m_V + src.m_V );
}

/// Addition: Vector += Vector
NOALIAS __forceinline void C_Vector::operator += ( const C_Vector PCREF src ) __restrict
{
    m_V += src.m_V;
}

/// Addition: Vector += Normal
NOALIAS __forceinline void C_Vector::operator += ( const C_Normal PCREF src ) __restrict
{
    m_V += src.m_V;
}

/// Addition: Vector + Vertex --> Vertex
NOALIAS __forceinline const C_Vertex C_Vector::operator + ( const C_Vertex PCREF src ) const __restrict
{
    return C_Vertex( m_V + src.m_V );
}

/// Addition: Vector + Origin --> Vertex
NOALIAS __forceinline const C_Vertex C_Vector::operator + ( const C_Origin PCREF /*src*/ ) const __restrict
{
    return C_Vertex( m_V );
}

/// Subtraction: Vector - Vector --> Vector
NOALIAS __forceinline const C_Vector C_Vector::operator - ( const C_Vector PCREF src ) const __restrict
{
    return C_Vector( m_V - src.m_V );
}

/// Subtraction: Vector -= Vector
NOALIAS __forceinline void C_Vector::operator -= ( const C_Vector PCREF src ) __restrict
{
    m_V -= src.m_V;
}

/// Subtraction: Vector -= Normal
NOALIAS __forceinline void C_Vector::operator -= ( const C_Normal PCREF src ) __restrict
{
    m_V -= src.m_V;
}

/// Multiplication: Vector * Vector --> Vector
NOALIAS __forceinline const C_Vector C_Vector::operator * ( const C_Vector PCREF src ) const __restrict
{
    return C_Vector( m_V * src.m_V );
}

/// Multiplication: Vector * HF32 --> Vector
/*NOALIAS __forceinline const C_Vector C_Vector::operator * ( const HF32 PCREF scalar ) const __restrict
{
    return C_Vector( m_V * scalar );
}*/

/// Multiplication: Vector *= HF32
NOALIAS __forceinline void C_Vector::operator *= ( const HF32 PCREF scalar ) __restrict
{
    m_V *= scalar;
}

/// Division: Vector / HF32 --> Vector
/*NOALIAS __forceinline const C_Vector C_Vector::operator / ( const HF32 PCREF scalar ) const __restrict
{
    return C_Vector( m_V / scalar );
}*/

/// Division: Vector /= HF32
NOALIAS __forceinline void C_Vector::operator /= ( const HF32 PCREF scalar ) __restrict
{
    m_V /= scalar;
}

/// Multiplication: Vector * HMat33 --> Vector
NOALIAS __forceinline const C_Vector C_Vector::operator * ( const HMat33& matrix ) const __restrict
{
    return C_Vector( m_V * matrix );
}

/// Multiplication: Vector * C_Quaternion --> Vector
//NOALIAS __forceinline const C_Vector C_Vector::operator * ( const C_Quaternion PCREF q ) const __restrict
//{
//    return C_Vector( m_V * q );
//}

/// Dot Product: Vector | Vector --> HF32
NOALIAS __forceinline const HF32 C_Vector::operator | ( const C_Vector PCREF src ) const __restrict
{
    return m_V | src.m_V;
}

/// Dot Product: Vector | Normal --> HF32
NOALIAS __forceinline const HF32 C_Vector::operator | ( const C_Normal PCREF src ) const __restrict
{
    return m_V | src.m_V;
}

/// Cross Product: Vector ^ Vector --> Vector
NOALIAS __forceinline const C_Vector C_Vector::operator ^ ( const C_Vector PCREF src ) const __restrict
{
    return C_Vector( m_V ^ src.m_V );
}

/// Cross Product: Vector ^ Normal --> Vector
NOALIAS __forceinline const C_Vector C_Vector::operator ^ ( const C_Normal PCREF src ) const __restrict
{
    return C_Vector( m_V ^ src.m_V );
}

/// Star function = Cross Product Matrix ( a* Matrix )
NOALIAS __forceinline const HMat33 C_Vector::Star() const __restrict
{
    return Maths::Star( m_V );
}

/// Normalised
NOALIAS __forceinline const C_Normalised C_Vector::Normalised() const __restrict
{
    //return m_V.Normalised();
    return NormalLengthSquaredAndReciprocal( m_V );
}

/// Safe Normalised
NOALIAS __forceinline const C_Normalised C_Vector::SafeNormalised( const C_Normal PCREF zeroLengthResult ) const __restrict
{
    return m_V.SafeNormalised( zeroLengthResult );
}

/// Vector Length
NOALIAS __forceinline const HF32 C_Vector::Length() const __restrict
{
    return Maths::Length( m_V );
}

/// Vector Length Squared
NOALIAS __forceinline const HF32 C_Vector::LengthSquared() const __restrict
{
    return Maths::LengthSquared( m_V );
}

/// Vector Reciprocal of Length
NOALIAS __forceinline const HF32 C_Vector::ReciprocalLength() const __restrict
{
    return Maths::ReciprocalLength( m_V );
}

/// Get component of this vector parallel to input normal
NOALIAS __forceinline const C_Vector C_Vector::Parallel( const C_Normal PCREF normal ) const __restrict
{
    return m_V.Parallel( normal );
}

/// Get component of this vector perpendicular to input normal
NOALIAS __forceinline const C_Vector C_Vector::Perpendicular( const C_Normal PCREF normal ) const __restrict
{
    return m_V.Perpendicular( normal );
}

/// Get components of this vector parallel & perpendicular to input normal
NOALIAS __forceinline void C_Vector::ParaPerp( const C_Normal PCREF normal, C_Vector& para, C_Vector& perp ) const __restrict
{
    //const HF32 dp( operator | ( normal ) );
    //para = normal * dp;
    //perp = operator - ( para );
    m_V.ParaPerp( normal, para.m_V, perp.m_V );
}

/// Comparison - Less Than
/*NOALIAS __forceinline const Vector3_Bool C_Vector::operator < ( const C_Vector PCREF src ) const __restrict
{
    return m_V < src.m_V;
}

/// Comparison - Less Equal
NOALIAS __forceinline const Vector3_Bool C_Vector::operator <= ( const C_Vector PCREF src ) const __restrict
{
    return m_V <= src.m_V;
}

/// Comparison - Greater Than
NOALIAS __forceinline const Vector3_Bool C_Vector::operator > ( const C_Vector PCREF src ) const __restrict
{
    return m_V > src.m_V;
}

/// Comparison - Greater Equal
NOALIAS __forceinline const Vector3_Bool C_Vector::operator >= ( const C_Vector PCREF src ) const __restrict
{
    return m_V >= src.m_V;
}

/// Comparison - Equal
NOALIAS __forceinline const Vector3_Bool C_Vector::operator == ( const C_Vector PCREF src ) const __restrict
{
    return m_V == src.m_V;
}

/// Comparison - Not Equal
NOALIAS __forceinline const Vector3_Bool C_Vector::operator != ( const C_Vector PCREF src ) const __restrict
{
    return m_V != src.m_V;
}*/

/// Comparison - Equal
NOALIAS __forceinline const U32 C_Vector::operator == ( const C_Vector PCREF src ) const __restrict
{
	return m_V.All_EQ(src.m_V);
}
 
/// Comparison - Not Equal
NOALIAS __forceinline const U32 C_Vector::operator != ( const C_Vector PCREF src ) const __restrict
{
	return m_V.Any_NE(src.m_V);
}

/// Finite test
NOALIAS __forceinline const Scalar_Bool C_Vector::Finite() const __restrict
{
    return m_V.Finite();
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Vector::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_V;
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Vector::Serialise( T_Stream& stream ) __restrict
{
    m_V.Serialise(stream);
}

NOALIAS __forceinline void      operator *= ( C_Vector& v, const F32& f )       { v *= HF32( f ); }
NOALIAS __forceinline void      operator /= ( C_Vector& v, const F32& f )       { v /= HF32( f ); }





//
// -----------------------------------------------------------------------------------------------------------
// |-- C_Normal : HVec3 with unit length normal semantics - caveat: uninitialised normals are zero length! --|
// -----------------------------------------------------------------------------------------------------------
//

/// Positive X Axis Aligned Normal Constructor
NOALIAS __forceinline const C_Normal C_Normal::AxisPositiveX()
{
    return C_Normal( HVec3( c_UnitVectorX ), UNIT_LENGTH_VECTOR );
}

/// Negative X Axis Aligned Normal Constructor
NOALIAS __forceinline const C_Normal C_Normal::AxisNegativeX()
{
    return -C_Normal( HVec3( c_UnitVectorX ), UNIT_LENGTH_VECTOR );
}

/// Positive Y Axis Aligned Normal Constructor
NOALIAS __forceinline const C_Normal C_Normal::AxisPositiveY()
{
    return C_Normal( HVec3( c_UnitVectorY ), UNIT_LENGTH_VECTOR );
}

/// Negative Y Axis Aligned Normal Constructor
NOALIAS __forceinline const C_Normal C_Normal::AxisNegativeY()
{
    return -C_Normal( HVec3( c_UnitVectorY ), UNIT_LENGTH_VECTOR );
}

/// Positive Z Axis Aligned Normal Constructor
NOALIAS __forceinline const C_Normal C_Normal::AxisPositiveZ()
{
    return C_Normal( HVec3( c_UnitVectorZ ), UNIT_LENGTH_VECTOR );
}

/// Negative Z Axis Aligned Normal Constructor
NOALIAS __forceinline const C_Normal C_Normal::AxisNegativeZ()
{
    return -C_Normal( HVec3( c_UnitVectorZ ), UNIT_LENGTH_VECTOR );
}

/// Zero Init Constructor
NOALIAS __forceinline C_Normal::C_Normal( const _ZERO_INIT /*zeroinit*/ ) :
    m_V( ZERO_INIT )
{
}

/// Default Constructor
NOALIAS __forceinline C_Normal::C_Normal()
{
}

/// Undefined Constructor
NOALIAS __forceinline C_Normal::C_Normal( const _UNDEFINED /*undefined*/ ) :
    m_V( UNDEFINED )
{
}

/// Unit Length Vector Constructor
NOALIAS __forceinline C_Normal::C_Normal( const HVec3 PCREF src, const _UNIT_LENGTH_VECTOR /*unit_length_vector*/ ) :
    m_V( src )
{
}

/// Copy Constructor
NOALIAS __forceinline C_Normal::C_Normal( const C_Normal& copy ) :
    m_V( copy.m_V )
{
}

/// C_Normalised Constructor
NOALIAS __forceinline C_Normal::C_Normal( const C_Normalised& src ) :
    m_V( src.m_Normal.m_V )
{
}

/// HVec3 Constructor
NOALIAS __forceinline C_Normal::C_Normal( const HVec3 PCREF src ) :
    //m_V( src.Normalised().m_Normal.m_V )
    m_V( Normalise( src ).m_V )
{
}

/// Vector Constructor
NOALIAS __forceinline C_Normal::C_Normal( const C_Vector PCREF src ) :
    //m_V( src.m_V.Normalised().m_Normal.m_V )
    m_V( Normalise( src.m_V ).m_V )
{
}

#if defined( _SIMD_ )
/// Explicit V128 Constructor
NOALIAS __forceinline C_Normal::C_Normal( const V128 PCREF src ) :
    m_V( src )
{
}
#endif

/// Explicit HVec4 Constructor
NOALIAS __forceinline C_Normal::C_Normal( const HVec4 PCREF src ) :
    //m_V( HVec3( src ).Normalised().m_Normal.m_V )
    m_V( Normalise( HVec3( src ) ).m_V )
{
}

/// /*Explicit*/ C_SiVector Constructor
template < class LHS >
NOALIAS __forceinline C_Normal::C_Normal( const C_SiVector<LHS> PCREF src ) :
    //m_V( src.Get().Normalised().m_Normal.m_V )
    //m_V( src.m_Vector.Normalised().m_Normal.m_V )
    m_V( Normalise( src.m_Vector ).m_V )
{
}

/// HF32 XYZ Constructor
NOALIAS __forceinline C_Normal::C_Normal( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z ) :
    //m_V( HVec3( x, y, z ).Normalised().m_Normal.m_V )
    m_V( Normalise( HVec3( x, y, z ) ).m_V )
{
}

/// F32 array constructor
NOALIAS __forceinline C_Normal::C_Normal( F32 const * const __restrict pXYZ ) :
    m_V( pXYZ )
{
}

/// Explicit Constructor from Float3 ptr
NOALIAS __forceinline C_Normal::C_Normal( MathsStorage::Float3 const * const __restrict pStorage ) :
    m_V( pStorage )
{
}

/// Cast to Float3
NOALIAS __forceinline C_Normal::operator const MathsStorage::Float3 () const __restrict
{
    return (MathsStorage::Float3)m_V;
}

/// Selector Constructor: Selects fields from the inputs based on the selector mask: result = ( ( selector & a ) | ( ~selector & b ) )
//FORCE_INLINE
//C_Normal::C_Normal( const Vector3_Bool& selector, const C_Normal a, const C_Normal b ) :
//    m_V( selector, a.m_V, b.m_V )
//{
//}

/// HVec3 Cast
//FORCE_INLINE
//C_Normal::operator const HVec3 () const
//{
//    return m_V;
//}

/// Copy Assignment
NOALIAS __forceinline const C_Normal C_Normal::operator = ( const C_Normal& copy )
{
    m_V = copy.m_V;
    return copy;
}

/// HVec3 Assignment
NOALIAS __forceinline const C_Normal C_Normal::operator = ( const HVec3 PCREF src ) __restrict
{
    //m_V = src.Normalised().m_Normal.m_V;
    m_V = Normalise( src ).m_V;
    return *this;
}

/// C_Vector Assignment
NOALIAS __forceinline const C_Normal C_Normal::operator = ( const C_Vector PCREF src ) __restrict
{
    m_V = src.Normalised().m_Normal.m_V;
    return *this;
}

/// C_SiVector Assignment
template < class LHS >
NOALIAS __forceinline const C_Normal C_Normal::operator = ( const C_SiVector<LHS> PCREF src ) __restrict
{
    m_V = src.Get().Normalised().m_Normal.m_V;
    return *this;
}

/// Accessor: X
NOALIAS __forceinline const HF32 C_Normal::X() const __restrict
{
    return m_V.X();
}

/// Accessor: Y
NOALIAS __forceinline const HF32 C_Normal::Y() const __restrict
{
    return m_V.Y();
}

/// Accessor: Z
NOALIAS __forceinline const HF32 C_Normal::Z() const __restrict
{
    return m_V.Z();
}

/// Negation: Normal --> Normal
NOALIAS __forceinline const C_Normal C_Normal::operator - () const __restrict
{
    return C_Normal( -m_V, UNIT_LENGTH_VECTOR );
}

/// Multiplication: Normal * HF32 -> Vector
//NOALIAS __forceinline const C_Vector C_Normal::operator * ( const HF32 PCREF scalar ) const __restrict
//{
//    return C_Vector( m_V * scalar );
//}

/// Multiplication: Normal * HMat33 --> Vector
NOALIAS __forceinline const C_Vector C_Normal::operator * ( const HMat33& matrix ) const __restrict
{
    return C_Vector( m_V * matrix );
}

/// Multiplication: Normal * C_Quaternion --> Normal
//NOALIAS __forceinline const C_Normal C_Normal::operator * ( const C_Quaternion q ) const __restrict
//{
//    //return C_Normal( ( m_V * q ).m_V );
//    return C_Normal( m_V * q, UNIT_LENGTH_VECTOR );
//}

/// Dot Product: Normal | HVec3 --> HF32
NOALIAS __forceinline const HF32 C_Normal::operator | ( const HVec3 PCREF src ) const __restrict
{
    return m_V | src;
}

/// Dot Product: Normal | Vector --> HF32
NOALIAS __forceinline const HF32 C_Normal::operator | ( const C_Vector PCREF src ) const __restrict
{
    return m_V | src.m_V;
}

/// Dot Product: Normal | Normal --> HF32
NOALIAS __forceinline const HF32 C_Normal::operator | ( const C_Normal PCREF src ) const __restrict
{
    return m_V | src.m_V;
}

/// Cross Product: Normal ^ HVec3 --> Vector
NOALIAS __forceinline const C_Vector C_Normal::operator ^ ( const HVec3 PCREF src ) const __restrict
{
    return C_Vector( m_V ^ src );
}

/// Cross Product: Normal ^ Vector --> Vector
NOALIAS __forceinline const C_Vector C_Normal::operator ^ ( const C_Vector PCREF src ) const __restrict
{
    return C_Vector( m_V ^ src.m_V );
}

/// Cross Product: Normal ^ Normal --> Vector
NOALIAS __forceinline const C_Vector C_Normal::operator ^ ( const C_Normal PCREF src ) const __restrict
{
    return C_Vector( m_V ^ src.m_V );
}

/// Selects fields from the inputs based on the selector mask: result = ( ( selector & a ) | ( ~selector & b ) )
//DEPRECATED NOALIAS __forceinline void C_Normal::Select( const Vector3_Bool& selector, const C_Normal a, const C_Normal b ) __restrict
//{
//    m_V.Select( selector, a, b );
//}

/// Finite Test
NOALIAS __forceinline const Scalar_Bool C_Normal::Finite() const __restrict
{
    return m_V.Finite();
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Normal::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_V;
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Normal::Serialise( T_Stream& stream ) __restrict
{
    m_V.Serialise( stream );
}

//NOALIAS __forceinline C_Vector  operator * ( const C_Normal PCREF v, const F32& f )   { return v * Load<HF32>( &f ); }
//NOALIAS __forceinline C_Vector  operator / ( const C_Normal PCREF v, const F32& f )   { return v * Reciprocal( Load<HF32>( &f ) ); }





//
// ----------------------------------------------------
// |-- C_VectorLength : HVec3 vector + HVecW length --|
// ----------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_VectorLength::C_VectorLength() :
    m_VectorLength( c_VectorZero )
{
}

/// Copy Constructor
NOALIAS __forceinline C_VectorLength::C_VectorLength( const C_VectorLength& copy ) :
    m_VectorLength( copy.m_VectorLength )
{
}

/// Copy Assignment
NOALIAS __forceinline const C_VectorLength& C_VectorLength::operator = ( const C_VectorLength& copy )
{
    m_VectorLength = copy.m_VectorLength;
    return copy;
}

/// C_Vector Constructor
NOALIAS __forceinline C_VectorLength::C_VectorLength( const C_Vector PCREF vector ) :
    m_VectorLength( HVec3( vector ) || HVecW( vector.Length() ) )
{
}

/// C_Normal + length Constructor
NOALIAS __forceinline C_VectorLength::C_VectorLength( const C_Normal PCREF normal, const HF32 PCREF length ) :
    m_VectorLength( HVec3( normal ) || HVecW( length ) )
{
}

/// Protected HVec4 Constructor
NOALIAS __forceinline C_VectorLength::C_VectorLength( const HVec4 PCREF src ) :
    m_VectorLength( src )
{
}

/// Vector Accessor
NOALIAS __forceinline const C_Vector C_VectorLength::Vector() const __restrict
{
    return C_Vector( m_VectorLength );
}

/// Length Accessor
NOALIAS __forceinline const HF32 C_VectorLength::Length() const __restrict
{
    return m_VectorLength.W();
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void C_VectorLength::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_VectorLength;
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void C_VectorLength::Serialise( T_Stream& stream ) __restrict
{
    m_VectorLength.Serialise(stream);
}





//
// -----------------------------------------------------------------|
// |-- C_VertexRcpLength : HVec3 vertex + HVecW reciprocal length --|
// -----------------------------------------------------------------|
//

/// Default Constructor
NOALIAS __forceinline C_VertexRcpLength::C_VertexRcpLength() :
    m_VertexRcpLength( c_VectorZero )
{
}

/// Copy Constructor
NOALIAS __forceinline C_VertexRcpLength::C_VertexRcpLength( const C_VertexRcpLength& copy ) :
    m_VertexRcpLength( copy.m_VertexRcpLength )
{
}

/// Copy Assignment
NOALIAS __forceinline const C_VertexRcpLength& C_VertexRcpLength::operator = ( const C_VertexRcpLength& copy )
{
    m_VertexRcpLength = copy.m_VertexRcpLength;
    return copy;
}

/// C_Vertex + reciprocal length Constructor
NOALIAS __forceinline C_VertexRcpLength::C_VertexRcpLength( const C_Vertex PCREF vertex, const HF32 PCREF rcpLength ) :
    m_VertexRcpLength( HVec3( vertex ) || HVecW( rcpLength ) )
{
}

/// Vertex Accessor
NOALIAS __forceinline const C_Vertex C_VertexRcpLength::Vertex() const __restrict
{
    return C_Vertex( m_VertexRcpLength );
}

/// Length Accessor
NOALIAS __forceinline const HF32 C_VertexRcpLength::RcpLength() const __restrict
{
    return m_VertexRcpLength.W();
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void C_VertexRcpLength::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_VertexRcpLength;
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void C_VertexRcpLength::Serialise( T_Stream& stream ) __restrict
{
    m_VertexRcpLength.Serialise(stream);
}




//
// -------------------------------------------------------------------------------------------------------
// |-- C_PackedNormalLength : HVec3 normal + HVecW length, packed into a single 4 element float vector --|
// -------------------------------------------------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_PackedNormalLength::C_PackedNormalLength() :
    m_NormalLength( c_VectorZero )
{
}

/// Copy Constructor
NOALIAS __forceinline C_PackedNormalLength::C_PackedNormalLength( const C_PackedNormalLength& copy ) :
    m_NormalLength( copy.m_NormalLength )
{
}

/// Copy Assignment
NOALIAS __forceinline const C_PackedNormalLength& C_PackedNormalLength::operator = ( const C_PackedNormalLength& copy )
{
    m_NormalLength = copy.m_NormalLength;
    return copy;
}

/// C_Vector Constructor
NOALIAS __forceinline C_PackedNormalLength::C_PackedNormalLength( const C_Vector PCREF vector ) :
    m_NormalLength( HVec3( Normalise( vector ) ) || HVecW( vector.Length() ) )
{
}

/// C_Normal + length Constructor
NOALIAS __forceinline C_PackedNormalLength::C_PackedNormalLength( const C_Normal PCREF normal, const HF32 PCREF length ) :
    m_NormalLength( HVec3( normal ) || HVecW( length ) )
{
}

/// Normal Accessor
NOALIAS __forceinline const C_Normal C_PackedNormalLength::Normal() const __restrict
{
    return C_Normal( HVec3( m_NormalLength ), C_Normal::UNIT_LENGTH_VECTOR );
}

/// Normal Accessor
NOALIAS __forceinline const C_Vector C_PackedNormalLength::Vector() const __restrict
{
    return C_Vector( HVec3( m_NormalLength ) * m_NormalLength.W() );
}

/// Length Accessor
NOALIAS __forceinline const HF32 C_PackedNormalLength::Length() const __restrict
{
    return m_NormalLength.W();
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void C_PackedNormalLength::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_NormalLength;
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void C_PackedNormalLength::Serialise( T_Stream& stream ) __restrict
{
    m_NormalLength.Serialise(stream);
}



//
// --------------------------------------------------------------------------------------------------
// |-- C_Normalised : HVec3 normal + HVecW length, stored in 2 separate vectors for fast access --|
// --------------------------------------------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_Normalised::C_Normalised() : m_Normal(Constants<HVec3>::Zero(), C_Normal::UNIT_LENGTH_VECTOR), m_Length(Constants<HF32>::Zero()),
		m_Length2(Constants<HF32>::Zero()), m_RcpLength(Constants<HF32>::Zero())
{
}

/// Undefined Constructor
NOALIAS __forceinline C_Normalised::C_Normalised( const _UNDEFINED /*undefined*/ ) : m_Normal(UNDEFINED), m_Length(UNDEFINED), m_Length2(UNDEFINED), m_RcpLength(UNDEFINED)
{
}

/// Copy Constructor
NOALIAS __forceinline C_Normalised::C_Normalised( const C_Normalised& copy ) :
    m_Normal( copy.m_Normal ),
    m_Length( copy.m_Length ),
    m_Length2( copy.m_Length2 ),
    m_RcpLength( copy.m_RcpLength )
{
}

/// Copy Assignment
NOALIAS __forceinline const C_Normalised& C_Normalised::operator = ( const C_Normalised& copy )
{
    m_Normal    = copy.m_Normal;
    m_Length    = copy.m_Length;
    m_Length2   = copy.m_Length2;
    m_RcpLength = copy.m_RcpLength;
    return copy;
}

/// C_Normal + HF32 length, length^2 & 1/length Constructor
NOALIAS __forceinline C_Normalised::C_Normalised( const C_Normal PCREF normal, const HF32 PCREF length, const HF32 PCREF length2, const HF32 PCREF rcpLength ) :
    m_Normal( normal ),
    m_Length( length ),
    m_Length2( length2 ),
    m_RcpLength( rcpLength )
{
}

/// C_Normal Cast
NOALIAS __forceinline C_Normalised::operator const C_Normal () const __restrict
{
    return m_Normal;
}

/// Normal Accessor
NOALIAS __forceinline const C_Normal C_Normalised::Normal() const __restrict
{
    return m_Normal;
}

/// Length Accessor
NOALIAS __forceinline const HF32 C_Normalised::Length() const __restrict
{
    return m_Length;
}

/// Length Squared Accessor
NOALIAS __forceinline const HF32 C_Normalised::Length2() const __restrict
{
    return m_Length2;
}

/// Reciprocal Length Accessor
NOALIAS __forceinline const HF32 C_Normalised::RcpLength() const __restrict
{
    return m_RcpLength;
}

/// Read from Stream
//template < typename T_Stream > __forceinline void C_Normalised::Deserialise( T_Stream& stream ) __restrict
//{
//    C_PackedNormalLength packed;
//    packed.Deserialise( stream );
//    m_Normal = packed.Normal();
//    m_Length = packed.Length();
//}

/// Write to Stream
//template < typename T_Stream > __forceinline void C_Normalised::Serialise( T_Stream& stream ) __restrict
//{
//    const C_PackedNormalLength packed( m_Normal, m_Length );
//    packed.Serialise( stream );
//}






//
// ---------------------------------------------------------------------
// |-- Trilinear coordinates for a vertex position within a triangle --|
// ---------------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_Trilinear::C_Trilinear()
{
}

/// Copy Constructor
NOALIAS __forceinline C_Trilinear::C_Trilinear( const C_Trilinear& copy ) :
    m_V( copy.m_V )
{
}

/// Copy Assignment
NOALIAS __forceinline const C_Trilinear& C_Trilinear::operator = ( const C_Trilinear& copy )
{
    m_V = copy.m_V;
    return copy;
}

/// HVec3 Constructor
NOALIAS __forceinline C_Trilinear::C_Trilinear( const HVec3 PCREF src ) :
    m_V( src )
{
}

/// HF32 XYZ Constructor
NOALIAS __forceinline C_Trilinear::C_Trilinear( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c ) :
    m_V( a, b, c )
{
}

/// Explicit F32 Array Constructor
NOALIAS __forceinline C_Trilinear::C_Trilinear( const F32* xyz ) :
    m_V( xyz )
{
}

/// HVec3 Cast
NOALIAS __forceinline C_Trilinear::operator const HVec3 () const __restrict
{
    return m_V;
}

/// Accessor: Alpha
NOALIAS __forceinline const HF32 C_Trilinear::Alpha() const __restrict
{
    return m_V.X();
}

/// Accessor: Beta
NOALIAS __forceinline const HF32 C_Trilinear::Beta() const __restrict
{
    return m_V.Y();
}

/// Accessor: Gamma
NOALIAS __forceinline const HF32 C_Trilinear::Gamma() const __restrict
{
    return m_V.Z();
}

/// Coordinates Inside Triangle Test
NOALIAS __forceinline const Scalar_Bool C_Trilinear::Inside() const __restrict
{
    const HVec3 zero(ZERO_INIT);
    const Vector3_Bool inside_xyz( m_V >= zero );
    return X( inside_xyz ) & Y( inside_xyz ) & Z( inside_xyz );
}

/// Finite test
NOALIAS __forceinline const Scalar_Bool C_Trilinear::Finite() const __restrict
{
    return m_V.Finite();
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Trilinear::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_V;
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Trilinear::Serialise( T_Stream& stream ) __restrict
{
    m_V.Serialise(stream);
}






//
// ---------------------------------------------------------------------
// |-- Trilinear coordinates for a vertex position within a triangle --|
// ---------------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_Areal::C_Areal()
{
}

/// Copy Constructor
NOALIAS __forceinline C_Areal::C_Areal( const C_Areal& copy ) :
    m_V( copy.m_V )
{
}

/// Copy Assignment
NOALIAS __forceinline const C_Areal& C_Areal::operator = ( const C_Areal& copy )
{
    m_V = copy.m_V;
    return copy;
}

/// HVec3 Constructor
NOALIAS __forceinline C_Areal::C_Areal( const HVec3 PCREF src ) :
    m_V( src )
{
}

/// HF32 XYZ Constructor
NOALIAS __forceinline C_Areal::C_Areal( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c ) :
    m_V( a, b, c )
{
}

/// Explicit F32 Array Constructor
NOALIAS __forceinline C_Areal::C_Areal( const F32* xyz ) :
    m_V( xyz )
{
}

/// C_Trilinear & scale constructor
NOALIAS __forceinline C_Areal::C_Areal( const C_Trilinear PCREF trilinear, const HVec3 PCREF trilinearToAreal ) :
    m_V( ((HVec3)trilinear) * trilinearToAreal )
{
}

/// HVec3 Cast
NOALIAS __forceinline C_Areal::operator const HVec3 () const __restrict
{
    return m_V;
}

/// Accessor: Alpha
NOALIAS __forceinline const HF32 C_Areal::Alpha() const __restrict
{
    return m_V.X();
}

/// Accessor: Beta
NOALIAS __forceinline const HF32 C_Areal::Beta() const __restrict
{
    return m_V.Y();
}

/// Accessor: Gamma
NOALIAS __forceinline const HF32 C_Areal::Gamma() const __restrict
{
    return m_V.Z();
}

/// Finite test
NOALIAS __forceinline const Scalar_Bool C_Areal::Finite() const __restrict
{
    return m_V.Finite();
}

/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Areal::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_V;
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void C_Areal::Serialise( T_Stream& stream ) __restrict
{
    m_V.Serialise(stream);
}












//
// -------------------------
// |-- Support Functions --|
// -------------------------
//

/// Selector: Result = ( selector ? resultIfTrue : resultIfFalse )
NOALIAS __forceinline const HVec2 Select( const Vector2_Bool PCREF selector, const HVec2 PCREF resultIfTrue, const HVec2 PCREF resultIfFalse )
{
    #if defined( _SPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = spu_sel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec2 result( UNDEFINED );
    result.m_V = vec_sel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec2 result( UNDEFINED );
    result.m_V = __vsel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec2( SIMD_Select( selector.m_V, resultIfTrue.m_V, resultIfFalse.m_V ) );
    #else
    return HVec2( selector.m_X != 0x00000000 ? resultIfTrue.m_V[VX] : resultIfFalse.m_V[VX],
                  selector.m_Y != 0x00000000 ? resultIfTrue.m_V[VY] : resultIfFalse.m_V[VY] );
    #endif
}

/// Selector: Result = ( selector ? resultIfTrue : resultIfFalse )
NOALIAS __forceinline const HVec3 Select( const Vector3_Bool PCREF selector, const HVec3 PCREF resultIfTrue, const HVec3 PCREF resultIfFalse )
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_sel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_sel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vsel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec3( SIMD_Select( selector.m_V, resultIfTrue.m_V, resultIfFalse.m_V ) );
    #else
    return HVec3( selector.m_X != 0x00000000 ? resultIfTrue.m_V[VX] : resultIfFalse.m_V[VX],
                  selector.m_Y != 0x00000000 ? resultIfTrue.m_V[VY] : resultIfFalse.m_V[VY],
                  selector.m_Z != 0x00000000 ? resultIfTrue.m_V[VZ] : resultIfFalse.m_V[VZ] );
    #endif
}

/// Selector: Result = ( selector ? resultIfTrue : resultIfFalse )
NOALIAS __forceinline const HVec4 Select( const Vector4_Bool PCREF selector, const HVec4 PCREF resultIfTrue, const HVec4 PCREF resultIfFalse )
{
    #if defined( _SPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = spu_sel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec4 result( UNDEFINED );
    result.m_V = vec_sel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec4 result( UNDEFINED );
    result.m_V = __vsel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    #elif defined( _SIMD_ )
    return HVec4( SIMD_Select( selector.m_V, resultIfTrue.m_V, resultIfFalse.m_V ) );
    #else
    return HVec4( selector.m_X != 0x00000000 ? resultIfTrue.m_V[VX] : resultIfFalse.m_V[VX],
                  selector.m_Y != 0x00000000 ? resultIfTrue.m_V[VY] : resultIfFalse.m_V[VY],
                  selector.m_Z != 0x00000000 ? resultIfTrue.m_V[VZ] : resultIfFalse.m_V[VZ],
                  selector.m_W != 0x00000000 ? resultIfTrue.m_V[VW] : resultIfFalse.m_V[VW] );
    #endif
}

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
NOALIAS __forceinline const C_Vertex Select( const Vector3_Bool PCREF selector, const C_Vertex PCREF a, const C_Vertex PCREF b )
{
    #if defined( _SPU_ )
    C_Vertex result( UNDEFINED );
    result.m_V.m_V = spu_sel( b.m_V.m_V, a.m_V.m_V, selector.m_V );
    return result;
    #elif defined( _PPU_ )
    C_Vertex result( UNDEFINED );
    result.m_V.m_V = vec_sel( b.m_V.m_V, a.m_V.m_V, selector.m_V );
    return result;
    #elif defined( _VMX128_ )
    C_Vertex result( UNDEFINED );
    result.m_V.m_V = __vsel( b.m_V.m_V, a.m_V.m_V, selector.m_V );
    return result;
    #else
    return C_Vertex( Select( selector, a.m_V, b.m_V ) );
    #endif
}

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
NOALIAS __forceinline const C_Vector Select( const Vector3_Bool PCREF selector, const C_Vector PCREF a, const C_Vector PCREF b )
{
    #if defined( _SPU_ )
    C_Vector result( UNDEFINED );
    result.m_V.m_V = spu_sel( b.m_V.m_V, a.m_V.m_V, selector.m_V );
    return result;
    #elif defined( _PPU_ )
    C_Vector result( UNDEFINED );
    result.m_V.m_V = vec_sel( b.m_V.m_V, a.m_V.m_V, selector.m_V );
    return result;
    #elif defined( _VMX128_ )
    C_Vector result( UNDEFINED );
    result.m_V.m_V = __vsel( b.m_V.m_V, a.m_V.m_V, selector.m_V );
    return result;
    #else
    return C_Vector( Select( selector, a.m_V, b.m_V ) );
    #endif
}

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
NOALIAS __forceinline const C_Normal Select( const Scalar_Bool PCREF selector, const C_Normal PCREF a, const C_Normal PCREF b )
{
    #if defined( _SPU_ )
    C_Normal result( UNDEFINED );
    result.m_V.m_V = spu_sel( b.m_V.m_V, a.m_V.m_V, selector.m_V );
    return result;
    #elif defined( _PPU_ )
    C_Normal result( UNDEFINED );
    result.m_V.m_V = vec_sel( b.m_V.m_V, a.m_V.m_V, selector.m_V );
    return result;
    #elif defined( _VMX128_ )
    C_Normal result( UNDEFINED );
    result.m_V.m_V = __vsel( b.m_V.m_V, a.m_V.m_V, selector.m_V );
    return result;
    #else
    return C_Normal( Select( selector, a.m_V, b.m_V ), C_Normal::UNIT_LENGTH_VECTOR );
    #endif
}




/// Refelct vector about normal
NOALIAS __forceinline const HVec3 Reflect( const HVec3 PCREF vector, const C_Normal PCREF normal )
{
    #if defined( _SPU_ )
    
    const vector float v( vector.m_V );
    const vector float n( normal.m_V );
    const vector float dp( spu_dot_product3( v, n ) );
    const vector float nx2( spu_add( n, n ) );
    HVec3 result( UNDEFINED );
    result.m_V = spu_nmsub( nx2, dp, v );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float v( vector.m_V );
    const vector float n( normal.m_V );
    const vector float dp( vec_dot_product3( v, n ) );
    const vector float nx2( vec_add( n, n ) );
    HVec3 result( UNDEFINED );
    result.m_V = vec_nmsub( nx2, dp, v );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 v( vector.m_V );
    const __vector4 n( normal.m_V );
    const __vector4 dp( __vmsum3fp( v, n ) );
    const __vector4 nx2( __vaddfp( n, n ) );
    HVec3 result( UNDEFINED );
    result.m_V = __vnmsubfp( nx2, dp, v );
    return result;
    
    #else
    
    const HF32 two( c_ScalarTwo );
    return vector - ( normal * two * ( vector | normal ) );
    
    #endif
}





/// Returns the average of the elements of a Vector
template < class T_Vector >
NOALIAS __forceinline const HF32 Average( const T_Vector PCREF src )
{
    return src.Sum() * ( 1.0f / T_Vector::SIZE );
}

/// Returns the length of a Vector
template < class T_Vector >
NOALIAS __forceinline const HF32 Length( const T_Vector PCREF src )
{
    return src.Length();
}

/// Returns the squared length of a Vector
template < class T_Vector >
NOALIAS __forceinline const HF32 Length2( const T_Vector PCREF src )
{
    return Maths::LengthSquared( src );
}

/// Returns the squared length of a Vector
//template < class T_Vector >
//FORCE_INLINE
//const HF32 LengthSquared( const T_Vector& src )
//{
//    return Maths::LengthSquared( src );
//}

/// Returns the reciprocal of the length of a Vector
template < class T_Vector >
NOALIAS __forceinline const HF32 RcpLength( const T_Vector PCREF src )
{
    return Maths::ReciprocalLength( src );
}

/// Returns the reciprocal of the length of a Vector
//template < class T_Vector >
//FORCE_INLINE
//const HF32 ReciprocalLength( const T_Vector& src )
//{
//    return Maths::ReciprocalLength( src );
//}

/// Returns a normalised vector
template < class T_Vector >
NOALIAS __forceinline const C_Normal Normalise( const T_Vector PCREF src )
{
    return src.Normalised().Normal();
}

/// Returns a normalised vector - safe version with additional parameter to use for zero length vectors!
template < class T_Vector >
NOALIAS __forceinline const C_Normal SafeNormalise( const T_Vector PCREF src, const C_Normal PCREF zeroLengthResult )
{
    return src.SafeNormalised( zeroLengthResult );
}

/// Returns the dot product of two vectors
template < class T_Vector >
NOALIAS __forceinline const HF32 DotProduct( const T_Vector PCREF lhs, const T_Vector PCREF rhs )
{
    return lhs | rhs;
}

/// Returns the cross product of two vectors
template < class T_Vector >
NOALIAS __forceinline const typename T_Vector::CrossProductType CrossProduct( const T_Vector PCREF lhs, const T_Vector PCREF rhs )
{
    return lhs ^ rhs;
}

/// Shuffle
//template < unsigned SX, unsigned SY, unsigned SZ, class T_Vector3 >
//FORCE_INLINE
//const T_Vector3 Shuffle( const T_Vector3& src )
//{
//    return src.Shuffle<SX,SY,SZ>();
//}

/// Horizontal sum of all elements
template < class T_Vector >
NOALIAS __forceinline const HF32 Sum( const T_Vector PCREF src )
{
    return src.Sum();
}

/// Box Half Surface Area : Vector represents dimensions of box
//template < class T_Vector3 >
//FORCE_INLINE
//const HF32 BoxHalfSurfaceArea( const T_Vector3& dimensionsXYZ )
//{
//    const T_Vector3 dimensionsYZX( Shuffle<VY,VZ,VX>( dimensionsXYZ ) );
//    // This generates: warning LNK4221: no public symbols found; archive member will be inaccessible. Why?!?
//    //const HF32 halfSurfaceArea( Sum( dimensionsXYZ * dimensionsYZX ) );
//    //return halfSurfaceArea;
//    return ( dimensionsXYZ * dimensionsYZX ).Sum();
//}
NOALIAS __forceinline const HF32 BoxHalfSurfaceArea( const HVec3 PCREF dimensionsXYZ )
{
    const HVec3 dimensionsYZX( Maths::Shuffle<VY,VZ,VX>( dimensionsXYZ ) );
    // This generates: warning LNK4221: no public symbols found; archive member will be inaccessible. Why?!?
    //const HF32 halfSurfaceArea( Sum( dimensionsXYZ * dimensionsYZX ) );
    //return halfSurfaceArea;
    return ( dimensionsXYZ * dimensionsYZX ).Sum();
}

/// Box Surface Area : Vector represents dimensions of box
template < class T_Vector3 >
NOALIAS __forceinline const HF32 BoxSurfaceArea( const T_Vector3 PCREF dimensionsXYZ )
{
    const HF32 halfSurfaceArea( BoxHalfSurfaceArea( dimensionsXYZ ) );
    return halfSurfaceArea + halfSurfaceArea;
}

/// Returns the bitwise AND of the two inputs: lhs & rhs
template < class T_Vector >
NOALIAS __forceinline const T_Vector AND( const T_Vector PCREF lhs, const T_Vector PCREF rhs )
{
    return lhs.AND( rhs );
}

/// Returns the bitwise AND-NOT of the two inputs: lhs & ~rhs
template < class T_Vector >
NOALIAS __forceinline const T_Vector ANDNOT( const T_Vector PCREF lhs, const T_Vector PCREF rhs )
{
    return lhs.ANDNOT( rhs );
}

/// Returns the bitwise OR of the two inputs: lhs | rhs
template < class T_Vector >
NOALIAS __forceinline const T_Vector OR( const T_Vector PCREF lhs, const T_Vector PCREF rhs )
{
    return lhs.OR( rhs );
}

/// Returns the bitwise XOR of the two inputs: lhs ^ rhs
template < class T_Vector >
NOALIAS __forceinline const T_Vector XOR( const T_Vector PCREF lhs, const T_Vector PCREF rhs )
{
    return lhs.XOR( rhs );
}


/// Returns the transpose of the matrix
template < class T_Matrix >
NOALIAS __forceinline const T_Matrix Transpose( const T_Matrix& src )
{
    return src.Transposed();
}

/// Returns the inverse of the matrix
template < class T_Matrix >
NOALIAS __forceinline const T_Matrix Inverse( const T_Matrix& src )
{
    return src.Inverse();
}

/// Returns the transpose of the inverse of a matrix ( for transforming normals ).
/// INFO: http://www.gignews.com/realtime020100.htm
template < class T_Matrix >
NOALIAS __forceinline const T_Matrix TransposeInverse( const T_Matrix& src )
{
    return Transpose( Inverse( src ) );
}







// Matrix multiplication of a column vector (lhs) by a row vector (rhs).
NOALIAS __forceinline const HMat33 TensorProduct( const HVec3 PCREF columnVector, const HVec3 PCREF rowVector )
{
    return HMat33(
        columnVector.X() * rowVector,
        columnVector.Y() * rowVector,
        columnVector.Z() * rowVector );
}












//
// ---------------------------------------------------------------------------
// |-- HVec3_DelayedOp : Pair of HVec3's to be multiplied/added/subtracted --|
// |-- Used for collapsing a multiply & add into a multiply-add etc        --|
// ---------------------------------------------------------------------------
//

/// Accessor: Get reference to sub-type
template < typename T_Op >
NOALIAS __forceinline T_Op& HVec3_DelayedOp< T_Op >::SubType()
{
    return (T_Op&)*this;
}

/// Accessor: Get const reference to sub-type
template < typename T_Op >
NOALIAS __forceinline const T_Op& HVec3_DelayedOp< T_Op >::SubType() const
{
    return (T_Op&)*this;
}

/// Accessor: Get X
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::X() const
{
    return HVec3( SubType() ).X();
}

/// Accessor: Get Y
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::Y() const
{
    return HVec3( SubType() ).Y();
}

/// Accessor: Get Z
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::Z() const
{
    return HVec3( SubType() ).Z();
}

/// Accessor: Get W - Required for templated accessors - should never be used!
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::W() const
{
    return HVec3( SubType() ).W();
}

//#if defined( _SIMD_ )
///// Accessor: Get W
//template < typename T_Op >
//FORCE_INLINE
//HF32 HVec3_DelayedOp< T_Op >::TestW() const
//{
//    return HVec3( SubType() ).TestW();
//}
//#endif

/// Shuffle
template < typename T_Op >
template < unsigned IX, unsigned IY, unsigned IZ >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::Shuffle() const
{
    return HVec3( SubType() ).Shuffle<IX,IY,IZ>();
}

/// Returns copy with zeroed element X
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::ZeroedX() const
{
    //return HVec3( SubType() ).ZeroedX();
    return ZeroX( HVec3( SubType() ) );
}

/// Returns copy with zeroed element Y
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::ZeroedY() const
{
    //return HVec3( SubType() ).ZeroedY();
    return ZeroY( HVec3( SubType() ) );
}

/// Returns copy with zeroed element Z
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::ZeroedZ() const
{
    //return HVec3( SubType() ).ZeroedZ();
    return ZeroZ( HVec3( SubType() ) );
}

/// Returns copy with zeroed elements XY
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::ZeroedXY() const
{
    //return HVec3( SubType() ).ZeroedXY();
    return ZeroXY( HVec3( SubType() ) );
}

/// Returns copy with zeroed elements XZ
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::ZeroedXZ() const
{
    //return HVec3( SubType() ).ZeroedXZ();
    return ZeroXZ( HVec3( SubType() ) );
}

/// Returns copy with zeroed elements YZ
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::ZeroedYZ() const
{
    //return HVec3( SubType() ).ZeroedYZ();
    return ZeroYZ( HVec3( SubType() ) );
}

/// Unary Negation Operator
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator - () const
{
    return -HVec3( SubType() );
}

/// Binary Addition Operator
template < typename T_Op >
NOALIAS __forceinline HVec3_Add HVec3_DelayedOp< T_Op >::operator + ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) + src;
}

/// Binary Subtraction Operator
template < typename T_Op >
NOALIAS __forceinline HVec3_Subtract HVec3_DelayedOp< T_Op >::operator - ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) - src;
}

/// Binary Multiplication Operator
template < typename T_Op >
NOALIAS __forceinline HVec3_Multiply HVec3_DelayedOp< T_Op >::operator * ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) * src;
}

/// Binary Division Operator
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator / ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) / src;
}

/// HF32 Multiplication Operator
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator * ( const HF32 PCREF src ) const
{
    return HVec3( SubType() ) * src;
}

/// HF32 Division Operator
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator / ( const HF32 PCREF src ) const
{
    return HVec3( SubType() ) / src;
}

/// HVecX Multiply Operator
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator * ( const HVecX PCREF src ) const
{
    return HVec3( SubType() ) * src;
}

/// HVecY Multiply Operator
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator * ( const HVecY PCREF src ) const
{
    return HVec3( SubType() ) * src;
}

/// HVecZ Multiply Operators
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator * ( const HVecZ PCREF src ) const
{
    return HVec3( SubType() ) * src;
}

/// HVecW Multiply Operators
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator * ( const HVecW PCREF src ) const
{
    return HVec3( SubType() ) * HF32( src );
}

/// Min
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::Min( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ).Min( src );
}

/// Max
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::Max( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ).Max( src );
}

/// Reciprocal of elements
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::Reciprocal() const
{
    return Maths::Reciprocal( HVec3( SubType() ) );
}

/// Horizontal sum of all elements
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::Sum() const
{
    return Maths::Sum( HVec3( SubType() ) );
}

/// Vector Length
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::Length() const
{
    return Maths::Length( HVec3( SubType() ) );
}

/// Vector Length Squared
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::Length2() const
{
    return Maths::LengthSquared( HVec3( SubType() ) );
}

/// Vector Length Squared
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::LengthSquared() const
{
    return Maths::LengthSquared( HVec3( SubType() ) );
}

/// Vector Reciprocal of Length
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::RcpLength() const
{
    return Maths::ReciprocalLength( HVec3( SubType() ) );
}

/// Vector Reciprocal of Length
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::ReciprocalLength() const
{
    return Maths::ReciprocalLength( HVec3( SubType() ) );
}

/// Dot product
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::operator | ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) | src;
}

/// Cross Product
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator ^ ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) ^ src;
}

/// Star function = Cross Product Matrix ( a* Matrix )
template < typename T_Op >
NOALIAS __forceinline HMat33 HVec3_DelayedOp< T_Op >::Star() const
{
    return HVec3( SubType() ).Star();
}

/// HMat33 Multiplication
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator * ( const HMat33& m ) const
{
    return HVec3( SubType() ) * m;
}

/// HMat43 Multiplication
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator * ( const HMat43& m ) const
{
    return HVec3( SubType() ) * m;
}

/// HMat44 Multiplication
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator * ( const HMat44& m ) const
{
    return HVec3( SubType() ) * m;
}

/// Quaternion Multiplication
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::operator * ( const C_Quaternion PCREF q ) const
{
    return HVec3( SubType() ) * q;
}

/// Vertex Distance
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::Distance( const HVec3 PCREF v ) const
{
    return HVec3( SubType() ).Distance( v );
}

/// Vertex Distance Squared
template < typename T_Op >
NOALIAS __forceinline HF32 HVec3_DelayedOp< T_Op >::DistanceSquared( const HVec3 PCREF v ) const
{
    return HVec3( SubType() ).DistanceSquared( v );
}

/// Lerp
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::Lerp( const HVec3 PCREF dst, const HF32 PCREF f ) const
{
    return HVec3( SubType() ).Lerp( dst, f );
}

/// Vector Normalised
template < typename T_Op >
NOALIAS __forceinline C_Normalised HVec3_DelayedOp< T_Op >::Normalised() const
{
    return HVec3( SubType() ).Normalised();
}

/// Vector Normalised - safe version with additional parameter to use for zero length vectors!
template < typename T_Op >
NOALIAS __forceinline C_Normalised HVec3_DelayedOp< T_Op >::SafeNormalised( const HVec3 PCREF zeroLengthResult ) const
{
    return HVec3( SubType() ).SafeNormalised( zeroLengthResult );
}

/// Get Vector of Magnitude
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::VectorOfMagnitude( const HF32 PCREF mag ) const
{
    return HVec3( SubType() ).VectorOfMagnitude( mag );
}

/// Get component of this vector parallel to input normal
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::Parallel( const C_Normal PCREF normal ) const
{
    return HVec3( SubType() ).Parallel( normal );
}

/// Get component of this vector perpendicular to input normal
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::Perpendicular( const C_Normal PCREF normal ) const
{
    return HVec3( SubType() ).Perpendicular( normal );
}

/// Get components of this vector parallel & perpendicular to input normal
template < typename T_Op >
NOALIAS __forceinline void HVec3_DelayedOp< T_Op >::ParaPerp( const C_Normal PCREF normal, HVec3& result_para, HVec3& result_perp ) const
{
    HVec3( SubType() ).ParaPerp( normal, result_para, result_perp );
}

/// Get axes perpendicular to vector.
template < typename T_Op >
NOALIAS __forceinline void HVec3_DelayedOp< T_Op >::PerpendicularAxes( HVec3& result_u, HVec3& result_v ) const
{
    HVec3( SubType() ).PerpendicularAxes( result_u, result_v );
}

/// Shuffle HVec3:XZ into a HVec2:XY
template < typename T_Op >
NOALIAS __forceinline HVec2 HVec3_DelayedOp< T_Op >::ShuffleXZtoXY() const
{
    return HVec3( SubType() ).ShuffleXZtoXY();
}

/// Comparison Operator ==
template < typename T_Op >
NOALIAS __forceinline Vector3_Bool HVec3_DelayedOp< T_Op >::operator == ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) == src;
}

/// Comparison Operator !=
template < typename T_Op >
NOALIAS __forceinline Vector3_Bool HVec3_DelayedOp< T_Op >::operator != ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) != src;
}

/// Comparison Operator <=
template < typename T_Op >
NOALIAS __forceinline Vector3_Bool HVec3_DelayedOp< T_Op >::operator <= ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) <= src;
}

/// Comparison Operator <
template < typename T_Op >
NOALIAS __forceinline Vector3_Bool HVec3_DelayedOp< T_Op >::operator < ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) < src;
}

/// Comparison Operator >=
template < typename T_Op >
NOALIAS __forceinline Vector3_Bool HVec3_DelayedOp< T_Op >::operator >= ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) >= src;
}

/// Comparison Operator >
template < typename T_Op >
NOALIAS __forceinline Vector3_Bool HVec3_DelayedOp< T_Op >::operator > ( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ) > src;
}

/// Bitwise AND: THIS & SRC
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::AND( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ).AND( src );
}

/// Bitwise AND-NOT: THIS & ~SRC
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::ANDNOT( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ).ANDNOT( src );
}

/// Bitwise OR: THIS | SRC
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::OR( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ).OR( src );
}

/// Bitwise XOR: THIS ^ SRC
template < typename T_Op >
NOALIAS __forceinline HVec3 HVec3_DelayedOp< T_Op >::XOR( const HVec3 PCREF src ) const
{
    return HVec3( SubType() ).XOR( src );
}


//
// --------------------------------------------------------------------
// |-- HVec3_Multiply : Pair of HVec3's to be multiplied            --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline HVec3_Multiply::HVec3_Multiply()
{
}

/// HVec3 Pair Constructor
NOALIAS __forceinline HVec3_Multiply::HVec3_Multiply( const HVec3 PCREF a, const HVec3 PCREF b )
    : m_A( a ),
      m_B( b )
{
}

/// Copy Constructor
NOALIAS __forceinline HVec3_Multiply::HVec3_Multiply( const HVec3_Multiply& copy )
    : m_A( copy.m_A ),
      m_B( copy.m_B )
{
}

/// Copy Assignment
NOALIAS __forceinline const HVec3_Multiply& HVec3_Multiply::operator = ( const HVec3_Multiply& copy )
{
    m_A = copy.m_A;
    m_B = copy.m_B;
    return copy;
}


//
// --------------------------------------------------------------------
// |-- HVec3_Add : Pair of HVec3's to be added                      --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline HVec3_Add::HVec3_Add()
{
}

/// HVec3 Pair Constructor
NOALIAS __forceinline HVec3_Add::HVec3_Add( const HVec3 PCREF a, const HVec3 PCREF b )
    : m_A( a ),
      m_B( b )
{
}

/// Copy Constructor
NOALIAS __forceinline HVec3_Add::HVec3_Add( const HVec3_Add& copy )
    : m_A( copy.m_A ),
      m_B( copy.m_B )
{
}

/// Copy Assignment
NOALIAS __forceinline const HVec3_Add& HVec3_Add::operator = ( const HVec3_Add& copy )
{
    m_A = copy.m_A;
    m_B = copy.m_B;
    return copy;
}


//
// --------------------------------------------------------------------
// |-- HVec3_Subtract : Pair of HVec3's to be subtracted            --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline HVec3_Subtract::HVec3_Subtract()
{
}

/// HVec3 Pair Constructor
NOALIAS __forceinline HVec3_Subtract::HVec3_Subtract( const HVec3 PCREF a, const HVec3 PCREF b )
    : m_A( a ),
      m_B( b )
{
}

/// Copy Constructor
NOALIAS __forceinline HVec3_Subtract::HVec3_Subtract( const HVec3_Subtract& copy )
    : m_A( copy.m_A ),
      m_B( copy.m_B )
{
}

/// Copy Assignment
NOALIAS __forceinline const HVec3_Subtract& HVec3_Subtract::operator = ( const HVec3_Subtract& copy )
{
    m_A = copy.m_A;
    m_B = copy.m_B;
    return copy;
}


// --------------------------------------------------------------------
// |-- HVec3_Multiply/Add/Subtract operators                        --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------

/// HVec3 Multiply-Add operator: a * b + c
NOALIAS __forceinline HVec3 operator + ( const HVec3_Multiply& ab, const HVec3 PCREF c )
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_madd( ab.m_A, ab.m_B, c.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_madd( ab.m_A, ab.m_B, c.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vmaddfp( ab.m_A, ab.m_B, c.m_V );
    return result;
    #else
    return HVec3( ab ) + c;
    #endif
}

/// HVec3 Add-Multiply operator: a + b * c
NOALIAS __forceinline HVec3 operator + ( const HVec3 PCREF a, const HVec3_Multiply& bc )
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_madd( bc.m_A, bc.m_B, a.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_madd( bc.m_A, bc.m_B, a.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vmaddfp( bc.m_A, bc.m_B, a.m_V );
    return result;
    #else
    return a + HVec3( bc );
    #endif
}

/// HVec3 Multiply-Subtract operator: a * b - c
NOALIAS __forceinline HVec3 operator - ( const HVec3_Multiply& ab, const HVec3 PCREF c )
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_msub( ab.m_A, ab.m_B, c.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_nmsub( ab.m_A, ab.m_B, c.m_V );
    return -(result);
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vnmsubfp( ab.m_A, ab.m_B, c.m_V );
    return -(result);
    #else
    return HVec3( ab ) - c;
    #endif
}

/// HVec3 Subtract-Multiply operator: a - b * c
NOALIAS __forceinline HVec3 operator - ( const HVec3 PCREF a, const HVec3_Multiply& bc )
{
    #if defined( _SPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = spu_nmsub( bc.m_A, bc.m_B, a.m_V );
    return result;
    #elif defined( _PPU_ )
    HVec3 result( UNDEFINED );
    result.m_V = vec_nmsub( bc.m_A, bc.m_B, a.m_V );
    return result;
    #elif defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vnmsubfp( bc.m_A, bc.m_B, a.m_V );
    return result;
    #else
    return a - HVec3( bc );
    #endif
}

/// HVec3 operator: ( a * b ) + ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator + ( const HVec3_Multiply& ab, const HVec3_Multiply& cd )
{
    return ab + HVec3( cd );
}

/// HVec3 operator: ( a * b ) - ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator - ( const HVec3_Multiply& ab, const HVec3_Multiply& cd )
{
    return ab - HVec3( cd );
}

/// HVec3 ( a + b ) + ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator + ( const HVec3_Add& ab, const HVec3_Multiply& cd )
{
    return HVec3( ab ) + cd;
}

/// HVec3 ( a - b ) + ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator + ( const HVec3_Subtract& ab, const HVec3_Multiply& cd )
{
    return HVec3( ab ) + cd;
}

/// HVec3 ( a + b ) - ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator - ( const HVec3_Add& ab, const HVec3_Multiply& cd )
{
    return HVec3( ab ) - cd;
}

/// HVec3 ( a - b ) - ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator - ( const HVec3_Subtract& ab, const HVec3_Multiply& cd )
{
    return HVec3( ab ) - cd;
}



NOALIAS __forceinline HVec3 operator * ( const HVec3_Multiply& v, const HF32 PCREF f )     { return v * HVec3( f ); }
NOALIAS __forceinline HVec3 operator / ( const HVec3_Multiply& v, const HF32 PCREF f )     { return v / HVec3( f ); }

NOALIAS __forceinline HVec3 operator * ( const HVec3_Add& v, const HF32 PCREF f )          { return v * HVec3( f ); }
NOALIAS __forceinline HVec3 operator / ( const HVec3_Add& v, const HF32 PCREF f )          { return v / HVec3( f ); }

NOALIAS __forceinline HVec3 operator * ( const HVec3_Subtract& v, const HF32 PCREF f )     { return v * HVec3( f ); }
NOALIAS __forceinline HVec3 operator / ( const HVec3_Subtract& v, const HF32 PCREF f )     { return v / HVec3( f ); }



NOALIAS __forceinline HVec3 operator * ( const HVec3_Multiply& v, const F32& f )     { return v * HVec3( HF32( f ) ); }
NOALIAS __forceinline HVec3 operator / ( const HVec3_Multiply& v, const F32& f )     { return v / HVec3( HF32( f ) ); }

NOALIAS __forceinline HVec3 operator * ( const HVec3_Add& v, const F32& f )          { return v * HVec3( HF32( f ) ); }
NOALIAS __forceinline HVec3 operator / ( const HVec3_Add& v, const F32& f )          { return v / HVec3( HF32( f ) ); }

NOALIAS __forceinline HVec3 operator * ( const HVec3_Subtract& v, const F32& f )     { return v * HVec3( HF32( f ) ); }
NOALIAS __forceinline HVec3 operator / ( const HVec3_Subtract& v, const F32& f )     { return v / HVec3( HF32( f ) ); }












//
// ---------------------------------------------------------------------------------
// |-- C_Vertex_DelayedOp : Pair of C_Vertex's to be multiplied/added/subtracted --|
// |-- Used for collapsing a multiply & add into a multiply-add etc              --|
// ---------------------------------------------------------------------------------
//

/// Accessor: Get reference to sub-type
template < typename T_Op >
NOALIAS __forceinline T_Op& C_Vertex_DelayedOp< T_Op >::SubType()
{
    return (T_Op&)*this;
}

/// Accessor: Get const reference to sub-type
template < typename T_Op >
NOALIAS __forceinline const T_Op& C_Vertex_DelayedOp< T_Op >::SubType() const
{
    return (T_Op&)*this;
}


//
// --------------------------------------------------------------------
// |-- C_Vertex_Multiply : Pair of C_Vertex's to be multiplied      --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

/// C_Vertex Pair Constructor
/*NOALIAS __forceinline C_Vertex_Multiply::C_Vertex_Multiply( const C_Vertex PCREF a, const C_Vertex PCREF b )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_A( a.m_V.m_V ),
      m_B( b.m_V.m_V )
    #elif defined( _VMX128_ )
    : m_A( a.m_V.m_V ),
      m_B( b.m_V.m_V )
    #else
    : m_A( a ),
      m_B( b )
    #endif
{
}

/// HVec3_Multiply Constructor
NOALIAS __forceinline C_Vertex_Multiply::C_Vertex_Multiply( const HVec3_Multiply& src )
    : m_A( src.m_A ),
      m_B( src.m_B )
{
}

/// C_Vertex Cast operator
NOALIAS __forceinline C_Vertex_Multiply::operator C_Vertex () const
{
    C_Vertex result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V.m_V = spu_mul( m_A, m_B );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_mul( m_A, m_B );
    #elif defined( _VMX128_ )
    result.m_V.m_V = __vmulfp( m_A, m_B );
    #else
    result = C_Vertex( m_A.m_V * m_B.m_V );
    #endif
    return result;
}


//
// --------------------------------------------------------------------
// |-- C_Vertex_Add : Pair of C_Vertex's to be added                --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

/// C_Vertex Pair Constructor
NOALIAS __forceinline C_Vertex_Add::C_Vertex_Add( const C_Vertex PCREF a, const C_Vertex PCREF b )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_A( a.m_V.m_V ),
      m_B( b.m_V.m_V )
    #elif defined( _VMX128_ )
    : m_A( a.m_V.m_V ),
      m_B( b.m_V.m_V )
    #else
    : m_A( a ),
      m_B( b )
    #endif
{
}

/// HVec3_Add Constructor
NOALIAS __forceinline C_Vertex_Add::C_Vertex_Add( const HVec3_Add& src )
    : m_A( src.m_A ),
      m_B( src.m_B )
{
}

/// C_Vertex Cast operator
NOALIAS __forceinline C_Vertex_Add::operator C_Vertex () const
{
    C_Vertex result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V.m_V = spu_add( m_A, m_B );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_add( m_A, m_B );
    #elif defined( _VMX128_ )
    result.m_V.m_V = __vaddfp( m_A, m_B );
    #else
    result = C_Vertex( m_A.m_V + m_B.m_V );
    #endif
    return result;
}*/


//
// --------------------------------------------------------------------
// |-- C_Vertex_Subtract : Pair of C_Vertex's to be subtracted      --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

/// C_Vertex Pair Constructor
NOALIAS __forceinline C_Vertex_Subtract::C_Vertex_Subtract( const C_Vertex PCREF a, const C_Vertex PCREF b )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_A( a.m_V.m_V ),
      m_B( b.m_V.m_V )
    #elif defined( _VMX128_ )
    : m_A( a.m_V.m_V ),
      m_B( b.m_V.m_V )
    #else
    : m_A( a ),
      m_B( b )
    #endif
{
}

/// HVec3_Subtract Constructor
NOALIAS __forceinline C_Vertex_Subtract::C_Vertex_Subtract( const HVec3_Subtract& src )
    : m_A( src.m_A ),
      m_B( src.m_B )
{
}

/// C_Vertex Cast operator
NOALIAS __forceinline C_Vertex_Subtract::operator C_Vertex () const
{
    C_Vertex result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V.m_V = spu_sub( m_A, m_B );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_sub( m_A, m_B );
    #elif defined( _VMX128_ )
    result.m_V.m_V = __vsubfp( m_A, m_B );
    #else
    result = C_Vertex( m_A.m_V - m_B.m_V );
    #endif
    return result;
}


//__forceinline HVec4&
//operator*=(HVec4& a, const V128 PCREF b) {
//	a.SetV128(SIMD_Multiply(a.GetV128(), b));
//	return	a;
//}

//__forceinline HVec3&
//operator*=(HVec3& a, const V128 PCREF b) {
//	a.SetV128(SIMD_Multiply(a.GetV128(), b));
//	return	a;
//}








//
// VECTOR LENGTH FUNCTIONS
//

/// Calculate normal, length, length^2 & 1/length of vector
NOALIAS __forceinline void NormalLength(
    const HVec3 PCREF   in_vector,
    C_Normal&           out_normal,
    HF32&               out_length,
    HF32&               out_length_squared,
    HF32&               out_rcp_length )
{
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float v( in_vector.m_V );
    const vector float len_squared( spu_length_squared3( v ) );
    vector float rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( len_squared, zero, rcp_len, valid );
    out_normal.m_V.m_V     = spu_mul( v, rcp_len );
    out_length.m_F         = spu_mul( len_squared, rcp_len );
    out_length_squared.m_F = len_squared;
    out_rcp_length.m_F     = rcp_len;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float v( in_vector.m_V );
    const vector float len_squared( vec_length_squared3( v ) );
    vector float rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( len_squared, zero, rcp_len, valid );
    out_normal.m_V.m_V     = vec_mul( v, rcp_len );
    out_length.m_F         = vec_mul( len_squared, rcp_len );
    out_length_squared.m_F = len_squared;
    out_rcp_length.m_F     = rcp_len;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( in_vector.m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 normal( __vmulfp( vector, vector_rcp_length ) );
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_rcp_length ) );
    
    out_normal.m_V.m_V     = normal;
    out_length.m_F         = vector_length;
    out_length_squared.m_F = vector_length_squared;
    out_rcp_length.m_F     = vector_rcp_length;
    
    #else
    
    const HF32     zero;
    const HF32     length2( in_vector | in_vector );
    const HF32     rcp_length( ReciprocalSqrt( length2 ) );
    const HF32     length( Select( rcp_length.Finite(), length2 * rcp_length, zero ) );
    const C_Normal normal( in_vector * rcp_length, C_Normal::UNIT_LENGTH_VECTOR );
    
    out_normal         = normal;
    out_length         = length;
    out_length_squared = length2;
    out_rcp_length     = rcp_length;
    
    #endif
}


NOALIAS __forceinline void VecSplatXYZ(const HVec4 PCREF srcIn, HVec4& xout,HVec4& yout, HVec4& zout) 
{
#if defined (_SIMD_)	
	xout.m_V=SIMD_Splat4<VX>(srcIn.GetV128());
	yout.m_V=SIMD_Splat4<VY>(srcIn.GetV128());
	zout.m_V=SIMD_Splat4<VZ>(srcIn.GetV128());
#else
	xout.SetAllComponents(srcIn.m_V[VX]);
	yout.SetAllComponents(srcIn.m_V[VY]);
	zout.SetAllComponents(srcIn.m_V[VZ]);
#endif
}

NOALIAS __forceinline void VecTranspose(
		HVec4& dstAX,
		HVec4& dstAY,
		HVec4& dstAZ,
		HVec4& dstAW,
		const HVec4 PCREF srcAX,
		const HVec4 PCREF srcAY,
		const HVec4 PCREF srcAZ,
		const HVec4 PCREF srcAW)
{
	#if defined (_SIMD_)	
		// Hope Mr Compiler can make it's necessary assumptions...
		SIMD_Transpose(
			dstAX.m_V,dstAY.m_V,dstAZ.m_V,dstAW.m_V,
			srcAX.GetV128(),
			srcAY.GetV128(),
			srcAZ.GetV128(),
			srcAW.GetV128()
		);
	#else
#define COPYCOMPONENTS(AXIS) \
		dstA ## AXIS .SetComponents(	srcAX.m_V[ V ## AXIS ], \
						srcAY.m_V[ V ## AXIS ], \
						srcAZ.m_V[ V ## AXIS ], \
						srcAW.m_V[ V ## AXIS ]);
	COPYCOMPONENTS(X)
	COPYCOMPONENTS(Y)
	COPYCOMPONENTS(Z)
	COPYCOMPONENTS(W)
#undef COPYCOMPONENT

	#endif
};







}; // namespace Maths



#endif
