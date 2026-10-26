#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

namespace Maths
{

//
// -------------------------------------------------------
// |-- HMat33 : Hardware 3 Row * 3 Column Matrix Class --|
// -------------------------------------------------------
//

// Cross products for deriving third matrix axis from 2 input axes
// const HVec3 ax = ay ^ az;
// const HVec3 ay = az ^ ax;
// const HVec3 az = ax ^ ay;

/// X Axis Rotation Constructor - clockwise when viewed back down the X axis
NOALIAS HMat33::HMat33( const RotX PCREF alpha )
{
    const HF32 cx = Cos( (HF32)alpha );
    const HF32 sx = Sin( (HF32)alpha );
    #if defined( _SSE_ )
    const __m128 ay = _mm_setr_ps( 0.0f,  cx, sx, 0.0f );
    const __m128 az = _mm_setr_ps( 0.0f, -sx, cx, 0.0f );
    #else
    const Maths::V128 ay = { 0.0f,  cx, sx };
    const Maths::V128 az = { 0.0f, -sx, cx };
    #endif
    SetAxes( HVec3( c_UnitVectorX ), HVec3( ay ), HVec3( az ) );
}

/// Y Axis Rotation Constructor - clockwise when viewed back down the Y axis
NOALIAS HMat33::HMat33( const RotY PCREF beta )
{
    const HF32 cy = Cos( (HF32)beta );
    const HF32 sy = Sin( (HF32)beta );
    #if defined( _SSE_ )
    const __m128 ax = _mm_setr_ps( cy, 0.0f, -sy, 0.0f );
    const __m128 az = _mm_setr_ps( sy, 0.0f,  cy, 0.0f );
    #else
    const Maths::V128 ax = { cy, 0.0f, -sy, 0.0f };
    const Maths::V128 az = { sy, 0.0f,  cy, 0.0f };
    #endif
    SetAxes( HVec3( ax ), HVec3( c_UnitVectorY ), HVec3( az ) );
}

/// Z Axis Rotation Constructor - clockwise when viewed back down the Z axis
NOALIAS HMat33::HMat33( const RotZ PCREF gamma )
{
    const HF32 cz = Cos( (HF32)gamma );
    const HF32 sz = Sin( (HF32)gamma );
    #if defined( _SSE_ )
    const __m128 ax = _mm_setr_ps(  cz, sz, 0.0f, 0.0f );
    const __m128 ay = _mm_setr_ps( -sz, cz, 0.0f, 0.0f );
    #else
    const Maths::V128 ax = {  cz, sz, 0.0f };
    const Maths::V128 ay = { -sz, cz, 0.0f };
    #endif
    SetAxes( HVec3( ax ), HVec3( ay ), HVec3( c_UnitVectorZ ) );
}

/// Constructor - Rotation matrix from euler angles - Roll-Pitch-Heading
NOALIAS HMat33::HMat33( const C_MatrixRotateRPH PCREF euler )
{
    // Matrix RPH =
    //            [col][X]                               [col][Y]        [col][Z]
    // [X][row](  cos(r)*cos(h) + sin(r)*sin(p)*sin(h),  sin(r)*cos(p), -cos(r)*sin(h) + sin(r)*sin(p)*cos(h) );
    // [Y][row]( -sin(r)*cos(h) + cos(r)*sin(p)*sin(h),  cos(r)*cos(p),  sin(r)*sin(h) + cos(r)*sin(p)*cos(h) );
    // [Z][row](  cos(p)*sin(h),                        -sin(p),         cos(p)*cos(h) );
    
    const HF32 r = euler.Roll();
    const HF32 p = euler.Pitch();
    const HF32 h = euler.Heading();
    
    const HF32 cr( Cos( r ) );
    const HF32 sr( Sin( r ) );
    const HF32 cp( Cos( p ) );
    const HF32 sp( Sin( p ) );
    const HF32 ch( Cos( h ) );
    const HF32 sh( Sin( h ) );
    
    const HVec3 ax(  cr*ch + sr*sp*sh,  sr*cp, -cr*sh + sr*sp*ch );
    const HVec3 ay( -sr*ch + cr*sp*sh,  cr*cp,  sr*sh + cr*sp*ch );
    const HVec3 az(  cp*sh,            -sp,     cp*ch );
    
    SetAxes( ax, ay, az );
}

#if 0 // DG - Moved to be inlined
/// Constructor - Rotation matrix from euler angles - Pitch-Heading-Roll
NOALIAS HMat33::HMat33( const C_MatrixRotatePHR PCREF euler )
{
    // Matrix PHR =
    //           [col][X]                            [col][Y]                            [col][Z]
    // [X][row]( cos(h)*cos(r),                      cos(h)*sin(r),                      -sin(h)        )
    // [Y][row]( sin(p)*sin(h)*cos(r)-cos(p)*sin(r), sin(p)*sin(h)*sin(r)+cos(p)*cos(r),  sin(p)*cos(h) )
    // [Z][row]( cos(p)*sin(h)*cos(r)+sin(p)*sin(r), cos(p)*sin(h)*sin(r)-sin(p)*cos(r),  cos(p)*cos(h) )
    
    const HF32 p = euler.Pitch();
    const HF32 h = euler.Heading();
    const HF32 r = euler.Roll();
    
    const HF32 cp( Cos( p ) ); // TODO: Use the HVec4 form of Cos & Sin to do these in parallel!!!
    const HF32 sp( Sin( p ) );
    const HF32 ch( Cos( h ) );
    const HF32 sh( Sin( h ) );
    const HF32 cr( Cos( r ) );
    const HF32 sr( Sin( r ) );
    
    const HVec3 ax( ch*cr,          ch*sr,          -sh    );
    const HVec3 ay( sp*sh*cr-cp*sr, sp*sh*sr+cp*cr,  sp*ch );
    const HVec3 az( cp*sh*cr+sp*sr, cp*sh*sr-sp*cr,  cp*ch );
    
    SetAxes( ax, ay, az );
}
#endif

/// Conversion to Euler angles - Roll-Pitch-Heading
NOALIAS const C_MatrixRotateRPH HMat33::GetRotationRPH() const __restrict
{
    // m[Z][Y] = -sin(p) --> p = asin( -m[Z][Y]
    
    #if defined( _VMX128_ ) || defined( _SPU_ ) || defined( _PPU_ )
    const HF32 sinPitch( -AZ().Y() ); // TODO: Single accessor for extracting a float element of a matrix!
    #elif defined( _SSE_ )
    const HF32 sinPitch( -m_AZ.Y() );
    #else
    const HF32 sinPitch( -m_A[MZ].Y() );
    #endif
    
    const HF32 p( ArcSin( sinPitch ) );
    HF32 h;
    HF32 r;
    
    if ( Abs( (HF32)sinPitch ) < 0.999f )
    {
        // m[X][Y] / m[Y][Y} = sin(r)cos(p) / cos(r)cos(p)
        //                   = sin(r) / cos(r)
        //                   = tan(r)
        // -->             r = atan( m[X][Y] / m[Y][Y] )
        
        // m[Z][X] / m[Z][Z] = cos(p)sin(h) / cos(p)cos(h)
        //                   = sin(h) / cos(h)
        //                   = tan(h)
        // -->             h = atan( m[Z][X] / m[Z][Z] )
        
        #if defined( _VMX128_ ) || defined( _SPU_ ) || defined( _PPU_ )
        const HF32 tanRollSin( AX().Y() );    // TODO: Single accessor for extracting a float element of a matrix!
        const HF32 tanRollCos( AY().Y() );    // TODO: Single accessor for extracting a float element of a matrix!
        const HF32 tanHeadingSin( AZ().X() ); // TODO: Single accessor for extracting a float element of a matrix!
        const HF32 tanHeadingCos( AZ().Z() ); // TODO: Single accessor for extracting a float element of a matrix!
        #elif defined( _SSE_ )
        const HF32 tanRollSin( m_AX.Y() );
        const HF32 tanRollCos( m_AY.Y() );
        const HF32 tanHeadingSin( m_AZ.X() );
        const HF32 tanHeadingCos( m_AZ.Z() );
        #else
        const HF32 tanRollSin( m_A[MX].Y() );
        const HF32 tanRollCos( m_A[MY].Y() );
        const HF32 tanHeadingSin( m_A[MZ].X() );
        const HF32 tanHeadingCos( m_A[MZ].Z() );
        #endif
        
        h = ArcTan( tanHeadingSin, tanHeadingCos );
        r = ArcTan( tanRollSin, tanRollCos );
    }
    else
    {
        // sin(p) ~= 1 therefore cos(p) ~= 0
        // m[Y][X] / m[Y][Z] = -sin(r)cos(h) + cos(r)sin(p)sin(h) / sin(r)sin(h) + cos(r)sin(p)cos(h)
        //                   = sin(h)cos(r) - cos(h)sin(r)        / cos(h)cos(r) + sin(h)sin(r)
        // using    sin(x-y) = sin(x)cos(y) - cos(x)sin(y)
        // and      cos(x-y) = cos(x)cos(y) + sin(x)sin(y)
        //                   = sin(h-r)                           / cos(h-r)
        // [ at poles h equivalent to r therefore assume r = 0 ]
        //                   = sin(h) / cos(h)
        //                 h = atan( sin(h) / cos(h) )
        //                 h = atan( m[Y][X] / m[Y][Z]
        
        #if defined( _VMX128_ ) || defined( _SPU_ ) || defined( _PPU_ )
        const HF32 tanHeadingSin( AY().X() ); // TODO: Single accessor for extracting a float element of a matrix!
        const HF32 tanHeadingCos( AY().Z() ); // TODO: Single accessor for extracting a float element of a matrix!
        #elif defined( _SSE_ )
        const HF32 tanHeadingSin( m_AY.X() );
        const HF32 tanHeadingCos( m_AY.Z() );
        #else
        const HF32 tanHeadingSin( m_A[MY].X() );
        const HF32 tanHeadingCos( m_A[MY].Z() );
        #endif
        
        h = (HF32)ArcTan( tanHeadingSin, tanHeadingCos );
        r = 0.0f;
    }
    
    return C_MatrixRotateRPH( r, p, h );
}

/// Conversion to Euler angles - Pitch-Heading-Roll
NOALIAS const C_MatrixRotatePHR HMat33::GetRotationPHR() const __restrict
{
    // m[X][Z] = -sin(h) --> h = asin( -m[X][Z]
    
    #if defined( _VMX128_ ) || defined( _SPU_ ) || defined( _PPU_ )
    const HF32 sinHeading( -AX().Z() ); // TODO: Single accessor for extracting a float element of a matrix!
    #elif defined( _SSE_ )
    const HF32 sinHeading( -m_AX.Z() );
    #else
    const HF32 sinHeading( -m_A[MX].Z() );
    #endif
    
    const HF32 h( ArcSin( sinHeading ) );
    HF32 p;
    HF32 r;
    
    if ( Abs( (HF32)sinHeading ) < 0.999f )
    {
        // m[Y][Z] / m[Z][Z} = sin(p)*cos(h) / cos(p)*cos(h)
        //                   = sin(p) / cos(p)
        //                   = tan(p)
        // -->             p = atan( m[Y][Z] / m[Z][Z] )
        
        // m[X][Y] / m[X][X] = cos(h)*sin(r) / cos(h)*cos(r)
        //                   = sin(r) / cos(r)
        //                   = tan(r)
        // -->             r = atan( m[X][Y] / m[X][X] )
        
        #if defined( _VMX128_ ) || defined( _SPU_ ) || defined( _PPU_ )
        const HF32 tanPitchSin( AY().Z() ); // TODO: Single accessor for extracting a float element of a matrix!
        const HF32 tanPitchCos( AZ().Z() ); // TODO: Single accessor for extracting a float element of a matrix!
        const HF32 tanRollSin( AX().Y() );  // TODO: Single accessor for extracting a float element of a matrix!
        const HF32 tanRollCos( AX().X() );  // TODO: Single accessor for extracting a float element of a matrix!
        #elif defined( _SSE_ )
        const HF32 tanPitchSin( m_AY.Z() );
        const HF32 tanPitchCos( m_AZ.Z() );
        const HF32 tanRollSin( m_AX.Y() );
        const HF32 tanRollCos( m_AX.X() );
        #else
        const HF32 tanPitchSin( m_A[MY].Z() );
        const HF32 tanPitchCos( m_A[MZ].Z() );
        const HF32 tanRollSin( m_A[MX].Y() );
        const HF32 tanRollCos( m_A[MX].X() );
        #endif
        
        p = (HF32)ArcTan( tanPitchSin, tanPitchCos );
        r = (HF32)ArcTan( tanRollSin, tanRollCos );
    }
    else
    {
        // sin(h) ~= 1 therefore cos(h) ~= 0
        // m[Y][X] / m[Z][X] = sin(p)sin(h)cos(r) - cos(p)sin(r) / cos(p)sin(h)cos(r) + sin(p)sin(r)
        //                   = sin(p)cos(r) - cos(p)sin(r)       / cos(p)cos(r) + sin(p)sin(r)
        // using    sin(x-y) = sin(x)cos(y) - cos(x)sin(y)
        // and      cos(x-y) = cos(x)cos(y) + sin(x)sin(y)
        //                   = sin(p-r)                          / cos(p-r)
        // [ at poles p equivalent to r therefore assume r = 0 ]
        //                   = sin(p) / cos(p)
        //                 p = atan( sin(p) / cos(p) )
        //                 p = atan( m[Y][X] / m[Z][X]
        
        #if defined( _VMX128_ ) || defined( _SPU_ ) || defined( _PPU_ )
        const HF32 tanPitchSin( AY().X() ); // TODO: Single accessor for extracting a float element of a matrix!
        const HF32 tanPitchCos( AZ().X() ); // TODO: Single accessor for extracting a float element of a matrix!
        #elif defined( _SSE_ )
        const HF32 tanPitchSin( m_AY.X() );
        const HF32 tanPitchCos( m_AZ.X() );
        #else
        const HF32 tanPitchSin( m_A[MY].X() );
        const HF32 tanPitchCos( m_A[MZ].X() );
        #endif
        
        p = (HF32)ArcTan( tanPitchSin, tanPitchCos );
        r = (HF32)0.0f;
    }
    
    return C_MatrixRotatePHR( p, h, r );
}

/// CONVERTED FROM TOOLS TEAM CODE - NEED DESCRIPTION FROM THEM
NOALIAS void HMat33::Align( HF32& a, const HVec3 PCREF vNormal ) __restrict
{
    //
    // S = sin (a)
    // C = cos (a)
    // H = sqrt (X*X + Z*Z)
    //
    // [   v   ]  [  C  S        ]  [  1           ]  [ -Z/H  X/H    ]
    //            [ -S  C        ]  [     H  Y     ]  [     1        ]
    //            [        1     ]  [    -Y  H     ]  [ -X/H -Z/H    ]
    //            [           1  ]  [           1  ]  [           1  ]
    //
    //
    
	static const F32 dtor = Maths::Pi / 180.0f;
    
    a *= dtor;
    
    const HF32 zero;
    const HF32 s( sin( a ) );
    const HF32 c( cos( a ) );
    
    const HVec3 normal( Normalise( vNormal ) );
    
    const HF32 h( Length( normal.ShuffleXZtoXY() ) );
    
    if ( h != zero )
    {
        const HF32 nx( normal.X() );
        const HF32 ny( normal.Y() );
        const HF32 nz( normal.Z() );
        const HF32 rcph( Reciprocal( h ) );
        
        AX( HVec3( ( -nz * c - s * ny * nx ) * rcph,
                    s * h,
                    ( c * nx - s * ny * nz ) * rcph ) );
        
        AY( HVec3( ( nz * s - c * ny * nx ) * rcph,
                    c * h,
                    ( -s * nx - c * ny * nz ) * rcph ) );
        
        AZ( -normal );
    }
    else
    {
        AX( HVec3( s, 0.0f, -c ) );
        AY( HVec3( c, 0.0f, s ) );
        AZ( HVec3( 0.0f, -1.0f, 0.0f ) );
    }
}

/// Jacobi Transformation Matrix for elimination of off-diagonal [ p, q ] : NOTE: matrix symmetric & p != q !
NOALIAS const HMat33 HMat33::JacobiTransformation( const U32 p, const U32 q ) const __restrict
{
    const HF32 zero;
    
    const HVec3 ax( AX() );
    const HVec3 ay( AY() );
    const HVec3 az( AZ() );
    
    F32 A[3][4] =
    {
        { ax.X(), ax.Y(), ax.Z(), zero },
        { ay.X(), ay.Y(), ay.Z(), zero },
        { az.X(), az.Y(), az.Z(), zero }
    };
    
    const HF32 Apq( A[p][q] );
    const HF32 App( A[p][p] );
    const HF32 Aqq( A[q][q] );
    
    const HF32 x( Apq + Apq );
    const HF32 y( Aqq - App );
    
    //const F32 theta( ArcTan( y, x ) * 0.5f );
    const HF32 theta( ArcTan( x, y ) * 0.5f );
    const HF32 cosTheta( Cos( theta ) );
    const HF32 sinTheta( Sin( theta ) );
    
    F32 M[3][4] =
    {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f }
    };
    
