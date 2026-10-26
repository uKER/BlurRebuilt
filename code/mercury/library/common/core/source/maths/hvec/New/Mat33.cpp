#include "CorePch.h"
#include <core/maths/hvec/New/IncludeGeometry.h>
#include <core/maths/hvec/New/IncludeGeometry.inl>

namespace Maths
{
/// @addtogroup Mat33
//! @{

const Mat33 Const< Mat33 >::m_Zero =
    MakeMat33( 0.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f );

const Mat33 Const< Mat33 >::m_Identity =
    MakeMat33( 1.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f,
               0.0f, 0.0f, 1.0f );

const Mat33 Const< Mat33 >::m_RotateX90CW =
    MakeMat33( 1.0f,  0.0f, 0.0f,
               0.0f,  0.0f, 1.0f,
               0.0f, -1.0f, 0.0f );

const Mat33 Const< Mat33 >::m_RotateY90CW =
    MakeMat33( 0.0f, 0.0f, -1.0f,
               0.0f, 1.0f,  0.0f,
               1.0f, 0.0f,  0.0f );

const Mat33 Const< Mat33 >::m_RotateZ90CW =
    MakeMat33(  0.0f, 1.0f, 0.0f,
               -1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f );

const Mat33 Const< Mat33 >::m_RotateX90ACW =
    MakeMat33( 1.0f, 0.0f,  0.0f,
               0.0f, 0.0f, -1.0f,
               0.0f, 1.0f,  0.0f );

const Mat33 Const< Mat33 >::m_RotateY90ACW =
    MakeMat33(  0.0f, 0.0f, 1.0f,
                0.0f, 1.0f, 0.0f,
               -1.0f, 0.0f, 0.0f );

const Mat33 Const< Mat33 >::m_RotateZ90ACW =
    MakeMat33( 0.0f, -1.0f, 0.0f,
               1.0f,  0.0f, 0.0f,
               0.0f,  0.0f, 1.0f );

const Mat33 Const< Mat33 >::m_RotateX180 =
    MakeMat33( 1.0f,  0.0f,  0.0f,
               0.0f, -1.0f,  0.0f,
               0.0f,  0.0f, -1.0f );

const Mat33 Const< Mat33 >::m_RotateY180 =
    MakeMat33( -1.0f, 0.0f,  0.0f,
                0.0f, 1.0f,  0.0f,
                0.0f, 0.0f, -1.0f );

const Mat33 Const< Mat33 >::m_RotateZ180 =
    MakeMat33( -1.0f,  0.0f, 0.0f,
                0.0f, -1.0f, 0.0f,
                0.0f,  0.0f, 1.0f );


// Cross products for deriving third matrix axis from 2 input axes
// ax = ay ^ az;
// ay = az ^ ax;
// az = ax ^ ay;

/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to z axis then x axis with y axis fully recomputed) 
NOALIAS const Mat33 OrthoNormaliseZX( const Mat33 MATREF src )
{
    Mat33 result;
    Vec4F32 row_x, row_y, row_z;
    GetRows( src, &row_x, &row_y, &row_z );
    row_y = row_z ^ row_x;
    row_x = row_y ^ row_z;
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    const VF32 rcp_length_x( X( rcp_length_xyz ) );
    const VF32 rcp_length_y( Y( rcp_length_xyz ) );
    const VF32 rcp_length_z( Z( rcp_length_xyz ) );
    SetRows( &result, row_x * rcp_length_x,
                      row_y * rcp_length_y,
                      row_z * rcp_length_z );
    return result;
}

/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to x axis then y axis with z axis fully recomputed) 
/// (faster but only 22 bit precision)
NOALIAS const Mat33 OrthoNormaliseXY( const Mat33 MATREF src )
{
    Mat33 result;
    Vec4F32 row_x, row_y, row_z;
    GetRows( src, &row_x, &row_y, &row_z );
    row_z = row_x ^ row_y;
    row_y = row_z ^ row_x;
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    const VF32 rcp_length_x( X( rcp_length_xyz ) );
    const VF32 rcp_length_y( Y( rcp_length_xyz ) );
    const VF32 rcp_length_z( Z( rcp_length_xyz ) );
    SetRows( &result, row_x * rcp_length_x,
                      row_y * rcp_length_y,
                      row_z * rcp_length_z );
    return result;
}

/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to x axis then y axis with z axis fully recomputed) 
/// (slower but full 24 bit precision)
NOALIAS const Mat33 OrthoNormaliseXY_24( const Mat33 MATREF src )
{
    Mat33 result;
    Vec4F32 row_x, row_y, row_z;
    GetRows( src, &row_x, &row_y, &row_z );
    row_z = row_x ^ row_y;
    row_y = row_z ^ row_x;
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt24( length_squared_xyz ) );
    const VF32 rcp_length_x( X( rcp_length_xyz ) );
    const VF32 rcp_length_y( Y( rcp_length_xyz ) );
    const VF32 rcp_length_z( Z( rcp_length_xyz ) );
    SetRows( &result, row_x * rcp_length_x,
                      row_y * rcp_length_y,
                      row_z * rcp_length_z );
    return result;
}

/// Ortho-Normalise the axes of a Mat33 (priority given to z axis then x axis with y axis fully recomputed) 
NOALIAS void OrthoNormaliseZX( Mat33* pDst )
{
    Vec4F32 row_x, row_y, row_z;
    GetRows( *pDst, &row_x, &row_y, &row_z );
    row_y = row_z ^ row_x;
    row_x = row_y ^ row_z;
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    const VF32 rcp_length_x( X( rcp_length_xyz ) );
    const VF32 rcp_length_y( Y( rcp_length_xyz ) );
    const VF32 rcp_length_z( Z( rcp_length_xyz ) );
    SetRows( pDst, row_x * rcp_length_x,
                   row_y * rcp_length_y,
                   row_z * rcp_length_z );
}

/// Ortho-Normalise the axes of a Mat33 (priority given to x axis then y axis with z axis fully recomputed) 
NOALIAS void OrthoNormaliseXY( Mat33* pDst )
{
    Vec4F32 row_x, row_y, row_z;
    GetRows( *pDst, &row_x, &row_y, &row_z );
    row_z = row_x ^ row_y;
    row_y = row_z ^ row_x;
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    const VF32 rcp_length_x( X( rcp_length_xyz ) );
    const VF32 rcp_length_y( Y( rcp_length_xyz ) );
    const VF32 rcp_length_z( Z( rcp_length_xyz ) );
    SetRows( pDst, row_x * rcp_length_x,
                   row_y * rcp_length_y,
                   row_z * rcp_length_z );
}


