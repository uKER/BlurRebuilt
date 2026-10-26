#include "CorePch.h"
#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>

namespace Maths
{
/// @addtogroup Quaternion
//! @{

/// Make unit rotation Quaternion from a Mat33 rotation matrix
NOALIAS const Quaternion MakeUnitQuaternion( const Mat33 MATREF m )
{    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 half( Const<VF32>::Half() );
    const Vector4 v_one( Const<Vector4>::One() );
    
    const VF32 m00( Element< E_RowX, E_ColX >( m ) );
    const VF32 m10( Element< E_RowX, E_ColY >( m ) );
    const VF32 m20( Element< E_RowX, E_ColZ >( m ) );
    const VF32 m01( Element< E_RowY, E_ColX >( m ) );
    const VF32 m11( Element< E_RowY, E_ColY >( m ) );
    const VF32 m21( Element< E_RowY, E_ColZ >( m ) );
    const VF32 m02( Element< E_RowZ, E_ColX >( m ) );
    const VF32 m12( Element< E_RowZ, E_ColY >( m ) );
    const VF32 m22( Element< E_RowZ, E_ColZ >( m ) );
    
    const Vector4 t0( NegateYZ( MakeVector4( m00 ) ) );
    const Vector4 t1( NegateXZ( MakeVector4( m11 ) ) );
    const Vector4 t2( NegateXY( MakeVector4( m22 ) ) );
    const Vector4 trace( ( v_one + t0 ) + ( t1 + t2 ) );
    
    const Vector4 sqrt_trace( Sqrt24( trace ) );
    
    const VF32 trace_x( X( trace ) );
    const VF32 trace_y( Y( trace ) );
    const VF32 trace_z( Z( trace ) );
    const VF32 trace_w( W( trace ) );
    
    const VBool master_x( ( trace_x > trace_y ) &
                          ( trace_x > trace_z ) &
                          ( trace_x > trace_w ) );
    const VBool master_y( ( trace_y > trace_z ) &
                          ( trace_y > trace_w ) );
    const VBool master_z( ( trace_z > trace_w ) );
    
    const VF32 m10_plus_m01( m10 + m01 );
    const VF32 m21_plus_m12( m21 + m12 );
    const VF32 m02_plus_m20( m02 + m20 );
    const VF32 m10_minus_m01( m10 - m01 );
    const VF32 m21_minus_m12( m21 - m12 );
    const VF32 m02_minus_m20( m02 - m20 );
    
    const Vector4 master_x_sign_xyzw( MakeVector4( zero,          m10_plus_m01,  m02_plus_m20,  m21_minus_m12 ) );
    const Vector4 master_y_sign_xyzw( MakeVector4( m10_plus_m01,  zero,          m21_plus_m12,  m02_minus_m20 ) );
    const Vector4 master_z_sign_xyzw( MakeVector4( m02_plus_m20,  m21_plus_m12,  zero,          m10_minus_m01 ) );
    const Vector4 master_w_sign_xyzw( MakeVector4( m21_minus_m12, m02_minus_m20, m10_minus_m01, zero          ) );
    
    const Vector4 sign_xyzw( Select( master_x, master_x_sign_xyzw,
                             Select( master_y, master_y_sign_xyzw,
                             Select( master_z, master_z_sign_xyzw,
                                               master_w_sign_xyzw ) ) ) );
    
    const Vector4 xyzw( CopySign( sqrt_trace * half, sign_xyzw ) );
    
    return MakeQuaternion( Select( W( xyzw ) >= zero, xyzw, -xyzw ) );
}

/// Make orthonormal rotation Mat33 from Quaternion
NOALIAS const Mat33 MakeMat33( const Quaternion PCREF q )
{
    #if 0
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    
    const VF32 qx( X( q.m_Q ) );
    const VF32 qy( Y( q.m_Q ) );
    const VF32 qz( Z( q.m_Q ) );
    const VF32 qw( W( q.m_Q ) );
    
    const VF32 _2x( qx + qx );
    const VF32 _2y( qy + qy ); 
    const VF32 _2z( qz + qz );
    
    const VF32 _2xx( qx * _2x );
    const VF32 _2xy( qx * _2y );
    const VF32 _2xz( qx * _2z );
                                
    const VF32 _2yy( qy * _2y );
    const VF32 _2yz( qy * _2z );
    const VF32 _2zz( qz * _2z );
                                
    const VF32 _2ax( qw * _2x );
    const VF32 _2ay( qw * _2y );
    const VF32 _2az( qw * _2z );
    
    const Vec4F32 row_x( MakeVec4F32( one - _2yy - _2zz,       _2xy + _2az,       _2xz - _2ay, zero ) );
    const Vec4F32 row_y( MakeVec4F32(       _2xy - _2az, one - _2xx - _2zz,       _2yz + _2ax, zero ) );
    const Vec4F32 row_z( MakeVec4F32(       _2xz + _2ay,       _2yz - _2ax, one - _2xx - _2yy, zero ) );
    
    Mat33 result;
    SetRows( &result, row_x, row_y, row_z );
    return result;
    
    #else
    
    const Vector3 unit_x( Const<Vector3>::UnitX() );
    const Vector3 unit_y( Const<Vector3>::UnitY() );
    const Vector3 unit_z( Const<Vector3>::UnitZ() );
    
    const Vec4F32 xyza( q.m_Q );
    const Vec4F32 _2xyza( xyza + xyza );
    const Vec4F32 _2x_xyza( X(xyza) * _2xyza );
    const Vec4F32 _2y_xyza( Y(xyza) * _2xyza );
    const Vec4F32 _2z_xyza( Z(xyza) * _2xyza );
    
    const VF32 _2xx( X( _2x_xyza ) );
    const VF32 _2xy( Y( _2x_xyza ) );
    const VF32 _2xz( Z( _2x_xyza ) );
    
    const VF32 _2yy( Y( _2y_xyza ) );
    const VF32 _2yz( Z( _2y_xyza ) );
    const VF32 _2zz( Z( _2z_xyza ) );
    
    const VF32 _2ax( W( _2x_xyza ) );
    const VF32 _2ay( W( _2y_xyza ) );
    const VF32 _2az( W( _2z_xyza ) );
    
    
    //const Vec4F32 row_x( MakeVec4F32( one - _2yy - _2zz,
    //                                        _2xy + _2az,
    //                                        _2xz - _2ay, zero ) );
    
    //const Vec4F32 row_y( MakeVec4F32(       _2xy - _2az,
    //                                  one - _2xx - _2zz,
    //                                        _2yz + _2ax, zero ) );
    
    //const Vec4F32 row_z( MakeVec4F32(       _2xz + _2ay,
    //                                        _2yz - _2ax,
    //                                  one - _2xx - _2yy, zero ) );
    
    const Vector3 row_x( unit_x + NegateX( MakeVector3( _2yy, _2xy, _2xz ) ) + NegateXZ( MakeVector3( _2zz, _2az, _2ay ) ) );
    const Vector3 row_y( unit_y + NegateY( MakeVector3( _2xy, _2xx, _2yz ) ) + NegateXY( MakeVector3( _2az, _2zz, _2ax ) ) );
    const Vector3 row_z( unit_z + NegateZ( MakeVector3( _2xz, _2yz, _2xx ) ) + NegateYZ( MakeVector3( _2ay, _2ax, _2yy ) ) );
    
    Mat33 result;
    SetRows( &result, row_x, row_y, row_z );
    return result;
    
    
    #endif
}


//! @}
}; // namespace Maths


