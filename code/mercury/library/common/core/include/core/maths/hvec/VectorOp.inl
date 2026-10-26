#ifndef MATHS_HVEC_VECTOR_OP_INL
#define MATHS_HVEC_VECTOR_OP_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

//
// +-------------------------------------+
// | *** HVec3 Operators & Functions *** |
// +-------------------------------------+
//

/// HVec3 Get X
NOALIAS __forceinline const HF32 X( const HVec3 PCREF v )
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_splat<VX>( v.m_V );
    #elif defined( _PPU_ )
    result.m_F = vec_splat( v.m_V, VX );
    #elif defined( _VMX128_ )
    result.m_F = __vspltw( v.m_V, VX );
    #elif defined( _SIMD_ )
    result.m_F = SIMD_Scalar_Vector3X( v.m_V );
    #else
    result = HF32( v.m_V[VX] );
    #endif
    return result;
}

/// HVec3 Get Y
NOALIAS __forceinline const HF32 Y( const HVec3 PCREF v )
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_splat<VY>( v.m_V );
    #elif defined( _PPU_ )
    result.m_F = vec_splat( v.m_V, VY );
    #elif defined( _VMX128_ )
    result.m_F = __vspltw( v.m_V, VY );
    #elif defined( _SIMD_ )
    result.m_F = SIMD_Scalar_Vector3Y( v.m_V );
    #else
    result = HF32( v.m_V[VY] );
    #endif
    return result;
}

/// HVec3 Get Z
NOALIAS __forceinline const HF32 Z( const HVec3 PCREF v )
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_splat<VZ>( v.m_V );
    #elif defined( _PPU_ )
    result.m_F = vec_splat( v.m_V, VZ );
    #elif defined( _VMX128_ )
    result.m_F = __vspltw( v.m_V, VZ );
    #elif defined( _SIMD_ )
    result.m_F = SIMD_Scalar_Vector3Z( v.m_V );
    #else
    result = HF32( v.m_V[VZ] );
    #endif
    return result;
}

/// HVec3 Set X
NOALIAS __forceinline const HVec3 SetX( const HVec3 PCREF v, const HF32 PCREF x )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_insert( spu_extract( x.m_F, VX ), v.m_V, VX );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V1X,V0Y,V0Z,V0W>( v.m_V, x.m_F );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v.m_V, x.m_F, VMX_MASK_INSERT_X, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_SetX( v.m_V, x.m_F );
    #else
    result = HVec3( x.m_F, v.m_V[VY], v.m_V[VZ] );
    #endif
    return result;
}

/// HVec3 Set Y
NOALIAS __forceinline const HVec3 SetY( const HVec3 PCREF v, const HF32 PCREF y )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_insert( spu_extract( y.m_F, VY ), v.m_V, VY );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V0X,V1Y,V0Z,V0W>( v.m_V, y.m_F );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v.m_V, y.m_F, VMX_MASK_INSERT_Y, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_SetY( v.m_V, y.m_F );
    #else
    result = HVec3( v.m_V[VX], y.m_F, v.m_V[VZ] );
    #endif
    return result;
}

/// HVec3 Set Z
NOALIAS __forceinline const HVec3 SetZ( const HVec3 PCREF v, const HF32 PCREF z )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_insert( spu_extract( z.m_F, VZ ), v.m_V, VZ );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V0X,V0Y,V1Z,V0W>( v.m_V, z.m_F );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v.m_V, z.m_F, VMX_MASK_INSERT_Z, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_SetZ( v.m_V, z.m_F );
    #else
    result = HVec3( v.m_V[VX], v.m_V[VY], z.m_F );
    #endif
    return result;
}

/// HVec3 Zero element X
NOALIAS __forceinline const HVec3 ZeroX( const HVec3 PCREF v )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_insert( 0.0f, v.m_V, VX );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V1X,V0Y,V0Z,V1W>( v.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v.m_V, __vzero(), VMX_MASK_INSERT_X, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Mask<0,1,1,0>( v.m_V );
    #else
    result = HVec3( 0.0f, v.m_V[VY], v.m_V[VZ] );
    #endif
    return result;
}

/// HVec3 Zero element Y
NOALIAS __forceinline const HVec3 ZeroY( const HVec3 PCREF v )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_insert( 0.0f, v.m_V, VY );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V0X,V1Y,V0Z,V1W>( v.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v.m_V, __vzero(), VMX_MASK_INSERT_Y, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Mask<1,0,1,0>( v.m_V );
    #else
    result = HVec3( v.m_V[VX], 0.0f, v.m_V[VZ] );
    #endif
    return result;
}

/// HVec3 Zero element Z
NOALIAS __forceinline const HVec3 ZeroZ( const HVec3 PCREF v )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_insert( 0.0f, v.m_V, VZ );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V0X,V0Y,V1Z,V1W>( v.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v.m_V, __vzero(), VMX_MASK_INSERT_Z, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Mask<1,1,0,0>( v.m_V );
    #else
    result = HVec3( v.m_V[VX], v.m_V[VY], 0.0f );
    #endif
    return result;
}

