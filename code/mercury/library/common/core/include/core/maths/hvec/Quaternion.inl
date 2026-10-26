#ifndef QUATERNION_INL
#define QUATERNION_INL

#ifdef _MSC_VER
#pragma once
#endif

#include <core/maths/hvec/simd.h>

namespace Maths
{
//
// ------------------------------------------------
// |-- C_Quaternion : Quaternion rotation class --|
// ------------------------------------------------
//

/// New Maths Quaternion Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion( const Quaternion& q )
{
    #if defined( _VMX128_ )
    C_Quaternion temp( UNDEFINED );
    temp.m_V = q.m_Q.m_V;
    #else
    C_Quaternion temp( UNDEFINED );
    temp.m_V = HVec4( q.m_Q );
    #endif
    m_V = (!temp).m_V;
    //m_V = temp.m_V;
}

/// New Maths Quaternion Cast
NOALIAS __forceinline C_Quaternion::operator const Quaternion() const __restrict
{
    Quaternion q;
    #if defined( _VMX128_ )
    q.m_Q.m_V = (!(*this)).m_V;
    #else
    q.m_Q = (!(*this)).m_V;
    #endif
    return q;
}

/// Default Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion() :
    m_V( c_UnitVectorW ) // Initialise to the Multiplicative Identity.
{
}

/// Undefined Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion( const C_Quaternion& copy ) :
    m_V( copy.m_V )
{
}

/// Copy Assignment
NOALIAS __forceinline const C_Quaternion& C_Quaternion::operator = ( const C_Quaternion& copy )
{
    m_V = copy.m_V;
    return copy;
}

/// Explicit V128 Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion( const V128 PCREF src ) :
    m_V( src )
{
}

/// Explicit HVec4 Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion( const HVec4 PCREF src ) :
    m_V( src )
{
}

#if 1 || !defined( _VMX128_ )
/// Axis-Angle Initialiser for Quaternion Constructor
/*NOALIAS*/ __forceinline const HVec4 InitQuaternion( const HVec3 PCREF axis, const HF32 PCREF theta )
{
    const HF32  half( Constants<HF32>::Half() );
    const HF32  halfTheta( theta * half );
    const HF32  sinHalfTheta( Sin( halfTheta ) );
    const HF32  cosHalfTheta( Cos( halfTheta ) );
    const HVec3 imag( Normalise( axis ) * sinHalfTheta );
    const HVecW real( cosHalfTheta );
    return imag || real;
}
#endif

/// HVec3 Axis + HF32 Angle Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion( const HVec3 PCREF axis, const HF32 PCREF theta )
    #if 1 || !defined( _VMX128_ )
    : m_V( InitQuaternion( axis, theta ) )
    #endif
{
    #if 0 && defined( _VMX128_ )
    
    const __vector4 half( c_ScalarHalf );
    const HF32 halfTheta( __vmulfp( theta.m_F, half ) );
    const HF32 sinHalfTheta( Sin( halfTheta ) ); // *** TODO: SinCos() ***
    const HF32 cosHalfTheta( Cos( halfTheta ) );
    
    const __vector4 axis_vector( axis.m_V );
    const __vector4 axis_vector_length_squared( __vmsum3fp( axis_vector, axis_vector ) );
    __vector4 axis_vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( axis_vector_length_squared, __vzero(), axis_vector_rcp_length, valid );
    const __vector4 axis_normal( __vmulfp( axis_vector, axis_vector_rcp_length ) );
    
    const __vector4 imag( __vmulfp( axis_normal, sinHalfTheta.m_F ) );
    const __vector4 real( cosHalfTheta.m_F );
    
    m_V = __vrlimi( imag, real, VMX_MASK_INSERT_W, 0 );
    
    #endif
}

/// HF32 Imag XYZ + Real Constructor ( NOTE: NOT axis + angle! )
NOALIAS __forceinline C_Quaternion::C_Quaternion( const HF32 PCREF imag_x, const HF32 PCREF imag_y, const HF32 PCREF imag_z, const HF32 PCREF real ) :
    #if defined( _VMX128_ )
    m_V( __vmrghw( __vmrghw( imag_x.m_F, imag_z.m_F ), __vmrghw( imag_y.m_F, real.m_F ) ) )
    #else
    m_V( HVec4( imag_x, imag_y, imag_z, real ) )
    #endif
{
}

/// HVec3 Imag XYZ + HVecW Real Constructor ( NOTE: NOT axis + angle! )
NOALIAS __forceinline C_Quaternion::C_Quaternion( const HVec3 PCREF imag, const HVecW PCREF real ) :
    #if defined( _VMX128_ )
    m_V( __vrlimi( imag.m_V, real.m_W, VMX_MASK_INSERT_W, 0 ) )
    #else
    m_V( imag || real )
    #endif
{
}

/// Explicit HVec3 Imaginary part Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion( const HVec3 PCREF imag ) :
    #if defined( _VMX128_ )
    m_V( __vrlimi( imag.m_V, __vzero(), VMX_MASK_INSERT_W, 0 ) )
    #else
    m_V( imag || HVecW( c_VectorZero ) ) // Ensure W is zero!
    #endif
{
}

/// Explicit Real part Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion( const HVecW PCREF real ) :
    #if defined( _VMX128_ )
    m_V( __vrlimi( __vzero(), real.m_W, VMX_MASK_INSERT_W, 0 ) )
    #else
    m_V( real )
    #endif
{
}

#if 1 || !defined( _VMX128_ )
/// Matrix Initialiser for Quaternion Constructor
NOALIAS __forceinline const HVec4 InitQuaternion( const HMat33& matrix )
{    
    const HF32 zero(ZERO_INIT);
    const HF32 half( c_ScalarHalf );
    const HF32 one( c_ScalarOne );
	const HF32 epsilon( Constants<HF32>::Epsilon() );
    
    const HVec3 m0( matrix.AX() );
    const HVec3 m1( matrix.AY() );
    const HVec3 m2( matrix.AZ() );
    
    const HF32 m00( m0.X() );
    const HF32 m01( m0.Y() );
    const HF32 m02( m0.Z() );
    const HF32 m10( m1.X() );
    const HF32 m11( m1.Y() );
    const HF32 m12( m1.Z() );
    const HF32 m20( m2.X() );
    const HF32 m21( m2.Y() );
    const HF32 m22( m2.Z() );
    
    const HVec4 trace_xyzw( one + m00 - m11 - m22,
                            one - m00 + m11 - m22,
                            one - m00 - m11 + m22,
                            one + m00 + m11 + m22 );
    
	const Scalar_Bool master_w( trace_xyzw.W().GT( trace_xyzw.X() ) &
		                        trace_xyzw.W().GT( trace_xyzw.Y() ) &
								trace_xyzw.W().GT( trace_xyzw.Z() ) );
	const Scalar_Bool master_x( trace_xyzw.X().GT( trace_xyzw.Y() ) &
		                        trace_xyzw.X().GT( trace_xyzw.Z() ) /* &
								trace_xyzw.X().GT( trace_xyzw.W() ) */ );
	const Scalar_Bool master_y( /* trace_xyzw.Y().GT( trace_xyzw.X() ) & */
		                        trace_xyzw.Y().GT( trace_xyzw.Z() ) /* &
								trace_xyzw.Y().GT( trace_xyzw.W() ) */ );

    const HVec4 x_sign_xyzw( zero, m01 + m10, m02 + m20, m21 - m12 );

    const HVec4 y_sign_xyzw( m01 + m10, zero, m12 + m21, m02 - m20 );
    
	const HVec4 z_sign_xyzw( m02 + m20, m12 + m21, zero, m10 - m01 );
    
	const HVec4 w_sign_xyzw( m21 - m12, m02 - m20, m10 - m01, zero );
    
	const HVec4 sign_xyzw( Select( master_w, w_sign_xyzw,
						   Select( master_x, x_sign_xyzw,
						   Select( master_y, y_sign_xyzw, z_sign_xyzw ) ) ) );
	
    const HVec4 xyzw( CopySign( Sqrt( trace_xyzw ) * half, sign_xyzw ) );
    
    return C_Quaternion( xyzw );
}
#endif

/// Explicit HMat33 Rotation Matrix Constructor
NOALIAS __forceinline C_Quaternion::C_Quaternion( const HMat33& matrix )
    #if 1 || !defined( _VMX128_ )
    : m_V( InitQuaternion( matrix ) )
    #endif
{
    #if 0 && defined( _VMX128_ )
    
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
    // quaternion.w = sqrt( max( 0, 1 + m00 + m11 + m22 ) ) / 2; 
    // quaternion.x = sqrt( max( 0, 1 + m00 - m11 - m22 ) ) / 2; 
    // quaternion.y = sqrt( max( 0, 1 - m00 + m11 - m22 ) ) / 2; 
    // quaternion.z = sqrt( max( 0, 1 - m00 - m11 + m22 ) ) / 2; 
    // Q.x = _copysign( Q.x, m21 - m12 ) 
    // Q.y = _copysign( Q.y, m02 - m20 ) 
    // Q.z = _copysign( Q.z, m10 - m01 ) 
    
    const __vector4 zero( __vzero() );
    const __vector4 quarter( c_ScalarQuarter );
    const __vector4 one( c_ScalarOne );
    const __vector4 sign_mask( c_VectorSignMask );
    
    const __vector4 matrix_column_x( matrix.m_CX );
    const __vector4 matrix_column_y( matrix.m_CY );
    const __vector4 matrix_column_z( matrix.m_CZ );
    
    const __vector4 m00( __vspltw( matrix_column_x, VX ) );
    const __vector4 m01( __vspltw( matrix_column_y, VX ) );
    const __vector4 m02( __vspltw( matrix_column_z, VX ) );
    const __vector4 m10( __vspltw( matrix_column_x, VY ) );
    const __vector4 m11( __vspltw( matrix_column_y, VY ) );
    const __vector4 m12( __vspltw( matrix_column_z, VY ) );
    const __vector4 m20( __vspltw( matrix_column_x, VZ ) );
    const __vector4 m21( __vspltw( matrix_column_y, VZ ) );
    const __vector4 m22( __vspltw( matrix_column_z, VZ ) );
    
    const __vector4 minus_m00( __vxor( m00, sign_mask ) );
    const __vector4 minus_m11( __vxor( m11, sign_mask ) );
    const __vector4 minus_m22( __vxor( m22, sign_mask ) );
    
    const __vector4 vector_trace_x( __vmrghw( __vmrghw( one,       m00 ), __vmrghw( minus_m11, minus_m22 ) ) );
    const __vector4 vector_trace_y( __vmrghw( __vmrghw( one, minus_m00 ), __vmrghw(       m11, minus_m22 ) ) );
    const __vector4 vector_trace_z( __vmrghw( __vmrghw( one, minus_m00 ), __vmrghw( minus_m11,       m22 ) ) );
    const __vector4 vector_trace_w( __vmrghw( __vmrghw( one,       m00 ), __vmrghw(       m11,       m22 ) ) );
    
    const __vector4 qx_squared( __vmsum4fp( vector_trace_x, quarter ) );
    const __vector4 qy_squared( __vmsum4fp( vector_trace_y, quarter ) );
    const __vector4 qz_squared( __vmsum4fp( vector_trace_z, quarter ) );
    const __vector4 qw_squared( __vmsum4fp( vector_trace_w, quarter ) );
    
    const __vector4 sign_x( __vsubfp( m21, m12 ) );
    const __vector4 sign_y( __vsubfp( m02, m20 ) );
    const __vector4 sign_z( __vsubfp( m10, m01 ) );
    const __vector4 sign_xyzw( __vmrghw( __vmrghw( sign_x, sign_z ),
                                         __vmrghw( sign_y, zero ) ) );
    
    const __vector4 q_squared( __vmrghw( __vmrghw( qx_squared, qz_squared ),
                                         __vmrghw( qy_squared, qw_squared ) ) );
    
    __vector4 rcp_q;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( q_squared, zero, rcp_q, valid );
    const __vector4 q( __vmulfp( q_squared, rcp_q ) );
    
    m_V = __vsel( q, sign_xyzw, sign_mask );
    
    #endif
}

/// Cast to HVec4
NOALIAS __forceinline C_Quaternion::operator const HVec4() const __restrict
{
    #if defined( _VMX128_ )
    
    HVec4 result( UNDEFINED );
    result.m_V = m_V;
    return result;
    
    #else
    return m_V;
    #endif
}


/// Cast to Fixed4_S16
template < int T_Precision >
NOALIAS __forceinline C_Quaternion::operator const MathsStorage::Fixed4_S16<T_Precision> () const __restrict
{
	// todo - maintainability of code - intermediate #defines to share code with other similar cases.
    MathsStorage::Fixed4_S16<T_Precision> result;
    #if defined( _SPU_ )
    const vector signed int temp = spu_convts( m_V.m_V, T_Precision );
    result.x = (S16)spu_extract( temp, VX );
    result.y = (S16)spu_extract( temp, VY );
    result.z = (S16)spu_extract( temp, VZ );
    result.w = (S16)spu_extract( temp, VW );
    #elif defined( _PPU_ )
    //const vector signed short packed( vec_vpkswss( vec_vctsxs( m_V, T_Precision ), (vector signed int)vec_vspltisw( 0 ) ) );
    const vector signed short packed( vec_packs( vec_cts( m_V.m_V, T_Precision ), vec_splat_s32( 0 ) ) );
    vec_stvehx( vec_vsplth( packed, 0 ), 0, &result.x );
    vec_stvehx( vec_vsplth( packed, 1 ), 0, &result.y );
    vec_stvehx( vec_vsplth( packed, 2 ), 0, &result.z );
    vec_stvehx( vec_vsplth( packed, 3 ), 0, &result.w );
    #elif defined( _VMX128_ )
    const __vector4 packed( __vpkswss( __vctsxs( m_V, T_Precision ), __vzero() ) );
    __stvehx( __vsplth( packed, 0 ), &result.x, 0 );
    __stvehx( __vsplth( packed, 1 ), &result.y, 0 );
    __stvehx( __vsplth( packed, 2 ), &result.z, 0 );
    __stvehx( __vsplth( packed, 3 ), &result.w, 0 );
    #elif defined (_SSE_)
    const F32 multiplier( 1 << T_Precision );
    result.x = (S16)( m_V.X() * multiplier );
    result.y = (S16)( m_V.Y() * multiplier );
    result.z = (S16)( m_V.Z() * multiplier );   
    result.w = (S16)( m_V.W() * multiplier );   
    #else
    const F32 multiplier( 1 << T_Precision );
    result.x = (S16)( m_V[VX] * multiplier );
    result.y = (S16)( m_V[VY] * multiplier );
    result.z = (S16)( m_V[VZ] * multiplier );
    result.w = (S16)( m_V[VW] * multiplier );
    #endif
    return result;
}


/// Explicit Constructor from Fixed3_S16 ptr
template < int T_Precision >
NOALIAS __forceinline C_Quaternion::C_Quaternion( MathsStorage::Fixed4_S16<T_Precision> const * pStorage )
    #if defined( _SPU_ )
	: m_V( spu_convtf( ( (vector signed int){ (signed int)pStorage->x,
                                            (signed int)pStorage->y,
                                            (signed int)pStorage->z,
											(signed int)pStorage->w	} ), T_Precision ) )
    #elif defined( _PPU_ )
    : m_V( vec_vcfsx( vec_vupkhsh( vec_load_unaligned( pStorage->v ) ), T_Precision ) )
    #elif defined( _VMX128_ )
        : m_V( __vcfsx( __vupkhsh( __loadunalignedvector( pStorage ) ), T_Precision ) )
    #endif
{
    #if defined( MATHS_CHECK_VECTOR_ALIGNMENT )
    if ( ((S32)this) & 0xF ) DebugBreak();
    #endif

    #if defined (_SSE_)
    const F32 multiplier( 1.0f / (F32)( 1 << T_Precision ) );
    *this = C_Quaternion(
				(F32)pStorage->x * multiplier,
				(F32)pStorage->y * multiplier,
				(F32)pStorage->z * multiplier,
				(F32)pStorage->w * multiplier
			);
    #elif !defined( _SIMD_ )
    const F32 multiplier( 1.0f / ((F32)( 1 << T_Precision ) ));
    m_V[VX] = (F32)pStorage->x * multiplier;
    m_V[VY] = (F32)pStorage->y * multiplier;
    m_V[VZ] = (F32)pStorage->z * multiplier;
    m_V[VW] = (F32)pStorage->w * multiplier;
    #endif
}



/// Returns the Imaginary part as a HVec3
NOALIAS __forceinline const HVec3 C_Quaternion::Imag() const __restrict
{
    #if defined( _VMX128_ )
    
    HVec3 result( UNDEFINED );
    result.m_V = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_W, 0 );
    return result;
    