/// Mat33 Extract Scale And Shear
NOALIAS const bool ExtractScaleAndShear( Mat33* const __restrict pMatrix, Vector3* const __restrict pScale, Vector3* const __restrict pShear )
{
    //This implementation follows the technique described in the paper by
    //Spencer W. Thomas in the Graphics Gems II article: "Decomposing a 
    //Matrix into Simple Transformations", p. 320.
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 epsilon( Const<VF32>::Epsilon() );
    
    //It's a shame there isn't a way of being able to efficiently loop thru the elements
    //of an HMat matrix. Hence the bloated code below. Oh well, that's another couple of loops
    //the compiler doesn't have to bother unrolling.
    const VF32 maxAbsValue( MaxAbsValue( *pMatrix ) );
    const VF32 rcpMaxAbsValue( Rcp24( maxAbsValue ) );
    
    //We normalize the 3x3 matrix here.
    //It was noticed that this can improve numerical stability significantly,
    //especially when many of the upper 3x3 matrix's coefficients are very
    //close to zero; we correct for this step at the end by multiplying the 
    //scaling factors by maxVal at the end (shear and rotation are not 
    //affected by the normalization).
    Vector3 rowX( RowX( *pMatrix ) * rcpMaxAbsValue );
    Vector3 rowY( RowY( *pMatrix ) * rcpMaxAbsValue );
    Vector3 rowZ( RowZ( *pMatrix ) * rcpMaxAbsValue );
    
    // Compute X scale factor.
    const VF32 scaleX( Length24( rowX ) );
    
    // Normalize first row.
    Normalise24( &rowX );
    
    // Compute XY shear factor and make 2nd row orthogonal to 1st.
    VF32 shearX( rowX | rowY );
    rowY -= rowX * shearX;
    
    // Now, compute Y scale.
    const VF32 scaleY( Length24( rowY ) );
    const VF32 rcp_scaleY( Rcp24( scaleY ) );
    
    // Normalize 2nd row and correct the XY shear factor for Y scaling.
    Normalise24( &rowY );
    //shearX /= scaleY;
    shearX *= rcp_scaleY;
    
    // Compute XZ and YZ shears, orthogonalize 3rd row.
    VF32 shearY( rowX | rowZ );
    rowZ -= rowX * shearY;
    
    VF32 shearZ( rowY | rowZ );
    rowZ -= rowY * shearZ;
    
    // Next, get Z scale.
    const VF32 scaleZ( Length24( rowZ ) );
    const VF32 rcp_scaleZ( Rcp24( scaleZ ) );
    
    // Normalize 3rd row and correct the XZ and YZ shear factors for Z scaling.
    Normalise24( &rowZ );
    shearY *= rcp_scaleZ;
    shearZ *= rcp_scaleZ;
    
    //At this point, the upper 3x3 matrix in mat is orthonormal.
    //Check for a coordinate system flip. If the determinant
    //is less than zero, then negate the matrix and the scaling factors.
    Mat33   matrix( MakeMat33( rowX, rowY, rowZ ) );
    Vector3 scale( MakeVector3( scaleX, scaleY, scaleZ ) );
    Vector3 shear( MakeVector3( shearX, shearY, shearZ ) );
    
    const VBool detPos( Determinant( matrix ) >= zero );
    matrix = Select( detPos, matrix, -matrix );
    scale  = Select( detPos, scale, -scale );
    
    // The upper 3x3 matrix in mat is now a rotation matrix.
    //Correct the scaling factors for the normalization step that we 
    //performed above; shear and rotation are not affected by the 
    //normalization.
    scale *= maxAbsValue;
    
    *pMatrix = matrix;
    *pScale = scale;
    *pShear = shear;
    
    return ( ( maxAbsValue >= epsilon ) &
             ( scaleX >= epsilon ) ) &
           ( ( scaleY >= epsilon ) &
             ( scaleZ >= epsilon ) );
}

/// Mat33 Extract Scale Shear And Rotate
NOALIAS const VBool DecomposeScaleShearRotate( const Mat33 MATREF          matrix,
                                               ScaleMat3* const __restrict pScale,
                                               ShearMat3* const __restrict pShear,
                                               Mat33*     const __restrict pRotate )
{
    const VF32 epsilon( Const<VF32>::Epsilon() );
    const VF32 maxAbsValue( MaxAbsValue( matrix ) );
    const VF32 rcpMaxAbsValue( Rcp24( maxAbsValue ) );
    
    // We normalize the 3x3 matrix here.
    // It was noticed that this can improve numerical stability significantly,
    // especially when many of the upper 3x3 matrix's coefficients are very
    // close to zero; we correct for this step at the end by multiplying the 
    // scaling factors by maxVal at the end (shear and rotation are not 
    // affected by the normalization).
    const Mat33 scaleShearRotateMatrix( matrix * rcpMaxAbsValue );
    
    // Compute the rotation part of the matrix
    const Mat33 rotateMatrix( OrthoNormaliseXY_24( scaleShearRotateMatrix ) );
    
    // Order of composition:
    // scale * shear * rotate
    
    // Remove the rotation component from the matrix, leaving the scale and the shear
    // scale * shear * rotate * inverse(rotate) = scale * shear
    const Mat33 inverseRotateMatrix( Transpose( rotateMatrix ) );
    const Mat33 scaleShearMatrix( scaleShearRotateMatrix * inverseRotateMatrix );
    
    // Since the x-axis was the primary axis in the ortho-normalise (i.e. unchanged except for normalisation),
    // after removing the rotation component the x-axis of the resulting scale-shear matrix is simply a scale vector
    // along the x-axis (SX, 0, 0). The y-axis was the secondary axis in the ortho-normalise, i.e. it was adjusted
    // only in the x-y plane to make it orthogonal to the x-axis, so it has a scale component plus a skew component
    // relative to the x-axis but has no dependency on the z-axis (SY.XY, SY, 0). The z-axis was adjusted in the
    // x-z plane and the y-z plane hence it has a scale component plus skew components relative to both the
    // x and the y axis (SZ.XZ, SZ.YZ, SZ).
    
    //  scale matrix     shear matrix     scale-shear matrix
    // [ SX  0   0  ]   [  1   0   0 ]   [ SX     0      0  ]
    // [ 0   SY  0  ] * [ XY   1   0 ] = [ SY.XY  SY     0  ]
    // [ 0   0   SZ ]   [ XZ  YZ   1 ]   [ SZ.XZ  SZ.YZ  SZ ]
    
    // Compute the scale part of the matrix
    const ScaleMat3 scaleMatrix( MakeScaleMat3( X( RowX( scaleShearMatrix ) ),
                                                Y( RowY( scaleShearMatrix ) ),
                                                Z( RowZ( scaleShearMatrix ) ) ) );
    
    const ScaleMat3 inverseScaleMatrix( Inverse24( scaleMatrix ) );
    
    // Remove the scaling component from the matrix, leaving the shear
    // inverse(scale) * scale * shear = shear
    const Mat33 shearMatrix( inverseScaleMatrix * scaleShearMatrix );
    const VF32  shearXY( Element<1,0>( shearMatrix ) );
    const VF32  shearXZ( Element<2,0>( shearMatrix ) );
    const VF32  shearYZ( Element<2,1>( shearMatrix ) );
    
    *pScale  = scaleMatrix * maxAbsValue;
    *pShear  = MakeShearMat3( shearXY, shearXZ, shearYZ );
    *pRotate = rotateMatrix;
    
    return ( maxAbsValue >= epsilon ) & Finite( inverseScaleMatrix );
}