    M[p][p] = cosTheta;
    M[q][q] = cosTheta;
    M[p][q] = sinTheta;
    M[q][p] = -sinTheta;
    
    return HMat33( HVec3( M[0] ), HVec3( M[1] ), HVec3( M[2] ) );
}

/// QR Decomposition
NOALIAS void HMat33::QR_Decomposition( HMat33& Q, HMat33& R ) const __restrict
{
    const HMat33 At( Transposed() );
    HMat33 Qt, Rt;
    At.RQ_Decomposition( Rt, Qt );
    Q = Qt.Transposed();
    R = Rt.Transposed();
}

/// RQ Decomposition
NOALIAS void HMat33::RQ_Decomposition( HMat33& R, HMat33& Q ) const __restrict
{
    #if 0
    
    const HF32 zero;
    const HF32 one( c_ScalarOne );
    
    const HVec3 a0( AX() );
    const HVec3 a1( AY() );
    const HVec3 a2( AZ() );
    
    const HF32 a00( a0.X() );
    const HF32 a01( a0.Y() );
    const HF32 a02( a0.Z() );
    const HF32 a10( a1.X() );
    const HF32 a11( a1.Y() );
    const HF32 a12( a1.Z() );
    const HF32 a20( a2.X() );
    const HF32 a21( a2.Y() );
    const HF32 a22( a2.Z() );
    
    // Solve for r2's & q2's
    
    const HF32 r22( Sqrt( a20 * a20 + a21 * a21 + a22 * a22 ) );
    const HF32 r21( zero );
    const HF32 r20( zero );
    
    const HF32 rcp_r22( rcp( r22 ) );
    const HF32 q20( a20 * rcp_r22 );
    const HF32 q21( a21 * rcp_r22 );
    const HF32 q22( a22 * rcp_r22 );
    
    // Solve for r1's & q1's
    
    const HF32 r12( q20 * a10 + q21 * a11 + q22 * a12 );
    const HF32 q10_r11( a10 - r12 * q20 );
    const HF32 q11_r11( a11 - r12 * q21 );
    const HF32 q12_r11( a12 - r12 * q22 );
    const HF32 r11( Sqrt( Square( q10_r11 ) + Square( q11_r11 ) + Square( q12_r11 ) ) );
    const HF32 r10( zero );
    
    const HF32 rcp_r11( rcp( r11 ) );
    const HF32 q10( q10_r11 * rcp_r11 );
    const HF32 q11( q11_r11 * rcp_r11 );
    const HF32 q12( q12_r11 * rcp_r11 );
    
    // Solve for r0's & q0's
    
    const HF32 r02( q20 * a00 + q21 * a01 + q22 * a02 );
    const HF32 r01( q10 * a00 + q11 * a01 + q12 * a02 );
    const HF32 q00_r00( a00 - r01 * q10 - r02 * q20 );
    const HF32 q01_r00( a01 - r01 * q11 - r02 * q21 );
    const HF32 q02_r00( a02 - r01 * q12 - r02 * q22 );
    const HF32 r00( Sqrt( Square( q00_r00 ) + Square( q01_r00 ) + Square( q02_r00 ) ) );
    
    const HF32 rcp_r00( rcp( r00 ) );
    const HF32 q00( q00_r00 * rcp_r00 );
    const HF32 q01( q01_r00 * rcp_r00 );
    const HF32 q02( q02_r00 * rcp_r00 );
    
    // Alternative solutions for q0's
    //const HF32 q00( Sqrt( one - Square( q10 ) - Square( q20 ) ) );
    //const HF32 q01( Sqrt( one - Square( q11 ) - Square( q21 ) ) );
    //const HF32 q02( Sqrt( one - Square( q12 ) - Square( q22 ) ) );
    
    #else
    
    const F64 zero( 0.0 );
    const F64 nearlyZero( 1.0e-9 );
    const F64 one( 1.0 );
    
    const HVec3 a0( AX() );
    const HVec3 a1( AY() );
    const HVec3 a2( AZ() );
    
    const F64 a00( a0.X() );
    const F64 a01( a0.Y() );
    const F64 a02( a0.Z() );
    const F64 a10( a1.X() );
    const F64 a11( a1.Y() );
    const F64 a12( a1.Z() );
    const F64 a20( a2.X() );
    const F64 a21( a2.Y() );
    const F64 a22( a2.Z() );
    
    // Solve for r2's & q2's
    
    const F64 r22( Sqrt( a20 * a20 + a21 * a21 + a22 * a22 ) );
    const F64 r21( zero );
    const F64 r20( zero );
    
    const F64 rcp_r22( one / r22 );
    const F64 q20( Abs( r22 ) > nearlyZero ? a20 * rcp_r22 : zero );
    const F64 q21( Abs( r22 ) > nearlyZero ? a21 * rcp_r22 : zero );
    const F64 q22( Abs( r22 ) > nearlyZero ? a22 * rcp_r22 : one );
    
    // Solve for r1's & q1's
    
    const F64 default0_q10( -q21 * q20 );
    const F64 default0_q11( q22 * q22 + q20 * q20 );
    const F64 default0_q12( -q21 * q22 );
    const F64 default0_q1_length( Sqrt( Square( default0_q10 ) + Square( default0_q11 ) + Square( default0_q12 ) ) );
    
    const F64 default1_q10( q21 * q21 + q22 * q22 );
    const F64 default1_q11( -q20 * q21 );
    const F64 default1_q12( -q22 * q21 );
    const F64 default1_q1_length( Sqrt( Square( default1_q10 ) + Square( default1_q11 ) + Square( default1_q12 ) ) );
    
    const F64 default_q10( default0_q1_length > default1_q1_length ? default0_q10 : default1_q10 );
    const F64 default_q11( default0_q1_length > default1_q1_length ? default0_q11 : default1_q11 );
    const F64 default_q12( default0_q1_length > default1_q1_length ? default0_q12 : default1_q12 );
    const F64 default_q1_rcp_length( one / ( default0_q1_length > default1_q1_length ? default0_q1_length : default1_q1_length ) );
    
    const F64 r12( q20 * a10 + q21 * a11 + q22 * a12 );
    const F64 q10_r11( a10 - r12 * q20 );
    const F64 q11_r11( a11 - r12 * q21 );
    const F64 q12_r11( a12 - r12 * q22 );
    const F64 r11( Sqrt( Square( q10_r11 ) + Square( q11_r11 ) + Square( q12_r11 ) ) );
    const F64 r10( zero );
    
    const F64 rcp_r11( one / r11 );
    const F64 q10( Abs( r11 ) > nearlyZero ? q10_r11 * rcp_r11 : default_q10 * default_q1_rcp_length );
    const F64 q11( Abs( r11 ) > nearlyZero ? q11_r11 * rcp_r11 : default_q11 * default_q1_rcp_length );
    const F64 q12( Abs( r11 ) > nearlyZero ? q12_r11 * rcp_r11 : default_q12 * default_q1_rcp_length );
    
    // Solve for r0's & q0's
    
    const F64 r02( q20 * a00 + q21 * a01 + q22 * a02 );
    const F64 r01( q10 * a00 + q11 * a01 + q12 * a02 );
    const F64 q00_r00( a00 - r01 * q10 - r02 * q20 );
    const F64 q01_r00( a01 - r01 * q11 - r02 * q21 );
    const F64 q02_r00( a02 - r01 * q12 - r02 * q22 );
    const F64 r00( Sqrt( Square( q00_r00 ) + Square( q01_r00 ) + Square( q02_r00 ) ) );
    
    const F64 def_q00( q11 * q22 - q12 * q21 );
    const F64 def_q01( q12 * q20 - q10 * q22 );
    const F64 def_q02( q10 * q21 - q11 * q20 );
    
    #ifndef PLATFORM_PS3
    const F64 def_q0_rcp_len( one / Sqrt( Square( def_q00 ) + Square( def_q01 ) + Square( def_q02 ) ) );
    #else
    const F64 def_q0_rcp_len( one / (F64)(sqrt( Square( def_q00 ) + Square( def_q01 ) + Square( def_q02 ) )) );
    #endif
    
    const F64 rcp_r00( one / r00 );
    const F64 q00( Abs( r00 ) > nearlyZero ? q00_r00 * rcp_r00 : def_q00 * def_q0_rcp_len );
    const F64 q01( Abs( r00 ) > nearlyZero ? q01_r00 * rcp_r00 : def_q01 * def_q0_rcp_len );
    const F64 q02( Abs( r00 ) > nearlyZero ? q02_r00 * rcp_r00 : def_q02 * def_q0_rcp_len );
    
    #endif
    
    R = HMat33( HVec3( (F32)r00, (F32)r01, (F32)r02 ),
                HVec3( (F32)r10, (F32)r11, (F32)r12 ),
                HVec3( (F32)r20, (F32)r21, (F32)r22 ) );
    
    Q = HMat33( HVec3( (F32)q00, (F32)q01, (F32)q02 ),
                HVec3( (F32)q10, (F32)q11, (F32)q12 ),
                HVec3( (F32)q20, (F32)q21, (F32)q22 ) );
}