    #else
    return (HVec3)m_V;
    #endif
}

/// Returns the Imaginary part as a HVec3
NOALIAS __forceinline const HVec3 Imag( const C_Quaternion PCREF src )
{
    #if defined( _VMX128_ )
    
    HVec3 result( UNDEFINED );
    result.m_V = __vrlimi( src.m_V, __vzero(), VMX_MASK_INSERT_W, 0 );
    return result;
    
    #else
    return (HVec3)src.m_V;
    #endif
}

/// Returns the Real part as a HVecW
NOALIAS __forceinline const HVecW C_Quaternion::Real() const __restrict
{
    #if defined( _VMX128_ )
    
    HVecW result( UNDEFINED );
    result.m_W = __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_XYZ, 0 );
    return result;
    
    #else
    return (HVecW)m_V;
    #endif
}

/// Returns the Real part as a HVecW
NOALIAS __forceinline const HVecW Real( const C_Quaternion PCREF src )
{
    #if defined( _VMX128_ )
    
    HVecW result( UNDEFINED );
    result.m_W = __vrlimi( src.m_V, __vzero(), VMX_MASK_INSERT_XYZ, 0 );
    return result;
    
    #else
    return (HVecW)src.m_V;
    #endif
}

/// Cos( theta / 2 ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32 C_Quaternion::CosHalfTheta() const __restrict
{
    #if defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vspltw( m_V, VW );
    return result;
    
    #else
    return m_V.W();
    #endif
}

/// Cos( theta / 2 ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32 CosHalfTheta( const C_Quaternion PCREF src )
{
    #if defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vspltw( src.m_V, VW );
    return result;
    
    #else
    return src.m_V.W();
    #endif
}

/// Sin( theta / 2 ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32 C_Quaternion::SinHalfTheta() const __restrict
{
    /// From Fundamental Identity: sin^2(x) + cos^2(x) = 1
    
    #if defined( _VMX128_ )
    
    const __vector4 one( c_ScalarOne );
    const __vector4 cosHalfTheta( __vspltw( m_V, VW ) );
    const __vector4 oneMinusCosHalfThetaSquared( __vnmsubfp( cosHalfTheta, cosHalfTheta, one ) );
    
    __vector4 rcpSqrt_oneMinusCosHalfThetaSquared;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( oneMinusCosHalfThetaSquared, __vzero(), rcpSqrt_oneMinusCosHalfThetaSquared, valid );
    
    HF32 result( UNDEFINED );
    result.m_F = __vmulfp( oneMinusCosHalfThetaSquared, rcpSqrt_oneMinusCosHalfThetaSquared );
    return result;
    
    #else
    
    const HF32 one( c_ScalarOne );
    const HF32 cosHalfTheta( CosHalfTheta() );
    return Sqrt( one - ( cosHalfTheta * cosHalfTheta ) );
    
    #endif
}