/// Mat33 RQ Decomposition
NOALIAS const VBool RQ_Decomposition( const Mat33 MATREF A, Mat33* const __restrict pR, Mat33* const __restrict pQ )
{
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    const VF32 nearlyZero( Const<VF32>::Epsilon() );
    
    const Vector3 a0( RowX( A ) );
    const Vector3 a1( RowY( A ) );
    const Vector3 a2( RowZ( A ) );
    
    const VF32 a00( X( a0 ) ); const VF32 a01( Y( a0 ) ); const VF32 a02( Z( a0 ) );
    const VF32 a10( X( a1 ) ); const VF32 a11( Y( a1 ) ); const VF32 a12( Z( a1 ) );
    const VF32 a20( X( a2 ) ); const VF32 a21( Y( a2 ) ); const VF32 a22( Z( a2 ) );
    
    // Solve for r2's & q2's
    
    //const VF32 r22( Sqrt24( a20 * a20 + a21 * a21 + a22 * a22 ) );
    const VF32 r22_squared( a20 * a20 + a21 * a21 + a22 * a22 );
    const VF32 r21( zero );
    const VF32 r20( zero );
    
    const VBool r22_nonZero( r22_squared > nearlyZero );
    
    //const VF32 rcp_r22( Rcp24( r22 ) );
    const VF32 rcp_r22( RcpSqrt24( r22_squared ) );
    const VF32 r22( r22_squared * rcp_r22 );
    
    //const VF32 q20( Select( Abs( r22 ) > nearlyZero, a20 * rcp_r22, zero ) );
    //const VF32 q21( Select( Abs( r22 ) > nearlyZero, a21 * rcp_r22, zero ) );
    //const VF32 q22( Select( Abs( r22 ) > nearlyZero, a22 * rcp_r22, one  ) );
    const VF32 q20( Select( r22_nonZero, a20 * rcp_r22, zero ) );
    const VF32 q21( Select( r22_nonZero, a21 * rcp_r22, zero ) );
    const VF32 q22( Select( r22_nonZero, a22 * rcp_r22, one  ) );
    
    // Solve for r1's & q1's
    
    const VF32 default0_q10( -q21 * q20 );
    const VF32 default0_q11( q22 * q22 + q20 * q20 );
    const VF32 default0_q12( -q21 * q22 );
    //const VF32 default0_q1_length( Sqrt24( Square( default0_q10 ) + Square( default0_q11 ) + Square( default0_q12 ) ) );
    const VF32 default0_q1_length_squared( Square( default0_q10 ) + Square( default0_q11 ) + Square( default0_q12 ) );
    
    const VF32 default1_q10( q21 * q21 + q22 * q22 );
    const VF32 default1_q11( -q20 * q21 );
    const VF32 default1_q12( -q22 * q21 );
    //const VF32 default1_q1_length( Sqrt24( Square( default1_q10 ) + Square( default1_q11 ) + Square( default1_q12 ) ) );
    const VF32 default1_q1_length_squared( Square( default1_q10 ) + Square( default1_q11 ) + Square( default1_q12 ) );
    
    //const VF32 default_q10( Select( default0_q1_length > default1_q1_length, default0_q10, default1_q10 ) );
    //const VF32 default_q11( Select( default0_q1_length > default1_q1_length, default0_q11, default1_q11 ) );
    //const VF32 default_q12( Select( default0_q1_length > default1_q1_length, default0_q12, default1_q12 ) );
    
    const VBool select_default0_q1( default0_q1_length_squared >= default1_q1_length_squared );
    
    const VF32 default_q10( Select( select_default0_q1, default0_q10, default1_q10 ) );
    const VF32 default_q11( Select( select_default0_q1, default0_q11, default1_q11 ) );
    const VF32 default_q12( Select( select_default0_q1, default0_q12, default1_q12 ) );
    
    //const VF32 default_q1_rcp_length( Rcp24( Select( default0_q1_length > default1_q1_length, default0_q1_length, default1_q1_length ) ) );
    const VF32 default_q1_rcp_length( RcpSqrt24( Select( select_default0_q1, default0_q1_length_squared, default1_q1_length_squared ) ) );
    
    const VF32 r12( q20 * a10 + q21 * a11 + q22 * a12 );
    const VF32 q10_r11( a10 - r12 * q20 );
    const VF32 q11_r11( a11 - r12 * q21 );
    const VF32 q12_r11( a12 - r12 * q22 );
    //const VF32 r11( Sqrt24( Square( q10_r11 ) + Square( q11_r11 ) + Square( q12_r11 ) ) );
    const VF32 r11_squared( Square( q10_r11 ) + Square( q11_r11 ) + Square( q12_r11 ) );
    const VF32 r10( zero );
    
    //const VF32 rcp_r11( Rcp24( r11 ) );
    const VF32 rcp_r11( RcpSqrt24( r11_squared ) );
    const VF32 r11( r11_squared * rcp_r11 );
    
    //const VF32 q10( Select( Abs( r11 ) > nearlyZero, q10_r11 * rcp_r11, default_q10 * default_q1_rcp_length ) );
    //const VF32 q11( Select( Abs( r11 ) > nearlyZero, q11_r11 * rcp_r11, default_q11 * default_q1_rcp_length ) );
    //const VF32 q12( Select( Abs( r11 ) > nearlyZero, q12_r11 * rcp_r11, default_q12 * default_q1_rcp_length ) );
    
    const VBool r11_nonZero( r11_squared > nearlyZero );
    
    const VF32 q10( Select( r11_nonZero, q10_r11 * rcp_r11, default_q10 * default_q1_rcp_length ) );
    const VF32 q11( Select( r11_nonZero, q11_r11 * rcp_r11, default_q11 * default_q1_rcp_length ) );
    const VF32 q12( Select( r11_nonZero, q12_r11 * rcp_r11, default_q12 * default_q1_rcp_length ) );
    
    // Solve for r0's & q0's
    
    const VF32 r02( q20 * a00 + q21 * a01 + q22 * a02 );
    const VF32 r01( q10 * a00 + q11 * a01 + q12 * a02 );
    const VF32 q00_r00( a00 - r01 * q10 - r02 * q20 );
    const VF32 q01_r00( a01 - r01 * q11 - r02 * q21 );
    const VF32 q02_r00( a02 - r01 * q12 - r02 * q22 );
    
    //const VF32 r00( Sqrt24( Square( q00_r00 ) + Square( q01_r00 ) + Square( q02_r00 ) ) );
    const VF32 r00_squared( Square( q00_r00 ) + Square( q01_r00 ) + Square( q02_r00 ) );
    
    const VF32 default_q00( q11 * q22 - q12 * q21 );
    const VF32 default_q01( q12 * q20 - q10 * q22 );
    const VF32 default_q02( q10 * q21 - q11 * q20 );
    
    const VF32 default_q0_rcp_length( RcpSqrt24( Square( default_q00 ) + Square( default_q01 ) + Square( default_q02 ) ) );
    
    //const VF32 rcp_r00( Rcp24( r00 ) );
    const VF32 rcp_r00( RcpSqrt24( r00_squared ) );
    const VF32 r00( r00_squared * rcp_r00 );
    
    const VBool r00_nonZero( r00_squared > nearlyZero );
    
    //const VF32 q00( Select( Abs( r00 ) > nearlyZero, q00_r00 * rcp_r00, default_q00 * default_q0_rcp_length ) );
    //const VF32 q01( Select( Abs( r00 ) > nearlyZero, q01_r00 * rcp_r00, default_q01 * default_q0_rcp_length ) );
    //const VF32 q02( Select( Abs( r00 ) > nearlyZero, q02_r00 * rcp_r00, default_q02 * default_q0_rcp_length ) );
    const VF32 q00( Select( r00_nonZero, q00_r00 * rcp_r00, default_q00 * default_q0_rcp_length ) );
    const VF32 q01( Select( r00_nonZero, q01_r00 * rcp_r00, default_q01 * default_q0_rcp_length ) );
    const VF32 q02( Select( r00_nonZero, q02_r00 * rcp_r00, default_q02 * default_q0_rcp_length ) );
    
    *pR = MakeMat33( MakeVector3( r00, r01, r02 ),
                     MakeVector3( r10, r11, r12 ),
                     MakeVector3( r20, r21, r22 ) );
    
    *pQ = MakeMat33( MakeVector3( q00, q01, q02 ),
                     MakeVector3( q10, q11, q12 ),
                     MakeVector3( q20, q21, q22 ) );
    
    return Const<VBool>::True();
}

