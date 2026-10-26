#include "CorePch.h"

namespace Maths
{

#if 0

void AssertEqual( const F32 a, const F32 b )
{
    if ( Abs( a - b ) > 0.001f )
    {
        DebugBreak();
    }
}

void Test()
{
    //
    // HMat33 Tests
    //
    
    if ( 1 ) // HMat33 Default Constructor
    {
        const HMat33 a;
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
     }
     
     if ( 1 ) // HMat33 Zero Constructor
     {
        const HMat33 a( ZERO_MATRIX );
        AssertEqual( a.AX().X(), 0.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 0.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 0.0f );
    }
    
    if ( 1 ) // HMat33 Copy Constructor
    {
        const HMat33 a;
        const HMat33 b( a );
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 0.0f ); AssertEqual( b.AX().Z(), 0.0f );
        AssertEqual( b.AY().X(), 0.0f ); AssertEqual( b.AY().Y(), 1.0f ); AssertEqual( b.AY().Z(), 0.0f );
        AssertEqual( b.AZ().X(), 0.0f ); AssertEqual( b.AZ().Y(), 0.0f ); AssertEqual( b.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 <- HMat43
    {
        const HMat43 a( HVec3( 1.0f,  2.0f,  3.0f ),
                        HVec3( 4.0f,  5.0f,  6.0f ),
                        HVec3( 7.0f,  8.0f,  9.0f ),
                        HVec3( 10.0f, 11.0f, 12.0f ) );
        const HMat33 b( a );
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 2.0f ); AssertEqual( b.AX().Z(), 3.0f );
        AssertEqual( b.AY().X(), 4.0f ); AssertEqual( b.AY().Y(), 5.0f ); AssertEqual( b.AY().Z(), 6.0f );
        AssertEqual( b.AZ().X(), 7.0f ); AssertEqual( b.AZ().Y(), 8.0f ); AssertEqual( b.AZ().Z(), 9.0f );
    }
    
    if ( 1 ) // HMat33 <- HMat44
    {
        const HMat44 a( HVec4( 1.0f,  2.0f,  3.0f,  4.0f ),
                        HVec4( 5.0f,  6.0f,  7.0f,  8.0f ),
                        HVec4( 9.0f,  10.0f, 11.0f, 12.0f ),
                        HVec4( 13.0f, 14.0f, 15.0f, 16.0f ) );
        const HMat33 b( a );
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 2.0f );  AssertEqual( b.AX().Z(), 3.0f );
        AssertEqual( b.AY().X(), 5.0f ); AssertEqual( b.AY().Y(), 6.0f );  AssertEqual( b.AY().Z(), 7.0f );
        AssertEqual( b.AZ().X(), 9.0f ); AssertEqual( b.AZ().Y(), 10.0f ); AssertEqual( b.AZ().Z(), 11.0f );
    }
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    if ( 1 ) // HMat33 <- ( vector float, vector float, vector float )
    {
        const vector float a = (vector float)( 1.0f, 2.0f,  3.0f,  4.0f );
        const vector float b = (vector float)( 5.0f, 6.0f,  7.0f,  8.0f );
        const vector float c = (vector float)( 9.0f, 10.0f, 11.0f, 12.0f );
        const HMat33 d( a, b, c );
        AssertEqual( d.AX().X(), 1.0f ); AssertEqual( d.AX().Y(), 2.0f );  AssertEqual( d.AX().Z(), 3.0f );
        AssertEqual( d.AY().X(), 5.0f ); AssertEqual( d.AY().Y(), 6.0f );  AssertEqual( d.AY().Z(), 7.0f );
        AssertEqual( d.AZ().X(), 9.0f ); AssertEqual( d.AZ().Y(), 10.0f ); AssertEqual( d.AZ().Z(), 11.0f );
    }
    #endif
    