/// Sin( theta / 2 ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32 SinHalfTheta( const C_Quaternion PCREF src )
{
    /// From Fundamental Identity: sin^2(x) + cos^2(x) = 1
    
    #if defined( _VMX128_ )
    
    const __vector4 one( c_ScalarOne );
    const __vector4 cosHalfTheta( __vspltw( src.m_V, VW ) );
    const __vector4 oneMinusCosHalfThetaSquared( __vnmsubfp( cosHalfTheta, cosHalfTheta, one ) );
    
    __vector4 rcpSqrt_oneMinusCosHalfThetaSquared;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( oneMinusCosHalfThetaSquared, __vzero(), rcpSqrt_oneMinusCosHalfThetaSquared, valid );
    
    HF32 result( UNDEFINED );
    result.m_F = __vmulfp( oneMinusCosHalfThetaSquared, rcpSqrt_oneMinusCosHalfThetaSquared );
    return result;
    
    #else
    
    const HF32 one( c_ScalarOne );
    const HF32 cosHalfTheta( CosHalfTheta( src ) );
    return Sqrt( one - ( cosHalfTheta * cosHalfTheta ) );
    
    #endif
}

/// Sin( theta / 2 ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32 C_Quaternion::ReciprocalSinHalfTheta() const __restrict
{
    /// From Fundamental Identity: sin^2(x) + cos^2(x) = 1
    
    #if defined( _VMX128_ )
    
    const __vector4 one( c_ScalarOne );
    const __vector4 cosHalfTheta( __vspltw( m_V, VW ) );
    const __vector4 oneMinusCosHalfThetaSquared( __vnmsubfp( cosHalfTheta, cosHalfTheta, one ) );
    
    __vector4 rcpSqrt_oneMinusCosHalfThetaSquared;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( oneMinusCosHalfThetaSquared, __vzero(), rcpSqrt_oneMinusCosHalfThetaSquared, valid );
    
    HF32 result( UNDEFINED );
    result.m_F = rcpSqrt_oneMinusCosHalfThetaSquared;
    return result;
    
    #else
    
    const HF32 one( c_ScalarOne );
    const HF32 cosHalfTheta( CosHalfTheta() );
    return ReciprocalSqrt( one - ( cosHalfTheta * cosHalfTheta ) );
    
    #endif
}

/// Sin( theta / 2 ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32 ReciprocalSinHalfTheta( const C_Quaternion PCREF src )
{
    /// From Fundamental Identity: sin^2(x) + cos^2(x) = 1
    
    #if defined( _VMX128_ )
    
    const __vector4 one( c_ScalarOne );
    const __vector4 cosHalfTheta( __vspltw( src.m_V, VW ) );
    const __vector4 oneMinusCosHalfThetaSquared( __vnmsubfp( cosHalfTheta, cosHalfTheta, one ) );
    
    __vector4 rcpSqrt_oneMinusCosHalfThetaSquared;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( oneMinusCosHalfThetaSquared, __vzero(), rcpSqrt_oneMinusCosHalfThetaSquared, valid );
    
    HF32 result( UNDEFINED );
    result.m_F = rcpSqrt_oneMinusCosHalfThetaSquared;
    return result;
    
    #else
    
    const HF32 one( c_ScalarOne );
    const HF32 cosHalfTheta( CosHalfTheta( src ) );
    return ReciprocalSqrt( one - ( cosHalfTheta * cosHalfTheta ) );
    
    #endif
}

/// Cos( theta ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32 C_Quaternion::CosTheta() const __restrict
{
    /// From Double Angle Formula: cos(2x) = 2 cos^2(x) - 1
    
    #if defined( _VMX128_ )
    
    const __vector4 one( c_ScalarOne );
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 cosHalfTheta( __vspltw( m_V, VW ) );
    const __vector4 cosHalfTheta_x2( __vaddfp( cosHalfTheta, cosHalfTheta ) );
    const __vector4 oneMinusTwoCosSquaredHalfTheta( __vnmsubfp( cosHalfTheta_x2, cosHalfTheta, one ) );
    const __vector4 cosTheta( __vxor( oneMinusTwoCosSquaredHalfTheta, sign_mask ) );
    
    HF32 result( UNDEFINED );
    result.m_F = cosTheta;
    return result;
    
    #else
    
    const HF32 one( c_ScalarOne );
    const HF32 two( c_ScalarTwo );
    const HF32 cosHalfTheta( CosHalfTheta() );
    return ( two * cosHalfTheta * cosHalfTheta ) - one;
    
    #endif
}

/// Cos( theta ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32 CosTheta( const C_Quaternion PCREF src )
{
    /// From Double Angle Formula: cos(2x) = 2 cos^2(x) - 1
    
    #if defined( _VMX128_ )
    
    const __vector4 one( c_ScalarOne );
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 cosHalfTheta( __vspltw( src.m_V, VW ) );
    const __vector4 cosHalfTheta_x2( __vaddfp( cosHalfTheta, cosHalfTheta ) );
    const __vector4 oneMinusTwoCosSquaredHalfTheta( __vnmsubfp( cosHalfTheta_x2, cosHalfTheta, one ) );
    const __vector4 cosTheta( __vxor( oneMinusTwoCosSquaredHalfTheta, sign_mask ) );
    
    HF32 result( UNDEFINED );
    result.m_F = cosTheta;
    return result;
    
    #else
    
    const HF32 one( c_ScalarOne );
    const HF32 two( c_ScalarTwo );
    const HF32 cosHalfTheta( CosHalfTheta( src ) );
    return ( two * cosHalfTheta * cosHalfTheta ) - one;
    
    #endif
}

/// Length
NOALIAS __forceinline const HF32 C_Quaternion::Length() const __restrict
{
    #if defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 vector_length_squared( __vmsum4fp( vector, vector ) );
    
    __vector4 vector_reciprocal_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_reciprocal_length, valid );
    
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_reciprocal_length ) );
    
    HF32 result( UNDEFINED );
    result.m_F = vector_length;
    return result;
    
    #else
    return m_V.Length();
    #endif
}

/// Length
NOALIAS __forceinline const HF32 Length( const C_Quaternion PCREF src )
{
    #if defined( _VMX128_ )
    
    const __vector4 vector( src.m_V );
    const __vector4 vector_length_squared( __vmsum4fp( vector, vector ) );
    
    __vector4 vector_reciprocal_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_reciprocal_length, valid );
    
    const __vector4 vector_length( __vmulfp( vector_length_squared, vector_reciprocal_length ) );
    
    HF32 result( UNDEFINED );
    result.m_F = vector_length;
    return result;
    
    #else
    return src.m_V.Length();
    #endif
}

/// Addition Operator
NOALIAS __forceinline const C_Quaternion operator + ( const C_Quaternion PCREF src0, const C_Quaternion PCREF src1 )
{
    #if defined( _VMX128_ )
    
    C_Quaternion result( UNDEFINED );
    result.m_V = __vaddfp( src0.m_V, src1.m_V );
    return result;
    
    #else
    return C_Quaternion( src0.m_V + src1.m_V );
    #endif
}

/// Subtraction Operator
NOALIAS __forceinline const C_Quaternion operator - ( const C_Quaternion PCREF src0, const C_Quaternion PCREF src1 )
{
    #if defined( _VMX128_ )
    
    C_Quaternion result( UNDEFINED );
    result.m_V = __vsubfp( src0.m_V, src1.m_V );
    return result;
    
    #else
    return C_Quaternion( src0.m_V - src1.m_V );
    #endif
}

