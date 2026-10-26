#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_DUAL_QUATERNION_H
#define SOFT_MATHS_HVEC_NEW_DUAL_QUATERNION_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_DUAL_QUATERNION_H
#define MATHS_HVEC_NEW_DUAL_QUATERNION_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup DualQuaternion
/// @{

/// Make DualQuaternion from a Vec8F32
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Vec8F32 MATREF v );

/// Make DualQuaternion from a Vector8
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Vector8& v );

/// Make DualQuaternion from a Quaternion
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Quaternion PCREF q );

/// Make DualQuaternion from a Vector3 translation
/// WARNING: The Dual Quaternion representation of a Point is NOT the same as the representation of a Translation! (There's a factor of 2 difference)
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Vector3 PCREF t );

/// Make DualQuaternion from a Point3
/// WARNING: The Dual Quaternion representation of a Point is NOT the same as the representation of a Translation! (There's a factor of 2 difference)
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Point3 PCREF p );

/// Make DualQuaternion from a Quaternion & a Vector3 translation
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Quaternion PCREF q, const Vector3 PCREF t );

/// Make DualQuaternion from a QuatTrans
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const QuatTrans& qt );


/// DualQuaternion Get Quat components as Vec4F32
NOALIAS __forceinline const Vec4F32 GetQuatVec4( const DualQuaternion& a );

/// DualQuaternion Get Dual components
NOALIAS __forceinline const Vec4F32 GetDualVec4( const DualQuaternion& a );

/// DualQuaternion Get Quaternion components
NOALIAS __forceinline const Quaternion GetQuaternion( const DualQuaternion& a );

/// DualQuaternion Get Translation components
NOALIAS __forceinline const Vector3 GetTranslation( const DualQuaternion& a );

/// DualQuaternion Get Quaternion components
NOALIAS __forceinline const Quaternion Quat( const DualQuaternion& a );

/// DualQuaternion Get Translation components
NOALIAS __forceinline const Vector3 Trans( const DualQuaternion& a );


/// DualQuaternion Select
NOALIAS __forceinline const DualQuaternion Select( const VBool PCREF condition, const DualQuaternion& result_if_true, const DualQuaternion& result_if_false );


/// DualQuaternion Norm: Result is a Dual number with the real part in X and the dual part in Y
NOALIAS __forceinline const DualScalar Norm( const DualQuaternion& a );

/// DualQuaternion Norm Squared: Result is a Dual number with the real part in X and the dual part in Y
NOALIAS __forceinline const DualScalar NormSquared( const DualQuaternion& a );

/// DualQuaternion Conjugate ( Multiplicative Inverse of Unit Dual Quaternion )
/// for Q = q0 + e.qe, multiplicative conjugate of Q, !Q = !q0 + e.!qe ( e (epsilon) is dual unit, e^2 = 0 )
NOALIAS __forceinline const DualQuaternion Conjugate( const DualQuaternion& a );

/// DualQuaternion Dual Conjugate
/// for Q = q0 + e.qe, dual conjugate of Q = q0 - e.qe ( e (epsilon) is dual unit, e^2 = 0 )
NOALIAS __forceinline const DualQuaternion DualConjugate( const DualQuaternion& a );

/// DualQuaternion Versor Conjugate (combination of standard conjugate & dual conjugate)
/// for Q = q0 + e.qe, versor conjugate of Q = !q0 - e.!qe ( e (epsilon) is dual unit, e^2 = 0 )
NOALIAS __forceinline const DualQuaternion VersorConjugate( const DualQuaternion& a );


/// DualQuaternion Scalar Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Div( const DualQuaternion& a, const VF32 VECREF b );

/// DualQuaternion Scalar Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Div24( const DualQuaternion& a, const VF32 VECREF b );


/// DualQuaternion Dual Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Div( const DualQuaternion& a, const DualScalar PCREF b );

/// DualQuaternion Dual Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Div24( const DualQuaternion& a, const DualScalar PCREF b );


