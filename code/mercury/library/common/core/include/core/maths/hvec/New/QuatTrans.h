#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_QUAT_TRANS_H
#define SOFT_MATHS_HVEC_NEW_QUAT_TRANS_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_QUAT_TRANS_H
#define MATHS_HVEC_NEW_QUAT_TRANS_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup QuatTrans
/// @{

/// QuatTrans Get Quaternion
NOALIAS __forceinline const Quaternion Quat( const QuatTrans& qt );

/// QuatTrans Get Translation
NOALIAS __forceinline const Vector3 Trans( const QuatTrans& qt );

/// QuatTrans Set Quaternion
NOALIAS __forceinline void SetQuat( QuatTrans* const __restrict pQT, const Quaternion PCREF q );

/// QuatTrans Set Translation
NOALIAS __forceinline void SetTrans( QuatTrans* const __restrict pQT, const Vector3 PCREF t );

/// Make QuatTrans from Mat43
NOALIAS __forceinline const QuatTrans MakeQuatTrans( const Mat43 MATREF m43 );

/// Make QuatTrans from Quaternion and a Vector3
NOALIAS __forceinline const QuatTrans MakeQuatTrans( const Quaternion PCREF q, const Vector3 PCREF t );

/// Make QuatTrans from a DualQuaternion
NOALIAS __forceinline const QuatTrans MakeQuatTrans( const DualQuaternion& dq );


/// Make Mat43 from QuatTrans
NOALIAS __forceinline const Mat43 MakeMat43( const QuatTrans& qt );

/// QuatTrans Nearest: Returns whichever one of the two equivalent destinations is nearest to the source
NOALIAS __forceinline const QuatTrans Nearest( const QuatTrans& src, const QuatTrans& dst );

/// QuatTrans Multiplication Operator: Vector3 * QuatTrans
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const QuatTrans& qt );

/// QuatTrans Multiplication Operator: Point3 * QuatTrans
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF v, const QuatTrans& qt );

/// QuatTrans Multiplication Operator: QuatTrans * QuatTrans
NOALIAS __forceinline const QuatTrans operator * ( const QuatTrans& a, const QuatTrans& b );

/// QuatTrans Inverse
NOALIAS __forceinline const QuatTrans Inverse( const QuatTrans& qt );

/// QuatTrans Normalise
NOALIAS __forceinline const QuatTrans Normalise( const QuatTrans& a );


/// QuatTrans Lerp: Linear Interpolation
NOALIAS __forceinline const QuatTrans Lerp( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t );

/// QuatTrans Lerp: Linear Interpolation with normalisation
NOALIAS __forceinline const QuatTrans LerpNormalised( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t );

/// QuatTrans Nearest Lerp: Nearest Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const QuatTrans NearestLerp( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t );

/// QuatTrans Nearest Lerp: Nearest Linear Interpolation - Negates dst if necessary - with normalisation
NOALIAS __forceinline const QuatTrans NearestLerpNormalised( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t );

/// QuatTrans Slerp: Spherical Linear Interpolation
NOALIAS __forceinline const QuatTrans Slerp( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t );

/// QuatTrans Nearest Slerp: Spherical Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const QuatTrans NearestSlerp( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t );



/// QuatTrans Serialise to Stream
template < typename T_Stream > void Serialise( const QuatTrans& src, T_Stream& stream );

/// QuatTrans Deserialise from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, QuatTrans* const __restrict pDst );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