    if ( 1 ) // HMat33 <- ( HVec3, HVec3, HVec3 )
    {
        const C_MatrixScale a( HVec3( 1.0f, 2.0f, 3.0f ) );
        const HMat33 b( a );
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 0.0f ); AssertEqual( b.AX().Z(), 0.0f );
        AssertEqual( b.AY().X(), 0.0f ); AssertEqual( b.AY().Y(), 2.0f ); AssertEqual( b.AY().Z(), 0.0f );
        AssertEqual( b.AZ().X(), 0.0f ); AssertEqual( b.AZ().Y(), 0.0f ); AssertEqual( b.AZ().Z(), 3.0f );
    }
    
    // Cross products for deriving third matrix axis from 2 input axes
    // const HVec3 ax = ay ^ az;
    // const HVec3 ay = az ^ ax;
    // const HVec3 az = ax ^ ay;
    
    if ( 1 ) // HMat33 <- AxisZ
    {
        const AxisZ a_axis_z( C_Normal::AxisPositiveZ() );
        const AxisZ b_axis_z( C_Normal::AxisPositiveY() );
        const HMat33 a( a_axis_z );
        const HMat33 b( b_axis_z );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 0.0f ); AssertEqual( b.AX().Z(), 0.0f );
        AssertEqual( b.AY().X(), 0.0f ); AssertEqual( b.AY().Y(), 0.0f ); AssertEqual( b.AY().Z(), -1.0f );
        AssertEqual( b.AZ().X(), 0.0f ); AssertEqual( b.AZ().Y(), 1.0f ); AssertEqual( b.AZ().Z(), 0.0f );
    }
    
    if ( 1 ) // HMat33 <- AxisX, AxisY
    {
        const AxisX axis_x( C_Normal::AxisPositiveX() );
        const AxisY axis_y( C_Normal::AxisPositiveY() );
        const HMat33 a( axis_x, axis_y );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 <- AxisX, AxisZ
    {
        const AxisX axis_x( C_Normal::AxisPositiveX() );
        const AxisZ axis_z( C_Normal::AxisPositiveZ() );
        const HMat33 a( axis_x, axis_z );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 <- AxisY, AxisX
    {
        const AxisY axis_y( C_Normal::AxisPositiveY() );
        const AxisX axis_x( C_Normal::AxisPositiveX() );
        const HMat33 a( axis_y, axis_x );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 <- AxisY, AxisZ
    {
        const AxisY axis_y( C_Normal::AxisPositiveY() );
        const AxisZ axis_z( C_Normal::AxisPositiveZ() );
        const HMat33 a( axis_y, axis_z );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 <- AxisZ, AxisX
    {
        const AxisZ axis_z( C_Normal::AxisPositiveZ() );
        const AxisX axis_x( C_Normal::AxisPositiveX() );
        const HMat33 a( axis_z, axis_x );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 <- AxisZ, AxisY
    {
        const AxisZ axis_z( C_Normal::AxisPositiveZ() );
        const AxisY axis_y( C_Normal::AxisPositiveY() );
        const HMat33 a( axis_z, axis_y );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 = HMat33
    {
        const HMat33 a;
        const HMat33 b = a;
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 0.0f ); AssertEqual( b.AX().Z(), 0.0f );
        AssertEqual( b.AY().X(), 0.0f ); AssertEqual( b.AY().Y(), 1.0f ); AssertEqual( b.AY().Z(), 0.0f );
        AssertEqual( b.AZ().X(), 0.0f ); AssertEqual( b.AZ().Y(), 0.0f ); AssertEqual( b.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 SetAxes
    {
        HMat33 a( ZERO_MATRIX );
        a.SetAxes( HVec3( 1.0f, 2.0f, 3.0f ),
                   HVec3( 4.0f, 5.0f, 6.0f ),
                   HVec3( 7.0f, 8.0f, 9.0f ) );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 2.0f ); AssertEqual( a.AX().Z(), 3.0f );
        AssertEqual( a.AY().X(), 4.0f ); AssertEqual( a.AY().Y(), 5.0f ); AssertEqual( a.AY().Z(), 6.0f );
        AssertEqual( a.AZ().X(), 7.0f ); AssertEqual( a.AZ().Y(), 8.0f ); AssertEqual( a.AZ().Z(), 9.0f );
    }
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    if ( 1 ) // HMat33 SetAxesXYZ
    {
        HMat33 a( ZERO_MATRIX );
        a.SetAxesXYZ( (vector float)( 1.0f, 2.0f, 3.0f, 0.0f ),
                      (vector float)( 4.0f, 5.0f, 6.0f, 0.0f ),
                      (vector float)( 7.0f, 8.0f, 9.0f, 0.0f ) );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 2.0f ); AssertEqual( a.AX().Z(), 3.0f );
        AssertEqual( a.AY().X(), 4.0f ); AssertEqual( a.AY().Y(), 5.0f ); AssertEqual( a.AY().Z(), 6.0f );
        AssertEqual( a.AZ().X(), 7.0f ); AssertEqual( a.AZ().Y(), 8.0f ); AssertEqual( a.AZ().Z(), 9.0f );
    }
    #endif
    
    if ( 1 ) // HMat33 Get Axis X
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HVec3 b( a.AX() );
        AssertEqual( b.X(), 1.0f ); AssertEqual( b.Y(), 2.0f ); AssertEqual( b.Z(), 3.0f );
    }
    
    if ( 1 ) // HMat33 Get Axis Y
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HVec3 b( a.AY() );
        AssertEqual( b.X(), 4.0f ); AssertEqual( b.Y(), 5.0f ); AssertEqual( b.Z(), 6.0f );
    }
    
    if ( 1 ) // HMat33 Get Axis Z
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HVec3 b( a.AZ() );
        AssertEqual( b.X(), 7.0f ); AssertEqual( b.Y(), 8.0f ); AssertEqual( b.Z(), 9.0f );
    }
    
    if ( 1 ) // HMat33 Set Axis X
    {
        HMat33 a( ZERO_MATRIX );
        a.AX( HVec3( 1.0f, 2.0f, 3.0f ) );
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 2.0f ); AssertEqual( a.AX().Z(), 3.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 0.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 0.0f );
    }
    
    if ( 1 ) // HMat33 Set Axis Y
    {
        HMat33 a( ZERO_MATRIX );
        a.AY( HVec3( 4.0f, 5.0f, 6.0f ) );
        AssertEqual( a.AX().X(), 0.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 4.0f ); AssertEqual( a.AY().Y(), 5.0f ); AssertEqual( a.AY().Z(), 6.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 0.0f );
    }
    
    if ( 1 ) // HMat33 Set Axis Z
    {
        HMat33 a( ZERO_MATRIX );
        a.AZ( HVec3( 7.0f, 8.0f, 9.0f ) );
        AssertEqual( a.AX().X(), 0.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 0.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 7.0f ); AssertEqual( a.AZ().Y(), 8.0f ); AssertEqual( a.AZ().Z(), 9.0f );
    }
    
    if ( 1 ) // HMat33 Set Identity
    {
        HMat33 a( ZERO_MATRIX );
        a.SetIdentity();
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 Transposed
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HMat33 b = a.Transposed();
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 4.0f ); AssertEqual( b.AX().Z(), 7.0f );
        AssertEqual( b.AY().X(), 2.0f ); AssertEqual( b.AY().Y(), 5.0f ); AssertEqual( b.AY().Z(), 8.0f );
        AssertEqual( b.AZ().X(), 3.0f ); AssertEqual( b.AZ().Y(), 6.0f ); AssertEqual( b.AZ().Z(), 9.0f );
    }
    
    if ( 1 ) // HMat33 Transpose
    {
        HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                  HVec3( 4.0f, 5.0f, 6.0f ),
                  HVec3( 7.0f, 8.0f, 9.0f ) );
        a.Transpose();
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 4.0f ); AssertEqual( a.AX().Z(), 7.0f );
        AssertEqual( a.AY().X(), 2.0f ); AssertEqual( a.AY().Y(), 5.0f ); AssertEqual( a.AY().Z(), 8.0f );
        AssertEqual( a.AZ().X(), 3.0f ); AssertEqual( a.AZ().Y(), 6.0f ); AssertEqual( a.AZ().Z(), 9.0f );
    }
    
    if ( 1 ) // HMat33 * HF32
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HMat33 b = a * 2.0f;
        AssertEqual( b.AX().X(), 2.0f );  AssertEqual( b.AX().Y(), 4.0f );  AssertEqual( b.AX().Z(), 6.0f );
        AssertEqual( b.AY().X(), 8.0f );  AssertEqual( b.AY().Y(), 10.0f ); AssertEqual( b.AY().Z(), 12.0f );
        AssertEqual( b.AZ().X(), 14.0f ); AssertEqual( b.AZ().Y(), 16.0f ); AssertEqual( b.AZ().Z(), 18.0f );
    }
    
    if ( 1 ) // HF32 * HMat33
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HMat33 b = 3.0f * a;
        AssertEqual( b.AX().X(), 3.0f );  AssertEqual( b.AX().Y(), 6.0f );  AssertEqual( b.AX().Z(), 9.0f );
        AssertEqual( b.AY().X(), 12.0f ); AssertEqual( b.AY().Y(), 15.0f ); AssertEqual( b.AY().Z(), 18.0f );
        AssertEqual( b.AZ().X(), 21.0f ); AssertEqual( b.AZ().Y(), 24.0f ); AssertEqual( b.AZ().Z(), 27.0f );
    }
    
    if ( 1 ) // HMat33 + HMat33
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HMat33 b( HVec3( 10.0f, 11.0f, 12.0f ),
                        HVec3( 13.0f, 14.0f, 15.0f ),
                        HVec3( 16.0f, 17.0f, 18.0f ) );
        const HMat33 c = a + b;
        AssertEqual( c.AX().X(), 11.0f ); AssertEqual( c.AX().Y(), 13.0f ); AssertEqual( c.AX().Z(), 15.0f );
        AssertEqual( c.AY().X(), 17.0f ); AssertEqual( c.AY().Y(), 19.0f ); AssertEqual( c.AY().Z(), 21.0f );
        AssertEqual( c.AZ().X(), 23.0f ); AssertEqual( c.AZ().Y(), 25.0f ); AssertEqual( c.AZ().Z(), 27.0f );
    }
    
    if ( 1 ) // HMat33 - HMat33
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HMat33 b( HVec3( 10.0f, 11.0f, 12.0f ),
                        HVec3( 13.0f, 14.0f, 15.0f ),
                        HVec3( 16.0f, 17.0f, 18.0f ) );
        const HMat33 c = b - a;
        AssertEqual( c.AX().X(), 9.0f ); AssertEqual( c.AX().Y(), 9.0f ); AssertEqual( c.AX().Z(), 9.0f );
        AssertEqual( c.AY().X(), 9.0f ); AssertEqual( c.AY().Y(), 9.0f ); AssertEqual( c.AY().Z(), 9.0f );
        AssertEqual( c.AZ().X(), 9.0f ); AssertEqual( c.AZ().Y(), 9.0f ); AssertEqual( c.AZ().Z(), 9.0f );
    }
    
    if ( 1 ) // HMat33 * HMat33
    {
        const HMat33 a( HVec3( 0.0f, 0.0f, -1.0f ),
                        HVec3( 0.0f, 1.0f, 0.0f ),
                        HVec3( 1.0f, 0.0f, 0.0f ) );
        const HMat33 b( HVec3( 1.0f, 0.0f, 0.0f ),
                        HVec3( 0.0f, 0.0f, -1.0f ),
                        HVec3( 0.0f, 1.0f, 0.0f ) );
        const HMat33 c = a * b;
        AssertEqual( c.AX().X(), 0.0f ); AssertEqual( c.AX().Y(), -1.0f ); AssertEqual( c.AX().Z(), 0.0f );
        AssertEqual( c.AY().X(), 0.0f ); AssertEqual( c.AY().Y(), 0.0f );  AssertEqual( c.AY().Z(), -1.0f );
        AssertEqual( c.AZ().X(), 1.0f ); AssertEqual( c.AZ().Y(), 0.0f );  AssertEqual( c.AZ().Z(), 0.0f );
    }
    
    if ( 1 ) // HMat33 * HVec3
    {
        const HMat33 a( HVec3( 0.0f, 0.0f, -1.0f ),
                        HVec3( 0.0f, 1.0f, 0.0f ),
                        HVec3( 1.0f, 0.0f, 0.0f ) );
        const HVec3 b( 1.0f, 0.0f, 0.0f );
        const HVec3 c = a * b;
        AssertEqual( c.X(), 0.0f ); AssertEqual( c.Y(), 0.0f ); AssertEqual( c.Z(), 1.0f );
    }
    
    if ( 1 ) // HVec3 * HMat33
    {
        const HVec3 a( 1.0f, 0.0f, 0.0f );
        const HMat33 b( HVec3( 0.0f, 0.0f, -1.0f ),
                        HVec3( 0.0f, 1.0f, 0.0f ),
                        HVec3( 1.0f, 0.0f, 0.0f ) );
        const HVec3 c = a * b;
        AssertEqual( c.X(), 0.0f ); AssertEqual( c.Y(), 0.0f ); AssertEqual( c.Z(), -1.0f );
    }
    
    if ( 1 ) // HMat43 <- HMat33 || HVec3
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HVec3 b( 10.0f, 11.0f, 12.0f );
        const HMat43 c = a || b;
        AssertEqual( c.AX().X(), 1.0f );  AssertEqual( c.AX().Y(), 2.0f );  AssertEqual( c.AX().Z(), 3.0f );
        AssertEqual( c.AY().X(), 4.0f );  AssertEqual( c.AY().Y(), 5.0f );  AssertEqual( c.AY().Z(), 6.0f );
        AssertEqual( c.AZ().X(), 7.0f );  AssertEqual( c.AZ().Y(), 8.0f );  AssertEqual( c.AZ().Z(), 9.0f );
        AssertEqual( c.AW().X(), 10.0f ); AssertEqual( c.AW().Y(), 11.0f ); AssertEqual( c.AW().Z(), 12.0f );
    }
    
    if ( 1 ) // HMat33 Trace
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HF32 b = a.Trace();
        AssertEqual( b, 15.0f );
    }
    
    if ( 1 ) // HMat33 Determinant
    {
        const HMat33 a( HVec3( 1.0f, 0.0f, 0.0f ),
                        HVec3( 0.0f, 1.0f, 0.0f ),
                        HVec3( 0.0f, 0.0f, 1.0f ) );
        const HF32 b = a.Determinant();
        AssertEqual( b, 1.0f );
    }
    
    if ( 1 ) // HMat33 NormaliseZY
    {
        HMat33 a( HVec3( 0.0f, 0.0f, 0.0f ),
                  HVec3( 0.0f, 1.0f, 0.0f ),
                  HVec3( 0.0f, 0.0f, 1.0f ) );
        a.NormaliseZY();
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 NormalisedZY
    {
        const HMat33 a( HVec3( 0.0f, 0.0f, 0.0f ),
                        HVec3( 0.0f, 1.0f, 0.0f ),
                        HVec3( 0.0f, 0.0f, 1.0f ) );
        const HMat33 b = a.NormalisedZY();
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 0.0f ); AssertEqual( b.AX().Z(), 0.0f );
        AssertEqual( b.AY().X(), 0.0f ); AssertEqual( b.AY().Y(), 1.0f ); AssertEqual( b.AY().Z(), 0.0f );
        AssertEqual( b.AZ().X(), 0.0f ); AssertEqual( b.AZ().Y(), 0.0f ); AssertEqual( b.AZ().Z(), 1.0f );
    }
    
    if ( 1 ) // HMat33 Inverse
    {
        const HMat33 a( HVec3( 2.0f, 0.0f, 0.0f ),
                        HVec3( 0.0f, 2.0f, 0.0f ),
                        HVec3( 0.0f, 0.0f, 2.0f ) );
        const HMat33 b( a.Inverse() );
        AssertEqual( b.AX().X(), 0.5f );
        AssertEqual( b.AX().Y(), 0.0f );
        AssertEqual( b.AX().Z(), 0.0f );
        AssertEqual( b.AY().X(), 0.0f );
        AssertEqual( b.AY().Y(), 0.5f );
        AssertEqual( b.AY().Z(), 0.0f );
        AssertEqual( b.AZ().X(), 0.0f );
        AssertEqual( b.AZ().Y(), 0.0f );
        AssertEqual( b.AZ().Z(), 0.5f );
    }
    
    //
    // HMat43 Tests
    //
    
    if ( 1 ) // HMat43 Default Constructor
    {
        const HMat43 a;
        AssertEqual( a.AX().X(), 1.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 1.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 1.0f );
        AssertEqual( a.AW().X(), 0.0f ); AssertEqual( a.AW().Y(), 0.0f ); AssertEqual( a.AW().Z(), 0.0f );
     }
     
     if ( 1 ) // HMat43 Zero Constructor
     {
        const HMat43 a( ZERO_MATRIX );
        AssertEqual( a.AX().X(), 0.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 0.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 0.0f );
        AssertEqual( a.AW().X(), 0.0f ); AssertEqual( a.AW().Y(), 0.0f ); AssertEqual( a.AW().Z(), 0.0f );
    }
    
    if ( 1 ) // HMat43 Copy Constructor
    {
        const HMat43 a;
        const HMat43 b( a );
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 0.0f ); AssertEqual( b.AX().Z(), 0.0f );
        AssertEqual( b.AY().X(), 0.0f ); AssertEqual( b.AY().Y(), 1.0f ); AssertEqual( b.AY().Z(), 0.0f );
        AssertEqual( b.AZ().X(), 0.0f ); AssertEqual( b.AZ().Y(), 0.0f ); AssertEqual( b.AZ().Z(), 1.0f );
        AssertEqual( b.AW().X(), 0.0f ); AssertEqual( b.AW().Y(), 0.0f ); AssertEqual( b.AW().Z(), 0.0f );
    }
    
    if ( 1 ) // HMat43 <-- HMat33
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HMat43 b( a );
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 2.0f ); AssertEqual( b.AX().Z(), 3.0f );
        AssertEqual( b.AY().X(), 4.0f ); AssertEqual( b.AY().Y(), 5.0f ); AssertEqual( b.AY().Z(), 6.0f );
        AssertEqual( b.AZ().X(), 7.0f ); AssertEqual( b.AZ().Y(), 8.0f ); AssertEqual( b.AZ().Z(), 9.0f );
        AssertEqual( b.AW().X(), 0.0f ); AssertEqual( b.AW().Y(), 0.0f ); AssertEqual( b.AW().Z(), 0.0f );
    }
    
    if ( 1 ) // HMat43 <-- HMat44
    {
        const HMat44 a( HVec4( 1.0f,  2.0f,  3.0f,  4.0f ),
                        HVec4( 5.0f,  6.0f,  7.0f,  8.0f ),
                        HVec4( 9.0f,  10.0f, 11.0f, 12.0f ),
                        HVec4( 13.0f, 14.0f, 15.0f, 16.0f ) );
        const HMat43 b( a );
        AssertEqual( b.AX().X(), 1.0f );  AssertEqual( b.AX().Y(), 2.0f );  AssertEqual( b.AX().Z(), 3.0f );
        AssertEqual( b.AY().X(), 5.0f );  AssertEqual( b.AY().Y(), 6.0f );  AssertEqual( b.AY().Z(), 7.0f );
        AssertEqual( b.AZ().X(), 9.0f );  AssertEqual( b.AZ().Y(), 10.0f ); AssertEqual( b.AZ().Z(), 11.0f );
        AssertEqual( b.AW().X(), 13.0f ); AssertEqual( b.AW().Y(), 14.0f ); AssertEqual( b.AW().Z(), 15.0f );
    }
    
    if ( 1 ) // HMat43 <-- ( HMat33, HVec3 )
    {
        const HMat33 a( HVec3( 1.0f, 2.0f, 3.0f ),
                        HVec3( 4.0f, 5.0f, 6.0f ),
                        HVec3( 7.0f, 8.0f, 9.0f ) );
        const HVec3 b( 10.0f, 11.0f, 12.0f );
        const HMat43 c( a, b );
        AssertEqual( c.AX().X(), 1.0f );  AssertEqual( c.AX().Y(), 2.0f );  AssertEqual( c.AX().Z(), 3.0f );
        AssertEqual( c.AY().X(), 4.0f );  AssertEqual( c.AY().Y(), 5.0f );  AssertEqual( c.AY().Z(), 6.0f );
        AssertEqual( c.AZ().X(), 7.0f );  AssertEqual( c.AZ().Y(), 8.0f );  AssertEqual( c.AZ().Z(), 9.0f );
        AssertEqual( c.AW().X(), 10.0f ); AssertEqual( c.AW().Y(), 11.0f ); AssertEqual( c.AW().Z(), 12.0f );
    }
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    if ( 1 ) // HMat43 <- ( vector float, vector float, vector float, vector float )
    {
        const vector float a = (vector float)( 1.0f,  2.0f,  3.0f,  4.0f );
        const vector float b = (vector float)( 5.0f,  6.0f,  7.0f,  8.0f );
        const vector float c = (vector float)( 9.0f,  10.0f, 11.0f, 12.0f );
        const vector float d = (vector float)( 13.0f, 14.0f, 15.0f, 16.0f );
        const HMat43 e( a, b, c, d );
        AssertEqual( e.AX().X(), 1.0f );  AssertEqual( e.AX().Y(), 2.0f );  AssertEqual( e.AX().Z(), 3.0f );
        AssertEqual( e.AY().X(), 5.0f );  AssertEqual( e.AY().Y(), 6.0f );  AssertEqual( e.AY().Z(), 7.0f );
        AssertEqual( e.AZ().X(), 9.0f );  AssertEqual( e.AZ().Y(), 10.0f ); AssertEqual( e.AZ().Z(), 11.0f );
        AssertEqual( e.AW().X(), 13.0f ); AssertEqual( e.AW().Y(), 14.0f ); AssertEqual( e.AW().Z(), 15.0f );
    }
    #endif
    
    if ( 1 ) // HMat43 <- ( HVec3, HVec3, HVec3, HVec3 )
    {
        const HVec3 a( HVec3( 1.0f,  2.0f,  3.0f ) );
        const HVec3 b( HVec3( 4.0f,  5.0f,  6.0f ) );
        const HVec3 c( HVec3( 7.0f,  8.0f,  9.0f ) );
        const HVec3 d( HVec3( 10.0f, 11.0f, 12.0f ) );
        const HMat43 e( a, b, c, d );
        AssertEqual( e.AX().X(), 1.0f );  AssertEqual( e.AX().Y(), 2.0f );  AssertEqual( e.AX().Z(), 3.0f );
        AssertEqual( e.AY().X(), 4.0f );  AssertEqual( e.AY().Y(), 5.0f );  AssertEqual( e.AY().Z(), 6.0f );
        AssertEqual( e.AZ().X(), 7.0f );  AssertEqual( e.AZ().Y(), 8.0f );  AssertEqual( e.AZ().Z(), 9.0f );
        AssertEqual( e.AW().X(), 10.0f ); AssertEqual( e.AW().Y(), 11.0f ); AssertEqual( e.AW().Z(), 12.0f );
    }
    
    if ( 1 ) // HMat43 <- ( AxisX, AxisY, HVec3 )
    {
        const AxisX a( C_Normal( HVec3( 1.0f, 0.0f, 0.0f ) ) );
        const AxisY b( C_Normal( HVec3( 0.0f, 1.0f, 0.0f ) ) );
        const HVec3 c( 2.0f, 3.0f, 4.0f );
        const HMat43 d( a, b, c );
        AssertEqual( d.AX().X(), 1.0f ); AssertEqual( d.AX().Y(), 0.0f ); AssertEqual( d.AX().Z(), 0.0f );
        AssertEqual( d.AY().X(), 0.0f ); AssertEqual( d.AY().Y(), 1.0f ); AssertEqual( d.AY().Z(), 0.0f );
        AssertEqual( d.AZ().X(), 0.0f ); AssertEqual( d.AZ().Y(), 0.0f ); AssertEqual( d.AZ().Z(), 1.0f );
        AssertEqual( d.AW().X(), 2.0f ); AssertEqual( d.AW().Y(), 3.0f ); AssertEqual( d.AW().Z(), 4.0f );
    }
    
    if ( 1 ) // HMat43 <- ( AxisX, AxisZ, HVec3 )
    {
        const AxisX a( C_Normal( HVec3( 1.0f, 0.0f, 0.0f ) ) );
        const AxisZ b( C_Normal( HVec3( 0.0f, 0.0f, 1.0f ) ) );
        const HVec3 c( 2.0f, 3.0f, 4.0f );
        const HMat43 d( a, b, c );
        AssertEqual( d.AX().X(), 1.0f ); AssertEqual( d.AX().Y(), 0.0f ); AssertEqual( d.AX().Z(), 0.0f );
        AssertEqual( d.AY().X(), 0.0f ); AssertEqual( d.AY().Y(), 1.0f ); AssertEqual( d.AY().Z(), 0.0f );
        AssertEqual( d.AZ().X(), 0.0f ); AssertEqual( d.AZ().Y(), 0.0f ); AssertEqual( d.AZ().Z(), 1.0f );
        AssertEqual( d.AW().X(), 2.0f ); AssertEqual( d.AW().Y(), 3.0f ); AssertEqual( d.AW().Z(), 4.0f );
    }
    
    if ( 1 ) // HMat43 <- ( AxisY, AxisX, HVec3 )
    {
        const AxisY a( C_Normal( HVec3( 0.0f, 1.0f, 0.0f ) ) );
        const AxisX b( C_Normal( HVec3( 1.0f, 0.0f, 0.0f ) ) );
        const HVec3 c( 2.0f, 3.0f, 4.0f );
        const HMat43 d( a, b, c );
        AssertEqual( d.AX().X(), 1.0f ); AssertEqual( d.AX().Y(), 0.0f ); AssertEqual( d.AX().Z(), 0.0f );
        AssertEqual( d.AY().X(), 0.0f ); AssertEqual( d.AY().Y(), 1.0f ); AssertEqual( d.AY().Z(), 0.0f );
        AssertEqual( d.AZ().X(), 0.0f ); AssertEqual( d.AZ().Y(), 0.0f ); AssertEqual( d.AZ().Z(), 1.0f );
        AssertEqual( d.AW().X(), 2.0f ); AssertEqual( d.AW().Y(), 3.0f ); AssertEqual( d.AW().Z(), 4.0f );
    }
    
    if ( 1 ) // HMat43 <- ( AxisY, AxisZ, HVec3 )
    {
        const AxisY a( C_Normal( HVec3( 0.0f, 1.0f, 0.0f ) ) );
        const AxisZ b( C_Normal( HVec3( 0.0f, 0.0f, 1.0f ) ) );
        const HVec3 c( 2.0f, 3.0f, 4.0f );
        const HMat43 d( a, b, c );
        AssertEqual( d.AX().X(), 1.0f ); AssertEqual( d.AX().Y(), 0.0f ); AssertEqual( d.AX().Z(), 0.0f );
        AssertEqual( d.AY().X(), 0.0f ); AssertEqual( d.AY().Y(), 1.0f ); AssertEqual( d.AY().Z(), 0.0f );
        AssertEqual( d.AZ().X(), 0.0f ); AssertEqual( d.AZ().Y(), 0.0f ); AssertEqual( d.AZ().Z(), 1.0f );
        AssertEqual( d.AW().X(), 2.0f ); AssertEqual( d.AW().Y(), 3.0f ); AssertEqual( d.AW().Z(), 4.0f );
    }
    
    if ( 1 ) // HMat43 <- ( AxisZ, AxisX, HVec3 )
    {
        const AxisZ a( C_Normal( HVec3( 0.0f, 0.0f, 1.0f ) ) );
        const AxisX b( C_Normal( HVec3( 1.0f, 0.0f, 0.0f ) ) );
        const HVec3 c( 2.0f, 3.0f, 4.0f );
        const HMat43 d( a, b, c );
        AssertEqual( d.AX().X(), 1.0f ); AssertEqual( d.AX().Y(), 0.0f ); AssertEqual( d.AX().Z(), 0.0f );
        AssertEqual( d.AY().X(), 0.0f ); AssertEqual( d.AY().Y(), 1.0f ); AssertEqual( d.AY().Z(), 0.0f );
        AssertEqual( d.AZ().X(), 0.0f ); AssertEqual( d.AZ().Y(), 0.0f ); AssertEqual( d.AZ().Z(), 1.0f );
        AssertEqual( d.AW().X(), 2.0f ); AssertEqual( d.AW().Y(), 3.0f ); AssertEqual( d.AW().Z(), 4.0f );
    }
    
    if ( 1 ) // HMat43 <- ( AxisZ, AxisY, HVec3 )
    {
        const AxisZ a( C_Normal( HVec3( 0.0f, 0.0f, 1.0f ) ) );
        const AxisY b( C_Normal( HVec3( 0.0f, 1.0f, 0.0f ) ) );
        const HVec3 c( 2.0f, 3.0f, 4.0f );
        const HMat43 d( a, b, c );
        AssertEqual( d.AX().X(), 1.0f ); AssertEqual( d.AX().Y(), 0.0f ); AssertEqual( d.AX().Z(), 0.0f );
        AssertEqual( d.AY().X(), 0.0f ); AssertEqual( d.AY().Y(), 1.0f ); AssertEqual( d.AY().Z(), 0.0f );
        AssertEqual( d.AZ().X(), 0.0f ); AssertEqual( d.AZ().Y(), 0.0f ); AssertEqual( d.AZ().Z(), 1.0f );
        AssertEqual( d.AW().X(), 2.0f ); AssertEqual( d.AW().Y(), 3.0f ); AssertEqual( d.AW().Z(), 4.0f );
    }
    
    if ( 1 ) // HMat43 <- ( AxisZ, AxisY, HVec3 )
    {
        const C_MatrixScale     s( HVec3( 2.0f, 3.0f, 4.0f ) );
        const C_MatrixRotateRPH r( HVec3( 0.0f, 0.0f, 0.0f ) );
        const C_MatrixTranslate t( HVec3( 5.0f, 6.0f, 7.0f ) );
        const HMat43 a( C_MatrixSRT( s, r, t ) );
        AssertEqual( a.AX().X(), 2.0f ); AssertEqual( a.AX().Y(), 0.0f ); AssertEqual( a.AX().Z(), 0.0f );
        AssertEqual( a.AY().X(), 0.0f ); AssertEqual( a.AY().Y(), 3.0f ); AssertEqual( a.AY().Z(), 0.0f );
        AssertEqual( a.AZ().X(), 0.0f ); AssertEqual( a.AZ().Y(), 0.0f ); AssertEqual( a.AZ().Z(), 4.0f );
        AssertEqual( a.AW().X(), 5.0f ); AssertEqual( a.AW().Y(), 6.0f ); AssertEqual( a.AW().Z(), 7.0f );
    }
    
    if ( 1 ) // HMat43 get Rotation & Translation
    {
        const HMat43 a( HVec3( 1.0f,  2.0f,  3.0f ),
                        HVec3( 4.0f,  5.0f,  6.0f ),
                        HVec3( 7.0f,  8.0f,  9.0f ),
                        HVec3( 10.0f, 11.0f, 12.0f ) );
        const HMat33 b( a.Rotation() );
        const HVec3  c( a.Translation() );
        AssertEqual( b.AX().X(), 1.0f ); AssertEqual( b.AX().Y(), 2.0f ); AssertEqual( b.AX().Z(), 3.0f );
        AssertEqual( b.AY().X(), 4.0f ); AssertEqual( b.AY().Y(), 5.0f ); AssertEqual( b.AY().Z(), 6.0f );
        AssertEqual( b.AZ().X(), 7.0f ); AssertEqual( b.AZ().Y(), 8.0f ); AssertEqual( b.AZ().Z(), 9.0f );
        AssertEqual( c.X(), 10.0f ); AssertEqual( c.Y(), 11.0f ); AssertEqual( c.Z(), 12.0f );
    }
    
    if ( 1 ) // HMat43 set Rotation & Translation
    {
        HMat43 a( ZERO_MATRIX );
        const HMat33 b( HVec3( 1.0f,  2.0f,  3.0f ),
                        HVec3( 4.0f,  5.0f,  6.0f ),
                        HVec3( 7.0f,  8.0f,  9.0f ) );
        const HVec3 c( 10.0f, 11.0f, 12.0f );
        a.Rotation( b );
        a.Translation( c );
        AssertEqual( a.AX().X(), 1.0f );  AssertEqual( a.AX().Y(), 2.0f );  AssertEqual( a.AX().Z(), 3.0f );
        AssertEqual( a.AY().X(), 4.0f );  AssertEqual( a.AY().Y(), 5.0f );  AssertEqual( a.AY().Z(), 6.0f );
        AssertEqual( a.AZ().X(), 7.0f );  AssertEqual( a.AZ().Y(), 8.0f );  AssertEqual( a.AZ().Z(), 9.0f );
        AssertEqual( a.AW().X(), 10.0f ); AssertEqual( a.AW().Y(), 11.0f ); AssertEqual( a.AW().Z(), 12.0f );
    }
    
    if ( 1 ) // HMat43 get axes AX, AY, AZ, AW
    {
        const HMat43 a( HVec3( 1.0f,  2.0f,  3.0f ),
                        HVec3( 4.0f,  5.0f,  6.0f ),
                        HVec3( 7.0f,  8.0f,  9.0f ),
                        HVec3( 10.0f, 11.0f, 12.0f ) );
        const HVec3 b( a.AX() );
        const HVec3 c( a.AY() );
        const HVec3 d( a.AZ() );
        const HVec3 e( a.AW() );
        AssertEqual( b.X(), 1.0f );  AssertEqual( b.Y(), 2.0f );  AssertEqual( b.Z(), 3.0f );
        AssertEqual( c.X(), 4.0f );  AssertEqual( c.Y(), 5.0f );  AssertEqual( c.Z(), 6.0f );
        AssertEqual( d.X(), 7.0f );  AssertEqual( d.Y(), 8.0f );  AssertEqual( d.Z(), 9.0f );
        AssertEqual( e.X(), 10.0f ); AssertEqual( e.Y(), 11.0f ); AssertEqual( e.Z(), 12.0f );
    }
    
    
    
    
    
}

#endif

}; // namespace Maths



