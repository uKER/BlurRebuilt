#ifndef MATHS_HVEC_VECMAT_OP_INL
#define MATHS_HVEC_VECMAT_OP_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

/// Star function = Cross Product Matrix ( a* Matrix )
NOALIAS __forceinline const HMat33 Star( const HVec3 PCREF v )
{
    HMat33 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    static const unsigned __ = VW;
    const vector float xyz0( spu_permute<V0X,V0Y,V0Z,V1W>( v.m_V, Const< vector float >::Zero() ) );
    result.m_AX = spu_sub( spu_permute<__,__,VY,__>( xyz0 ), spu_permute<__,VZ,__,__>( xyz0 ) );
    result.m_AY = spu_sub( spu_permute<VZ,__,__,__>( xyz0 ), spu_permute<__,__,VX,__>( xyz0 ) );
    result.m_AZ = spu_sub( spu_permute<__,VX,__,__>( xyz0 ), spu_permute<VY,__,__,__>( xyz0 ) );
    
    #elif defined( _PPU_ )
    
    static const unsigned __ = VW;
    const vector float xyz0( vec_permute<V0X,V0Y,V0Z,V1W>( v.m_V, Const< vector float >::Zero() ) );
    result.m_AX = vec_sub( vec_permute<__,__,VY,__>( xyz0 ), vec_permute<__,VZ,__,__>( xyz0 ) );
    result.m_AY = vec_sub( vec_permute<VZ,__,__,__>( xyz0 ), vec_permute<__,__,VX,__>( xyz0 ) );
    result.m_AZ = vec_sub( vec_permute<__,VX,__,__>( xyz0 ), vec_permute<VY,__,__,__>( xyz0 ) );
    
    #elif defined( _VMX128_ )
    
    static const unsigned __ = VW;
    const __vector4 signMask( c_VectorSignMask );
    const __vector4 xyz0( __vrlimi( v.m_V, __vzero(), VMX_MASK_INSERT_W, 0 ) );
    const __vector4 xyz_neg( __vxor( v.m_V, signMask ) );
    const __vector4 cx( __vrlimi( __vpermwi( xyz0, VPERMWI_CONST(__,VZ,__,__) ), __vpermwi( xyz_neg, VPERMWI_CONST(__,__,VY,__) ), VMX_MASK_INSERT_Z, 0 ) );
    const __vector4 cy( __vrlimi( __vpermwi( xyz0, VPERMWI_CONST(__,__,VX,__) ), __vpermwi( xyz_neg, VPERMWI_CONST(VZ,__,__,__) ), VMX_MASK_INSERT_X, 0 ) );
    const __vector4 cz( __vrlimi( __vpermwi( xyz0, VPERMWI_CONST(VY,__,__,__) ), __vpermwi( xyz_neg, VPERMWI_CONST(__,VX,__,__) ), VMX_MASK_INSERT_Y, 0 ) );
    result.m_CX = cx;
    result.m_CY = cy;
    result.m_CZ = cz;
    
    #elif defined( _SIMD_ )
    
    static const unsigned __ = VW;
    const HVec4 xyz0( v );
    result.m_AX.m_V = SIMD_Subtract( SIMD_Permute<__,__,VY,__>( xyz0 ), SIMD_Permute<__,VZ,__,__>( xyz0 ) );
    result.m_AY.m_V = SIMD_Subtract( SIMD_Permute<VZ,__,__,__>( xyz0 ), SIMD_Permute<__,__,VX,__>( xyz0 ) );
    result.m_AZ.m_V = SIMD_Subtract( SIMD_Permute<__,VX,__,__>( xyz0 ), SIMD_Permute<VY,__,__,__>( xyz0 ) );
    
    #else
    result = HMat33( HVec3(  0.0f,      -v.m_V[VZ],  v.m_V[VY] ),
                     HVec3(  v.m_V[VZ],  0.0f,      -v.m_V[VX] ),
                     HVec3( -v.m_V[VY],  v.m_V[VX],  0.0f    ) );
    #endif
    
    return result;
}

