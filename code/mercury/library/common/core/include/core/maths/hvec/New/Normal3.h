#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_NORMAL3_H
#define SOFT_MATHS_HVEC_NEW_NORMAL3_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_NORMAL3_H
#define MATHS_HVEC_NEW_NORMAL3_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Normal3
/// @{

/// Load Normal3 from StorageNormal3
NOALIAS __forceinline const Normal3 Load( StorageNormal3 const * const __restrict pSrc );

/// Store Normal3 in StorageNormal3
NOALIAS __forceinline void Store( const Normal3 PCREF data, StorageNormal3 * const __restrict pDst );


/// Make Normal3 from Vec4F32
/// WARNING: This does NOT normalise the input vector! It is assumed that the input vector is already normalised.
/// Use the Vector3 Normalise() or the Vec4F32 Normalise3() functions to ensure that the input vector is normalised!
NOALIAS __forceinline const Normal3 MakeNormal3( const Vec4F32 VECREF src );

/// Get Vec4F32 from Normal3
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Normal3 PCREF src );

/// Make Normal3 from Vector3
/// WARNING: This does NOT normalise the input vector! It is assumed that the input vector is already normalised.
/// Use the Vector3 Normalise() function instead if the input vector is not guaranteed to be normalised!
NOALIAS __forceinline const Normal3 MakeNormal3( const Vector3 PCREF src );


/// Normal3 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Normal3 PCREF src );

/// Normal3 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Normal3 PCREF src );

/// Normal3 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Normal3 PCREF src );



/// Normal3 Write to Stream
template < typename T_Stream > void Serialise( const Normal3 PCREF src, T_Stream& stream );

/// Normal3 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Normal3* const __restrict pDst );



/// Normal3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Normal3 Permute( const Normal3 PCREF a );



/// Normal3 Unary Positive Operator
NOALIAS __forceinline const Normal3 operator + ( const Normal3 PCREF x );

/// Normal3 Unary Negation Operator
NOALIAS __forceinline const Normal3 operator - ( const Normal3 PCREF x );

/// Normal3 Sum
NOALIAS __forceinline const Vector3 operator + ( const Normal3 PCREF a, const Normal3 PCREF b );

/// Normal3 Difference
NOALIAS __forceinline const Vector3 operator - ( const Normal3 PCREF a, const Normal3 PCREF b );


/// Normal3 Scalar Multiplication
NOALIAS __forceinline const Vector3 operator * ( const Normal3 PCREF n, const VF32 VECREF s );

/// Normal3 Scalar Multiplication
NOALIAS __forceinline const Vector3 operator * ( const VF32 VECREF s, const Normal3 PCREF n );

/// Normal3 Scalar Division
NOALIAS __forceinline const Vector3 operator / ( const Normal3 PCREF n, const VF32 VECREF s );



/// Normal3 Matrix Multiplication: Normal3 * Mat33
NOALIAS __forceinline const Vector3 operator * ( const Normal3 PCREF n, const Mat33 MATREF m );

/// Normal3 Matrix Multiplication: Normal3 * Mat43
NOALIAS __forceinline const Vector3 operator * ( const Normal3 PCREF n, const Mat43 MATREF m );

/// Normal3 Matrix Multiplication: Normal3 * Mat44
NOALIAS __forceinline const Vector4 operator * ( const Normal3 PCREF n, const Mat44 MATREF m );

/// Normal3 Matrix Multiplication Update: Normal3 *= Mat33
/// WARNING: This does NOT normalise the transformed normal!
NOALIAS __forceinline void operator *= ( Normal3& n, const Mat33 MATREF m );

/// Normal3 Matrix Multiplication Update: Normal3 *= Mat43
/// WARNING: This does NOT normalise the transformed normal!
NOALIAS __forceinline void operator *= ( Normal3& n, const Mat43 MATREF m );



/// Normal3 dot product
NOALIAS __forceinline const VF32 Dot( const Normal3 PCREF a, const Normal3 PCREF b );

/// Normal3-Point3 dot product ( normal | ( point - origin ) )
NOALIAS __forceinline const VF32 Dot( const Normal3 PCREF a, const Point3 PCREF b );

/// Point3-Normal3 dot product ( normal | ( point - origin ) )
NOALIAS __forceinline const VF32 Dot( const Point3 PCREF a, const Normal3 PCREF b );


/// Normal3 cross product
NOALIAS __forceinline const Vector3 Cross( const Normal3 PCREF a, const Normal3 PCREF b );


/// Normal3 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Normal3 PCREF a, const Normal3 PCREF b );

/// Normal3-Point3 dot product operator ( normal | ( point - origin ) )
NOALIAS __forceinline const VF32 operator | ( const Normal3 PCREF a, const Point3 PCREF b );

/// Point3-Normal3 dot product operator ( normal | ( point - origin ) )
NOALIAS __forceinline const VF32 operator | ( const Point3 PCREF a, const Normal3 PCREF b );


/// Normal3 cross product operator
NOALIAS __forceinline const Vector3 operator ^ ( const Normal3 PCREF a, const Normal3 PCREF b );



/// Normal3 Renormalise (to correct for accumulated float errors) (faster but only 22 bit precision)
NOALIAS __forceinline const Normal3 Normalise( const Normal3 PCREF n );

/// Normal3 Renormalise (to correct for accumulated float errors) (slower but full 24 bit precision)
NOALIAS __forceinline const Normal3 Normalise24( const Normal3 PCREF n );

/// Normal3 Renormalise (to correct for accumulated float errors) (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Normal3* const __restrict pDst );

/// Normal3 Renormalise (to correct for accumulated float errors) (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise24( Normal3* const __restrict pDst );




/// Normal3 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z
NOALIAS __forceinline const VBool operator == ( const Normal3 PCREF a, const Normal3 PCREF b );

/// Normal3 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z
NOALIAS __forceinline const VBool operator != ( const Normal3 PCREF a, const Normal3 PCREF b );



/// Normal3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Normal3 Select( const VBool PCREF condition, const Normal3 PCREF result_if_true, const Normal3 PCREF result_if_false );

/// Normal3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Normal3 Select( const Vec4Bool PCREF condition, const Normal3 PCREF result_if_true, const Normal3 PCREF result_if_false );


/// Normal3: Calculate Perpendicular Axes
/// Output normals obey same handedness as matrices, with input as axis z
NOALIAS __forceinline void PerpendicularAxes( const Normal3 PCREF axisZ, Normal3* const __restrict pAxisX, Normal3* const __restrict pAxisY );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

