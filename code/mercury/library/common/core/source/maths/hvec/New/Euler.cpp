#include "CorePch.h"
#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>

namespace Maths
{
/// @addtogroup Euler
//! @{

// Euler angle rotation about X axis [ clockwise when viewed back down the axis ]
// [ 1,  0,      0      ]
// [ 0,  cos(x), sin(x) ]
// [ 0, -sin(x), cos(x) ]

// Euler angle rotation about Y axis [ clockwise when viewed back down the axis ]
// [ cos(y), 0, -sin(y) ]
// [ 0,      1,  0      ]
// [ sin(y), 0,  cos(y) ]

// Euler angle rotation about Z axis [ clockwise when viewed back down the axis ]
// [  cos(z), sin(z), 0 ]
// [ -sin(z), cos(z), 0 ]
// [  0,      0,      1 ]


/// Make EulerXYZ from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerXYZ MakeEulerXYZ( const Mat33 MATREF m )
{
    // Matrix XYZ =
    
    // [ rowX ]   [ 1,  0,      0      ]   [ cos(y), 0, -sin(y) ]   [  cos(z), sin(z), 0 ]
    // [ rowY ] = [ 0,  cos(x), sin(x) ] * [ 0,      1,  0      ] * [ -sin(z), cos(z), 0 ]
    // [ rowZ ]   [ 0, -sin(x), cos(x) ]   [ sin(y), 0,  cos(y) ]   [  0,      0,      1 ]
    
    // [ rowX ]   [ cos(y),         0,      -sin(y)        ]   [  cos(z), sin(z), 0 ]
    // [ rowY ] = [ sin(x).sin(y),  cos(x),  sin(x).cos(y) ] * [ -sin(z), cos(z), 0 ]
    // [ rowZ ]   [ cos(x).sin(y), -sin(x),  cos(x).cos(y) ]   [  0,      0,      1 ]
    
    // [ rowX ]   [ cos(y).cos(z),                        cos(y).sin(z),                        -sin(y)        ]
    // [ rowY ] = [ sin(x).sin(y).cos(z) - cos(x).sin(z), sin(x).sin(y).sin(z) + cos(x).cos(z),  sin(x).cos(y) ] 
    // [ rowZ ]   [ cos(x).sin(y).cos(z) + sin(x).sin(z), cos(x).sin(y).sin(z) - sin(x).cos(z),  cos(x).cos(y) ]
    
    const Vector3 rowX( RowX( m ) );
    const Vector3 rowY( RowY( m ) );
    const Vector3 rowZ( RowZ( m ) );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 nearlyOne( Const<VF32>::Frac<9999999,10000000>() );
    const VF32 sinY( -Z( rowX ) );
    
    const VBool abs_sinY_LT_nearlyOne( Abs( sinY ) < nearlyOne );
    
    // For y < 90 degrees
    const VF32 sinX( Z( rowY ) ); // sin(x).cos(y), cos(y) cancels out in ArcTan division
    const VF32 cosX( Z( rowZ ) ); // cos(x).cos(y)
    const VF32 sinZ( Y( rowX ) ); // cos(y).sin(z)
    const VF32 cosZ( X( rowX ) ); // cos(y).cos(z)
    
    // For y ~= 90 degrees = pi/2 radians, sinY ~= 1, cosY ~= 0
    // When y = 90 degrees, rotations about x & z are equivalent (rotations about the same axis),
    // so we can set x or z to zero and solve for the other. For x=0, sin(x) = 0, cos(x) = 1
    //const VF32 sinZ2( -X( rowY ) ); // sin(x).sin(y).cos(z) - cos(x).sin(z) = -sin(z)
    //const VF32 cosZ2(  X( rowZ ) ); // cos(x).sin(y).cos(z) + sin(x).sin(z) =  cos(z)
    // When y = 90 degrees, rotations about x & z are equivalent (rotations about the same axis),
    // so we can set x or z to zero and solve for the other. For z=0, sin(z) = 0, cos(z) = 1
    const VF32 sinX2( -Y( rowZ ) ); // -cos(x).sin(y).sin(z) + sin(x).cos(z) = sin(x) // NOTE: less variables/assumptions in this case!
    const VF32 cosX2(  Y( rowY ) ); //  sin(x).sin(y).sin(z) + cos(x).cos(z) = cos(x)
    
    //const Vector3 arctan_x( MakeVector3( sinX, sinZ, sinZ2 ) );
    //const Vector3 arctan_y( MakeVector3( cosX, cosZ, cosZ2 ) );
    //const Vector3 x1_z1_z2( ArcTan( arctan_x, arctan_y ) );
    //const VF32 x1( X( x1_z1_z2 ) );
    //const VF32 z1( Y( x1_z1_z2 ) );
    //const VF32 x2( zero );
    //const VF32 z2( Z( x1_z1_z2 ) );
    const Vector3 arctan_x( MakeVector3( sinX, sinZ, sinX2 ) );
    const Vector3 arctan_y( MakeVector3( cosX, cosZ, cosX2 ) );
    const Vector3 x1_z1_x2( ArcTan( arctan_x, arctan_y ) );
    const VF32 x1( X( x1_z1_x2 ) );
    const VF32 z1( Y( x1_z1_x2 ) );
    const VF32 x2( Z( x1_z1_x2 ) );
    const VF32 z2( zero );
    
    const VF32 x( Select( abs_sinY_LT_nearlyOne, x1, x2 ) );
    const VF32 y( ArcSin( sinY ) );
    const VF32 z( Select( abs_sinY_LT_nearlyOne, z1, z2 ) );
    
    return MakeEulerXYZ( x, y, z );
}

/// Make EulerXZY from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerXZY MakeEulerXZY( const Mat33 MATREF m )
{
    // Matrix XZY =
    
    // [ rowX ]   [ 1,  0,      0      ]   [  cos(z), sin(z), 0 ]   [ cos(y), 0, -sin(y) ]
    // [ rowY ] = [ 0,  cos(x), sin(x) ] * [ -sin(z), cos(z), 0 ] * [ 0,      1,  0      ]
    // [ rowZ ]   [ 0, -sin(x), cos(x) ]   [  0,      0,      1 ]   [ sin(y), 0,  cos(y) ]
    
    // [ rowX ]   [  cos(z),         sin(z),        0      ]   [ cos(y), 0, -sin(y) ]
    // [ rowY ] = [ -cos(x).sin(z),  cos(x).cos(z), sin(x) ] * [ 0,      1,  0      ]
    // [ rowZ ]   [  sin(x).sin(z), -sin(x).cos(z), cos(x) ]   [ sin(y), 0,  cos(y) ]
    
    // [ rowX ]   [  cos(z).cos(y),                         sin(z),        -cos(z).sin(y)                        ]
    // [ rowY ] = [ -cos(x).sin(z).cos(y) + sin(x).sin(y),  cos(x).cos(z),  cos(x).sin(z).sin(y) + sin(x).cos(y) ]
    // [ rowZ ]   [  sin(x).sin(z).cos(y) + cos(x).sin(y), -sin(x).cos(z), -sin(x).sin(z).sin(y) + cos(x).cos(y) ]
    
    const Vector3 rowX( RowX( m ) );
    const Vector3 rowY( RowY( m ) );
    const Vector3 rowZ( RowZ( m ) );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 nearlyOne( Const<VF32>::Frac<9999999,10000000>() );
    const VF32 sinZ( Y( rowX ) );
    
    const VBool abs_sinZ_LT_nearlyOne( Abs( sinZ ) < nearlyOne );
    
    // For z < 90 degrees
    const VF32 sinX( -Y( rowZ ) ); // sin(x).cos(z), cos(z) cancels out in ArcTan division
    const VF32 cosX(  Y( rowY ) ); // cos(x).cos(z)
    const VF32 sinY( -Z( rowX ) ); // cos(z).sin(y)
    const VF32 cosY(  X( rowX ) ); // cos(z).cos(y)
    
    // For z ~= 90 degrees = pi/2 radians, sinZ ~= 1, cosZ ~= 0
    // When z = 90 degrees, rotations about x & y are equivalent (rotations about the same axis),
    // so we can set x or y to zero and solve for the other. For x=0, sin(x) = 0, cos(x) = 1
    //const VF32 sinY2( Z( rowY ) ); //  cos(x).sin(y) + sin(x).cos(y) = sin(y)
    //const VF32 cosY2( Z( rowZ ) ); // -sin(x).sin(y) + cos(x).cos(y) = cos(y)
    // When z = 90 degrees, rotations about x & y are equivalent (rotations about the same axis),
    // so we can set x or y to zero and solve for the other. For y=0, sin(y) = 0, cos(y) = 1
    const VF32 sinX2( Z( rowY ) ); //  cos(x).sin(z).sin(y) + sin(x).cos(y) = sin(x) // NOTE: less variables/assumptions in this case!
    const VF32 cosX2( Z( rowZ ) ); // -sin(x).sin(z).sin(y) + cos(x).cos(y) = cos(x)
    
    //const Vector3 arctan_x( MakeVector3( sinX, sinY, sinY2 ) );
    //const Vector3 arctan_y( MakeVector3( cosX, cosY, cosY2 ) );
    //const Vector3 x1_y1_y2( ArcTan( arctan_x, arctan_y ) );
    //const VF32 x1( X( x1_y1_y2 ) );
    //const VF32 y1( Y( x1_y1_y2 ) );
    //const VF32 x2( zero );
    //const VF32 y2( Z( x1_y1_y2 ) );
    const Vector3 arctan_x( MakeVector3( sinX, sinY, sinX2 ) );
    const Vector3 arctan_y( MakeVector3( cosX, cosY, cosX2 ) );
    const Vector3 x1_y1_x2( ArcTan( arctan_x, arctan_y ) );
    const VF32 x1( X( x1_y1_x2 ) );
    const VF32 y1( Y( x1_y1_x2 ) );
    const VF32 x2( Z( x1_y1_x2 ) );
    const VF32 y2( zero );
    
    const VF32 x( Select( abs_sinZ_LT_nearlyOne, x1, x2 ) );
    const VF32 y( Select( abs_sinZ_LT_nearlyOne, y1, y2 ) );
    const VF32 z( ArcSin( sinZ ) );
    
    return MakeEulerXZY( x, y, z );
}

/// Make EulerYXZ from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerYXZ MakeEulerYXZ( const Mat33 MATREF m )
{
    // Matrix YXZ =
    
    // [ rowX ]   [ cos(y), 0, -sin(y) ]   [ 1,  0,      0      ]   [  cos(z), sin(z), 0 ]
    // [ rowY ] = [ 0,      1,  0      ] * [ 0,  cos(x), sin(x) ] * [ -sin(z), cos(z), 0 ]
    // [ rowZ ]   [ sin(y), 0,  cos(y) ]   [ 0, -sin(x), cos(x) ]   [  0,      0,      1 ]
    
    // [ rowX ]   [ cos(y),  sin(y).sin(x), -sin(y).cos(x) ]   [  cos(z), sin(z), 0 ]
    // [ rowY ] = [ 0,       cos(x),         sin(x)        ] * [ -sin(z), cos(z), 0 ]
    // [ rowZ ]   [ sin(y), -cos(y).sin(x),  cos(y).cos(x) ]   [  0,      0,      1 ]
    
    // [ rowX ]   [  cos(y).cos(z) - sin(y).sin(x).sin(z), cos(y).sin(z) + sin(y).sin(x).cos(z), -sin(y).cos(x) ]
    // [ rowY ] = [ -cos(x).sin(z),                        cos(x).cos(z),                         sin(x)        ]
    // [ rowZ ]   [  sin(y).cos(z) + cos(y).sin(x).sin(z), sin(y).sin(z) - cos(y).sin(x).cos(z),  cos(y).cos(x) ]
    
    const Vector3 rowX( RowX( m ) );
    const Vector3 rowY( RowY( m ) );
    const Vector3 rowZ( RowZ( m ) );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 nearlyOne( Const<VF32>::Frac<9999999,10000000>() );
    const VF32 sinX( Z( rowY ) );
    
    const VBool abs_sinX_LT_nearlyOne( Abs( sinX ) < nearlyOne );
    
    // For x < 90 degrees
    const VF32 sinY( -Z( rowX ) ); // sin(y).cos(x), cos(x) cancels out in ArcTan division
    const VF32 cosY(  Z( rowZ ) ); // cos(y).cos(x)
    const VF32 sinZ( -X( rowY ) ); // cos(x).sin(z)
    const VF32 cosZ(  Y( rowY ) ); // cos(x).cos(z)
    
    // For x ~= 90 degrees = pi/2 radians, sinX ~= 1, cosX ~= 0
    // When x = 90 degrees, rotations about y & z are equivalent (rotations about the same axis),
    // so we can set y or z to zero and solve for the other. For z=0, sin(z) = 0, cos(z) = 1
    const VF32 sinY2( X( rowZ ) ); // sin(y).cos(z) + cos(y).sin(x).sin(z) = sin(y)
    const VF32 cosY2( X( rowX ) ); // cos(y).cos(z) - sin(y).sin(x).sin(z) = cos(y)
    
    const Vector3 arctan_x( MakeVector3( sinY, sinZ, sinY2 ) );
    const Vector3 arctan_y( MakeVector3( cosY, cosZ, cosY2 ) );
    const Vector3 y1_z1_y2( ArcTan( arctan_x, arctan_y ) );
    const VF32 y1( X( y1_z1_y2 ) );
    const VF32 z1( Y( y1_z1_y2 ) );
    const VF32 y2( Z( y1_z1_y2 ) );
    const VF32 z2( zero );
    
    const VF32 x( ArcSin( sinX ) );
    const VF32 y( Select( abs_sinX_LT_nearlyOne, y1, y2 ) );
    const VF32 z( Select( abs_sinX_LT_nearlyOne, z1, z2 ) );
    
    return MakeEulerYXZ( x, y, z );
}

/// Make EulerYZX from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerYZX MakeEulerYZX( const Mat33 MATREF m )
{
    // Matrix YZX =
    
    // [ rowX ]   [ cos(y), 0, -sin(y) ]   [  cos(z), sin(z), 0 ]   [ 1,  0,      0      ]
    // [ rowY ] = [ 0,      1,  0      ] * [ -sin(z), cos(z), 0 ] * [ 0,  cos(x), sin(x) ]
    // [ rowZ ]   [ sin(y), 0,  cos(y) ]   [  0,      0,      1 ]   [ 0, -sin(x), cos(x) ]
    
    // [ rowX ]   [  cos(y).cos(z), cos(y).sin(z), -sin(y) ]   [ 1,  0,      0      ]
    // [ rowY ] = [ -sin(z),        cos(z),         0      ] * [ 0,  cos(x), sin(x) ]
    // [ rowZ ]   [  sin(y).cos(z), sin(y).sin(z),  cos(y) ]   [ 0, -sin(x), cos(x) ]
    
    // [ rowX ]   [  cos(y).cos(z), cos(y).sin(z).cos(x) + sin(y).sin(x), cos(y).sin(z).sin(x) - sin(y).cos(x) ]
    // [ rowY ] = [ -sin(z),        cos(z).cos(x),                        cos(z).sin(x)                        ]
    // [ rowZ ]   [  sin(y).cos(z), sin(y).sin(z).cos(x) - cos(y).sin(x), sin(y).sin(z).sin(x) + cos(y).cos(x) ]
    
    const Vector3 rowX( RowX( m ) );
    const Vector3 rowY( RowY( m ) );
    const Vector3 rowZ( RowZ( m ) );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 nearlyOne( Const<VF32>::Frac<9999999,10000000>() );
    const VF32 sinZ( -X( rowY ) );
    
    const VBool abs_sinZ_LT_nearlyOne( Abs( sinZ ) < nearlyOne );
    
    // For z < 90 degrees
    const VF32 sinX( Z( rowY ) ); // cos(z).sin(x), cos(z) cancels out in ArcTan division
    const VF32 cosX( Y( rowY ) ); // cos(z).cos(x)
    const VF32 sinY( X( rowZ ) ); // sin(y).cos(z)
    const VF32 cosY( X( rowX ) ); // cos(y).cos(z)
    
    // For z ~= 90 degrees = pi/2 radians, sinZ ~= 1, cosZ ~= 0
    // When z = 90 degrees, rotations about x & y are equivalent (rotations about the same axis),
    // so we can set x or y to zero and solve for the other. For x=0, sin(x) = 0, cos(x) = 1
    const VF32 sinY2( -Z( rowX ) ); // -cos(y).sin(z).sin(x) + sin(y).cos(x) = sin(y)
    const VF32 cosY2(  Z( rowZ ) ); //  sin(y).sin(z).sin(x) + cos(y).cos(x) = cos(y)
    
    const Vector3 arctan_x( MakeVector3( sinX, sinY, sinY2 ) );
    const Vector3 arctan_y( MakeVector3( cosX, cosY, cosY2 ) );
    const Vector3 x1_y1_y2( ArcTan( arctan_x, arctan_y ) );
    const VF32 x1( X( x1_y1_y2 ) );
    const VF32 x2( zero );
    const VF32 y1( Y( x1_y1_y2 ) );
    const VF32 y2( Z( x1_y1_y2 ) );
    
    const VF32 x( Select( abs_sinZ_LT_nearlyOne, x1, x2 ) );
    const VF32 y( Select( abs_sinZ_LT_nearlyOne, y1, y2 ) );
    const VF32 z( ArcSin( sinZ ) );
    
    return MakeEulerYZX( x, y, z );
}

/// Make EulerZXY from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerZXY MakeEulerZXY( const Mat33 MATREF m )
{
    // Matrix ZXY =
    
    // [ rowX ]   [  cos(z), sin(z), 0 ]   [ 1,  0,      0      ]   [ cos(y), 0, -sin(y) ]
    // [ rowY ] = [ -sin(z), cos(z), 0 ] * [ 0,  cos(x), sin(x) ] * [ 0,      1,  0      ]
    // [ rowZ ]   [  0,      0,      1 ]   [ 0, -sin(x), cos(x) ]   [ sin(y), 0,  cos(y) ]
    
    // [ rowX ]   [  cos(z),  sin(z).cos(x), sin(z).sin(x) ]   [ cos(y), 0, -sin(y) ]
    // [ rowY ] = [ -sin(z),  cos(z).cos(x), cos(z).sin(x) ] * [ 0,      1,  0      ]
    // [ rowZ ]   [  0,      -sin(x),        cos(x)        ]   [ sin(y), 0,  cos(y) ]
    
    // [ rowX ]   [  cos(z).cos(y) + sin(z).sin(x).sin(y),  sin(z).cos(x), -cos(z).sin(y) + sin(z).sin(x).cos(y) ]
    // [ rowY ] = [ -sin(z).cos(y) + cos(z).sin(x).sin(y),  cos(z).cos(x),  sin(z).sin(y) + cos(z).sin(x).cos(y) ]
    // [ rowZ ]   [  cos(x).sin(y),                        -sin(x),         cos(x).cos(y)                        ]
    
    const Vector3 rowX( RowX( m ) );
    const Vector3 rowY( RowY( m ) );
    const Vector3 rowZ( RowZ( m ) );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 nearlyOne( Const<VF32>::Frac<9999999,10000000>() );
    const VF32 sinX( -Y( rowZ ) );
    
    const VBool abs_sinX_LT_nearlyOne( Abs( sinX ) < nearlyOne );
    
    // For x < 90 degrees
    const VF32 sinY( X( rowZ ) ); // cos(x).sin(y), cos(x) cancels out in ArcTan division
    const VF32 cosY( Z( rowZ ) ); // cos(x).cos(y)
    const VF32 sinZ( Y( rowX ) ); // sin(z).cos(x)
    const VF32 cosZ( Y( rowY ) ); // cos(z).cos(x)
    
    // For x ~= 90 degrees = pi/2 radians, sinX ~= 1, cosX ~= 0
    // When x = 90 degrees, rotations about y & z are equivalent (rotations about the same axis),
    // so we can set y or z to zero and solve for the other. For y=0, sin(y) = 0, cos(y) = 1
    const VF32 sinZ2( -X( rowY ) ); // sin(z).cos(y) - cos(z).sin(x).sin(y) = sin(z)
    const VF32 cosZ2(  X( rowX ) ); // cos(z).cos(y) + sin(z).sin(x).sin(y) = cos(z)
    
    const Vector3 arctan_x( MakeVector3( sinY, sinZ, sinZ2 ) );
    const Vector3 arctan_y( MakeVector3( cosY, cosZ, cosZ2 ) );
    const Vector3 y1_z1_z2( ArcTan( arctan_x, arctan_y ) );
    const VF32 y1( X( y1_z1_z2 ) );
    const VF32 y2( zero );
    const VF32 z1( Y( y1_z1_z2 ) );
    const VF32 z2( Z( y1_z1_z2 ) );
    
    const VF32 x( ArcSin( sinX ) );
    const VF32 y( Select( abs_sinX_LT_nearlyOne, y1, y2 ) );
    const VF32 z( Select( abs_sinX_LT_nearlyOne, z1, z2 ) );
    
    return MakeEulerZXY( x, y, z );
}

/// Make EulerZYX from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerZYX MakeEulerZYX( const Mat33 MATREF m )
{
    // Matrix ZYX =
    
    // [ rowX ]   [  cos(z), sin(z), 0 ]   [ cos(y), 0, -sin(y) ]   [ 1,  0,      0      ]
    // [ rowY ] = [ -sin(z), cos(z), 0 ] * [ 0,      1,  0      ] * [ 0,  cos(x), sin(x) ]
    // [ rowZ ]   [  0,      0,      1 ]   [ sin(y), 0,  cos(y) ]   [ 0, -sin(x), cos(x) ]
    
    // [ rowX ]   [  cos(z).cos(y), sin(z), -cos(z).sin(y) ]   [ 1,  0,      0      ]
    // [ rowY ] = [ -sin(z).cos(y), cos(z),  sin(z).sin(y) ] * [ 0,  cos(x), sin(x) ]
    // [ rowZ ]   [  sin(y),        0,       cos(y)        ]   [ 0, -sin(x), cos(x) ]
    
    // [ rowX ]   [  cos(z).cos(y),  sin(z).cos(x) + cos(z).sin(y).sin(x), sin(z).sin(x) - cos(z).sin(y).cos(x) ]
    // [ rowY ] = [ -sin(z).cos(y),  cos(z).cos(x) - sin(z).sin(y).sin(x), cos(z).sin(x) + sin(z).sin(y).cos(x) ]
    // [ rowZ ]   [  sin(y),        -cos(y).sin(x),                        cos(y).cos(x)                        ]
    
    const Vector3 rowX( RowX( m ) );
    const Vector3 rowY( RowY( m ) );
    const Vector3 rowZ( RowZ( m ) );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 nearlyOne( Const<VF32>::Frac<9999999,10000000>() );
    const VF32 sinY( X( rowZ ) ); // WARNING: deviation from aftk MatrixToEulerZYX()!!!
    
    const VBool abs_sinY_LT_nearlyOne( Abs( sinY ) < nearlyOne );
    
    // For y < 90 degrees
    const VF32 sinX( -Y( rowZ ) ); // cos(y).sin(x), cos(y) cancels out in ArcTan division
    const VF32 cosX(  Z( rowZ ) ); // cos(y).cos(x)
    const VF32 sinZ( -X( rowY ) ); // sin(z).cos(y)
    const VF32 cosZ(  X( rowX ) ); // cos(z).cos(y)
    
    // For y ~= 90 degrees = pi/2 radians, sinY ~= 1, cosY ~= 0
    // When y = 90 degrees, rotations about x & z are equivalent (rotations about the same axis),
    // so we can set x or z to zero and solve for the other. For x=0, sin(x) = 0, cos(x) = 1
    const VF32 sinZ2( Y( rowX ) ); // sin(z).cos(x) + cos(z).sin(y).sin(x) = sin(z)
    const VF32 cosZ2( Y( rowY ) ); // cos(z).cos(x) - sin(z).sin(y).sin(x) = cos(z)
    
    const Vector3 arctan_x( MakeVector3( sinX, sinZ, sinZ2 ) );
    const Vector3 arctan_y( MakeVector3( cosX, cosZ, cosZ2 ) );
    const Vector3 x1_z1_z2( ArcTan( arctan_x, arctan_y ) );
    const VF32 x1( X( x1_z1_z2 ) );
    const VF32 x2( zero );
    const VF32 z1( Y( x1_z1_z2 ) );
    const VF32 z2( Z( x1_z1_z2 ) );
    
    const VF32 x( Select( abs_sinY_LT_nearlyOne, x1, x2 ) );
    const VF32 y( ArcSin( sinY ) );
    const VF32 z( Select( abs_sinY_LT_nearlyOne, z1, z2 ) );
    
    return MakeEulerZYX( x, y, z );
}




//! @}
}; // namespace Maths