/// HVec3 * HMat33 Multiplication
NOALIAS __forceinline const HVec3 operator * ( const HVec3 PCREF v, const HMat33& m )
{
    HVec3 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    result.m_V = spu_madd( spu_splat<VX>( v.m_V ), m.m_AX,
                 spu_madd( spu_splat<VY>( v.m_V ), m.m_AY,
                 spu_mul ( spu_splat<VZ>( v.m_V ), m.m_AZ ) ) );
    
    #elif defined( _PPU_ )
    
    result.m_V = vec_madd( vec_splat( v.m_V, VX ), m.m_AX,
                 vec_madd( vec_splat( v.m_V, VY ), m.m_AY,
                 vec_mul ( vec_splat( v.m_V, VZ ), m.m_AZ ) ) );
    
    #elif defined( _VMX128_ )
    
    const __vector4 x( __vmsum3fp( v.m_V, m.m_CX ) );
    const __vector4 y( __vmsum3fp( v.m_V, m.m_CY ) );
    const __vector4 z( __vmsum3fp( v.m_V, m.m_CZ ) );
    result.m_V = __vmrghw( __vmrghw( x, z ), __vmrghw( y, __vzero() ) );
    
    #elif defined( _SSE_ )
    
    result.m_V = SIMD_MultiplyAdd( SIMD_Splat3<VX>( v.m_V ), m.m_AX.m_V,
                 SIMD_MultiplyAdd( SIMD_Splat3<VY>( v.m_V ), m.m_AY.m_V,
                 SIMD_Multiply   ( SIMD_Splat3<VZ>( v.m_V ), m.m_AZ.m_V ) ) );
    
    #else
    
    const F32 x = v.m_V[VX] * m.m_A[MX].m_V[VX] + v.m_V[VY] * m.m_A[MY].m_V[VX] + v.m_V[VZ] * m.m_A[MZ].m_V[VX];
    const F32 y = v.m_V[VX] * m.m_A[MX].m_V[VY] + v.m_V[VY] * m.m_A[MY].m_V[VY] + v.m_V[VZ] * m.m_A[MZ].m_V[VY];
    const F32 z = v.m_V[VX] * m.m_A[MX].m_V[VZ] + v.m_V[VY] * m.m_A[MY].m_V[VZ] + v.m_V[VZ] * m.m_A[MZ].m_V[VZ];
    result = HVec3( x, y, z );
    
    #endif
    
    return result;
}