/// HVec3 Zero elements X & Y
NOALIAS __forceinline const HVec3 ZeroXY( const HVec3 PCREF v )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_permute<V1X,V1Y,V0Z,V1W>( v.m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V1X,V1Y,V0Z,V1W>( v.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v.m_V, __vzero(), VMX_MASK_INSERT_XY, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Mask<0,0,1,0>( v.m_V );
    #else
    result = HVec3( 0.0f, 0.0f, v.m_V[VZ] );
    #endif
    return result;
}

/// HVec3 Zero elements X & Z
NOALIAS __forceinline const HVec3 ZeroXZ( const HVec3 PCREF v )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_permute<V1X,V0Y,V1Z,V1W>( v.m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V1X,V0Y,V1Z,V1W>( v.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v.m_V, __vzero(), VMX_MASK_INSERT_XZ, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Mask<0,1,0,0>( v.m_V );
    #else
    result = HVec3( 0.0f, v.m_V[VY], 0.0f );
    #endif
    return result;
}

/// HVec3 Zero elements Y & Z
NOALIAS __forceinline const HVec3 ZeroYZ( const HVec3 PCREF v )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_permute<V0X,V1Y,V1Z,V1W>( v.m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V0X,V1Y,V1Z,V1W>( v.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v.m_V, __vzero(), VMX_MASK_INSERT_YZ, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Mask<1,0,0,0>( v.m_V );
    #else
    result = HVec3( v.m_V[VX], 0.0f, 0.0f );
    #endif
    return result;
}

/// HVec3 Shuffle
template < unsigned IX, unsigned IY, unsigned IZ >
NOALIAS __forceinline const HVec3 Shuffle( const HVec3 PCREF v )
{
    StaticAssert( IX < 3, IX_MUST_BE_LESS_THAN_3 );
    StaticAssert( IY < 3, IY_MUST_BE_LESS_THAN_3 );
    StaticAssert( IZ < 3, IZ_MUST_BE_LESS_THAN_3 );
    
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_permute<IX,IY,IZ,VW>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<IX,IY,IZ,VW>( v.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vpermwi( v.m_V, VPERMWI_CONST( IX, IY, IZ, VW ) );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Permute<IX,IY,IZ,3>( v.m_V );
    #else
    result = HVec3( v.m_V[IX], v.m_V[IY], v.m_V[IZ] );
    #endif
    return result;
}

/// HVec3 Unary Negation Operator
NOALIAS __forceinline const HVec3 operator - ( const HVec3 PCREF v )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_negate( v.m_V );
    #elif defined( _PPU_ )
    //const vector float sign_mask = (vector float)(vector unsigned int)0x80000000;
    //const vector float sign_mask = (vector float)(vector unsigned int)( 0x80000000, 0x80000000, 0x80000000, 0x80000000 );
    const vector float sign_mask = Const< vector float >::SignMask();
    result.m_V = vec_vxor( v.m_V, sign_mask );
    #elif defined( _VMX128_ )
    const __vector4 signMask( c_VectorSignMask );
    result.m_V = __vxor( v.m_V, signMask );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_XOR( v.m_V, (__m128&)c_VectorSignMask );
    #else
    result = HVec3( -v.m_V[VX], -v.m_V[VY], -v.m_V[VZ] );
    #endif
    return result;
}

/// HVec3 Binary Addition Operator
NOALIAS __forceinline const HVec3_Add operator + ( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HVec3_Add result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_A = v0.m_V;
    result.m_B = v1.m_V;
    #elif defined( _VMX128_ )
    result.m_A = v0.m_V;
    result.m_B = v1.m_V;
    #elif defined( _SIMD_ )
    result.m_A.m_V = v0.m_V;
    result.m_B.m_V = v1.m_V;
    #else
    //return HVec3_Add( v0, v1 );
    result.m_A = v0;
    result.m_B = v1;
    #endif
    return result;
}

/// HVec3 Binary Subtraction Operator
NOALIAS __forceinline const HVec3_Subtract operator - ( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HVec3_Subtract result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_A = v0.m_V;
    result.m_B = v1.m_V;
    #elif defined( _VMX128_ )
    result.m_A = v0.m_V;
    result.m_B = v1.m_V;
    #elif defined( _SIMD_ )
    result.m_A.m_V = v0.m_V;
    result.m_B.m_V = v1.m_V;
    #else
    //return HVec3_Subtract( v0, v1 );
    result.m_A = v0;
    result.m_B = v1;
    #endif
    return result;
}

/// HVec3 Binary Multiplication Operator
NOALIAS __forceinline const HVec3_Multiply operator * ( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HVec3_Multiply result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_A = v0.m_V;
    result.m_B = v1.m_V;
    #elif defined( _VMX128_ )
    result.m_A = v0.m_V;
    result.m_B = v1.m_V;
    #elif defined( _SIMD_ )
    result.m_A.m_V = v0.m_V;
    result.m_B.m_V = v1.m_V;
    #else
    //return HVec3_Multiply( v0, v1 );
    result.m_A = v0;
    result.m_B = v1;
    #endif
    return result;
}

/// HVec3 Binary Division Operator
NOALIAS __forceinline const HVec3_Multiply operator / ( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HVec3_Multiply result;
    
    #if defined( _SPU_ )
    
    const vector float A( v0.m_V );
    const vector float B( v1.m_V );
    vector float rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    result.m_A = A;
    result.m_B = rcp_B;
    
    #elif defined( _PPU_ )
    
    const vector float A( v0.m_V );
    const vector float B( v1.m_V );
    vector float rcp_B;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    result.m_A = A;
    result.m_B = rcp_B;
    
    #elif defined( _VMX128_ )
    
    const __vector4 A( v0.m_V );
    const __vector4 B( v1.m_V );
    __vector4 rcp_B;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL( B, __vzero(), rcp_B, valid );
    result.m_A = A;
    result.m_B = rcp_B;
    
    #else
    //return HVec3_Multiply( v0, Reciprocal( v1 ) );
    result.m_A = v0;
    result.m_B = Reciprocal( v1 );
    #endif
    
    return result;
}

/// HVec3 Add Update Operator
NOALIAS __forceinline void operator += ( HVec3& v0, const HVec3 PCREF v1 )
{
    #if defined( _SPU_ )
    v0.m_V = spu_add( v0.m_V, v1.m_V );
    #elif defined( _PPU_ )
    v0.m_V = vec_add( v0.m_V, v1.m_V );
    #elif defined( _VMX128_ )
    v0.m_V = __vaddfp( v0.m_V, v1.m_V );
    #elif defined( _SIMD_ )
    v0.m_V = SIMD_Add( v0.m_V, v1.m_V );
    #else
    v0.m_V[VX] += v1.m_V[VX];
    v0.m_V[VY] += v1.m_V[VY];
    v0.m_V[VZ] += v1.m_V[VZ];
    #endif
}

/// HVec3 Subtract Update Operator
NOALIAS __forceinline void operator -= ( HVec3& v0, const HVec3 PCREF v1 )
{
    #if defined( _SPU_ )
    v0.m_V = spu_sub( v0.m_V, v1.m_V );
    #elif defined( _PPU_ )
    v0.m_V = vec_sub( v0.m_V, v1.m_V );
    #elif defined( _VMX128_ )
    v0.m_V = __vsubfp( v0.m_V, v1.m_V );
    #elif defined( _SIMD_ )
    v0.m_V = SIMD_Subtract( v0.m_V, v1.m_V );
    #else
    v0.m_V[VX] -= v1.m_V[VX];
    v0.m_V[VY] -= v1.m_V[VY];
    v0.m_V[VZ] -= v1.m_V[VZ];
    #endif
}

/// HVec3 Multiply Update Operator
NOALIAS __forceinline void operator *= ( HVec3& v0, const HVec3 PCREF v1 )
{
    #if defined( _SPU_ )
    v0.m_V = spu_mul( v0.m_V, v1.m_V );
    #elif defined( _PPU_ )
    v0.m_V = vec_mul( v0.m_V, v1.m_V );
    #elif defined( _VMX128_ )
    v0.m_V = __vmulfp( v0.m_V, v1.m_V );
    #elif defined( _SIMD_ )
    v0.m_V = SIMD_Multiply( v0.m_V, v1.m_V );
    #else
    v0.m_V[VX] *= v1.m_V[VX];
    v0.m_V[VY] *= v1.m_V[VY];
    v0.m_V[VZ] *= v1.m_V[VZ];
    #endif
}

/// HVec3 Divide Update Operator
NOALIAS __forceinline void operator /= ( HVec3& v0, const HVec3 PCREF v1 )
{
    #if defined( _SPU_ )
    
    const vector float A( v0.m_V );
    const vector float B( v1.m_V );
    vector float rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    v0.m_V = spu_mul( A, rcp_B );
    
    #elif defined( _PPU_ )
    
    const vector float A( v0.m_V );
    const vector float B( v1.m_V );
    vector float rcp_B;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    v0.m_V = vec_mul( A, rcp_B );
    
    #elif defined( _VMX128_ )
    
    const __vector4 A( v0.m_V );
    const __vector4 B( v1.m_V );
    __vector4 rcp_B;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL( B, __vzero(), rcp_B, valid );
    const __vector4 A_div_B( __vmulfp( A, rcp_B ) );
    
    #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
    v0.m_V = __vrlimi( A_div_B, __vzero(), VMX_MASK_INSERT_W, 0 );
    #else
    v0.m_V = A_div_B;
    #endif
    
    #elif defined( _SIMD_ )
    v0.m_V = SIMD_Multiply( v0.m_V, SIMD_Reciprocal3( v1.m_V ) );
    #else
    v0.m_V[VX] *= Reciprocal( v1.m_V[VX] );
    v0.m_V[VY] *= Reciprocal( v1.m_V[VY] );
    v0.m_V[VZ] *= Reciprocal( v1.m_V[VZ] );
    #endif
}

/// Multiply HVec3 * HF32
NOALIAS __forceinline const HVec3_Multiply operator * ( const HVec3 PCREF v, const HF32 PCREF f )
{
    HVec3_Multiply result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_A = v.m_V;
    result.m_B = f.m_F;
    #elif defined( _VMX128_ )
    result.m_A = v.m_V;
    result.m_B = f.m_F;
    #elif defined( _SIMD_ )
    //return HVec3_Multiply( v, HVec3( SIMD_Vector3_Scalar( f.m_F ) ) );
    result.m_A = v;
    result.m_B = HVec3( SIMD_Vector3_Scalar( f.m_F ) );
    #else
    //return HVec3_Multiply( v, HVec3( f ) );
    result.m_A = v;
    result.m_B = HVec3( f );
    #endif
    return result;
}

/// Multiply HF32 * HVec3
NOALIAS __forceinline const HVec3_Multiply operator * ( const HF32 PCREF f, const HVec3 PCREF v )
{
    HVec3_Multiply result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_A = v.m_V;
    result.m_B = f.m_F;
    #elif defined( _VMX128_ )
    result.m_A = v.m_V;
    result.m_B = f.m_F;
    #elif defined( _SIMD_ )
    //return HVec3_Multiply( HVec3( SIMD_Vector3_Scalar( f.m_F ) ), v );
    result.m_A = HVec3( SIMD_Vector3_Scalar( f.m_F ) );
    result.m_B = v;
    #else
    //return HVec3_Multiply( HVec3( f ), v );
    result.m_A = HVec3( f );
    result.m_B = v;
    #endif
    return result;
}

/// Multiply HVec3 * F32
NOALIAS __forceinline const HVec3_Multiply operator * ( const HVec3 PCREF v, const F32& f )
{
    // TODO: VMX128 optimised version!
    return v * Load<HF32>( &f );
}

/// Multiply F32 * HVec3
NOALIAS __forceinline const HVec3_Multiply operator * ( const F32& f, const HVec3 PCREF v )
{
    // TODO: VMX128 optimised version!
    return v * Load<HF32>( &f );
}

/// HVec3 Division by HF32 Operator
NOALIAS __forceinline const HVec3_Multiply operator / ( const HVec3 PCREF v, const HF32 PCREF f )
{
    HVec3_Multiply result;
    
    #if defined( _SPU_ )
    
    const vector float A( v.m_V );
    const vector float B( f.m_F );
    vector float rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    result.m_A = A;
    result.m_B = rcp_B;
    
    #elif defined( _PPU_ )
    
    const vector float A( v.m_V );
    const vector float B( f.m_F );
    vector float rcp_B;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    result.m_A = A;
    result.m_B = rcp_B;
    
    #elif defined( _VMX128_ )
    
    const __vector4 A( v.m_V );
    const __vector4 B( f.m_F );
    __vector4 rcp_B;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL( B, __vzero(), rcp_B, valid );
    
    result.m_A = A;
    #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
    result.m_B = __vrlimi( rcp_B, __vzero(), VMX_MASK_INSERT_W, 0 );
    #else
    result.m_B = rcp_B;
    #endif
    
    #elif defined( _SIMD_ )
    //return HVec3_Multiply( v, HVec3( SIMD_Vector4_Scalar( SIMD_ScalarReciprocal( f.m_F ) ) ) );
    result.m_A = v;
    result.m_B = HVec3( SIMD_Vector4_Scalar( SIMD_ScalarReciprocal( f.m_F ) ) );
    #else
    //return HVec3_Multiply( v, HVec3( f.Reciprocal() ) );
    result.m_A = v;
    result.m_B = HVec3( f.Reciprocal() );
    #endif
    
    return result;
}

/// HF32 Division by HVec3 Operator
NOALIAS __forceinline const HVec3_Multiply operator / ( const HF32 PCREF f, const HVec3 PCREF v )
{
    HVec3_Multiply result;
    
    #if defined( _SPU_ )
    
    const vector float A( f.m_F );
    const vector float B( v.m_V );
    vector float rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    result.m_A = A;
    result.m_B = rcp_B;
    
    #elif defined( _PPU_ )
    
    const vector float A( f.m_F );
    const vector float B( v.m_V );
    vector float rcp_B;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    result.m_A = A;
    result.m_B = rcp_B;
    
    #elif defined( _VMX128_ )
    
    const __vector4 A( f.m_F );
    const __vector4 B( v.m_V );
    __vector4 rcp_B;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL( B, __vzero(), rcp_B, valid );
    
    result.m_A = A;
    #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
    result.m_B = __vrlimi( rcp_B, __vzero(), VMX_MASK_INSERT_W, 0 );
    #else
    result.m_B = rcp_B;
    #endif
    
    #elif defined( _SIMD_ )
    //return HVec3_Multiply( HVec3( SIMD_Vector4_Scalar( f.m_F ) ), HVec3( SIMD_Reciprocal3( v ) ) );
    result.m_A = HVec3( SIMD_Vector4_Scalar( f.m_F ) );
    result.m_B = HVec3( SIMD_Reciprocal3( v ) );
    #else
    //return HVec3_Multiply( HVec3( f ), Reciprocal( v ) );
    result.m_A = HVec3( f );
    result.m_B = Reciprocal( v );
    #endif
    
    return result;
}

/// Divide HVec3 / F32 Operator
NOALIAS __forceinline const HVec3_Multiply operator / ( const HVec3 PCREF v, const F32& f )
{
    // TODO: VMX128 optimised version!
    return v / Load<HF32>( &f );
}

/// Divide F32 / HVec3 Operator
NOALIAS __forceinline const HVec3_Multiply operator / ( const F32& f, const HVec3 PCREF v )
{
    // TODO: VMX128 optimised version!
    return Load<HF32>( &f ) / v;
}

/// HF32 Multiply Update Operator
NOALIAS __forceinline void operator *= ( HVec3& v, const HF32 PCREF f )
{
    #if defined( _SPU_ )
    v.m_V = spu_mul( v.m_V, f.m_F );
    #elif defined( _PPU_ )
    v.m_V = vec_mul( v.m_V, f.m_F );
    #elif defined( _VMX128_ )
    v.m_V = __vmulfp( v.m_V, f.m_F );
    #elif defined( _SIMD_ )
    v.m_V = SIMD_Multiply( v.m_V, SIMD_Splat4<VX>( f.m_F ) );
    #else
    v.m_V[VX] *= f.m_F;
    v.m_V[VY] *= f.m_F;
    v.m_V[VZ] *= f.m_F;
    #endif
}

/// HF32 Divide Update Operator
NOALIAS __forceinline void operator /= ( HVec3& v, const HF32 PCREF f )
{
    #if defined( _SPU_ )
    
    const vector float A( v.m_V );
    const vector float B( f.m_F );
    vector float rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    v.m_V = spu_mul( A, rcp_B );
    
    #elif defined( _PPU_ )
    
    const vector float A( v.m_V );
    const vector float B( f.m_F );
    vector float rcp_B;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    v.m_V = vec_mul( A, rcp_B );
    
    #elif defined( _VMX128_ )
    
    const __vector4 A( v.m_V );
    const __vector4 B( f.m_F );
    __vector4 rcp_B;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL( B, __vzero(), rcp_B, valid );
    const __vector4 A_div_B( __vmulfp( A, rcp_B ) );
    
    #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
    v.m_V = __vrlimi( A_div_B, __vzero(), VMX_MASK_INSERT_W, 0 );
    #else
    v.m_V = A_div_B;
    #endif
    
    #elif defined( _SIMD_ )
    v.m_V = SIMD_Multiply( v.m_V, SIMD_Vector4_Scalar( SIMD_ScalarReciprocal( f.m_F ) ) );
    #else
    const F32 rcp_f( Reciprocal( f.m_F ) );
    v.m_V[VX] *= rcp_f;
    v.m_V[VY] *= rcp_f;
    v.m_V[VZ] *= rcp_f;
    #endif
}

/// Multiply HVec3 * F32 Update Operator
NOALIAS __forceinline void operator *= ( HVec3& v, const F32& f )
{
    // TODO: VMX128 optimised version!
    v *= Load<HF32>( &f );
}

/// Divide HVec3 / F32 Update Operator
NOALIAS __forceinline void operator /= ( HVec3& v, const F32& f )
{
    // TODO: VMX128 optimised version!
    v /= Load<HF32>( &f );
}

/// HVec3 Abs
NOALIAS __forceinline const HVec3 Abs( const HVec3 PCREF v0 )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_abs( v0.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_abs( v0.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( v0.m_V, c_VectorAbsMask );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_AND( v0.m_V, (__m128&)c_VectorAbsMask );
    #else
    result = HVec3( Abs( v0.m_V[VX] ),
                    Abs( v0.m_V[VY] ),
                    Abs( v0.m_V[VZ] ) );
    #endif
    return result;
}

/// HVec3 Min
NOALIAS __forceinline const HVec3 Min( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_min( v0.m_V, v1.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_min( v0.m_V, v1.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vminfp( v0.m_V, v1.m_V );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Min( v0.m_V, v1.m_V );
    #else
    result = HVec3( Min( v0.m_V[VX], v1.m_V[VX] ),
                    Min( v0.m_V[VY], v1.m_V[VY] ),
                    Min( v0.m_V[VZ], v1.m_V[VZ] ) );
    #endif
    return result;
}

/// HVec3 Max
NOALIAS __forceinline const HVec3 Max( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_max( v0.m_V, v1.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_max( v0.m_V, v1.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vmaxfp( v0.m_V, v1.m_V );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Max( v0.m_V, v1.m_V );
    #else
    result = HVec3( Max( v0.m_V[VX], v1.m_V[VX] ),
                    Max( v0.m_V[VY], v1.m_V[VY] ),
                    Max( v0.m_V[VZ], v1.m_V[VZ] ) );
    #endif
    return result;
}

/// Reciprocal of elements
NOALIAS __forceinline const HVec3 Reciprocal( const HVec3 PCREF v )
{
    HVec3 result;
    
    #if defined( _SPU_ )
    
    const vector float B( v.m_V );
    vector float rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    result.m_V = rcp_B;
    
    #elif defined( _PPU_ )
    
    const vector float B( v.m_V );
    vector float rcp_B;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL( B, Const< vector float >::Zero(), rcp_B, valid );
    result.m_V = rcp_B;
    
    #elif defined( _VMX128_ )
    
    const __vector4 B( v.m_V );
    __vector4 rcp_B;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL( B, __vzero(), rcp_B, valid );
    
    #if defined( MATHS_VMX_ZERO_PAD_ELEMENTS )
    result.m_V = __vrlimi( rcp_B, __vzero(), VMX_MASK_INSERT_W, 0 );
    #else
    result.m_V = rcp_B;
    #endif
    
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Reciprocal3( v.m_V );
    #else
    result = HVec3( Reciprocal( v.m_V[VX] ),
                    Reciprocal( v.m_V[VY] ),
                    Reciprocal( v.m_V[VZ] ) );
    #endif
    
    return result;
}

/// Horizontal sum of all elements
NOALIAS __forceinline const HF32 Sum( const HVec3 PCREF v )
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_sum3( v.m_V );
    #elif defined( _PPU_ )
    result.m_F = vec_sum3( v.m_V );
    #elif defined( _VMX128_ )
    const __vector4 one( c_VectorOne );
    result.m_F = __vmsum3fp( v.m_V, one );
    #elif defined( _SIMD_ )
    result.m_F = SIMD_Sum3( v.m_V );
    #else
    result = HF32( v.m_V[VX] + v.m_V[VY] + v.m_V[VZ] );
    #endif
    return result;
}

/// Vector Length
NOALIAS __forceinline const HF32 Length( const HVec3 PCREF v )
{
    HF32 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float vec( v.m_V );
    const vector float vec_len_squared( spu_dot_product3( vec, vec ) );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_F = spu_mul( vec_len_squared, vec_rcp_len );
    
    #elif defined( _PPU_ )
    
    const vector float vec( v.m_V );
    const vector float vec_len_squared( vec_dot_product3( vec, vec ) );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_F = vec_mul( vec_len_squared, vec_rcp_len );
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( v.m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_rcp_length ) );
    result.m_F = vector_length;
    
    #else
    result = Sqrt( LengthSquared( v ) );
    #endif
    
    return result;
}

/// Vector Length Squared
NOALIAS __forceinline const HF32 LengthSquared( const HVec3 PCREF v )
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_length_squared3( v.m_V );
    #elif defined( _PPU_ )
    result.m_F = vec_length_squared3( v.m_V );
    #elif defined( _VMX128_ )
    result.m_F = __vmsum3fp( v.m_V, v.m_V );
    #elif defined( _SIMD_ )
    result.m_F = SIMD_DotProduct3( v.m_V, v.m_V );
    #else
    result = Sum( v * v );
    #endif
    return result;
}

/// Vector Reciprocal of Length
NOALIAS __forceinline const HF32 ReciprocalLength( const HVec3 PCREF v )
{
    HF32 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float vec( v.m_V );
    const vector float vec_len_squared( spu_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_F = vec_rcp_len;
    
    #elif defined( _PPU_ )
    
    const vector float vec( v.m_V );
    const vector float vec_len_squared( vec_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_F = vec_rcp_len;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( v.m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    result.m_F = vector_rcp_length;
    
    #else
    result = RcpSqrt( LengthSquared( v ) );
    #endif
    
    return result;
}

/// Combine with HVecW to form a HVec4
NOALIAS __forceinline const HVec4 operator || ( const HVec3 PCREF xyz, const HVecW PCREF w )
{
    HVec4 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_permute<V0X,V0Y,V0Z,V1W>( xyz.m_V, w.m_W );
    #elif defined( _PPU_ )
    result.m_V = vec_permute<V0X,V0Y,V0Z,V1W>( xyz.m_V, w.m_W );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( xyz.m_V, w.m_W, VMX_MASK_INSERT_W, 0 );
    #elif defined( _SIMD_ )
    result.m_V = SIMD_Vector4_Vector3W( xyz.m_V, w.m_W );
    #else
    result = HVec4( xyz.m_V[VX], xyz.m_V[VY], xyz.m_V[VZ], w.m_W );
    #endif
    return result;
}

/// Dot product
NOALIAS __forceinline const HF32 operator | ( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_dot_product3( v0.m_V, v1.m_V );
    #elif defined( _PPU_ )
    result.m_F = vec_dot_product3( v0.m_V, v1.m_V );
    #elif defined( _VMX128_ )
    result.m_F = __vmsum3fp( v0.m_V, v1.m_V );
    #elif defined( _SIMD_ )
    result.m_F = SIMD_DotProduct3( v0.m_V, v1.m_V );
    #else
    result = Sum( v0 * v1 );
    #endif
    return result;
}

/// Cross Product
NOALIAS __forceinline const HVec3 operator ^ ( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HVec3 result( UNDEFINED );
    
    #if defined( _SPU_ )
    result.m_V = spu_cross_product3( v0.m_V, v1.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cross_product3( v0.m_V, v1.m_V );
    #elif defined( _VMX128_ )
    
    const __vector4 l_yzx( __vpermwi( v0.m_V, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 l_zxy( __vpermwi( v0.m_V, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 r_yzx( __vpermwi( v1.m_V, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 r_zxy( __vpermwi( v1.m_V, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 l_x_y( __vnmsubfp( l_zxy, r_yzx, __vmulfp( l_yzx, r_zxy ) ) );
    result.m_V = l_x_y;
    
    #elif defined( _SIMD_ )
    
    static const unsigned __ = VW;
    const V128 l_yzx = SIMD_Permute<VY,VZ,VX,__>( v0.m_V );
    const V128 l_zxy = SIMD_Permute<VZ,VX,VY,__>( v0.m_V );
    const V128 r_yzx = SIMD_Permute<VY,VZ,VX,__>( v1.m_V );
    const V128 r_zxy = SIMD_Permute<VZ,VX,VY,__>( v1.m_V );
    result.m_V = SIMD_SubtractMultiply( SIMD_Multiply( l_yzx, r_zxy ), l_zxy, r_yzx );
    
    #else
    
    result = HVec3( v0.m_V[VY] * v1.m_V[VZ] - v0.m_V[VZ] * v1.m_V[VY],
                    v0.m_V[VZ] * v1.m_V[VX] - v0.m_V[VX] * v1.m_V[VZ],
                    v0.m_V[VX] * v1.m_V[VY] - v0.m_V[VY] * v1.m_V[VX] );
    
    #endif
    
    return result;
}

/// Transform: Apply rotation then translation
NOALIAS __forceinline const HVec3 Transform( const HVec3 PCREF vertex, const C_Quaternion PCREF rotation, const HVec3 PCREF translation )
{
	return ( vertex * rotation ) + translation;
}

/// Vertex Distance
NOALIAS __forceinline const HF32 Distance( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HF32 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float point_A( v0.m_V );
    const vector float point_B( v1.m_V );
    const vector float vec( spu_sub( point_B, point_A ) );
    const vector float vec_len_squared( spu_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_F = spu_mul( vec_len_squared, vec_rcp_len );
    
    #elif defined( _PPU_ )
    
    const vector float point_A( v0.m_V );
    const vector float point_B( v1.m_V );
    const vector float vec( vec_sub( point_B, point_A ) );
    const vector float vec_len_squared( vec_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_F = vec_mul( vec_len_squared, vec_rcp_len );
    
    #elif defined( _VMX128_ )
    
    const __vector4 point_A( v0.m_V );
    const __vector4 point_B( v1.m_V );
    const __vector4 vector( __vsubfp( point_B, point_A ) );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_rcp_length ) );
    result.m_F = vector_length;
    
    #else
    result = Length( v1 - v0 );
    #endif
    
    return result;
}

/// Vertex Distance Squared
NOALIAS __forceinline const HF32 DistanceSquared( const HVec3 PCREF v0, const HVec3 PCREF v1 )
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_length_squared3( spu_sub( v1.m_V, v0.m_V ) );
    #elif defined( _PPU_ )
    result.m_F = vec_length_squared3( vec_sub( v1.m_V, v0.m_V ) );
    #elif defined( _VMX128_ )
    const __vector4 point_A( v0.m_V );
    const __vector4 point_B( v1.m_V );
    const __vector4 vector( __vsubfp( point_B, point_A ) );
    const __vector4 distance_squared( __vmsum3fp( vector, vector ) );
    result.m_F = distance_squared;
    #else
    result = LengthSquared( v1 - v0 );
    #endif
    return result;
}

/// Lerp
NOALIAS __forceinline const HVec3 Lerp( const HVec3 PCREF src, const HVec3 PCREF dst, const HF32 PCREF f )
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V = spu_madd( spu_sub( dst.m_V, src.m_V ), f.m_F, src.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_madd( vec_sub( dst.m_V, src.m_V ), f.m_F, src.m_V );
    #elif defined( _VMX128_ )
    const __vector4 v_src( src.m_V );
    const __vector4 v_dst( dst.m_V );
    const __vector4 v_src_to_dst( __vsubfp( v_dst, v_src ) );
    const __vector4 v_f( f.m_F );
    const __vector4 x( __vmaddfp( v_src_to_dst, v_f, v_src ) );
    result.m_V = x;
    #else
    result = src + ( ( dst - src ) * f );
    #endif
    return result;
}

NOALIAS __forceinline const C_Vector Lerp( const C_Vector PCREF src, const C_Vector PCREF dst, const HF32 PCREF f )
{
	return Lerp((HVec3&) src, (HVec3&) dst, f);
}

NOALIAS __forceinline const C_Vertex Lerp( const C_Vertex PCREF src, const C_Vertex PCREF dst, const HF32 PCREF f )
{
	return Lerp((HVec3&) src, (HVec3&) dst, f);
}

/// Calculates vector length, length squared and reciprocal of length simultaneously
NOALIAS __forceinline void LengthSquaredAndReciprocal( const HVec3 PCREF v, HF32& length, HF32& length_squared, HF32& reciprocal_length )
{
    #if defined( _SPU_ )
    
    const vector float vec_len_squared( spu_length_squared3( v.m_V ) );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    length.m_F = spu_mul( vec_len_squared, vec_rcp_len );
    length_squared.m_F = vec_len_squared;
    reciprocal_length.m_F = vec_rcp_len;
    
    #elif defined( _PPU_ )
    
    const vector float vec_len_squared( vec_length_squared3( v.m_V ) );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    length.m_F = vec_mul( vec_len_squared, vec_rcp_len );
    length_squared.m_F = vec_len_squared;
    reciprocal_length.m_F = vec_rcp_len;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( v.m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_rcp_length ) );
    length.m_F = vector_length;
    length_squared.m_F = vector_length_squared;
    reciprocal_length.m_F = vector_rcp_length;
    
    #else
    length_squared = LengthSquared( v );
    reciprocal_length = ReciprocalLength( v );
    length = length_squared * reciprocal_length;
    #endif
}

/// Calculates vector normal, length, length squared and reciprocal of length simultaneously
NOALIAS __forceinline const C_Normalised NormalLengthSquaredAndReciprocal( const HVec3 PCREF v )
{
    C_Normalised result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float vec( v.m_V );
    const vector float vec_len_squared( spu_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_Normal.m_V.m_V = spu_mul( vec, vec_rcp_len );
    result.m_Length.m_F     = spu_mul( vec_len_squared, vec_rcp_len );
    result.m_Length2.m_F    = vec_len_squared;
    result.m_RcpLength.m_F  = vec_rcp_len;
    
    #elif defined( _PPU_ )
    
    const vector float vec( v.m_V );
    const vector float vec_len_squared( vec_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_Normal.m_V.m_V = vec_mul( vec, vec_rcp_len );
    result.m_Length.m_F     = vec_mul( vec_len_squared, vec_rcp_len );
    result.m_Length2.m_F    = vec_len_squared;
    result.m_RcpLength.m_F  = vec_rcp_len;
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( v.m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_rcp_length ) );
    const __vector4 normal( __vmulfp( vector, vector_rcp_length ) );
    result.m_Normal.m_V.m_V = normal;
    result.m_Length.m_F     = vector_length;
    result.m_Length2.m_F    = vector_length_squared;
    result.m_RcpLength.m_F  = vector_rcp_length;
    
    #else
    
    const HF32  length2( LengthSquared( v ) );
    const HF32  reciprocalLength( ReciprocalSqrt( length2 ) );
    const HVec3 normalisedVector( v * reciprocalLength );
    
    #if defined( MATHS_DEBUG_BREAK_ON_ZERO_LENGTH_NORMALISE )
    if ( length2 <= nearlyZero ) DebugBreak();
    #endif
    
    const C_Normal normal( normalisedVector, C_Normal::UNIT_LENGTH_VECTOR );
    //return C_Normalised( normal, length2 * reciprocalLength, length2, reciprocalLength );
    result.m_Normal    = normal;
    result.m_Length    = length2 * reciprocalLength;
    result.m_Length2   = length2;
    result.m_RcpLength = reciprocalLength;
    
    #endif
    
    return result;
}

/// Vector Normalise
NOALIAS __forceinline const C_Normal Normalise( const HVec3 PCREF v )
{
    C_Normal result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float vec( v.m_V );
    const vector float vec_len_squared( spu_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_V.m_V = spu_mul( vec, vec_rcp_len );
    
    #elif defined( _PPU_ )
    
    const vector float vec( v.m_V );
    const vector float vec_len_squared( vec_length_squared3( vec ) );
    vector float vec_rcp_len;
    vector bool int valid;
    PPU_VECTOR_RECIPROCAL_SQRT( vec_len_squared, Const< vector float >::Zero(), vec_rcp_len, valid );
    result.m_V.m_V = vec_mul( vec, vec_rcp_len );
    
    #elif defined( _VMX128_ )
    
    const __vector4 vector( v.m_V );
    const __vector4 vector_length_squared( __vmsum3fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 normal( __vmulfp( vector, vector_rcp_length ) );
    result.m_V.m_V = normal;
    
    #else
    
    const HF32  length2( LengthSquared( v ) );
    const HVec3 normal( v * ReciprocalSqrt( length2 ) );
    
    #if defined( MATHS_DEBUG_BREAK_ON_ZERO_LENGTH_NORMALISE )
    if ( length2 <= nearlyZero ) DebugBreak();
    #endif
    
    result = C_Normal( normal, C_Normal::UNIT_LENGTH_VECTOR );
    
    #endif
    
    return result;
}



/// Combine HVec3 with HF32 to form a HVec4
NOALIAS __forceinline const HVec4 operator || ( const HVec3 PCREF xyz, const HF32 PCREF w )
{
    #if defined( _VMX128_ )
    
    HVec4 result( UNDEFINED );
    result.m_V = __vrlimi( xyz.m_V, w.m_F, VMX_MASK_INSERT_W, 0 );
    return result;
    
    #elif defined( _SIMD_ )
    return HVec4( xyz.X(), xyz.Y(), xyz.Z(), w);
    #else
    return HVec4( xyz.m_V[VX], xyz.m_V[VY], xyz.m_V[VZ], w.m_F );
    #endif
}

/// Combine HF32 with HVec3 to form a HVec4
NOALIAS __forceinline const HVec4 operator || ( const HF32 PCREF x, const HVec3 PCREF yzw )
{
    #if defined( _VMX128_ )
    
    HVec4 result( UNDEFINED );
    result.m_V = __vrlimi( x.m_F, yzw.m_V, VMX_MASK_INSERT_YZW, 3 );
    return result;
    
    #elif defined( _SIMD_ )
    return HVec4( x, yzw.X(), yzw.Y(), yzw.Z() );
    #else
    return HVec4( x.m_F, yzw.m_V[VX], yzw.m_V[VY], yzw.m_V[VZ] );
    #endif
}








/// HVec3_Multiply Abs
NOALIAS __forceinline const HVec3 Abs( const HVec3_Multiply PCREF v0 )  { return Abs( HVec3( v0 ) ); }

/// HVec3_Add Abs
NOALIAS __forceinline const HVec3 Abs( const HVec3_Add PCREF v0 )       { return Abs( HVec3( v0 ) ); }

/// HVec3_Subtract Abs
NOALIAS __forceinline const HVec3 Abs( const HVec3_Subtract PCREF v0 )  { return Abs( HVec3( v0 ) ); }









/// C_Vertex Abs
NOALIAS __forceinline const C_Vertex Abs( const C_Vertex PCREF v )
{
    C_Vertex result( UNDEFINED );
    result.m_V = Abs( v.m_V );
    return result;
}

/// C_Vertex Sum
NOALIAS __forceinline const HF32 Sum( const C_Vertex PCREF v )
{
    return Sum( v.m_V );
}





/// C_Vector Abs
NOALIAS __forceinline const C_Vector Abs( const C_Vector PCREF v )
{
    C_Vector result( UNDEFINED );
    result.m_V = Abs( v.m_V );
    return result;
}

/// C_Vector Sum
NOALIAS __forceinline const HF32 Sum( const C_Vector PCREF v )
{
    return Sum( v.m_V );
}





/// Calculate distances from vertex to four planes - TODO: Move to GeometryOp.inl
NOALIAS __forceinline void DistanceToPlanes( const HVec3 PCREF v,
                                             const C_Plane PCREF p0, const C_Plane PCREF p1, const C_Plane PCREF p2, const C_Plane PCREF p3, 
                                             HF32&               d0, HF32&               d1, HF32&               d2, HF32&               d3 )
{
    #if defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 vertex( v.m_V );
    const __vector4 vertex_xyz1( __vrlimi( vertex, one, VMX_MASK_INSERT_W, 0 ) );
    const __vector4 plane0( p0.m_V );
    const __vector4 plane1( p1.m_V );
    const __vector4 plane2( p2.m_V );
    const __vector4 plane3( p3.m_V );
    d0.m_F = __vmsum4fp( vertex_xyz1, plane0 );
    d1.m_F = __vmsum4fp( vertex_xyz1, plane1 );
    d2.m_F = __vmsum4fp( vertex_xyz1, plane2 );
    d3.m_F = __vmsum4fp( vertex_xyz1, plane3 );
    
    #else
    
    d0 = p0.Distance( v );
    d1 = p1.Distance( v );
    d2 = p2.Distance( v );
    d3 = p3.Distance( v );
    
    #endif
}

}; // namespace Maths

#endif