/// Solve R * x = y, where R is an Upper Triangular matrix
NOALIAS const HVec3 HMat33::Solve_Rx_Eq_y( const HVec3 PCREF y ) const __restrict
{
    const HF32 one( c_ScalarOne );
    const HF32 nearlyZero( 1.0e-4f );
    
    const HF32 y0( y.X() );
    const HF32 y1( y.Y() );
    const HF32 y2( y.Z() );
    
    const HVec3 r0( AX() );
    const HVec3 r1( AY() );
    const HVec3 r2( AZ() );
    
    const HF32 r00( r0.X() );
    const HF32 r01( r0.Y() );
    const HF32 r02( r0.Z() );
    
    const HF32 r11( r1.Y() );
    const HF32 r12( r1.Z() );
    
    const HF32 r22( r2.Z() );
    
    const HF32 x2( Select( r22.GT( nearlyZero ), y2 / r22, one ) );
    const HF32 x1( Select( r11.GT( nearlyZero ), ( y1 - r12 * x2 ) / r11, one ) );
    const HF32 x0( Select( r00.GT( nearlyZero ), ( y0 - r01 * x1 - r02 * x2 ) / r00, one ) );
    
    return HVec3( x0, x1, x2 );
}

/// Eigen Decomposition
NOALIAS const C_Eigen33 HMat33::EigenDecomposition() const __restrict
{
    const HF32 zero;
    const HF32 one( c_ScalarOne );
    const HF32 nearlyZero( 1.0e-5f );
    const HF32 real_limit( 0.0001f );
    
    const HVec3 vzero;
    
    // TODO: VMX & SSE extraction of elements
    
    const HMat33 A = *this;
    
    const HVec3 A0( A.AX() );
    const HVec3 A1( A.AY() );
    const HVec3 A2( A.AZ() );
    
    const HF32 A00( A0.X() );
    const HF32 A01( A0.Y() );
    const HF32 A02( A0.Z() );
    const HF32 A10( A1.X() );
    const HF32 A11( A1.Y() );
    const HF32 A12( A1.Z() );
    const HF32 A20( A2.X() );
    const HF32 A21( A2.Y() );
    const HF32 A22( A2.Z() );
    
    const HF32 a3( -one );
    const HF32 a2( A00 + A11 + A22 );
    const HF32 a1( A12*A21 + A01*A10 + A02*A20 - A00*A11 - A00*A22 - A11*A22 );
    const HF32 a0( A00*A11*A22 + A01*A12*A20 + A02*A10*A21 - A00*A12*A21 - A01*A10*A22 - A02*A11*A20 );
    
    const C_CubicResult roots = SolveCubic( a3, a2, a1, a0 );
    
    const HF32 abs_root0_imag( Abs( roots.m_Root0.Imag() ) );
    const HF32 abs_root1_imag( Abs( roots.m_Root1.Imag() ) );
    const HF32 abs_root2_imag( Abs( roots.m_Root2.Imag() ) );
    const HF32 abs_root0_real( Abs( roots.m_Root0.Real() ) );
    const HF32 abs_root1_real( Abs( roots.m_Root1.Real() ) );
    const HF32 abs_root2_real( Abs( roots.m_Root2.Real() ) );
    
    // TODO: Need a selector function for 'finite'
    const Scalar_Bool finite_lambda0( abs_root0_imag.GE( zero ) & abs_root0_real.GE( zero ) );
    const Scalar_Bool finite_lambda1( abs_root1_imag.GE( zero ) & abs_root1_real.GE( zero ) );
    const Scalar_Bool finite_lambda2( abs_root2_imag.GE( zero ) & abs_root2_real.GE( zero ) );
    
    const Scalar_Bool real_lambda0( abs_root0_imag.LE( real_limit ) );
    const Scalar_Bool real_lambda1( abs_root1_imag.LE( real_limit ) );
    const Scalar_Bool real_lambda2( abs_root2_imag.LE( real_limit ) );
    
    const HF32 lambda0( Select( real_lambda0 & finite_lambda0, roots.m_Root0.Real(), zero ) );
    const HF32 lambda1( Select( real_lambda1 & finite_lambda1, roots.m_Root1.Real(), zero ) );
    const HF32 lambda2( Select( real_lambda2 & finite_lambda2, roots.m_Root2.Real(), zero ) );
    
    const HMat33 I;
    
    const HMat33 A_minus_I_lambda0( A - I * lambda0 );
    const HMat33 A_minus_I_lambda1( A - I * lambda1 );
    const HMat33 A_minus_I_lambda2( A - I * lambda2 );
    
    HMat33 R, Q;
    
    A_minus_I_lambda0.RQ_Decomposition( R, Q );
    const HVec3 x0( R.Solve_Rx_Eq_y( vzero ) * Q );
    
    A_minus_I_lambda1.RQ_Decomposition( R, Q );
    const HVec3 x1( R.Solve_Rx_Eq_y( vzero ) * Q );
    
    A_minus_I_lambda2.RQ_Decomposition( R, Q );
    const HVec3 x2( R.Solve_Rx_Eq_y( vzero ) * Q );
    
    const HVec3 v_zero;
    return C_Eigen33( HVec3( lambda0, lambda1, lambda2 ),
                      Select( Abs( lambda0 ).GE( nearlyZero ), Normalise( x0 ), vzero ),
                      Select( Abs( lambda1 ).GE( nearlyZero ), Normalise( x1 ), vzero ),
                      Select( Abs( lambda2 ).GE( nearlyZero ), Normalise( x2 ), vzero ) );
}