/// HVec3 * HMat33 Abs Transform : Similar to HVec3 * HMat33 Multiplication but takes abs of all products before summing
NOALIAS __forceinline const HVec3 AbsTransform( const HVec3 PCREF v, const HMat33& m )
{
    HVec3 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    //result.m_V = spu_madd( spu_splat<VX>( v.m_V ), m.m_AX,
    //             spu_madd( spu_splat<VY>( v.m_V ), m.m_AY,
    //             spu_mul ( spu_splat<VZ>( v.m_V ), m.m_AZ ) ) );
    const vector float abs_v( spu_abs( v.m_V ) );
    result.m_V = spu_madd( spu_splat<VX>( abs_v ), spu_abs( m.m_AX ),
                 spu_madd( spu_splat<VY>( abs_v ), spu_abs( m.m_AY ),
                 spu_mul ( spu_splat<VZ>( abs_v ), spu_abs( m.m_AZ ) ) ) );
    
    #elif defined( _PPU_ )
    
    //result.m_V = vec_madd( vec_splat( v.m_V, VX ), m.m_AX,
    //             vec_madd( vec_splat( v.m_V, VY ), m.m_AY,
    //             vec_mul ( vec_splat( v.m_V, VZ ), m.m_AZ ) ) );
    const vector float abs_v( vec_abs( v.m_V ) );
    result.m_V = vec_madd( vec_splat( abs_v, VX ), vec_abs( m.m_AX ),
                 vec_madd( vec_splat( abs_v, VY ), vec_abs( m.m_AY ),
                 vec_mul ( vec_splat( abs_v, VZ ), vec_abs( m.m_AZ ) ) ) );
    
    #elif defined( _VMX128_ )
    
    //const __vector4 x( __vmsum3fp( v.m_V, m.m_CX ) );
    //const __vector4 y( __vmsum3fp( v.m_V, m.m_CY ) );
    //const __vector4 z( __vmsum3fp( v.m_V, m.m_CZ ) );
    const __vector4 abs_mask( c_VectorAbsMask );
    const __vector4 abs_v( __vand( v.m_V, abs_mask ) );
    const __vector4 x( __vmsum3fp( abs_v, __vand( m.m_CX, abs_mask ) ) );
    const __vector4 y( __vmsum3fp( abs_v, __vand( m.m_CY, abs_mask ) ) );
    const __vector4 z( __vmsum3fp( abs_v, __vand( m.m_CZ, abs_mask ) ) );
    
    result.m_V = __vmrghw( __vmrghw( x, z ), __vmrghw( y, __vzero() ) );
    
    #elif defined( _SSE_ )
    
    //result.m_V = SIMD_MultiplyAdd( SIMD_Splat3<VX>( v.m_V ), m.m_AX.m_V,
    //             SIMD_MultiplyAdd( SIMD_Splat3<VY>( v.m_V ), m.m_AY.m_V,
    //             SIMD_Multiply   ( SIMD_Splat3<VZ>( v.m_V ), m.m_AZ.m_V ) ) );
    result.m_V = SIMD_Add( SIMD_Add( SIMD_Abs( SIMD_Multiply( SIMD_Splat3<VX>( v.m_V ), m.m_AX.m_V ) ),
                                     SIMD_Abs( SIMD_Multiply( SIMD_Splat3<VY>( v.m_V ), m.m_AY.m_V ) ) ),
                                     SIMD_Abs( SIMD_Multiply( SIMD_Splat3<VZ>( v.m_V ), m.m_AZ.m_V ) ) );
    
    #else
    
    const F32 x = Abs( v.m_V[VX] * m.m_A[MX].m_V[VX] ) + Abs( v.m_V[VY] * m.m_A[MY].m_V[VX] ) + Abs( v.m_V[VZ] * m.m_A[MZ].m_V[VX] );
    const F32 y = Abs( v.m_V[VX] * m.m_A[MX].m_V[VY] ) + Abs( v.m_V[VY] * m.m_A[MY].m_V[VY] ) + Abs( v.m_V[VZ] * m.m_A[MZ].m_V[VY] );
    const F32 z = Abs( v.m_V[VX] * m.m_A[MX].m_V[VZ] ) + Abs( v.m_V[VY] * m.m_A[MY].m_V[VZ] ) + Abs( v.m_V[VZ] * m.m_A[MZ].m_V[VZ] );
    result = HVec3( x, y, z );
    
    #endif
    
    return result;
}

/// HVec3 * HMat43 Multiplication
NOALIAS __forceinline const HVec3 operator * ( const HVec3 PCREF v, const HMat43& m )
{
    HVec3 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    result.m_V = spu_add( spu_madd( spu_splat<VX>( v.m_V ), m.m_AX,
                          spu_mul ( spu_splat<VY>( v.m_V ), m.m_AY ) ),
                          spu_madd( spu_splat<VZ>( v.m_V ), m.m_AZ,
                                                            m.m_AW ) );
    
    #elif defined( _PPU_ )
    
    result.m_V = vec_add( vec_madd( vec_splat( v.m_V, VX ), m.m_AX,
                          vec_mul ( vec_splat( v.m_V, VY ), m.m_AY ) ),
                          vec_madd( vec_splat( v.m_V, VZ ), m.m_AZ,
                                                            m.m_AW ) );
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 v_xyz1( __vrlimi( v.m_V, one, VMX_MASK_INSERT_W, 0 ) );
    const __vector4 m_cx( m.m_CX );
    const __vector4 m_cy( m.m_CY );
    const __vector4 m_cz( m.m_CZ );
    const __vector4 x( __vmsum4fp( v_xyz1, m_cx ) );
    const __vector4 y( __vmsum4fp( v_xyz1, m_cy ) );
    const __vector4 z( __vmsum4fp( v_xyz1, m_cz ) );
    result.m_V = __vmrghw( __vmrghw( x, z ), __vmrghw( y, __vzero() ) );
    
    #elif defined( _SSE_ )
    
    result.m_V = SIMD_Add( SIMD_MultiplyAdd( SIMD_Splat3<VX>( v.m_V ), m.m_Rotate.m_AX.m_V,
                           SIMD_Multiply   ( SIMD_Splat3<VY>( v.m_V ), m.m_Rotate.m_AY.m_V ) ),
                           SIMD_MultiplyAdd( SIMD_Splat3<VZ>( v.m_V ), m.m_Rotate.m_AZ.m_V,
                                                                       m.m_Translate.m_V ) );
    
    #else
    
    const F32 x = v.m_V[VX] * m.m_Rotate.m_A[MX].m_V[VX] + v.m_V[VY] * m.m_Rotate.m_A[MY].m_V[VX] + v.m_V[VZ] * m.m_Rotate.m_A[MZ].m_V[VX] + m.m_Translate.m_V[VX];
    const F32 y = v.m_V[VX] * m.m_Rotate.m_A[MX].m_V[VY] + v.m_V[VY] * m.m_Rotate.m_A[MY].m_V[VY] + v.m_V[VZ] * m.m_Rotate.m_A[MZ].m_V[VY] + m.m_Translate.m_V[VY];
    const F32 z = v.m_V[VX] * m.m_Rotate.m_A[MX].m_V[VZ] + v.m_V[VY] * m.m_Rotate.m_A[MY].m_V[VZ] + v.m_V[VZ] * m.m_Rotate.m_A[MZ].m_V[VZ] + m.m_Translate.m_V[VZ];
    result = HVec3( x, y, z );
    
    #endif
    
    return result;
}