/// DualQuaternion Negation Operator
NOALIAS __forceinline const DualQuaternion operator - ( const DualQuaternion& a );

/// DualQuaternion Addition Operator
NOALIAS __forceinline const DualQuaternion operator + ( const DualQuaternion& a, const DualQuaternion& b );

/// DualQuaternion Subtraction Operator
NOALIAS __forceinline const DualQuaternion operator - ( const DualQuaternion& a, const DualQuaternion& b );

/// DualQuaternion Addition Update Operator
NOALIAS __forceinline const DualQuaternion operator += ( DualQuaternion& a, const DualQuaternion& b );

/// DualQuaternion Subtraction Update Operator
NOALIAS __forceinline const DualQuaternion operator -= ( DualQuaternion& a, const DualQuaternion& b );


/// DualQuaternion Scalar Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualQuaternion& a, const VF32 VECREF b );

/// DualQuaternion Scalar Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const VF32 VECREF a, const DualQuaternion& b );

/// DualQuaternion Scalar Division Operator
NOALIAS __forceinline const DualQuaternion operator / ( const DualQuaternion& a, const VF32 VECREF b );

/// DualQuaternion Dual Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualScalar PCREF a, const DualQuaternion& b );

/// DualQuaternion Dual Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualQuaternion& a, const DualScalar PCREF b );

/// DualQuaternion Dual Division Operator
NOALIAS __forceinline const DualQuaternion operator / ( const DualQuaternion& a, const DualScalar PCREF b );

/// DualQuaternion Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualQuaternion& a, const DualQuaternion& b );


/// DualQuaternion Multiplicative Conjugate Operator ( Multiplicative Inverse of Unit Dual Quaternion )
NOALIAS __forceinline const DualQuaternion operator ! ( const DualQuaternion& a );

/// DualQuaternion Right Division Operator: num / denom = num * !denom
NOALIAS __forceinline const DualQuaternion operator / ( const DualQuaternion& num, const DualQuaternion& denom );


/// DualQuaternion * Quaternion Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualQuaternion& a, const Quaternion PCREF b );

/// Quaternion * DualQuaternion Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const Quaternion PCREF a, const DualQuaternion& b );


/// DualQuaternion Point3 Multiplication Operator: Point3 * DualQuaternion --> Point3
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const DualQuaternion& dq );

/// DualQuaternion Vector3 Multiplication Operator: Vector3 * DualQuaternion --> Vector3
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const DualQuaternion& dq );

/// DualQuaternion Normal3 Multiplication Operator: Normal3 * DualQuaternion --> Normal3
NOALIAS __forceinline const Normal3 operator * ( const Normal3 PCREF n, const DualQuaternion& dq );


/// DualQuaternion Nearest: Returns whichever one of the two equivalent destinations is nearest to the source
NOALIAS __forceinline const DualQuaternion Nearest( const DualQuaternion& src, const DualQuaternion& dst );


/// DualQuaternion Left Division: denom \ num = !denom * num
NOALIAS __forceinline const DualQuaternion LeftDivide( const DualQuaternion& denom, const DualQuaternion& num );

/// DualQuaternion Right Division: num / denom = num * !denom
NOALIAS __forceinline const DualQuaternion RightDivide( const DualQuaternion& num, const DualQuaternion& denom );

/// DualQuaternion Left Division: denom \ num = !denom * num - negates denom if necessary
NOALIAS __forceinline const DualQuaternion NearestLeftDivide( const DualQuaternion& denom, const DualQuaternion& num );

/// DualQuaternion Nearest Right Division: num / denom = num * !denom - negates denom if necessary
NOALIAS __forceinline const DualQuaternion NearestRightDivide( const DualQuaternion& num, const DualQuaternion& denom );


/// DualQuaternion Inverse (faster but only 22 bit precision)
/// NOTE: The inverse of a unit dual quaternion is just the conjugate!
NOALIAS __forceinline const DualQuaternion Inverse( const DualQuaternion& a );

