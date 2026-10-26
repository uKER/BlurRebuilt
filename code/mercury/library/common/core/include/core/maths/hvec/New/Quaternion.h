#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_QUATERNION_H
#define SOFT_MATHS_HVEC_NEW_QUATERNION_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_QUATERNION_H
#define MATHS_HVEC_NEW_QUATERNION_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Quaternion
/// @{

/// Load Quaternion from an F32 array
template <>
NOALIAS __forceinline const Quaternion Load( F32 const * const __restrict pSrc );

/// Store Quaternion into an F32 array
NOALIAS __forceinline void Store( const Quaternion PCREF src, F32* const __restrict pDst );


/// Quaternion Get Vec4F32 components: imaginary vector part in result.XYZ, real scalar part in result.W
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Quaternion PCREF q );

/// Quaternion Get Vector3 components: imaginary vector part in result.XYZ
NOALIAS __forceinline const Vector3 GetVector3( const Quaternion PCREF q );

/// Quaternion Get Vector4 components: imaginary vector part in result.XYZ, real scalar part in result.W
NOALIAS __forceinline const Vector4 GetVector4( const Quaternion PCREF q );

/// Make generic Quaternion from a Vec4F32
NOALIAS __forceinline const Quaternion MakeQuaternion( const Vec4F32 VECREF v );

/// Make generic Quaternion from a Vector3 : the real W component is set to zero
NOALIAS __forceinline const Quaternion MakeQuaternion( const Vector3 PCREF v );

/// Make generic Quaternion from a Point3 : the real W component is set to zero
NOALIAS __forceinline const Quaternion MakeQuaternion( const Point3 PCREF p );

/// Make generic Quaternion from a Normal3 : the real W component is set to zero
NOALIAS __forceinline const Quaternion MakeQuaternion( const Normal3 PCREF n );

/// Make generic Quaternion from a Vector4
NOALIAS __forceinline const Quaternion MakeQuaternion( const Vector4 PCREF v );

/// Make generic Quaternion from a set of 4 VF32 components
NOALIAS __forceinline const Quaternion MakeQuaternion( const VF32 VECREF imag_x, const VF32 VECREF imag_y, const VF32 VECREF imag_z, const VF32 VECREF real_w );

/// Make unit rotation Quaternion from a Normal3 axis and VF32 rotation angle (radians)
NOALIAS __forceinline const Quaternion MakeUnitQuaternion( const Normal3 PCREF axis, const VF32 VECREF theta );

/// Make unit rotation Quaternion from a Vector3 axis and VF32 rotation angle (radians)
NOALIAS __forceinline const Quaternion MakeUnitQuaternion( const Vector3 PCREF axis, const VF32 VECREF theta );


/// Get Quaternion Imaginary part as Vector3
NOALIAS __forceinline const Vector3 Imag( const Quaternion PCREF q );

/// Get Quaternion Real part as VF32 == Cos( theta / 2 ), where theta is the rotation angle
NOALIAS __forceinline const VF32 Real( const Quaternion PCREF q );

/// Quaternion Length
NOALIAS __forceinline const VF32 Length( const Quaternion PCREF q );

/// Quaternion Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Quaternion PCREF q );

/// Quaternion Rotation Axis (normalised)
NOALIAS __forceinline const Normal3 RotationAxis( const Quaternion PCREF q );

/// Quaternion Rotation Angle
NOALIAS __forceinline const VF32 RotationAngle( const Quaternion PCREF q );

//Quaternion Rotation Vector
NOALIAS __forceinline const Vector3 RotationVector( const Quaternion PCREF q );

/// Quaternion Select
NOALIAS __forceinline const Quaternion Select( const VBool PCREF condition, const Quaternion PCREF result_if_true, const Quaternion PCREF result_if_false );

/// Quaternion Negation Operator
NOALIAS __forceinline const Quaternion operator - ( const Quaternion PCREF q );

/// Quaternion Addition Operator
NOALIAS __forceinline const Quaternion operator + ( const Quaternion PCREF a, const Quaternion PCREF b );

/// Quaternion Subtraction Operator
NOALIAS __forceinline const Quaternion operator - ( const Quaternion PCREF a, const Quaternion PCREF b );

