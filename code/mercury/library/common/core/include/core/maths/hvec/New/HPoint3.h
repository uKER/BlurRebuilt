#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_HPOINT3_H
#define SOFT_MATHS_HVEC_NEW_HPOINT3_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_HPOINT3_H
#define MATHS_HVEC_NEW_HPOINT3_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup HPoint3
/// @{

/// Load HPoint3 from StorageHPoint3
NOALIAS __forceinline const HPoint3 Load( StorageHPoint3 const * const __restrict pSrc );

/// Store HPoint3 in StorageHPoint3
NOALIAS __forceinline void Store( const HPoint3 PCREF data, StorageHPoint3 * const __restrict pDst );


/// Make HPoint3 from Vec4F32
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Vec4F32 VECREF src );

/// Get Vec4F32 from HPoint3
NOALIAS __forceinline const Vec4F32 GetVec4F32( const HPoint3 PCREF src );


/// Make HPoint3 from Vector3
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Vector3 PCREF v3 );

/// Make HPoint3 from Vector3 xyz & VF32 w
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Vector3 PCREF xyz, const VF32 VECREF w );

/// Make HPoint3 from Point3
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Point3 PCREF p3 );

/// Make HPoint3 from Normal3
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Normal3 PCREF n3 );

/// Make HPoint3 from Vector4
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Vector4 PCREF v4 );


/// Make HPoint3 from 4 F32's
NOALIAS __forceinline const HPoint3 MakeHPoint3( const F32& x, const F32& y, const F32& z, const F32& w );

/// Make HPoint3 from 3 VF32's
NOALIAS __forceinline const HPoint3 MakeHPoint3( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z, const VF32 VECREF w );


/// HPoint3 Accessor: Get X
NOALIAS __forceinline const VF32 X( const HPoint3 PCREF src );

/// HPoint3 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const HPoint3 PCREF src );

/// HPoint3 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const HPoint3 PCREF src );

/// HPoint3 Accessor: Get W
NOALIAS __forceinline const VF32 W( const HPoint3 PCREF src );


/// HPoint3 Accessor: Set X
NOALIAS __forceinline void SetX( HPoint3* const __restrict pDst, const VF32 VECREF x );

/// HPoint3 Accessor: Set Y
NOALIAS __forceinline void SetY( HPoint3* const __restrict pDst, const VF32 VECREF y );

/// HPoint3 Accessor: Set Z
NOALIAS __forceinline void SetZ( HPoint3* const __restrict pDst, const VF32 VECREF z );

/// HPoint3 Accessor: Set W
NOALIAS __forceinline void SetW( HPoint3* const __restrict pDst, const VF32 VECREF w );


/// HPoint3 Accessor: Get Vector3 XYZ
NOALIAS __forceinline const Vector3 XYZ( const HPoint3 PCREF src );

/// HPoint3 Accessor: Get Vector4 XYZW
NOALIAS __forceinline const Vector4 XYZW( const HPoint3 PCREF src );


/// HPoint3 Write to Stream
template < typename T_Stream >
void Serialise( const HPoint3 PCREF src, T_Stream& stream );

/// HPoint3 Read from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, HPoint3* const __restrict pDst );


/// HPoint3 Sum Operator
NOALIAS __forceinline const HPoint3 operator + ( const HPoint3 PCREF p0, const HPoint3 PCREF p1 );

/// HPoint3 Difference Operator
NOALIAS __forceinline const Vector3 operator - ( const HPoint3 PCREF p0, const HPoint3 PCREF p1 );


/// HPoint3 + Vector3 Addition Operator
NOALIAS __forceinline const HPoint3 operator + ( const HPoint3 PCREF p, const Vector3 PCREF v );

/// HPoint3 - Vector3 Subtraction Operator
NOALIAS __forceinline const HPoint3 operator - ( const HPoint3 PCREF p, const Vector3 PCREF v );


/// HPoint3 * VF32 Multiplication Operator
NOALIAS __forceinline const HPoint3 operator * ( const HPoint3 PCREF a, const VF32 VECREF b );

/// VF32 * HPoint3 Multiplication Operator
NOALIAS __forceinline const HPoint3 operator * ( const VF32 VECREF a, const HPoint3 PCREF b );


/// HPoint3 / VF32 Division Operator
NOALIAS __forceinline const HPoint3 operator / ( const HPoint3 PCREF a, const VF32 VECREF b );


/// HPoint3 Normalise: returns Point3( x/w, y/w, z/w )
NOALIAS __forceinline const Point3 Normalise( const HPoint3 PCREF a );

/// HPoint3 Normalise: returns Point3( x/w, y/w, z/w )
NOALIAS __forceinline const Point3 Normalise24( const HPoint3 PCREF a );


/// HPoint3 Normalise: ( x, y, z, w ) -> ( x/w, y/w, z/w, 1 )
NOALIAS __forceinline void Normalise( HPoint3* const __restrict pA );

/// HPoint3 Normalise: ( x, y, z, w ) -> ( x/w, y/w, z/w, 1 )
NOALIAS __forceinline void Normalise24( HPoint3* const __restrict pA );





/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