/// DualQuaternion Inverse (slower but full 24 bit precision)
/// NOTE: The inverse of a unit dual quaternion is just the conjugate!
NOALIAS __forceinline const DualQuaternion Inverse24( const DualQuaternion& a );


/// DualQuaternion Normalise (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Normalise( const DualQuaternion& a );

/// DualQuaternion Normalise (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Normalise24( const DualQuaternion& a );


/// Dual Quaternion Logarithm (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Log( const DualQuaternion& q );

/// Dual Quaternion Logarithm (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Log24( const DualQuaternion& q );


/// Dual Quaternion Exponential (faster but only 22 bit precision)
/// p must have zero scalar part and non-zero non-dual part [ i.e. result from Log() ]
NOALIAS __forceinline const DualQuaternion Exp( const DualQuaternion& p );

/// Dual Quaternion Exponential (slower but full 24 bit precision)
/// p must have zero scalar part and non-zero non-dual part [ i.e. result from Log() ]
NOALIAS __forceinline const DualQuaternion Exp24( const DualQuaternion& p );


/// Dual Quaternion Power: q^u = exp( u * log(q) ) (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Pow( const DualQuaternion& q, const VF32 VECREF u );

/// Dual Quaternion Power: q^u = exp( u * log(q) ) (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Pow24( const DualQuaternion& q, const VF32 VECREF u );


/// Dual Quaternion Linear Blend (DLB)
NOALIAS __forceinline const DualQuaternion Lerp( const DualQuaternion& src, const DualQuaternion& dst, const VF32 VECREF t );

/// Dual Quaternion Nearest Linear Blend (Nearest DLB)
NOALIAS __forceinline const DualQuaternion NearestLerp( const DualQuaternion& src, const DualQuaternion& dst, const VF32 VECREF t );

/// Dual Quaternion Linear Blend (DLB)
NOALIAS __forceinline const DualQuaternion DLB(
    const DualQuaternion& dq1,
    const DualQuaternion& dq2,
    const DualQuaternion& dq3,
    const VF32 VECREF w1,
    const VF32 VECREF w2,
    const VF32 VECREF w3 );

/// Dual Quaternion Linear Blend (DLB)
NOALIAS __forceinline const DualQuaternion DLB(
    const DualQuaternion& dq1,
    const DualQuaternion& dq2,
    const DualQuaternion& dq3,
    const DualQuaternion& dq4,
    const VF32 VECREF w1,
    const VF32 VECREF w2,
    const VF32 VECREF w3,
    const VF32 VECREF w4 );

/// Dual Quaternion Linear Blend (DLB)
NOALIAS __forceinline const DualQuaternion DLB(
    DualQuaternion const * const __restrict pQ,  ///< Array of dual quaternions to blend
    VF32           const * const __restrict pW,  ///< Array of weights for each dual quaternion
    const U32                               n ); ///< Number of elements in arrays


/// DualQuaternion Slerp: Screw Linear Interpolation
NOALIAS __forceinline const DualQuaternion Slerp( const DualQuaternion& src, const DualQuaternion& dst, const VF32 VECREF t );

/// DualQuaternion Nearest Slerp: Screw Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const DualQuaternion NearestSlerp( const DualQuaternion& src, const DualQuaternion& dst, const VF32 VECREF t );

/// Dual Quaternion Iterative Blending (DIB)
NOALIAS __forceinline const DualQuaternion DIB(
    DualQuaternion const * const __restrict pQ,  ///< Array of dual quaternions to blend
    VF32           const * const __restrict pW,  ///< Array of weights for each dual quaternion
    const U32                               n,   ///< Number of elements in arrays
    const VF32 VECREF                        p ); ///< Required precision

/// Make Mat43 from DualQuaternion
NOALIAS __forceinline const Mat43 MakeMat43( const DualQuaternion& dq );


/// DualQuaternion Serialise to Stream
template < typename T_Stream >
void Serialise( const DualQuaternion& src, T_Stream& stream );

/// DualQuaternion Deserialise from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, DualQuaternion* const __restrict pDst );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