/// Quaternion Dot Product: Returns 4 element dot product == cosine( angle / 2 ) between the 2 quaternions!
NOALIAS __forceinline const VF32 operator | ( const Quaternion PCREF a, const Quaternion PCREF b );

/// Quaternion Scalar Multiplication Operator
NOALIAS __forceinline const Quaternion operator * ( const Quaternion PCREF a, const VF32 VECREF b );

/// Quaternion Scalar Multiplication Operator
NOALIAS __forceinline const Quaternion operator * ( const VF32 VECREF a, const Quaternion PCREF b );

/// Quaternion Scalar Division Operator
NOALIAS __forceinline const Quaternion operator / ( const Quaternion PCREF a, const VF32 VECREF b );

/// Quaternion Multiplication Operator
NOALIAS __forceinline const Quaternion operator * ( const Quaternion PCREF a, const Quaternion PCREF b );

/// Quaternion Conjugate Operator ( Multiplicative Inverse of Unit Quaternion )
NOALIAS __forceinline const Quaternion operator ! ( const Quaternion PCREF q );

/// Quaternion Right Division: num / denom = num * !denom
NOALIAS __forceinline const Quaternion operator / ( const Quaternion PCREF num, const Quaternion PCREF denom );

/// Quaternion Vector3 Multiplication Operator: Vector3 * Quaternion --> Vector3
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const Quaternion PCREF q );

/// Quaternion Point3 Multiplication Operator: Point3 * Quaternion --> Point3
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const Quaternion PCREF q );

/// Quaternion Normal3 Multiplication Operator: Normal3 * Quaternion --> Normal3
NOALIAS __forceinline const Normal3 operator * ( const Normal3 PCREF n, const Quaternion PCREF q );

/// Quaternion Normalise: Returns normalised copy of this quaternion
NOALIAS __forceinline const Quaternion Normalise( const Quaternion PCREF q );

/// Quaternion Normalise: Normalise this quaternion
NOALIAS __forceinline void Normalise( Quaternion* const __restrict pQ );

/// Quaternion Nearest: Returns whichever one of the two equivalent destinations is nearest to the source
NOALIAS __forceinline const Quaternion Nearest( const Quaternion PCREF src, const Quaternion PCREF dst );


/// Quaternion MulAdd: a * b + c
NOALIAS __forceinline const Quaternion MulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion MulSub: a * b - c
NOALIAS __forceinline const Quaternion MulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion NegMulAdd: -( a * b + c )
NOALIAS __forceinline const Quaternion NegMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion NegMulSub: -( a * b - c ) = c - a * b
NOALIAS __forceinline const Quaternion NegMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );


/// Quaternion Nearest MulAdd: Nearest(c,a) * b + c
NOALIAS __forceinline const Quaternion NearMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion Nearest MulSub: Nearest(c,a) * b - c
NOALIAS __forceinline const Quaternion NearMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion Nearest NegMulAdd: -( Nearest(c,a) * b + c )
NOALIAS __forceinline const Quaternion NearNegMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion Nearest NegMulSub: -( Nearest(c,a) * b - c ) = c - Nearest(c,a) * b
NOALIAS __forceinline const Quaternion NearNegMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );


/// Quaternion Normalised MulAdd: Normalise( a * b + c )
NOALIAS __forceinline const Quaternion NormMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion Normalised MulSub: Normalise( a * b - c )
NOALIAS __forceinline const Quaternion NormMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion Normalised NegMulAdd: Normalise( -( a * b + c ) )
NOALIAS __forceinline const Quaternion NormNegMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion Normalised NegMulSub: Normalise( -( a * b - c ) ) = Normalise( c - a * b )
NOALIAS __forceinline const Quaternion NormNegMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );


/// Quaternion Normalised Nearest MulAdd: Normalise( Nearest(c,a) * b + c )
NOALIAS __forceinline const Quaternion NormNearMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion Normalised Nearest MulSub: Normalise( Nearest(c,a) * b - c )
NOALIAS __forceinline const Quaternion NormNearMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion Normalised Nearest NegMulAdd: Normalise( -( Nearest(c,a) * b + c ) )
NOALIAS __forceinline const Quaternion NormNearNegMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );

/// Quaternion Normalised Nearest NegMulSub: Normalise( -( Nearest(c,a) * b - c ) ) = Normalise( c - Nearest(c,a) * b )
NOALIAS __forceinline const Quaternion NormNearNegMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c );


/// Quaternion Right Division: num / denom = num * !denom  - negates denom if necessary
NOALIAS __forceinline const Quaternion NearestDivide( const Quaternion PCREF num, const Quaternion PCREF denom );

/// Quaternion Difference ( Left Division )
NOALIAS __forceinline const Quaternion Difference( const Quaternion PCREF src, const Quaternion PCREF dst );

/// Quaternion Nearest Difference ( Left Division ) : returns difference quaternion between src and the nearest representation of dst to src
NOALIAS __forceinline const Quaternion NearestDifference( const Quaternion PCREF src, const Quaternion PCREF dst );

/// Quaternion Differential ( Differential of the left division of dst by src )
NOALIAS __forceinline const Vector3 Differential( const Quaternion PCREF src, const Quaternion PCREF dst );


/// Quaternion Left Division: denom \ num = !denom * num
NOALIAS __forceinline const Quaternion LeftDivide( const Quaternion PCREF denom, const Quaternion PCREF num );

/// Quaternion Right Division: num / denom = num * !denom
NOALIAS __forceinline const Quaternion RightDivide( const Quaternion PCREF num, const Quaternion PCREF denom );

/// Quaternion Nearest Left Division: denom \ num = !denom * num  - negates denom if necessary
NOALIAS __forceinline const Quaternion NearestLeftDivide( const Quaternion PCREF denom, const Quaternion PCREF num );

/// Quaternion Nearest Right Division: num / denom = num * !denom  - negates denom if necessary
NOALIAS __forceinline const Quaternion NearestRightDivide( const Quaternion PCREF num, const Quaternion PCREF denom );


/// Quaternion Left Differential ( Differential of the left division of num by denom )
NOALIAS __forceinline const Vector3 LeftDifferential( const Quaternion PCREF denom, const Quaternion PCREF num );

/// Quaternion Right Differential ( Differential of the right division of num by denom )
NOALIAS __forceinline const Vector3 RightDifferential( const Quaternion PCREF num, const Quaternion PCREF denom );

/// Quaternion Nearest Left Differential ( Differential of the left division of num by denom )
NOALIAS __forceinline const Vector3 NearestLeftDifferential( const Quaternion PCREF denom, const Quaternion PCREF num );

/// Quaternion Nearest Right Differential ( Differential of the right division of num by denom )
NOALIAS __forceinline const Vector3 NearestRightDifferential( const Quaternion PCREF num, const Quaternion PCREF denom );


/// Quaternion Lerp: Linear Interpolation
NOALIAS __forceinline const Quaternion Lerp( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t );

/// Quaternion Lerp: Linear Interpolation with normalisation
NOALIAS __forceinline const Quaternion LerpNormalised( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t );

/// Quaternion Nearest Lerp: Nearest Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const Quaternion NearestLerp( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t );

/// Quaternion Nearest Lerp: Nearest Linear Interpolation - Negates dst if necessary - with normalisation
NOALIAS __forceinline const Quaternion NearestLerpNormalised( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t );

/// Quaternion Slerp: Spherical Linear Interpolation
NOALIAS __forceinline const Quaternion Slerp( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t );

/// Quaternion Nearest Slerp: Spherical Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const Quaternion NearestSlerp( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t );


/// Make unit rotation Quaternion from a Mat33 rotation matrix
NOALIAS const Quaternion MakeUnitQuaternion( const Mat33 MATREF m );

/// Make orthonormal rotation Mat33 from Quaternion
NOALIAS const Mat33 MakeMat33( const Quaternion PCREF q );

/// Quaternion get matrix row X
NOALIAS __forceinline const Vector3 MatrixRowX( const Quaternion PCREF q );

/// Quaternion get matrix row Y
NOALIAS __forceinline const Vector3 MatrixRowY( const Quaternion PCREF q );

/// Quaternion get matrix row Z
NOALIAS __forceinline const Vector3 MatrixRowZ( const Quaternion PCREF q );


/// Quaternion Serialise to Stream
template < typename T_Stream > void Serialise( const Quaternion PCREF src, T_Stream& stream );

/// Quaternion Deserialise from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Quaternion* const __restrict pDst );




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