/// Solve R * x = y, where R is an Upper Triangular matrix
NOALIAS const VBool Solve_Rx_Eq_y( const Mat33 MATREF R, const Vector3 PCREF y, Vector3* const __restrict pX )
{
    const VF32 one( Const<VF32>::One() );
    const VF32 nearlyZero( Const<VF32>::Frac<1,10000>() );
    
    const VF32 y0( X( y ) );
    const VF32 y1( Y( y ) );
    const VF32 y2( Z( y ) );
    
    const Vector3 r0( RowX( R ) );
    const Vector3 r1( RowY( R ) );
    const Vector3 r2( RowZ( R ) );
    
    const VF32 r00( X( r0 ) );
    const VF32 r01( Y( r0 ) );
    const VF32 r02( Z( r0 ) );
    const VF32 r11( Y( r1 ) );
    const VF32 r12( Z( r1 ) );
    const VF32 r22( Z( r2 ) );
    
    const VBool r00_valid( Abs( r00 ) > nearlyZero );
    const VBool r11_valid( Abs( r11 ) > nearlyZero );
    const VBool r22_valid( Abs( r22 ) > nearlyZero );
    
    const VF32 x2( Select( r22_valid, Div24( y2, r22 ), one ) );
    const VF32 x1( Select( r11_valid, Div24( ( y1 - r12 * x2 ), r11 ), one ) );
    const VF32 x0( Select( r00_valid, Div24( ( y0 - r01 * x1 - r02 * x2 ), r00 ), one ) );
    
    *pX = MakeVector3( x0, x1, x2 );
    
    //return r00_valid &
    //       r11_valid &
    //       r22_valid;
    // Return true if zero or one free variables
    return ( r00_valid & r11_valid ) |
           ( r00_valid & r22_valid ) |
           ( r11_valid & r22_valid );
}