/// HVec3 * HMat44 Multiplication
NOALIAS __forceinline const HVec3 operator * ( const HVec3 PCREF v, const HMat44& m )
{
    HVec3 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    result.m_V = spu_add( spu_madd( spu_splat<VX>( v.m_V ), m.m_AX,
                          spu_mul ( spu_splat<VY>( v.m_V ), m.m_AY ) ),
                          spu_madd( spu_splat<VZ>( v.m_V ), m.m_AZ,
                                                            m.m_AW ) );
    
    #elif defined( _PPU_ )
    
    result.m_V = vec_add( vec_madd( vec_splat( v.m_V, VX ), m.m_AX,
                          vec_mul ( vec_splat( v.m_V, VY ), m.m_AY ) ),
                          vec_madd( vec_splat( v.m_V, VZ ), m.m_AZ,
                                                            m.m_AW ) );
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 v_xyz1( __vrlimi( v.m_V, one, VMX_MASK_INSERT_W, 0 ) );
    const __vector4 m_cx( m.m_CX );
    const __vector4 m_cy( m.m_CY );
    const __vector4 m_cz( m.m_CZ );
    const __vector4 x( __vmsum4fp( v_xyz1, m_cx ) );
    const __vector4 y( __vmsum4fp( v_xyz1, m_cy ) );
    const __vector4 z( __vmsum4fp( v_xyz1, m_cz ) );
    result.m_V = __vmrghw( __vmrghw( x, z ), __vmrghw( y, __vzero() ) );
    
    #elif defined( _SSE_ )
    
    result.m_V = SIMD_Add( SIMD_MultiplyAdd( SIMD_Splat3<VX>( v.m_V ), m.m_AX.m_V,
                           SIMD_Multiply   ( SIMD_Splat3<VY>( v.m_V ), m.m_AY.m_V ) ),
                           SIMD_MultiplyAdd( SIMD_Splat3<VZ>( v.m_V ), m.m_AZ.m_V,
                           SIMD_AND        ( SIMD_Mask<1,1,1,0>(),     m.m_AW.m_V ) ) );
    
    #else
    
    const F32 x = v.m_V[VX] * m.m_A[MX].m_V[VX] + v.m_V[VY] * m.m_A[MY].m_V[VX] + v.m_V[VZ] * m.m_A[MZ].m_V[VX] + m.m_A[MW].m_V[VX];
    const F32 y = v.m_V[VX] * m.m_A[MX].m_V[VY] + v.m_V[VY] * m.m_A[MY].m_V[VY] + v.m_V[VZ] * m.m_A[MZ].m_V[VY] + m.m_A[MW].m_V[VY];
    const F32 z = v.m_V[VX] * m.m_A[MX].m_V[VZ] + v.m_V[VY] * m.m_A[MY].m_V[VZ] + v.m_V[VZ] * m.m_A[MZ].m_V[VZ] + m.m_A[MW].m_V[VZ];
    result = HVec3( x, y, z );
    
    #endif
    
    return result;
}




}; // namespace Maths

#endif