/// Unary Negation Operator
NOALIAS __forceinline const C_Quaternion operator - ( const C_Quaternion PCREF src )
{
    #if defined( _VMX128_ )
    
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 vector( src.m_V );
    const __vector4 negated_vector( __vxor( vector, sign_mask ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = negated_vector;
    return result;
    
    #else
    return C_Quaternion( -src.m_V );
    #endif
}

/// Conjugate Operator (Multiplicative Inverse)
NOALIAS __forceinline const C_Quaternion operator ! ( const C_Quaternion PCREF src )
{
    #if defined( _VMX128_ )
    
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 vector( src.m_V );
    const __vector4 negated_vector( __vxor( vector, sign_mask ) );
    const __vector4 conjugate( __vrlimi( negated_vector, vector, VMX_MASK_INSERT_W, 0 ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = conjugate;
    return result;
    
    #else
    return C_Quaternion( -src.Imag() || src.Real() );
    #endif
}

/// Scalar Multiplication Operator
NOALIAS __forceinline const C_Quaternion operator * ( const C_Quaternion PCREF q, const HF32 PCREF f )
{
    #if defined( _VMX128_ )
    
    const __vector4 vector( q.m_V );
    const __vector4 scalar( f.m_F );
    const __vector4 vector_x_scalar( __vmulfp( vector, scalar ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = vector_x_scalar;
    return result;
    
    #else
    return C_Quaternion( q.m_V * f );
    #endif
}

/// Multiplication Operator: C_Quaternion * C_Quaternion --> C_Quaternion
NOALIAS __forceinline const C_Quaternion operator * ( const C_Quaternion PCREF q1, const C_Quaternion PCREF q2 )
{
    // q1q2 = ( a1*a2 - v1|v2, a1*v2 + a2*v1 + v1^v2 )
    
    #if defined( _VMX128_ )
    
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 vector_q1( q1.m_V );
    const __vector4 vector_q2( q2.m_V );
    const __vector4 scalar_q1( __vspltw( vector_q1, VW ) );
    const __vector4 scalar_q2( __vspltw( vector_q2, VW ) );
    
    const __vector4 vector_q1_yzx( __vpermwi( vector_q1, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_q1_zxy( __vpermwi( vector_q1, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 vector_q2_yzx( __vpermwi( vector_q2, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_q2_zxy( __vpermwi( vector_q2, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    
    const __vector4 imag_scalar_x_vector( __vmaddfp( vector_q1, scalar_q2,
                                           __vmulfp( vector_q2, scalar_q1 ) ) );
    
    const __vector4 imag_vector_x_vector( __vnmsubfp( vector_q1_yzx, vector_q2_zxy,
                                            __vmulfp( vector_q1_zxy, vector_q2_yzx ) ) );
    
    const __vector4 imag( __vaddfp( imag_scalar_x_vector, imag_vector_x_vector ) );
    
    const __vector4 real_dp( __vmsum3fp( vector_q1, vector_q2 ) );
    const __vector4 real_dp_negated( __vxor( real_dp, sign_mask ) );
    const __vector4 real( __vmaddfp( scalar_q1, scalar_q2, real_dp_negated ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = __vrlimi( imag, real, VMX_MASK_INSERT_W, 0 );
    return result;
    
    #else
    
    const HVec3 imag1( q1.Imag() );
    const HVec3 imag2( q2.Imag() );
    const HF32  real1( q1.Real() );
    const HF32  real2( q2.Real() );
    
    // Scalar * Vector components a1*v2 + a2*v1 +
    // Vector cross product component v1^v2
    const HVec3 imag( ( imag1 * real2 ) + ( imag2 * real1 ) + ( imag2 ^ imag1 ) );
    
    // Scalar multiplication component a1*a2 -
    // Vector dot product component v1|v2
    const HVecW real( ( real1 * real2 ) - ( imag1 | imag2 ) );
    
    // Combine real & imaginary parts
    return C_Quaternion( imag || real );
    
    #endif
}

/// Multiplication Operator: HVec3 * C_Quaternion --> HVec3
NOALIAS __forceinline const HVec3 operator * ( const HVec3 PCREF v, const C_Quaternion PCREF q )
{
    #if defined( _VMX128_ )
    
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 vector_v( v.m_V );
    const __vector4 vector_q( q.m_V );
    
    // QV = Q * V
    
    const __vector4 scalar_q( __vspltw( vector_q, VW ) );
    //const __vector4 scalar_v( __vzero() );
    
    const __vector4 vector_q_yzx( __vpermwi( vector_q, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_q_zxy( __vpermwi( vector_q, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 vector_v_yzx( __vpermwi( vector_v, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_v_zxy( __vpermwi( vector_v, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    
    //const __vector4 vector_qv_scalar_x_vector( __vmaddfp( vector_v, scalar_q, __vmulfp( vector_q, scalar_v ) ) );
    const __vector4 vector_qv_scalar_x_vector( __vmulfp( vector_v, scalar_q ) ); // vector_q * scalar_v = 0
    
    const __vector4 vector_qv_vector_x_vector( __vnmsubfp( vector_q_yzx, vector_v_zxy,
                                                 __vmulfp( vector_q_zxy, vector_v_yzx ) ) );
    
    const __vector4 vector_qv( __vaddfp( vector_qv_scalar_x_vector, vector_qv_vector_x_vector ) );
    
    const __vector4 scalar_qv_dp( __vmsum3fp( vector_v, vector_q ) );
    const __vector4 scalar_qv_dp_negated( __vxor( scalar_qv_dp, sign_mask ) );
    //const __vector4 scalar_qv( __vmaddfp( scalar_v, scalar_q, scalar_qv_dp_negated ) );
    const __vector4 scalar_qv( scalar_qv_dp_negated ); // scalar_v * scalar_q = 0
    
    // QC = !Q
    
    const __vector4 vector_qc( __vxor( vector_q, sign_mask ) );
    const __vector4 scalar_qc( __vspltw( vector_q, VW ) );
    
    // QVQC = QV * QC
    
    const __vector4 vector_qv_yzx( __vpermwi( vector_qv, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_qv_zxy( __vpermwi( vector_qv, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 vector_qc_yzx( __vpermwi( vector_qc, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_qc_zxy( __vpermwi( vector_qc, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    
    const __vector4 vector_qvqc_scalar_x_vector( __vmaddfp( vector_qv, scalar_qc,
                                                  __vmulfp( vector_qc, scalar_qv ) ) );
    
    const __vector4 vector_qvqc_vector_x_vector( __vnmsubfp( vector_qv_yzx, vector_qc_zxy,
                                                   __vmulfp( vector_qv_zxy, vector_qc_yzx ) ) );
    
    const __vector4 vector_qvqc( __vaddfp( vector_qvqc_scalar_x_vector, vector_qvqc_vector_x_vector ) );
    
    //const __vector4 scalar_qvqc_dp( __vmsum3fp( vector_qv, vector_qc ) );
    //const __vector4 scalar_qvqc_dp_negated( __vxor( scalar_qvqc_dp, sign_mask ) );
    //const __vector4 scalar_qvqc( __vmaddfp( scalar_qv, scalar_qc, scalar_qvqc_dp_negated ) );
    
    HVec3 result( UNDEFINED );
    //result.m_V = __vrlimi( vector_qvqc, scalar_qvqc, VMX_MASK_INSERT_W, 0 );
    result.m_V = __vrlimi( vector_qvqc, __vzero(), VMX_MASK_INSERT_W, 0 );
    return result;
    
    #else
    return ( q * C_Quaternion( v ) * !q ).Imag();
    #endif
}

/// Division Operator: num / denom = num * !denom
NOALIAS __forceinline const C_Quaternion operator / ( const C_Quaternion PCREF num, const C_Quaternion PCREF denom )
{
    #if defined( _VMX128_ )
    
    const __vector4 sign_mask( c_VectorSignMask );
    
    const __vector4 quat_num( num.m_V );
    const __vector4 quat_denom( denom.m_V );
    
    const __vector4 vector_q1( quat_num );
    const __vector4 scalar_q1( __vspltw( quat_num, VW ) );
    
    const __vector4 vector_q2( __vxor( quat_denom, sign_mask ) );
    const __vector4 scalar_q2( __vspltw( quat_denom, VW ) );
    
    const __vector4 vector_q1_yzx( __vpermwi( vector_q1, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_q1_zxy( __vpermwi( vector_q1, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 vector_q2_yzx( __vpermwi( vector_q2, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_q2_zxy( __vpermwi( vector_q2, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    
    const __vector4 imag_scalar_x_vector( __vmaddfp( vector_q1, scalar_q2,
                                           __vmulfp( vector_q2, scalar_q1 ) ) );
    
    const __vector4 imag_vector_x_vector( __vnmsubfp( vector_q1_yzx, vector_q2_zxy,
                                            __vmulfp( vector_q1_zxy, vector_q2_yzx ) ) );
    
    const __vector4 imag( __vaddfp( imag_scalar_x_vector, imag_vector_x_vector ) );
    
    const __vector4 real_dp( __vmsum3fp( vector_q1, vector_q2 ) );
    const __vector4 real_dp_negated( __vxor( real_dp, sign_mask ) );
    const __vector4 real( __vmaddfp( scalar_q1, scalar_q2, real_dp_negated ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = __vrlimi( imag, real, VMX_MASK_INSERT_W, 0 );
    return result;
    
    #else
    return num * !denom;
    #endif
}

/// Shortest Division: num / denom = num * !denom
NOALIAS __forceinline const C_Quaternion C_Quaternion::ShortestDivide( const C_Quaternion PCREF denom ) const __restrict
{
    #if defined( _VMX128_ )
    
    const __vector4 zero( __vzero() );
    const __vector4 sign_mask( c_VectorSignMask );
    
    const __vector4 quat_num( m_V );
    const __vector4 quat_denom( denom.m_V );
    const __vector4 quat_minus_denom( __vxor( quat_denom, sign_mask ) );
    
    const __vector4 cosHalfTheta( __vmsum4fp( quat_num, quat_denom ) );
    const __vector4 cosHalfTheta_GE_zero( __vcmpgefp( cosHalfTheta, zero ) );
    const __vector4 quat_nearest_denom( __vsel( quat_minus_denom, quat_denom, cosHalfTheta_GE_zero ) );
    
    const __vector4 vector_q1( quat_num );
    const __vector4 scalar_q1( __vspltw( quat_num, VW ) );
    
    const __vector4 vector_q2( __vxor( quat_nearest_denom, sign_mask ) );
    const __vector4 scalar_q2( __vspltw( quat_nearest_denom, VW ) );
    
    const __vector4 vector_q1_yzx( __vpermwi( vector_q1, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_q1_zxy( __vpermwi( vector_q1, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 vector_q2_yzx( __vpermwi( vector_q2, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_q2_zxy( __vpermwi( vector_q2, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    
    const __vector4 imag_scalar_x_vector( __vmaddfp( vector_q1, scalar_q2,
                                           __vmulfp( vector_q2, scalar_q1 ) ) );
    
    const __vector4 imag_vector_x_vector( __vnmsubfp( vector_q1_yzx, vector_q2_zxy,
                                            __vmulfp( vector_q1_zxy, vector_q2_yzx ) ) );
    
    const __vector4 imag( __vaddfp( imag_scalar_x_vector, imag_vector_x_vector ) );
    
    const __vector4 real_dp( __vmsum3fp( vector_q1, vector_q2 ) );
    const __vector4 real_dp_negated( __vxor( real_dp, sign_mask ) );
    const __vector4 real( __vmaddfp( scalar_q1, scalar_q2, real_dp_negated ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = __vrlimi( imag, real, VMX_MASK_INSERT_W, 0 );
    return result;
    
    #else
    const C_Quaternion& num = *this;
    const HF32 zero;
    const HF32 cosHalfTheta( num | denom );
    const C_Quaternion nearest_denom( Select( cosHalfTheta.GE( zero ), denom, -denom ) );
    return C_Quaternion( num * !nearest_denom );
    #endif
}

/// Shortest Division: num / denom = num * !denom
NOALIAS __forceinline const C_Quaternion ShortestDivide( const C_Quaternion PCREF num, const C_Quaternion PCREF denom )
{
    #if defined( _VMX128_ )
    
    const __vector4 zero( __vzero() );
    const __vector4 sign_mask( c_VectorSignMask );
    
    const __vector4 quat_num( num.m_V );
    const __vector4 quat_denom( denom.m_V );
    const __vector4 quat_minus_denom( __vxor( quat_denom, sign_mask ) );
    
    const __vector4 cosHalfTheta( __vmsum4fp( quat_num, quat_denom ) );
    const __vector4 cosHalfTheta_GE_zero( __vcmpgefp( cosHalfTheta, zero ) );
    const __vector4 quat_nearest_denom( __vsel( quat_minus_denom, quat_denom, cosHalfTheta_GE_zero ) );
    
    const __vector4 vector_q1( quat_num );
    const __vector4 scalar_q1( __vspltw( quat_num, VW ) );
    
    const __vector4 vector_q2( __vxor( quat_nearest_denom, sign_mask ) );
    const __vector4 scalar_q2( __vspltw( quat_nearest_denom, VW ) );
    
    const __vector4 vector_q1_yzx( __vpermwi( vector_q1, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_q1_zxy( __vpermwi( vector_q1, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 vector_q2_yzx( __vpermwi( vector_q2, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 vector_q2_zxy( __vpermwi( vector_q2, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    
    const __vector4 imag_scalar_x_vector( __vmaddfp( vector_q1, scalar_q2,
                                           __vmulfp( vector_q2, scalar_q1 ) ) );
    
    const __vector4 imag_vector_x_vector( __vnmsubfp( vector_q1_yzx, vector_q2_zxy,
                                            __vmulfp( vector_q1_zxy, vector_q2_yzx ) ) );
    
    const __vector4 imag( __vaddfp( imag_scalar_x_vector, imag_vector_x_vector ) );
    
    const __vector4 real_dp( __vmsum3fp( vector_q1, vector_q2 ) );
    const __vector4 real_dp_negated( __vxor( real_dp, sign_mask ) );
    const __vector4 real( __vmaddfp( scalar_q1, scalar_q2, real_dp_negated ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = __vrlimi( imag, real, VMX_MASK_INSERT_W, 0 );
    return result;
    
    #else
    const HF32 zero;
    const HF32 cosHalfTheta( num | denom );
    const C_Quaternion nearest_denom( Select( cosHalfTheta.GE( zero ), denom, -denom ) );
    return C_Quaternion( num * !nearest_denom );
    #endif
}

/// C_Quaternion Left Division: denom \ num = !denom * num
NOALIAS __forceinline const C_Quaternion LeftDivide( const C_Quaternion PCREF denom, const C_Quaternion PCREF num )
{
    return !denom * num;
}

/// C_Quaternion Right Division: num / denom = num * !denom
NOALIAS __forceinline const C_Quaternion RightDivide( const C_Quaternion PCREF num, const C_Quaternion PCREF denom )
{
    return num * !denom;
}

/// C_Quaternion Nearest Left Division: denom \ num = !denom * num  - negates denom if necessary
NOALIAS __forceinline const C_Quaternion NearestLeftDivide( const C_Quaternion PCREF denom, const C_Quaternion PCREF num )
{
    //return !denom * Nearest( denom, num );
    return !Nearest( num, denom ) * num;
}

/// C_Quaternion Nearest Right Division: num / denom = num * !denom  - negates denom if necessary
NOALIAS __forceinline const C_Quaternion NearestRightDivide( const C_Quaternion PCREF num, const C_Quaternion PCREF denom )
{
    return num * !Nearest( num, denom );
}


/// C_Quaternion Left Differential ( Differential of the left division of num by denom )
NOALIAS __forceinline const HVec3 LeftDifferential( const C_Quaternion PCREF denom, const C_Quaternion PCREF num )
{
    return Imag( !denom * num ) * Constants<HF32>::Two();
}

/// C_Quaternion Right Differential ( Differential of the right division of num by denom )
NOALIAS __forceinline const HVec3 RightDifferential( const C_Quaternion PCREF num, const C_Quaternion PCREF denom )
{
    return Imag( num * !denom ) * Constants<HF32>::Two();
}

/// C_Quaternion Nearest Left Differential ( Differential of the left division of num by denom )
NOALIAS __forceinline const HVec3 NearestLeftDifferential( const C_Quaternion PCREF denom, const C_Quaternion PCREF num )
{
    //return Imag( !denom * Nearest( denom, num ) ) * Const<VF32>::Two();
    return Imag( !Nearest( num, denom ) * num ) * Constants<HF32>::Two();
}

/// C_Quaternion Nearest Right Differential ( Differential of the right division of num by denom )
NOALIAS __forceinline const HVec3 NearestRightDifferential( const C_Quaternion PCREF num, const C_Quaternion PCREF denom )
{
    return Imag( num * !Nearest( num, denom ) ) * Constants<HF32>::Two();
}



/// Dot Product Operator: Returns 4 element dot product == cosine( angle / 2 ) between the 2 quaternions
NOALIAS __forceinline const HF32 operator | ( const C_Quaternion PCREF src0, const C_Quaternion PCREF src1 )
{
    #if defined( _VMX128_ )
    
    const __vector4 q0( src0.m_V );
    const __vector4 q1( src1.m_V );
    const __vector4 q0_dot_q1( __vmsum4fp( q0, q1 ) );
    
    HF32 result( UNDEFINED );
    result.m_F = q0_dot_q1;
    return result;
    
    #else
    return src0.m_V | src1.m_V;
    #endif
}

/// Normalise this Quaternion
NOALIAS __forceinline void C_Quaternion::Normalise() __restrict
{
    #if defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 vector_length_squared( __vmsum4fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 normal( __vmulfp( vector, vector_rcp_length ) );
	m_V = normal;
    
	#else
    m_V.Normalise();
    #endif
}

/// Returns normalised (unit length) copy of this Quaternion
NOALIAS __forceinline const C_Quaternion C_Quaternion::Normalised() const __restrict
{
    #if defined( _VMX128_ )
    
    const __vector4 vector( m_V );
    const __vector4 vector_length_squared( __vmsum4fp( vector, vector ) );
    __vector4 vector_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_length_squared, __vzero(), vector_rcp_length, valid );
    const __vector4 normal( __vmulfp( vector, vector_rcp_length ) );
    
	C_Quaternion result( UNDEFINED );
	result.m_V = normal;
    return result;
    
	#else
    return C_Quaternion( m_V.Normalised() );
    #endif
}

/// Lerp: Linear Interpolation
NOALIAS __forceinline const C_Quaternion C_Quaternion::Lerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict
{
    #if defined( _VMX128_ )
    
    const __vector4 vector_src( m_V );
    const __vector4 vector_dst( dst.m_V );
    const __vector4 scalar_t( t.m_F );
    const __vector4 vector_delta( __vsubfp( vector_dst, vector_src ) );
    const __vector4 vector_lerp( __vmaddfp( vector_delta, scalar_t, vector_src ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = vector_lerp;
    return result;
    
    #else
    const C_Quaternion& src = *this;
    const HF32 one( c_ScalarOne );
    return C_Quaternion( src + ( ( dst - src ) * t ) );
    #endif
}

/// Lerp: Linear Interpolation
NOALIAS __forceinline const C_Quaternion C_Quaternion::LerpNormalised( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict
{
    #if defined( _VMX128_ )
    
    const __vector4 vector_src( m_V );
    const __vector4 vector_dst( dst.m_V );
    const __vector4 scalar_t( t.m_F );
    const __vector4 vector_delta( __vsubfp( vector_dst, vector_src ) );
    const __vector4 vector_lerp( __vmaddfp( vector_delta, scalar_t, vector_src ) );
    
    const __vector4 vector_lerp_length_squared( __vmsum4fp( vector_lerp, vector_lerp ) );
    __vector4 vector_lerp_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_lerp_length_squared, __vzero(), vector_lerp_rcp_length, valid );
    const __vector4 vector_lerp_normal( __vmulfp( vector_lerp, vector_lerp_rcp_length ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = vector_lerp_normal;
    return result;
    
    #else
    const C_Quaternion& src = *this;
    const HF32 one( c_ScalarOne );
    const C_Quaternion lerp( src + ( ( dst - src ) * t ) );
    return lerp.Normalised();
    #endif
}

/// Shortest Lerp: Shortest Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const C_Quaternion C_Quaternion::ShortestLerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict
{
    #if defined( _VMX128_ )
    
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 scalar_t( t.m_F );
    const __vector4 vector_src( m_V );
    const __vector4 vector_initial_dst( dst.m_V );
    
    const __vector4 scalar_dp( __vmsum4fp( vector_src, vector_initial_dst ) );
    
    const __vector4 vector_negated_dst( __vxor( vector_initial_dst, sign_mask ) );
    
    const __vector4 scalar_dp_ge_zero( __vcmpgefp( scalar_dp, __vzero() ) );
    
    const __vector4 vector_dst( __vsel( vector_negated_dst, vector_initial_dst, scalar_dp_ge_zero ) );
    
    const __vector4 vector_delta( __vsubfp( vector_dst, vector_src ) );
    const __vector4 vector_lerp( __vmaddfp( vector_delta, scalar_t, vector_src ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = vector_lerp;
    return result;
    
    #else
    const C_Quaternion& src = *this;
    const HF32 zero;
    const HF32 cosHalfTheta( src | dst );
    const C_Quaternion nearestDst( Select( cosHalfTheta.GE( zero ), dst.m_V, (-dst).m_V ) );
    return Lerp( nearestDst, t );
    #endif
}

/// Shortest Lerp: Shortest Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const C_Quaternion C_Quaternion::ShortestLerpNormalised( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict
{
    #if defined( _VMX128_ )
    
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 scalar_t( t.m_F );
    const __vector4 vector_src( m_V );
    const __vector4 vector_initial_dst( dst.m_V );
    
    const __vector4 scalar_dp( __vmsum4fp( vector_src, vector_initial_dst ) );
    
    const __vector4 vector_negated_dst( __vxor( vector_initial_dst, sign_mask ) );
    
    const __vector4 scalar_dp_ge_zero( __vcmpgefp( scalar_dp, __vzero() ) );
    
    const __vector4 vector_dst( __vsel( vector_negated_dst, vector_initial_dst, scalar_dp_ge_zero ) );
    
    const __vector4 vector_delta( __vsubfp( vector_dst, vector_src ) );
    const __vector4 vector_lerp( __vmaddfp( vector_delta, scalar_t, vector_src ) );
    
    const __vector4 vector_lerp_length_squared( __vmsum4fp( vector_lerp, vector_lerp ) );
    __vector4 vector_lerp_rcp_length;
    __vector4 valid;
    VMX_VECTOR_RECIPROCAL_SQRT( vector_lerp_length_squared, __vzero(), vector_lerp_rcp_length, valid );
    const __vector4 vector_lerp_normal( __vmulfp( vector_lerp, vector_lerp_rcp_length ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = vector_lerp_normal;
    return result;
    
    #else
    const C_Quaternion& src = *this;
    const HF32 zero;
    const HF32 cosHalfTheta( src | dst );
    const C_Quaternion nearestDst( Select( cosHalfTheta.GE( zero ), dst.m_V, (-dst).m_V ) );
    return LerpNormalised( nearestDst, t );
    #endif
}

/// Slerp: Spherical Linear Interpolation
NOALIAS __forceinline const C_Quaternion C_Quaternion::Slerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict
{
    const C_Quaternion& src = *this;
    
    const C_Quaternion q_delta( dst - src );
    
    const HF32 zero(ZERO_INIT);
    const HF32 half( c_ScalarHalf );
    const HF32 one( c_ScalarOne );
    const HF32 minusOne( -one );
    const HF32 threshold( 0.0005f );
    const HF32 cosHalfTheta( src | dst );
    const HF32 oneMinusT( one - t );
    
    // From Fundamental Identity: sin^2(x) + cos^2(x) = 1
    const HF32 reciprocalSinHalfTheta( ReciprocalSqrt( one - ( cosHalfTheta * cosHalfTheta ) ) );
    
    const HF32 halfTheta( ArcCos( Clamped( cosHalfTheta, minusOne, one ) ) );
    const HVec4 angles( oneMinusT * halfTheta, t * halfTheta, zero, zero );
    const HVec4 sinAngles( Sin( angles ) );
    const HF32 scale0( sinAngles.X() );
    const HF32 scale1( sinAngles.Y() );
    
    const C_Quaternion result_slerp( ( ( src * scale0 ) + ( dst * scale1 ) ) * reciprocalSinHalfTheta );
    const C_Quaternion result_nlerp( ( src + ( q_delta * t ) ).Normalised() );
    
    return Select( ( one - cosHalfTheta ).GE( threshold ), result_slerp, result_nlerp );
}

/// Shortest Slerp: Spherical Linear Interpolation
NOALIAS __forceinline const C_Quaternion C_Quaternion::ShortestSlerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict
{
    const C_Quaternion& src = *this;
    const HF32 zero(ZERO_INIT);
    const HF32 cosHalfTheta( src | dst );
    const C_Quaternion nearestDst( Select( cosHalfTheta.GE( zero ), dst, -dst ) );
    return Slerp( nearestDst, t );
}

/// Nearest: Returns whichever one of the two equivalent targets is nearest to the source
NOALIAS __forceinline const C_Quaternion Nearest( const C_Quaternion PCREF source, const C_Quaternion PCREF target )
{
    #if defined( _VMX128_ )
    
    const __vector4 src( source.m_V );
    const __vector4 dst( target.m_V );
    const __vector4 dp( __vmsum4fp( src, dst ) );
    const __vector4 zero( __vzero() );
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 negated_dst( __vxor( dst, sign_mask ) );
    const __vector4 dp_ge_zero( __vcmpgefp( dp, zero ) );
    
    C_Quaternion result( UNDEFINED );
    result.m_V = __vsel( negated_dst, dst, dp_ge_zero );
    return result;
    
    #else
    
    const HF32 zero;
    const HF32 cosHalfTheta( source | target );
    return C_Quaternion( Select( cosHalfTheta.GE( zero ), target, -target ) );
    
    #endif
}

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
NOALIAS __forceinline const C_Quaternion Select( const Vector4_Bool PCREF selector, const C_Quaternion PCREF resultIfTrue, const C_Quaternion PCREF resultIfFalse )
{
    #if defined( _VMX128_ )
    
    C_Quaternion result( UNDEFINED );
    result.m_V = __vsel( resultIfFalse.m_V, resultIfTrue.m_V, selector.m_V );
    return result;
    
    #else
    return C_Quaternion( Select( selector, resultIfTrue.m_V, resultIfFalse.m_V ) );
    #endif
}

/// Multiplication Operator: C_Vector * C_Quaternion --> C_Vector
NOALIAS __forceinline const C_Vector operator * ( const C_Vector PCREF v, const C_Quaternion PCREF q )
{
    return C_Vector( v.m_V * q );
}

/// Multiplication Operator: Normal * C_Quaternion --> Normal
NOALIAS __forceinline const C_Normal operator * ( const C_Normal PCREF n, const C_Quaternion PCREF q )
{
    return C_Normal( n.m_V * q, C_Normal::UNIT_LENGTH_VECTOR );
}

/// Multiplication Operator: C_Quaternion * F32 --> C_Quaternion
NOALIAS __forceinline const C_Quaternion operator * ( const C_Quaternion PCREF q, const F32& f )
{
    return q * Load<HF32>( &f );
}

/// Multiplication Update Operator: C_Quaternion *= F32
NOALIAS __forceinline void operator *= ( C_Quaternion& q, const F32& f )
{
    q *= Load<HF32>( &f );
}

/// Division Operator: C_Quaternion / F32 --> C_Quaternion
NOALIAS __forceinline const C_Quaternion operator / ( const C_Quaternion PCREF q, const F32& f )
{
    //return q / HF32( f );
    return q * Reciprocal( Load<HF32>( &f ) );
}

/// Division Update Operator: C_Quaternion /= F32
NOALIAS __forceinline void operator /= ( C_Quaternion& q, const F32& f )
{
    //q /= HF32( f );
    q *= Reciprocal( Load<HF32>( &f ) );
}

/// Cast to HMat33 Rotation Matrix
NOALIAS __forceinline C_Quaternion::operator const HMat33 () const __restrict
{
    const C_Quaternion conjugate( !(*this) );
    
    HMat33 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float one  = Const< vector float >::One();
    
    const vector float xyza = conjugate.m_V;
    const vector float x = spu_splat<QX>( xyza );
    const vector float y = spu_splat<QY>( xyza );
    const vector float z = spu_splat<QZ>( xyza );
    const vector float a = spu_splat<QA>( xyza );
    
    const vector float _2x = spu_add( x, x );
    const vector float _2y = spu_add( y, y );
    const vector float _2z = spu_add( z, z );
    
    const vector float _2xx = spu_mul( _2x, x );
    const vector float _2xy = spu_mul( _2x, y );
    const vector float _2xz = spu_mul( _2x, z );
    
    const vector float _2yy = spu_mul( _2y, y );
    const vector float _2yz = spu_mul( _2y, z );
    const vector float _2zz = spu_mul( _2z, z );
    
    const vector float _2ax = spu_mul( _2x, a );
    const vector float _2ay = spu_mul( _2y, a );
    const vector float _2az = spu_mul( _2z, a );
    
    // ax = ( 1 - 2yy - 2zz,     2xy + 2az,     2xz - 2ay )
    // ay = (     2xy - 2az, 1 - 2xx - 2zz,     2yz + 2ax )
    // az = (     2xz + 2ay,     2yz - 2ax, 1 - 2xx - 2yy )
    
    const vector float ax_x = spu_sub( spu_sub( one, _2yy ), _2zz );
    const vector float ax_y = spu_add( _2xy, _2az );
    const vector float ax_z = spu_sub( _2xz, _2ay );
    
    const vector float ay_x = spu_sub( _2xy, _2az );
    const vector float ay_y = spu_sub( spu_sub( one, _2xx ), _2zz );
    const vector float ay_z = spu_add( _2yz, _2ax );
    
    const vector float az_x = spu_add( _2xz, _2ay );
    const vector float az_y = spu_sub( _2yz, _2ax );
    const vector float az_z = spu_sub( spu_sub( one, _2xx ), _2yy );
    
    //result.m_AX = (vector float)( spu_extract( ax_x, VX ), spu_extract( ax_y, VY ), spu_extract( ax_z, VZ ), 0.0f );
    //result.m_AY = (vector float)( spu_extract( ay_x, VX ), spu_extract( ay_y, VY ), spu_extract( ay_z, VZ ), 0.0f );
    //result.m_AZ = (vector float)( spu_extract( az_x, VX ), spu_extract( az_y, VY ), spu_extract( az_z, VZ ), 0.0f );
    result.m_AX = spu_insert( spu_extract( ax_x, VX ), spu_insert( spu_extract( ax_y, VY ), spu_insert( spu_extract( ax_z, VZ ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    result.m_AY = spu_insert( spu_extract( ay_x, VX ), spu_insert( spu_extract( ay_y, VY ), spu_insert( spu_extract( ay_z, VZ ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    result.m_AZ = spu_insert( spu_extract( az_x, VX ), spu_insert( spu_extract( az_y, VY ), spu_insert( spu_extract( az_z, VZ ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float one  = Const< vector float >::One();
    
    const vector float xyza = conjugate.m_V;
    const vector float x = vec_splat( xyza, QX );
    const vector float y = vec_splat( xyza, QY );
    const vector float z = vec_splat( xyza, QZ );
    const vector float a = vec_splat( xyza, QA );
    
    const vector float _2x = vec_add( x, x );
    const vector float _2y = vec_add( y, y );
    const vector float _2z = vec_add( z, z );
    
    const vector float _2xx = vec_mul( x, _2x );
    const vector float _2xy = vec_mul( x, _2y );
    const vector float _2xz = vec_mul( x, _2z );
    
    const vector float _2yy = vec_mul( y, _2y );
    const vector float _2yz = vec_mul( y, _2z );
    const vector float _2zz = vec_mul( z, _2z );
    
    const vector float _2ax = vec_mul( a, _2x );
    const vector float _2ay = vec_mul( a, _2y );
    const vector float _2az = vec_mul( a, _2z );
    
    // ax = ( 1 - 2yy - 2zz,     2xy + 2az,     2xz - 2ay )
    // ay = (     2xy - 2az, 1 - 2xx - 2zz,     2yz + 2ax )
    // az = (     2xz + 2ay,     2yz - 2ax, 1 - 2xx - 2yy )
    
    const vector float ax_x = vec_sub( vec_sub( one, _2yy ), _2zz );
    const vector float ax_y = vec_add( _2xy, _2az );
    const vector float ax_z = vec_sub( _2xz, _2ay );
    
    const vector float ay_x = vec_sub( _2xy, _2az );
    const vector float ay_y = vec_sub( vec_sub( one, _2xx ), _2zz );
    const vector float ay_z = vec_add( _2yz, _2ax );
    
    const vector float az_x = vec_add( _2xz, _2ay );
    const vector float az_y = vec_sub( _2yz, _2ax );
    const vector float az_z = vec_sub( vec_sub( one, _2xx ), _2yy );
    
    result.m_AX = vec_mergeh( vec_mergeh( ax_x, ax_z ), vec_mergeh( ax_y, zero ) );
    result.m_AY = vec_mergeh( vec_mergeh( ay_x, ay_z ), vec_mergeh( ay_y, zero ) );
    result.m_AZ = vec_mergeh( vec_mergeh( az_x, az_z ), vec_mergeh( az_y, zero ) );
    
    #elif defined( _SIMD_ )
    
    #if defined( _VMX128_ )
    const V128 xyza( conjugate.m_V );
    #else
    const V128 xyza( conjugate.m_V.m_V );
    #endif
    
    const V128 xyza2( SIMD_Add( xyza, xyza ) );
    
    const HVec3 ax( SIMD_Vector3_Vector4( SIMD_Add( c_UnitVectorX, SIMD_Add( SIMD_Multiply( SIMD_Multiply( SIMD_Permute<VY,VX,VX,0>( xyza ),
                                                                                                           SIMD_Permute<VY,VY,VZ,0>( xyza2 ) ),
                                                                                                           SIMD_Permute<1,0,0,3>( c_VectorP1M1ZZ ) ),
                                                                             SIMD_Multiply( SIMD_Multiply( SIMD_Permute<VZ,VW,VW,0>( xyza ),
                                                                                                           SIMD_Permute<VZ,VZ,VY,0>( xyza2 ) ),
                                                                                                           SIMD_Permute<1,0,1,3>( c_VectorP1M1ZZ ) ) ) ) ) );
    
    const HVec3 ay( SIMD_Vector3_Vector4( SIMD_Add( c_UnitVectorY, SIMD_Add( SIMD_Multiply( SIMD_Multiply( SIMD_Permute<VX,VX,VY,0>( xyza ),
                                                                                                           SIMD_Permute<VY,VX,VZ,0>( xyza2 ) ),
                                                                                                           SIMD_Permute<0,1,0,3>( c_VectorP1M1ZZ ) ),
                                                                             SIMD_Multiply( SIMD_Multiply( SIMD_Permute<VW,VZ,VW,0>( xyza ),
                                                                                                           SIMD_Permute<VZ,VZ,VX,0>( xyza2 ) ),
                                                                                                           SIMD_Permute<1,1,0,3>( c_VectorP1M1ZZ ) ) ) ) ) );
    
    const HVec3 az( SIMD_Vector3_Vector4( SIMD_Add( c_UnitVectorZ, SIMD_Add( SIMD_Multiply( SIMD_Multiply( SIMD_Permute<VX,VY,VX,0>( xyza ),
                                                                                                           SIMD_Permute<VZ,VZ,VX,0>( xyza2 ) ),
                                                                                                           SIMD_Permute<0,0,1,3>( c_VectorP1M1ZZ ) ),
                                                                             SIMD_Multiply( SIMD_Multiply( SIMD_Permute<VW,VW,VY,0>( xyza ),
                                                                                                           SIMD_Permute<VY,VX,VY,0>( xyza2 ) ),
                                                                                                           SIMD_Permute<0,1,1,3>( c_VectorP1M1ZZ ) ) ) ) ) );
    
    // (1,0,0) + (y,x,x) * 2(y,y,z) * (-1,+1,+1) + (z,a,a) * 2(z,z,y) * (-1,+1,-1)
    // (0,1,0) + (x,x,y) * 2(y,x,z) * (+1,-1,+1) + (a,z,a) * 2(z,z,x) * (-1,-1,+1)
    // (0,0,1) + (x,y,x) * 2(z,z,x) * (+1,+1,-1) + (a,a,y) * 2(y,x,y) * (+1,-1,-1)
    // ax = ( 1 - 2yy - 2zz,     2xy + 2az,     2xz - 2ay )
    // ay = (     2xy - 2az, 1 - 2xx - 2zz,     2yz + 2ax )
    // az = (     2xz + 2ay,     2yz - 2ax, 1 - 2xx - 2yy )
    
    result.AX( ax );
    result.AY( ay );
    result.AZ( az );
    
    #else
    
    // calculate coefficients
    
    const F32 _2x = conjugate.m_V[QX] + conjugate.m_V[QX]; // quat->x + quat->x;
    const F32 _2y = conjugate.m_V[QY] + conjugate.m_V[QY]; // quat->y + quat->y; 
    const F32 _2z = conjugate.m_V[QZ] + conjugate.m_V[QZ]; // quat->z + quat->z;
    
    const F32 _2xx = conjugate.m_V[QX] * _2x; // quat->x * 2x;
    const F32 _2xy = conjugate.m_V[QX] * _2y; // quat->x * 2y;
    const F32 _2xz = conjugate.m_V[QX] * _2z; // quat->x * 2z;
                                                                
    const F32 _2yy = conjugate.m_V[QY] * _2y; // quat->y * 2y;
    const F32 _2yz = conjugate.m_V[QY] * _2z; // quat->y * 2z;
    const F32 _2zz = conjugate.m_V[QZ] * _2z; // quat->z * 2z;
                                                                
    const F32 _2ax = conjugate.m_V[QA] * _2x; // quat->a * 2x;
    const F32 _2ay = conjugate.m_V[QA] * _2y; // quat->a * 2y;
    const F32 _2az = conjugate.m_V[QA] * _2z; // quat->a * 2z;
    
    const HVec3 ax( 1.0f - _2yy - _2zz,        _2xy + _2az,        _2xz - _2ay );
    const HVec3 ay(        _2xy - _2az, 1.0f - _2xx - _2zz,        _2yz + _2ax );
    const HVec3 az(        _2xz + _2ay,        _2yz - _2ax, 1.0f - _2xx - _2yy );
    
    result.m_A[MX] = ax;
    result.m_A[MY] = ay;
    result.m_A[MZ] = az;
    
    #endif
    
    return result;
}

/// Finite Test
NOALIAS __forceinline const Scalar_Bool C_Quaternion::Finite() const __restrict
{
    #if defined( _VMX128_ )
    return HVec4( m_V ).Finite();
    #else
    return m_V.Finite();
    #endif
}
    
/// Read from Stream
template < typename T_Stream > __forceinline void C_Quaternion::Deserialise( T_Stream& stream ) __restrict
{
#if defined( _VMX128_ )
    #if defined( _SPU_ )
    F32 x, y, z, w;
    stream >> x;
    stream >> y;
    stream >> z;
    stream >> w;
    //m_V = (vector float){ x, y, z, w };
    m_V = spu_insert( x, spu_insert( y, spu_insert( z, spu_promote( w, QA ), QZ ), QY ), QX );
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
    stream >> m_V[QX];
    stream >> m_V[QY];
    stream >> m_V[QZ];
    stream >> m_V[QA];
    #endif
#else
	stream >> m_V;
#endif
}

/// Write to Stream
template < typename T_Stream > __forceinline void C_Quaternion::Serialise( T_Stream& stream ) __restrict
{
#if defined( _VMX128_ )
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<QX>( m_V );
    stream << (F32)result;
    result.m_F = spu_splat<QY>( m_V );
    stream << (F32)result;
    result.m_F = spu_splat<QZ>( m_V );
    stream << (F32)result;
    result.m_F = spu_splat<QA>( m_V );
    stream << (F32)result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
	result.m_F = vec_splat( m_V, QX );
    stream << (F32)result;
    result.m_F = vec_splat( m_V, QY );
    stream << (F32)result;
    result.m_F = vec_splat( m_V, QZ );
    stream << (F32)result;
    result.m_F = vec_splat( m_V, QA );
    stream << (F32)result;
    #elif defined( _SIMD_ )
    stream << (F32)HF32( SIMD_Scalar_Vector4X( m_V ) );
    stream << (F32)HF32( SIMD_Scalar_Vector4Y( m_V ) );
    stream << (F32)HF32( SIMD_Scalar_Vector4Z( m_V ) );
    stream << (F32)HF32( SIMD_Scalar_Vector4W( m_V ) );
    #else
    stream << (F32)m_V[QX];
    stream << (F32)m_V[QY];
    stream << (F32)m_V[QZ];
    stream << (F32)m_V[QA];
    #endif
#else
	stream << m_V;
#endif
}







//
// -------------------------------------------------------------
// |-- C_QuatTrans : Quaternion rotation & translation class --|
// -------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_QuatTrans::C_QuatTrans()
{
}

/// Copy Constructor
NOALIAS __forceinline C_QuatTrans::C_QuatTrans( const C_QuatTrans& copy ) :
    m_Quaternion( copy.m_Quaternion ),
    m_Translation( copy.m_Translation )
{
}

/// Copy Assignment
NOALIAS __forceinline const C_QuatTrans& C_QuatTrans::operator = ( const C_QuatTrans& copy )
{
    m_Quaternion = copy.m_Quaternion;
    m_Translation = copy.m_Translation;
    return copy;
}

/// Quaternion & HVec3 Constructor
NOALIAS __forceinline C_QuatTrans::C_QuatTrans( const C_Quaternion PCREF q, const HVec3 PCREF t ) :
    m_Quaternion( q ),
    m_Translation( t )
{
}

/// HMat43 Constructor
NOALIAS __forceinline C_QuatTrans::C_QuatTrans( const HMat43& matrix ) :
    m_Quaternion( matrix.Rotation() ),
    m_Translation( matrix.Translation() )
{
}

/// Accessor: Get Quaternion
NOALIAS __forceinline const C_Quaternion C_QuatTrans::Quaternion() const __restrict
{
    return m_Quaternion;
}

/// Accessor: Get Translation
NOALIAS __forceinline const HVec3 C_QuatTrans::Translation() const __restrict
{
    return m_Translation;
}

/// Accessor: Set Quaternion
NOALIAS __forceinline void C_QuatTrans::Quaternion( const C_Quaternion PCREF q ) __restrict
{
    m_Quaternion = q;
}

/// Accessor: Set Translation
NOALIAS __forceinline void C_QuatTrans::Translation( const HVec3 PCREF t ) __restrict
{
    m_Translation = t;
}

/// Accessor: Convert to HMat43
NOALIAS __forceinline const HMat43 C_QuatTrans::Matrix() const __restrict
{
    return HMat33( m_Quaternion ) || m_Translation;
}

/// Read from Stream
template < typename T_Stream > __forceinline void C_QuatTrans::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_Quaternion;
    stream >> m_Translation;
}

/// Write to Stream
template < typename T_Stream > __forceinline void C_QuatTrans::Serialise( T_Stream& stream ) __restrict
{
    stream << m_Quaternion;
    stream << m_Translation;
}

/// Multiplication Operator: C_QuatTrans * C_QuatTrans --> C_QuatTrans
NOALIAS __forceinline const C_QuatTrans operator * ( const C_QuatTrans& qt1, const C_QuatTrans& qt2 )
{
    return C_QuatTrans( qt2.m_Quaternion * qt1.m_Quaternion, ( qt1.m_Translation * qt2.m_Quaternion ) + qt2.m_Translation );
}

/// Multiplication Operator: C_Vertex * C_QuatTrans --> C_Vertex
NOALIAS __forceinline const C_Vertex operator * ( const C_Vertex PCREF v, const C_QuatTrans& qt )
{
    return C_Vertex( ( v.m_V * qt.m_Quaternion ) + qt.m_Translation );
}

/// Inverse: C_QuatTrans
NOALIAS __forceinline const C_QuatTrans Inverse( const C_QuatTrans& qt )
{
    const C_Quaternion qConjugate( !qt.m_Quaternion );
    return C_QuatTrans( qConjugate, -qt.m_Translation * qConjugate );
}

/// Nearest: Returns whichever one of the two equivalent targets is nearest to the source
NOALIAS __forceinline const C_QuatTrans Nearest( const C_QuatTrans& source, const C_QuatTrans& target )
{
    return C_QuatTrans( Nearest( source.m_Quaternion, target.m_Quaternion ), target.m_Translation );
}

//FORCE_INLINE V128	MulVec3Quat ( const V128 vector_v, const V128 vector_q )
//{
//    #if defined( _VMX_ )
//    
//    const __vector4 sign_mask( c_VectorSignMask );
//    
//    // QV = Q * V
//    
//    const __vector4 scalar_q( __vspltw( vector_q, VW ) );
//    //const __vector4 scalar_v( __vzero() );
//    
//    const __vector4 vector_q_yzx( __vpermwi( vector_q, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
//    const __vector4 vector_q_zxy( __vpermwi( vector_q, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
//    const __vector4 vector_v_yzx( __vpermwi( vector_v, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
//    const __vector4 vector_v_zxy( __vpermwi( vector_v, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
//    
//    //const __vector4 vector_qv_scalar_x_vector( __vmaddfp( vector_v, scalar_q, __vmulfp( vector_q, scalar_v ) ) );
//    const __vector4 vector_qv_scalar_x_vector( __vmulfp( vector_v, scalar_q ) ); // vector_q * scalar_v = 0
//    const __vector4 vector_qv_vector_x_vector( __vnmsubfp( vector_q_yzx, vector_v_zxy, __vmulfp( vector_q_zxy, vector_v_yzx ) ) );
//    const __vector4 vector_qv( __vaddfp( vector_qv_scalar_x_vector, vector_qv_vector_x_vector ) );
//    
//    const __vector4 scalar_qv_dp( __vmsum3fp( vector_v, vector_q ) );
//    const __vector4 scalar_qv_dp_negated( __vxor( scalar_qv_dp, sign_mask ) );
//    //const __vector4 scalar_qv( __vmaddfp( scalar_v, scalar_q, scalar_qv_dp_negated ) );
//    const __vector4 scalar_qv( scalar_qv_dp_negated ); // scalar_v * scalar_q = 0
//    
//    // QC = !Q
//    
//    const __vector4 vector_qc( __vxor( vector_q, sign_mask ) );
//    const __vector4 scalar_qc( __vspltw( vector_q, VW ) );
//    
//    // QVQC = QV * QC
//    
//    const __vector4 vector_qv_yzx( __vpermwi( vector_qv, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
//    const __vector4 vector_qv_zxy( __vpermwi( vector_qv, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
//    const __vector4 vector_qc_yzx( __vpermwi( vector_qc, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
//    const __vector4 vector_qc_zxy( __vpermwi( vector_qc, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
//    
//    const __vector4 vector_qvqc_scalar_x_vector( __vmaddfp( vector_qv, scalar_qc, __vmulfp( vector_qc, scalar_qv ) ) );
//    const __vector4 vector_qvqc_vector_x_vector( __vnmsubfp( vector_qv_yzx, vector_qc_zxy, __vmulfp( vector_qv_zxy, vector_qc_yzx ) ) );
//    const __vector4 vector_qvqc( __vaddfp( vector_qvqc_scalar_x_vector, vector_qvqc_vector_x_vector ) );
//    
//    //const __vector4 scalar_qvqc_dp( __vmsum3fp( vector_qv, vector_qc ) );
//    //const __vector4 scalar_qvqc_dp_negated( __vxor( scalar_qvqc_dp, sign_mask ) );
//    //const __vector4 scalar_qvqc( __vmaddfp( scalar_qv, scalar_qc, scalar_qvqc_dp_negated ) );
//    
//    V128 result = __vrlimi( vector_qvqc, __vzero(), VMX_MASK_INSERT_W, 0 );
//    return result;
//    
//    #else
//	C_Quaternion _q(vector_q);
//	HVec3	_v(vector_v);
//    return ( _q * C_Quaternion( _v ) * !_q ).Imag();
//    #endif
//}





}; // namespace Maths

#include "simd.h"
namespace Maths 
{
//__forceinline
//C_Quaternion&	operator*=(C_Quaternion& a, const V128 PCREF b) {
	//a.SetV128(SIMD_Multiply(a.GetV128(), b));
	//return	a;
//}
}

#endif