/// Symmetric Mat33 Eigen Decomposition: A=input matrix, P=eigen vector matrix, D=eigen value diagonal matrix, A = P * D * inverse(P)
NOALIAS const VBool SymmetricEigenDecomposition(
    const Mat33 MATREF A,
    Mat33    * const __restrict pEigenVectorMatrix,
    ScaleMat3* const __restrict pEigenValueMatrix )
{
    // http://mathworld.wolfram.com/Eigenvalue.html
    
    // Det( A - lambda * I ) = 0
    
    //    ( [ a00-lambda, a01,        a02        ] )
    // Det( [ a10,        a11-lambda, a12        ] ) = 0
    //    ( [ a20,        a21,        a22-lambda ] )
    
    // + (a00-lambda) * ( (a11-lambda) * (a22-lambda) - a12          * a21 )
    // - a01          * ( a10          * (a22-lambda) - a12          * a20 )
    // + a02          * ( a10          * a21          - (a11-lambda) * a20 ) = 0
    
    //   (a00-lambda).(a11-lambda).(a22-lambda)
    // - (a00-lambda).a12.a21
    // - a01.a10.(a22-lambda)
    // + a01.a12.a20
    // + a02.a10.a21
    // - a02.(a11-lambda).a20 = 0
    
    //   (a00-lambda).( a11.a22 - a11.lambda - a22.lambda + lambda^2 )
    // - a00.a12.a21 + a12.a21.lambda
    // - a01.a10.a22 + a01.a10.lambda
    // + a01.a12.a20
    // + a02.a10.a21
    // - a02.a20.a11 + a02.a20.lambda = 0
    
    //   a00.a11.a22 - a00.a11.lambda - a00.a22.lambda + a00.lambda^2 - a11.a22.lambda + a11.lambda^2 + a22.lambda^2 - lambda^3
    // + (a12.a21 + a01.a10 + a02.a20).lambda
    // + a01.a12.a20 + a02.a10.a21 - a02.a20.a11 - a01.a10.a22 - a00.a12.a21 = 0
    
    // - lambda^3
    // + (a00 + a11 + a22).lambda^2
    // + (a01.a10 + a02.a20 + a12.a21 - a00.a11 - a00.a22 - a11.a22).lambda
    // + a00.a11.a22 + a01.a12.a20 + a02.a10.a21 - a00.a12.a21 - a01.a10.a22 - a02.a11.a20 = 0
    
    VBool valid( Const<VBool>::True() );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    const VF32 two( Const<VF32>::Two() );
    //const VF32 nearlyZero( Const<VF32>::Epsilon() );
    const VF32 nearlyZero( Const<VF32>::Frac<1,100000>() );
    const VF32 realLimit( Const<VF32>::Frac<1,1000>() );
    const VF32 equalLambdaTolerance( Const<VF32>::Frac<1,1000>() );
    
    const Vector3 a0( RowX( A ) );
    const Vector3 a1( RowY( A ) );
    const Vector3 a2( RowZ( A ) );
    
    const VF32 a00( X( a0 ) );
    const VF32 a01( Y( a0 ) );
    const VF32 a02( Z( a0 ) );
    const VF32 a10( X( a1 ) );
    const VF32 a11( Y( a1 ) );
    const VF32 a12( Z( a1 ) );
    const VF32 a20( X( a2 ) );
    const VF32 a21( Y( a2 ) );
    const VF32 a22( Z( a2 ) );
    
    const VF32 z3( -one );
    const VF32 z2( a00 + a11 + a22 );
    const VF32 z1( a01*a10 + a02*a20 + a12*a21 - a00*a11 - a00*a22 - a11*a22 );
    const VF32 z0( a00*a11*a22 + a01*a12*a20 + a02*a10*a21 - a00*a12*a21 - a01*a10*a22 - a02*a11*a20 );
    
    Complex root1, root2, root3;
    SolveCubic( z3, z2, z1, z0, &root1, &root2, &root3 );
    
    const VF32 real_root1( Real( root1 ) );
    const VF32 real_root2( Real( root2 ) );
    const VF32 real_root3( Real( root3 ) );
    const VF32 imag_root1( Imag( root1 ) );
    const VF32 imag_root2( Imag( root2 ) );
    const VF32 imag_root3( Imag( root3 ) );
    
    VBool valid_lambda1( Finite( real_root1 ) & Finite( imag_root1 ) & ( Abs( imag_root1 ) <= realLimit ) );
    VBool valid_lambda2( Finite( real_root2 ) & Finite( imag_root2 ) & ( Abs( imag_root2 ) <= realLimit ) );
    VBool valid_lambda3( Finite( real_root3 ) & Finite( imag_root3 ) & ( Abs( imag_root3 ) <= realLimit ) );
    
    VF32 lambda1( Select( valid_lambda1, real_root1, zero ) );
    VF32 lambda2( Select( valid_lambda2, real_root2, zero ) );
    VF32 lambda3( Select( valid_lambda3, real_root3, zero ) );
    
    // If there are any invalid lambda's, swap the order such that the valid ones are first
    if ( !valid_lambda1 )
    {
        if ( valid_lambda3 )
        {
            lambda1 = lambda3;
            valid_lambda1 = valid_lambda3;
            valid_lambda3 = Const<VBool>::False();
        }
        else if ( valid_lambda2 )
        {
            lambda1 = lambda2;
            valid_lambda1 = valid_lambda2;
            valid_lambda2 = Const<VBool>::False();
        }
    }
    else if ( !valid_lambda2 & valid_lambda3 )
    {
        lambda2 = lambda3;
        valid_lambda2 = valid_lambda3;
        valid_lambda3 = Const<VBool>::False();
    }
    
    // Eliminate identical lambda's
    if ( valid_lambda2 & ( Abs( lambda1 - lambda2 ) < equalLambdaTolerance ) )
    {
        // lambda2 == lambda1 so eliminate lambda2
        lambda2 = lambda3;
        valid_lambda2 = valid_lambda3;
        valid_lambda3 = Const<VBool>::False();
        
        if ( valid_lambda2 & ( Abs( lambda1 - lambda2 ) < equalLambdaTolerance ) )
        {
            // lambda2 == lambda1 so eliminate lambda2
            valid_lambda2 = Const<VBool>::False();
        }
    }
    else if ( valid_lambda3 & ( ( Abs( lambda1 - lambda3 ) < equalLambdaTolerance ) |
                                ( Abs( lambda2 - lambda3 ) < equalLambdaTolerance ) ) )
    {
        // lambda3 == lambda1 || lambda3 == lambda2 so eliminate lambda3
        valid_lambda3 = Const<VBool>::False();
    }
    
    const S32 roots = (S32)(bool)valid_lambda1 +
                      (S32)(bool)valid_lambda2 +
                      (S32)(bool)valid_lambda3;
    
    const Mat33 I( Const<Mat33>::Identity() );
    
    Mat33 R, Q;
    Vector3 Qx;
    const Vector3 vzero( Const<Vector3>::Zero() );
    
    if ( roots == 3 )
    {
        const Mat33   A_minus_I_lambda1( A - I * lambda1 );
        const VBool   RQ1_valid( RQ_Decomposition( A_minus_I_lambda1, &R, &Q ) & valid_lambda1 );
        const VBool   x1_valid( Solve_Rx_Eq_y( R, vzero, &Qx ) & RQ1_valid );
        const Vector3 x1( Select( x1_valid, Qx * Q, Normalise( ColX( A ) ) ) );
        
        const Mat33   A_minus_I_lambda2( A - I * lambda2 );
        const VBool   RQ2_valid( RQ_Decomposition( A_minus_I_lambda2, &R, &Q ) & valid_lambda2 );
        const VBool   x2_valid( Solve_Rx_Eq_y( R, vzero, &Qx ) & RQ2_valid );
        const Vector3 x2( Select( x2_valid, Qx * Q, Normalise( ColY( A ) ) ) ); // TODO: default orthogonal to x1!
        
        const Mat33   A_minus_I_lambda3( A - I * lambda3 );
        const VBool   RQ3_valid( RQ_Decomposition( A_minus_I_lambda3, &R, &Q ) & valid_lambda3 );
        const VBool   x3_valid( Solve_Rx_Eq_y( R, vzero, &Qx ) & RQ3_valid );
        const Vector3 x3( Select( x3_valid, Qx * Q, Normalise( ColZ( A ) ) ) ); // TODO: default orthogonal to x1 and x2!
        
        SetColumns( pEigenVectorMatrix, x1, x2, x3 );
        *pEigenValueMatrix = MakeScaleMat3( lambda1, lambda2, lambda3 );
        
        valid = x1_valid & x2_valid & x3_valid;
    }
    else if ( roots == 2 )
    {
        assert( 0 && "EigenDecomposition: Solution for 2 roots not finished yet!\n" );
        
        const Mat33   A_minus_I_lambda1( A - I * lambda1 );
        const VBool   RQ1_valid( RQ_Decomposition( A_minus_I_lambda1, &R, &Q ) & valid_lambda1 );
        const VBool   x1_valid( Solve_Rx_Eq_y( R, vzero, &Qx ) & RQ1_valid );
        const Vector3 x1( Select( x1_valid, Qx * Q, Normalise( ColX( A ) ) ) );
        
        const Mat33   A_minus_I_lambda2( A - I * lambda2 );
        const VBool   RQ2_valid( RQ_Decomposition( A_minus_I_lambda2, &R, &Q ) & valid_lambda2 );
        const VBool   x2_valid( Solve_Rx_Eq_y( R, vzero, &Qx ) & RQ2_valid );
        const Vector3 x2( Select( x2_valid, Qx * Q, Normalise( ColY( A ) ) ) ); // TODO: default orthogonal to x1!
        
        SetColumns( pEigenVectorMatrix, x1, x2, MakeVector3( Normalise( x1 ^ x2 ) ) );
        *pEigenValueMatrix = MakeScaleMat3( lambda1, lambda2, lambda2 ); // ( lambda1, lambda2, lambda2 ) or ( lambda1, lambda2, lambda1 ) ???
        
        //valid = x1_valid & x2_valid;
        valid = Const<VBool>::False();
    }
    else if ( roots == 1 )
    {
        const Mat33   A_minus_I_lambda1( A - I * lambda1 );
        const VBool   RQ1_valid( RQ_Decomposition( A_minus_I_lambda1, &R, &Q ) & valid_lambda1 );
        const VBool   x1_valid( Solve_Rx_Eq_y( R, vzero, &Qx ) & RQ1_valid );
        const Vector3 x1( Select( x1_valid, Qx * Q, Const<Vector3>::UnitX() ) ); // Note: Inverse(Q) * Qx = Transpose(Q) * Qx = Qx * Q
        
        // [ a00 - lambda, a01,          a02          ]   [ xn.x ]   [ 0 ]
        // [ a10,          a11 - lambda, a12          ] * [ xn.y ] = [ 0 ]
        // [ a20,          a21,          a22 - lambda ]   [ xn.z ]   [ 0 ]
        // 
        //                      [ b00, b01, b02 ] = [ a00 - lambda, a01,          a02          ]
        // B = A - lambda * I = [ b10, b11, b12 ] = [ a10,          a11 - lambda, a12          ]
        //                      [ b20, b21, b22 ] = [ a20,          a21,          a22 - lambda ]
        // 
        // (A - lambda * I) * x1 = 0, [ b00, b01, b02 ] . x1 = 0, [ b10, b11, b12 ] . x1 = 0, [ b20, b21, b22 ] . x1 = 0
        // (A - lambda * I) * x2 = 0, [ b00, b01, b02 ] . x2 = 0, [ b10, b11, b12 ] . x2 = 0, [ b20, b21, b22 ] . x2 = 0
        // (A - lambda * I) * x3 = 0, [ b00, b01, b02 ] . x3 = 0, [ b10, b11, b12 ] . x3 = 0, [ b20, b21, b22 ] . x3 = 0
        // 
        // x2.x2 = 1, (x2.x * x2.x) + (x2.y * x2.y) + (x2.z * x2.z) = 1
        // x3.x3 = 1, (x3.x * x3.x) + (x3.y * x3.y) + (x3.z * x3.z) = 1
        // 
        // (x2.x * x2.x) = 1 - (x2.y * x2.y) - (x2.z * x2.z),  x2.x = Sqrt( 1 - (x2.y * x2.y) - (x2.z * x2.z) )
        // (x2.y * x2.y) = 1 - (x2.x * x2.x) - (x2.z * x2.z),  x2.y = Sqrt( 1 - (x2.x * x2.x) - (x2.z * x2.z) )
        // (x2.z * x2.z) = 1 - (x2.x * x2.x) - (x2.y * x2.y),  x2.z = Sqrt( 1 - (x2.x * x2.x) - (x2.y * x2.y) )
        // 
        // 
        // x1.x2 = 0, (x1.x * x2.x) + (x1.y * x2.y) + (x1.z * x2.z) = 0
        // x1.x3 = 0, (x1.x * x3.x) + (x1.y * x3.y) + (x1.z * x3.z) = 0
        // x2.x3 = 0, (x2.x * x3.x) + (x2.y * x3.y) + (x2.z * x3.z) = 0
        // 
        // (x1.x * x2.x) = -( (x1.y * x2.y) + (x1.z * x2.z) ),  x2.x = -( (x1.y * x2.y) + (x1.z * x2.z) ) / x1.x
        // (x1.y * x2.y) = -( (x1.x * x2.x) + (x1.z * x2.z) ),  x2.y = -( (x1.x * x2.x) + (x1.z * x2.z) ) / x1.y
        // (x1.z * x2.z) = -( (x1.x * x2.x) + (x1.y * x2.y) ),  x2.z = -( (x1.x * x2.x) + (x1.y * x2.y) ) / x1.z
        // 
        // 
        // [ b00, b01, b02 ] . x2 = 0, (x2.x * b00) + (x2.y * b01) + (x2.z * b02) = 0
        // [ b10, b11, b12 ] . x2 = 0, (x2.x * b10) + (x2.y * b11) + (x2.z * b12) = 0
        // [ b20, b21, b22 ] . x2 = 0, (x2.x * b20) + (x2.y * b21) + (x2.z * b22) = 0
        // 
        // (x2.x * b00) = -( (x2.y * b01) + (x2.z * b02) ),  x2.x = -( (x2.y * b01) + (x2.z * b02) ) / b00
        // (x2.x * b10) = -( (x2.y * b11) + (x2.z * b12) ),  x2.x = -( (x2.y * b11) + (x2.z * b12) ) / b10
        // (x2.x * b20) = -( (x2.y * b21) + (x2.z * b22) ),  x2.x = -( (x2.y * b21) + (x2.z * b22) ) / b20
        // 
        // (x2.y * b01) = -( (x2.x * b00) + (x2.z * b02) ),  x2.y = -( (x2.x * b00) + (x2.z * b02) ) / b01
        // (x2.y * b11) = -( (x2.x * b10) + (x2.z * b12) ),  x2.y = -( (x2.x * b10) + (x2.z * b12) ) / b11
        // (x2.y * b21) = -( (x2.x * b20) + (x2.z * b22) ),  x2.y = -( (x2.x * b20) + (x2.z * b22) ) / b21
        // 
        // (x2.z * b02) = -( (x2.x * b00) + (x2.y * b01) ),  x2.z = -( (x2.x * b00) + (x2.y * b01) ) / b02
        // (x2.z * b12) = -( (x2.x * b10) + (x2.y * b11) ),  x2.z = -( (x2.x * b10) + (x2.y * b11) ) / b12
        // (x2.z * b22) = -( (x2.x * b20) + (x2.y * b21) ),  x2.z = -( (x2.x * b20) + (x2.y * b21) ) / b22
        // 
        // 
        
        const VF32 b00( X( RowX( A_minus_I_lambda1 ) ) );
        const VF32 b01( Y( RowX( A_minus_I_lambda1 ) ) );
        const VF32 b02( Z( RowX( A_minus_I_lambda1 ) ) );
        const VF32 b10( X( RowY( A_minus_I_lambda1 ) ) );
        const VF32 b11( Y( RowY( A_minus_I_lambda1 ) ) );
        const VF32 b12( Z( RowY( A_minus_I_lambda1 ) ) );
        const VF32 b20( X( RowZ( A_minus_I_lambda1 ) ) );
        const VF32 b21( Y( RowZ( A_minus_I_lambda1 ) ) );
        const VF32 b22( Z( RowZ( A_minus_I_lambda1 ) ) );
        
        //if ( 1 ||
        //     ( Abs(X(x1)) >= Abs(Y(x1)) ) &
        //     ( Abs(X(x1)) >= Abs(Z(x1)) ) )
        //{
            // Eliminate x2.x: ( x1.x >= x1.y ) && ( x1.x >= x1.z )
            // x2.x = Sqrt( 1 - (x2.y * x2.y) - (x2.z * x2.z) )         Length(x2)=1
            //      = -( (x1.y * x2.y) + (x1.z * x2.z) ) / x1.x         x1.x2=0
            //      = -( (x2.y * b01) + (x2.z * b02) ) / b00            B*x2=0
            //      = -( (x2.y * b11) + (x2.z * b12) ) / b10            B*x2=0
            //      = -( (x2.y * b21) + (x2.z * b22) ) / b20            B*x2=0
            // 
            // ( (x1.y * x2.y) + (x1.z * x2.z) ) / x1.x = ( (x2.y * b01) + (x2.z * b02) ) / b00
            //                                          = ( (x2.y * b11) + (x2.z * b12) ) / b10
            //                                          = ( (x2.y * b21) + (x2.z * b22) ) / b20
            // 
            // Eliminate x2.y:
            // x2.y = x2.z * ( b02 * x1.x / b00 - x1.z ) / ( x1.y - b01 * x1.x / b00 )
            // x2.y = x2.z * ( b12 * x1.x / b10 - x1.z ) / ( x1.y - b11 * x1.x / b10 )
            // x2.y = x2.z * ( b22 * x1.x / b20 - x1.z ) / ( x1.y - b21 * x1.x / b20 )
            // 
            // x2.y = k*x2.z: k = ( b02 * x1.x / b00 - x1.z ) / ( x1.y - b01 * x1.x / b00 )
            //                k = ( b12 * x1.x / b10 - x1.z ) / ( x1.y - b11 * x1.x / b10 )
            //                k = ( b22 * x1.x / b20 - x1.z ) / ( x1.y - b21 * x1.x / b20 )
            // 
            // Sqrt( 1 - (x2.y * x2.y) - (x2.z * x2.z) ) = -( (x1.y * x2.y) + (x1.z * x2.z) ) / x1.x
            // 
            // 1 - x2.y^2 - x2.z^2 = ( (x1.y * x2.y) + (x1.z * x2.z) )^2 / x1.x^2
            // 
            // x1.x^2( 1 - x2.y^2 - x2.z^2 ) = ( x1.y * x2.y + x1.z * x2.z )^2
            // 
            // x1.x^2 - (x1.x^2 * x2.y^2) - (x1.x^2 * x2.z^2) - (x1.y^2 * x2.y^2) - (x1.z^2 * x2.z^2) - 2( x1.y * x2.y * x1.z * x2.z ) = 0
            // 
            // x1.x^2 - x1.x^2 * (k*x2.z)^2 - x1.x^2 * x2.z^2 - x1.y^2 * (k*x2.z)^2 - x1.z^2 * x2.z^2 - 2 * x1.y * k*x2.z * x1.z * x2.z = 0
            // 
            // ( -(x1.x^2 * k^2) - (x1.x^2) - (x1.y^2 * k^2) - (x1.z^2) - (2 * x1.y * k * x1.z) )(x2.z^2) + (0 * x2.z) + x1.x^2 = 0
            // quadratic in x2.z:
            // a = -(x1.x^2 * k^2) - (x1.x^2) - (x1.y^2 * k^2) - (x1.z^2) - (2 * x1.y * k * x1.z)
            // b = 0
            // c = x1.x^2
            // 
            
            // 
            // Eliminate x2.y: (again...)
            // Sqrt( 1 - (x2.y * x2.y) - (x2.z * x2.z) ) = -( (x2.y * b01) + (x2.z * b02) ) / b00
            // 
            // 1 - (x2.y * x2.y) - (x2.z * x2.z) = ( (x2.y * b01) + (x2.z * b02) )^2 / b00^2
            // 1 - (k*x2.z)^2 - x2.z^2 = ( (k * x2.z * b01) + (x2.z * b02) )^2 / b00^2
            // b00^2( 1 - (k*x2.z)^2 - x2.z^2 ) = ( (k*x2.z * b01) + (x2.z * b02) )^2
            // b00^2( 1 - (k*x2.z)^2 - x2.z^2 ) =  (k*x2.z * b01)^2 + (x2.z * b02)^2 + 2( k*x2.z * b01 * x2.z * b02 )
            // ( b00^2 * k^2 * x2.z^2 ) + ( b00^2 * x2.z^2 ) + ( k^2 * x2.z^2 * b01^2 ) + ( x2.z^2 * b02^2 ) + 2( k*x2.z * b01 * x2.z * b02 ) = b00^2
            // 
            // x2.z^2 * ( ( b00^2 * k^2 ) + b00^2 + ( k^2 * b01^2 ) + ( b02^2 + 2 * k * b01 * b02 ) ) = b00^2
            // 
            // x2.z = Sqrt( b00^2 / ( ( b00 * k )^2 + b00^2 + ( k * b01 )^2 + b02^2 + ( 2 * k * b01 * b02 ) ) )
            // 
            
            // 
            // Eliminate x2.x: (again...)
            // x2.x = -( (x2.y * b01) + (x2.z * b02) ) / b00            B*x2=0
            //      = -( (x2.y * b11) + (x2.z * b12) ) / b10            B*x2=0
            //      = -( (x2.y * b21) + (x2.z * b22) ) / b20            B*x2=0
            // 
            // x2.y * ( b01 / b00 - b11 / b10 ) = x2.z * ( b12 / b10 - b02 / b00 )
            // x2.y = x2.z * ( b12 / b10 - b02 / b00 ) / ( b01 / b00 - b11 / b10 )
            // x2.y = k * x2.z, k = ( b12 / b10 - b02 / b00 ) / ( b01 / b00 - b11 / b10 )
            // 
            // x2.y = x2.z * ( b12 * b00 - b02 * b10 ) / ( b01 * b10 - b11 * b00 )
            // 
            // ( (x2.y * b11) + (x2.z * b12) ) / b10 = ( (x2.y * b21) + (x2.z * b22) ) / b20
            // x2.y * ( b11 * b20 - b21 * b10 ) = x2.z * ( b22 * b10 - b12 * b20 )
            // x2.y = x2.z * ( b22 * b10 - b12 * b20 ) / ( b11 * b20 - b21 * b10 )
            // 
            
            const VF32 x1x_div_b00( Div24( X(x1), b00 ) );
            const VF32 x1x_div_b10( Div24( X(x1), b10 ) );
            const VF32 x1x_div_b20( Div24( X(x1), b20 ) );
            
            const VF32 k1( Div24( b02 * x1x_div_b00 - Z(x1), Y(x1) - b01 * x1x_div_b00 ) );
            const VF32 k2( Div24( b12 * x1x_div_b10 - Z(x1), Y(x1) - b11 * x1x_div_b10 ) );
            const VF32 k3( Div24( b22 * x1x_div_b20 - Z(x1), Y(x1) - b21 * x1x_div_b20 ) );
            const VF32 k( Select( ( Abs( b00 ) >= Abs( b10 ) ) & ( Abs( b00 ) >= Abs( b20 ) ), k1,
                          Select( ( Abs( b10 ) >= Abs( b00 ) ) & ( Abs( b10 ) >= Abs( b20 ) ), k2,
                                                                                               k3 ) ) );
            
            const VF32 a( -Square( X(x1) * k ) - Square( X(x1) ) - Square( Y(x1) * k ) - Square( Z(x1) ) - two * Y(x1) * k * Z(x1) );
            const VF32 c( Square( X(x1) ) );
            // az^2 + bz + c = 0
            // z^2 = -c/a
            // z = Sqrt(-c/a)
            const VF32 x2z( Sqrt24( Div24( -c, a ) ) );
            const VF32 x2y( x2z * k );
            const VF32 x2x( -Div24( x2y * b01 + x2z * b02, b00 ) );
            
            const Vector3 x2( MakeVector3( x2x, x2y, x2z ) );
            const Vector3 x3( x1 ^ x2 );
            
            const Vector3 test1( A_minus_I_lambda1 * x1 ); // == 0
            const Vector3 test2( A_minus_I_lambda1 * x2 ); // == 0
            const Vector3 test3( A_minus_I_lambda1 * x3 ); // == 0
            const VF32    test4( x1 | x2 );                // == 0
            const VF32    test5( x1 | x3 );                // == 0
            const VF32    test6( x2 | x3 );                // == 0
            const VF32    test7( LengthSquared( x1 ) );    // == 1
            const VF32    test8( LengthSquared( x2 ) );    // == 1
            const VF32    test9( LengthSquared( x3 ) );    // == 1
            
            const VBool t1_valid( LengthSquared( test1 ) <= nearlyZero );
            const VBool t2_valid( LengthSquared( test2 ) <= nearlyZero );
            const VBool t3_valid( LengthSquared( test3 ) <= nearlyZero );
            const VBool t4_valid( Abs( test4 ) <= nearlyZero );
            const VBool t5_valid( Abs( test5 ) <= nearlyZero );
            const VBool t6_valid( Abs( test6 ) <= nearlyZero );
            const VBool t7_valid( Abs( test7 - one ) <= nearlyZero );
            const VBool t8_valid( Abs( test8 - one ) <= nearlyZero );
            const VBool t9_valid( Abs( test9 - one ) <= nearlyZero );
            
            valid = ( ( x1_valid & t1_valid ) &
                      ( t2_valid & t3_valid ) ) &
                    ( ( t4_valid & t5_valid ) &
                      ( t6_valid & t7_valid ) ) &
                      ( t8_valid & t9_valid );
        //}
        //else if ( ( Abs(Y(x1)) >= Abs(X(x1)) ) &
        //          ( Abs(Y(x1)) >= Abs(Z(x1)) ) )
        //{
        //    // Eliminate x2.y: ( x1.y >= x1.x ) && ( x1.y >= x1.z )
        //    // x2.y = Sqrt( 1 - (x2.x * x2.x) - (x2.z * x2.z) )
        //    //      = -( (x1.x * x2.x) + (x1.z * x2.z) ) / x1.y
        //    //      = -( (x2.x * b00) + (x2.z * b02) ) / b01
        //    //      = -( (x2.x * b10) + (x2.z * b12) ) / b11
        //    //      = -( (x2.x * b20) + (x2.z * b22) ) / b21
        //    // 
        //    // ( (x1.x * x2.x) + (x1.z * x2.z) ) / x1.y = ( (x2.x * b00) + (x2.z * b02) ) / b01
        //    //                                          = ( (x2.x * b10) + (x2.z * b12) ) / b11
        //    //                                          = ( (x2.x * b20) + (x2.z * b22) ) / b21
        //    // 
        //    
        //    DebugBreak();
        //}
        //else // if ( ( Abs(Z(x1)) >= Abs(X(x1)) ) &
        //     //      ( Abs(Z(x1)) >= Abs(Y(x1)) ) )
        //{
        //    // Eliminate x2.z: ( x1.z >= x1.x ) && ( x1.z >= x1.y )
        //    // x2.z = Sqrt( 1 - (x2.x * x2.x) - (x2.y * x2.y) )
        //    //      = -( (x1.x * x2.x) + (x1.y * x2.y) ) / x1.z
        //    //      = -( (x2.x * b00) + (x2.y * b01) ) / b02
        //    //      = -( (x2.x * b10) + (x2.y * b11) ) / b12
        //    //      = -( (x2.x * b20) + (x2.y * b21) ) / b22
        //    // 
        //    // ( (x1.x * x2.x) + (x1.y * x2.y) ) / x1.z = ( (x2.x * b00) + (x2.y * b01) ) / b02
        //    //                                          = ( (x2.x * b10) + (x2.y * b11) ) / b12
        //    //                                          = ( (x2.x * b20) + (x2.y * b21) ) / b22
        //    // 
        //    // Eliminate x2.y:
        //    // x2.y = ( x2.x * b00 / b02 - x1.x * x2.x / x1.z ) / ( x1.y / x1.z - b01 / b02 )
        //    // x2.y = ( x2.x * b10 / b12 - x1.x * x2.x / x1.z ) / ( x1.y / x1.z - b11 / b12 )
        //    // x2.y = ( x2.x * b20 / b22 - x1.x * x2.x / x1.z ) / ( x1.y / x1.z - b21 / b22 )
        //    // 
        //    // x2.y = x2.x * ( b00 / b02 - x1.x / x1.z ) / ( x1.y / x1.z - b01 / b02 )
        //    // x2.y = x2.x * ( b10 / b12 - x1.x / x1.z ) / ( x1.y / x1.z - b11 / b12 )
        //    // x2.y = x2.x * ( b20 / b22 - x1.x / x1.z ) / ( x1.y / x1.z - b21 / b22 )
        //    // 
        //    
        //    const VF32 test1( ( b00 / b02 - X(x1) / Z(x1) ) / ( Y(x1) / Z(x1) - b01 / b02 ) );
        //    const VF32 test2( ( b10 / b12 - X(x1) / Z(x1) ) / ( Y(x1) / Z(x1) - b11 / b12 ) );
        //    const VF32 test3( ( b20 / b22 - X(x1) / Z(x1) ) / ( Y(x1) / Z(x1) - b21 / b22 ) );
        //    
        //    DebugBreak();
        //}
        
        SetColumns( pEigenVectorMatrix, x1, x2, x3 );
        *pEigenValueMatrix = MakeScaleMat3( lambda1, lambda1, lambda1 );
    }
    else
    {
        *pEigenVectorMatrix = Const<Mat33>::Identity();
        *pEigenValueMatrix = Const<ScaleMat3>::Zero();
    }
    
    #if defined( _DEBUG ) && defined( USER_JDENTON )
    const Mat33 P( *pEigenVectorMatrix );
    const Mat33 D( MakeMat33( *pEigenValueMatrix ) );
    const Mat33 A2( P * D * Inverse( P ) );
    #endif
    
    return valid;
}

/// Mat33 Sqrt of Symmetric Matrix
NOALIAS const VBool SymmetricSqrt( const Mat33 MATREF A, Mat33* const pResult )
{
    Mat33     P;
    ScaleMat3 D;
    
    const VF32 det( Determinant( A ) );
    
    const VBool valid( SymmetricEigenDecomposition( A, &P, &D ) );
    
    const ScaleMat3 sqrtD( MakeScaleMat3( Sqrt24( GetSX( D ) ),
                                          Sqrt24( GetSY( D ) ),
                                          Sqrt24( GetSZ( D ) ) ) );
    
    //*pResult = P * sqrtD * Inverse( P );
    *pResult = P * sqrtD * Transpose( P );
    
    return valid & ( det >= Const<VF32>::Zero() );
}




//! @}
}; // namespace Maths