//#ifdef SHOOTER_VECMAT_DEBUG_PRINT
//void HMat33::DbgPrint( char* text ) const
//{
//    if ( text != "" )
//    {
//        OutputDebugString( text );
//        OutputDebugString( "\n" );
//    }
//    AX().DbgPrint( "AX" );
//    AY().DbgPrint( "AY" );
//    AZ().DbgPrint( "AZ" );
//    OutputDebugString( "\n" );
//}
//#endif











//
// -------------------------------------------------------
// |-- HMat43 : Hardware 4 Row * 3 Column Matrix Class --|
// -------------------------------------------------------
//

/// X Axis Rotation Constructor - clockwise when viewed back down the X axis
NOALIAS HMat43::HMat43( const RotX PCREF rx, const HVec3 PCREF aw )
{
    const HF32 alpha = rx;
    const HF32 cx = Cos( alpha );
    const HF32 sx = Sin( alpha );
    #if defined( _SPU_ ) || defined( _PPU_ )
    LocalConstV3F32( ay ) = { 0.0f,  cx, sx };
    LocalConstV3F32( az ) = { 0.0f, -sx, cx };
    m_AX = c_UnitVectorX;
    m_AY = ay;
    m_AZ = az;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    m_CX = __vrlimi( c_UnitVectorX,                 aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( SIMD_Vector3( 0.0f, cx, -sx ), aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( SIMD_Vector3( 0.0f, sx,  cx ), aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #else
        #if defined( _SSE_ )
        const __m128 ay = _mm_setr_ps( 0.0f,  cx, sx, 0.0f );
        const __m128 az = _mm_setr_ps( 0.0f, -sx, cx, 0.0f );
        #else
        LocalConstV3F32( ay ) = { 0.0f,  cx, sx };
        LocalConstV3F32( az ) = { 0.0f, -sx, cx };
        #endif
    m_Rotate = HMat33( HVec3( c_UnitVectorX ), HVec3( ay ), HVec3( az ) );
    m_Translate = aw;
    #endif
}

/// Y Axis Rotation Constructor - clockwise when viewed back down the Y axis
NOALIAS HMat43::HMat43( const RotY PCREF ry, const HVec3 PCREF aw )
{
    const HF32 beta = ry;
    const HF32 cy = Cos( beta );
    const HF32 sy = Sin( beta );
    #if defined( _SPU_ ) || defined( _PPU_ )
    LocalConstV3F32( ax ) = { cy, 0.0f, -sy };
    LocalConstV3F32( az ) = { sy, 0.0f,  cy };
    m_AX = ax;
    m_AY = c_UnitVectorY;
    m_AZ = az;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    m_CX = __vrlimi( SIMD_Vector3( cy,   0.0f, sy ), aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( c_UnitVectorY,                  aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( SIMD_Vector3( -sy,  0.0f, cy ), aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #else
        #if defined( _SSE_ )
        const __m128 ax = _mm_setr_ps( cy, 0.0f, -sy, 0.0f );
        const __m128 az = _mm_setr_ps( sy, 0.0f,  cy, 0.0f );
        #else
        LocalConstV3F32( ax ) = { cy, 0.0f, -sy };
        LocalConstV3F32( az ) = { sy, 0.0f,  cy };
        #endif
    m_Rotate = HMat33( HVec3( ax ), HVec3( c_UnitVectorY ), HVec3( az ) );
    m_Translate = aw;
    #endif
}

/// Z Axis Rotation Constructor - clockwise when viewed back down the Z axis
NOALIAS HMat43::HMat43( const RotZ PCREF rz, const HVec3 PCREF aw )
{
    const HF32 gamma = rz;
    const HF32 cz = Cos( gamma );
    const HF32 sz = Sin( gamma );
    #if defined( _SPU_ ) || defined( _PPU_ )
    LocalConstV3F32( ax ) = {  cz, sz, 0.0f };
    LocalConstV3F32( ay ) = { -sz, cz, 0.0f };
    m_AX = ax;
    m_AY = ay;
    m_AZ = c_UnitVectorZ;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    m_CX = __vrlimi( SIMD_Vector3( cz, -sz, 0.0f ), aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( SIMD_Vector3( sz, cz,  0.0f ), aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( c_UnitVectorZ,                 aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #else
        #if defined( _SSE_ )
        const __m128 ax = _mm_setr_ps(  cz, sz, 0.0f, 0.0f );
        const __m128 ay = _mm_setr_ps( -sz, cz, 0.0f, 0.0f );
        #else
        LocalConstV3F32( ax ) = {  cz, sz, 0.0f };
        LocalConstV3F32( ay ) = { -sz, cz, 0.0f };
        #endif
    m_Rotate = HMat33( HVec3( ax ), HVec3( ay ), HVec3( c_UnitVectorZ ) );
    m_Translate = aw;
    #endif
}

//#ifdef SHOOTER_VECMAT_DEBUG_PRINT
//void HMat43::DbgPrint( char* text ) const
//{
//    if ( text != "" )
//    {
//        OutputDebugString( text );
//        OutputDebugString( "\n" );
//    }
//    #if defined( _VMX128_ )
//    const V128 zero( __vzero() );
//    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
//    const V128 tmp1( __vmrghw( m_CY, zero ) );
//    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
//    const V128 tmp3( __vmrglw( m_CY, zero ) );
//    const V128 ax( __vmrghw( tmp0, tmp1 ) );
//    const V128 ay( __vmrglw( tmp0, tmp1 ) );
//    const V128 az( __vmrghw( tmp2, tmp3 ) );
//    const V128 aw( __vmrglw( tmp2, tmp3 ) );
//    HVec3(ax).DbgPrint( "AX" );
//    HVec3(ay).DbgPrint( "AY" );
//    HVec3(az).DbgPrint( "AZ" );
//    HVec3(aw).DbgPrint( "AW" );
//    #else
//    m_Rotate.AX().DbgPrint( "AX" );
//    m_Rotate.AY().DbgPrint( "AY" );
//    m_Rotate.AZ().DbgPrint( "AZ" );
//    m_Translate.DbgPrint( "AW" );
//    #endif
//    OutputDebugString( "\n" );
//}
//#endif








//
// -------------------------------------------------------
// |-- HMat44 : Hardware 4 Row * 4 Column Matrix Class --|
// -------------------------------------------------------
//

#if defined( _SSE_ )
/// Code from Intel AP-928 : Streaming SIMD Extensions - Inverse of 4x4 Matrix - SIMD version
const HMat44 HMat44::Matrix44InverseCramersRuleSSE() const __restrict
{
    __m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1 = {0}, row2, row3 = {0};
	__m128 det, tmp1 = {0};

	float *src = (float*)this;
    
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src+ 4));
	row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src+8)), (__m64*)(src+12));
	row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
	row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src+ 2)), (__m64*)(src+ 6));
	row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src+10)), (__m64*)(src+14));
	row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
	row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row2, row3 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor0 = _mm_mul_ps( row1, tmp1 );
    minor1 = _mm_mul_ps( row0, tmp1 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor0 = _mm_sub_ps( _mm_mul_ps( row1, tmp1 ), minor0 );
    minor1 = _mm_sub_ps( _mm_mul_ps( row0, tmp1 ), minor1 );
    minor1 = _mm_shuffle_ps( minor1, minor1, 0x4E );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row1, row2 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor0 = _mm_add_ps( _mm_mul_ps( row3, tmp1 ), minor0 );
    minor3 = _mm_mul_ps( row0, tmp1 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor0 = _mm_sub_ps( minor0, _mm_mul_ps( row3, tmp1 ) );
    minor3 = _mm_sub_ps( _mm_mul_ps( row0, tmp1 ), minor3 );
    minor3 = _mm_shuffle_ps( minor3, minor3, 0x4E );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( _mm_shuffle_ps( row1, row1, 0x4E ), row3 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    row2   = _mm_shuffle_ps( row2, row2, 0x4E );
    minor0 = _mm_add_ps( _mm_mul_ps( row2, tmp1 ), minor0 );
    minor2 = _mm_mul_ps( row0, tmp1 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor0 = _mm_sub_ps( minor0, _mm_mul_ps( row2, tmp1 ) );
    minor2 = _mm_sub_ps( _mm_mul_ps( row0, tmp1 ), minor2 );
    minor2 = _mm_shuffle_ps( minor2, minor2, 0x4E );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row0, row1 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor2 = _mm_add_ps( _mm_mul_ps( row3, tmp1 ), minor2 );
    minor3 = _mm_sub_ps( _mm_mul_ps( row2, tmp1 ), minor3 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor2 = _mm_sub_ps( _mm_mul_ps( row3, tmp1 ), minor2 );
    minor3 = _mm_sub_ps( minor3, _mm_mul_ps( row2, tmp1 ) );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row0, row3 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor1 = _mm_sub_ps( minor1, _mm_mul_ps( row2, tmp1 ) );
    minor2 = _mm_add_ps( _mm_mul_ps( row1, tmp1 ), minor2 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor1 = _mm_add_ps( _mm_mul_ps( row2, tmp1 ), minor1 );
    minor2 = _mm_sub_ps( minor2, _mm_mul_ps( row1, tmp1 ) );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row0, row2 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor1 = _mm_add_ps( _mm_mul_ps( row3, tmp1 ), minor1 );
    minor3 = _mm_sub_ps( minor3, _mm_mul_ps( row1, tmp1 ) );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor1 = _mm_sub_ps( minor1, _mm_mul_ps( row3, tmp1 ) );
    minor3 = _mm_add_ps( _mm_mul_ps( row1, tmp1 ), minor3 );
    
    // -----------------------------------------------
    
    det  = _mm_mul_ps( row0, minor0 );
    det  = _mm_add_ps( _mm_shuffle_ps( det, det, 0x4E ), det );
    det  = _mm_add_ss( _mm_shuffle_ps( det, det, 0xB1 ), det );
    tmp1 = _mm_rcp_ss( det );
    det  = _mm_sub_ss( _mm_add_ss( tmp1, tmp1 ), _mm_mul_ss( det, _mm_mul_ss( tmp1, tmp1 ) ) );
    det  = _mm_shuffle_ps( det, det, 0x00 );
    
    minor0 = _mm_mul_ps( det, minor0 );
    minor1 = _mm_mul_ps( det, minor1 );
    minor2 = _mm_mul_ps( det, minor2 );
    minor3 = _mm_mul_ps( det, minor3 );
    
    return HMat44( minor0, minor1, minor2, minor3 );
}
#endif


//#ifdef SHOOTER_VECMAT_DEBUG_PRINT
//void HMat44::DbgPrint( char* text ) const
//{
//    if ( text != "" )
//    {
//        OutputDebugString( text );
//        OutputDebugString( "\n" );
//    }
//    AX().DbgPrint( "AX" );
//    AY().DbgPrint( "AY" );
//    AZ().DbgPrint( "AZ" );
//    AW().DbgPrint( "AW" );
//    OutputDebugString( "\n" );
//}
//#endif








//
// ------------------------------------
// |-- Standalone Support Functions --|
// ------------------------------------
//

/// Code from Intel AP-928 : Streaming SIMD Extensions - Inverse of 4x4 Matrix - FP version
void Matrix44InverseCramersRuleFP( const F32* mat, F32* dst )
{
    F32 tmp[12];  // temp array for pairs
    F32 src[16];  // array of transpose source matrix
    F32 det;      // determinant
    
    // transpose matrix
    for ( int i = 0; i < 4; i++ )
    {
        src[i]      = mat[i*4];
        src[i + 4]  = mat[i*4 + 1];
        src[i + 8]  = mat[i*4 + 2];
        src[i + 12] = mat[i*4 + 3];
    }
    
    // calculate pairs for first 8 elements (cofactors)
    tmp[0]  = src[10] * src[15];
    tmp[1]  = src[11] * src[14];
    tmp[2]  = src[9]  * src[15];
    tmp[3]  = src[11] * src[13];
    tmp[4]  = src[9]  * src[14];
    tmp[5]  = src[10] * src[13];
    tmp[6]  = src[8]  * src[15];
    tmp[7]  = src[11] * src[12];
    tmp[8]  = src[8]  * src[14];
    tmp[9]  = src[10] * src[12];
    tmp[10] = src[8]  * src[13];
    tmp[11] = src[9]  * src[12];
    
    // calculate first 8 elements (cofactors)
    dst[0]  = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4]  * src[7];
    dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5]  * src[7];
    dst[1]  = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9]  * src[7];
    dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8]  * src[7];
    dst[2]  = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
    dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
    dst[3]  = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
    dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
    dst[4]  = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5]  * src[3];
    dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4]  * src[3];
    dst[5]  = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8]  * src[3];
    dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9]  * src[3];
    dst[6]  = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
    dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
    dst[7]  = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
    dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];
    
    // calculate pairs for second 8 elements (cofactors)
    tmp[0]  = src[2] * src[7];
    tmp[1]  = src[3] * src[6];
    tmp[2]  = src[1] * src[7];
    tmp[3]  = src[3] * src[5];
    tmp[4]  = src[1] * src[6];
    tmp[5]  = src[2] * src[5];
    tmp[6]  = src[0] * src[7];
    tmp[7]  = src[3] * src[4];
    tmp[8]  = src[0] * src[6];
    tmp[9]  = src[2] * src[4];
    tmp[10] = src[0] * src[5];
    tmp[11] = src[1] * src[4];
    
    // calculate second 8 elements (cofactors)
    dst[8]   = tmp[0]  * src[13] + tmp[3]  * src[14] + tmp[4]  * src[15];
    dst[8]  -= tmp[1]  * src[13] + tmp[2]  * src[14] + tmp[5]  * src[15];
    dst[9]   = tmp[1]  * src[12] + tmp[6]  * src[14] + tmp[9]  * src[15];
    dst[9]  -= tmp[0]  * src[12] + tmp[7]  * src[14] + tmp[8]  * src[15];
    dst[10]  = tmp[2]  * src[12] + tmp[7]  * src[13] + tmp[10] * src[15];
    dst[10] -= tmp[3]  * src[12] + tmp[6]  * src[13] + tmp[11] * src[15];
    dst[11]  = tmp[5]  * src[12] + tmp[8]  * src[13] + tmp[11] * src[14];
    dst[11] -= tmp[4]  * src[12] + tmp[9]  * src[13] + tmp[10] * src[14];
    dst[12]  = tmp[2]  * src[10] + tmp[5]  * src[11] + tmp[1]  * src[9];
    dst[12] -= tmp[4]  * src[11] + tmp[0]  * src[9]  + tmp[3]  * src[10];
    dst[13]  = tmp[8]  * src[11] + tmp[0]  * src[8]  + tmp[7]  * src[10];
    dst[13] -= tmp[6]  * src[10] + tmp[9]  * src[11] + tmp[1]  * src[8];
    dst[14]  = tmp[6]  * src[9]  + tmp[11] * src[11] + tmp[3]  * src[8];
    dst[14] -= tmp[10] * src[11] + tmp[2]  * src[8]  + tmp[7]  * src[9];
    dst[15]  = tmp[10] * src[10] + tmp[4]  * src[8]  + tmp[9]  * src[9];
    dst[15] -= tmp[8]  * src[9]  + tmp[11] * src[10] + tmp[5]  * src[8];
    
    // calculate determinant
    det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];
    
    // calculate matrix inverse
    det = 1/det;
    
    for ( int j = 0; j < 16; j++ )
    {
        dst[j] *= det;
    }
}




}; // namespace Maths



