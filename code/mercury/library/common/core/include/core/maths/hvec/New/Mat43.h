#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT43_H
#define SOFT_MATHS_HVEC_NEW_MAT43_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT43_H
#define MATHS_HVEC_NEW_MAT43_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat43
/// @{

/// Load Mat43 from an F32 array. NOTE: Matrix transposition is platform dependent!
template <> NOALIAS __forceinline const Mat43 Load( F32 const * const __restrict pSrc );

/// Store Mat43 into an F32 array. NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat43 MATREF src, F32* const __restrict pDst );


/// Load Mat43 from a Float43. NOTE: Matrix transposition is platform dependent!
template <> NOALIAS __forceinline const Mat43 Load( MathsStorage::Float43 const * const __restrict pSrc );

/// Store Mat43 into a Float43. NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat43 MATREF src, MathsStorage::Float43* const __restrict pDst );


/// Mat43 constants
template <> struct Const< Mat43 > 
{
    NOALIAS __forceinline static const Mat43& Zero();     ///< Mat43 constant: Zero
    NOALIAS __forceinline static const Mat43& Identity(); ///< Mat43 constant: Identity
    
private:
    
    static const Mat43 m_Zero;     ///< Mat43 constant: (private)
    static const Mat43 m_Identity; ///< Mat43 constant: (private)
};


/// Make Mat43 from 12 float elements (row major)
NOALIAS __forceinline const Mat43 MakeMat43( const F32& m00, const F32& m01, const F32& m02,
                                             const F32& m10, const F32& m11, const F32& m12,
                                             const F32& m20, const F32& m21, const F32& m22,
                                             const F32& m30, const F32& m31, const F32& m32 );


/// Make Mat43 from 4 row vectors (3 element)
NOALIAS __forceinline const Mat43 MakeMat43( const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ, const Vec4F32 VECREF rowW );

/// Make Mat43 from 4 row vectors (3 element)
NOALIAS __forceinline const Mat43 MakeMat43( const Vector3 PCREF rowX, const Vector3 PCREF rowY, const Vector3 PCREF rowZ, const Vector3 PCREF rowW );

/// Make Mat43 from 4 row normals (3 element)
NOALIAS __forceinline const Mat43 MakeMat43( const Normal3 PCREF rowX, const Normal3 PCREF rowY, const Normal3 PCREF rowZ, const Normal3 PCREF rowW );


/// Make Mat43 from a Mat33 - missing elements are zeroed
NOALIAS __forceinline const Mat43 MakeMat43( const Mat33 MATREF src );

/// Make Mat43 from a Mat44
NOALIAS __forceinline const Mat43 MakeMat43( const Mat44 MATREF src );

/// Make Mat43 from a Mat33 rotation matrix & a Vec4F32 translation
NOALIAS __forceinline const Mat43 MakeMat43( const Mat33 MATREF r, const Vec4F32 VECREF t );

/// Make Mat43 from a Mat33 rotation matrix & a Vector3 translation
NOALIAS __forceinline const Mat43 MakeMat43( const Mat33 MATREF r, const Vector3 PCREF t );

/// Make Mat43 from a Mat33 rotation matrix & a Point3 position
NOALIAS __forceinline const Mat43 MakeMat43( const Mat33 MATREF r, const Point3 PCREF p );

/// Make Mat43 camera 'LookAt' matrix, NOTE: this returns an inverse camera matrix
NOALIAS __forceinline const Mat43 MakeMat43_CameraLookAt( const Point3 PCREF cameraPosition, const Point3 PCREF targetPosition );


/// Make Mat43 from a Reflection Plane
NOALIAS __forceinline const Mat43 MakeMat43( const Plane PCREF plane );


/// Mat43 Write to Stream
template < typename T_Stream > void Serialise( const Mat43 MATREF src, T_Stream& stream );

/// Mat43 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Mat43* const __restrict pDst );


/// Get rows from a Mat43
NOALIAS __forceinline void GetRows( const Mat43 MATREF src, Vec4F32* __restrict pRowX, Vec4F32* __restrict pRowY, Vec4F32* __restrict pRowZ, Vec4F32* __restrict pRowW );

/// Set rows of a Mat43
NOALIAS __forceinline void SetRows( Mat43* pDst, const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ, const Vec4F32 VECREF rowW );


/// Get rows from a Mat43
NOALIAS __forceinline void GetRows( const Mat43 MATREF src, Vector4* __restrict pRowX, Vector4* __restrict pRowY, Vector4* __restrict pRowZ, Vector4* __restrict pRowW );

/// Set rows of a Mat43
NOALIAS __forceinline void SetRows( Mat43* pDst, const Vector4 PCREF rowX, const Vector4 PCREF rowY, const Vector4 PCREF rowZ, const Vector4 PCREF rowW );


/// Get Row X from a Mat43
NOALIAS __forceinline const Vector3 RowX( const Mat43 MATREF src );

/// Get Row Y from a Mat43
NOALIAS __forceinline const Vector3 RowY( const Mat43 MATREF src );

/// Get Row Z from a Mat43
NOALIAS __forceinline const Vector3 RowZ( const Mat43 MATREF src );

/// Get Row W from a Mat43
NOALIAS __forceinline const Vector3 RowW( const Mat43 MATREF src );


/// Get Col X from a Mat43
NOALIAS __forceinline const Vector4 ColX( const Mat43 MATREF src );

/// Get Col Y from a Mat43
NOALIAS __forceinline const Vector4 ColY( const Mat43 MATREF src );

/// Get Col Z from a Mat43
NOALIAS __forceinline const Vector4 ColZ( const Mat43 MATREF src );


/// Get element from a Mat43
template < unsigned int Row, unsigned int Col > NOALIAS __forceinline const VF32 Element( const Mat43 MATREF src );


/// Set Row X of a Mat43
NOALIAS __forceinline void SetRowX( Mat43* pDst, const Vec4F32 VECREF src );

/// Set Row Y of a Mat43
NOALIAS __forceinline void SetRowY( Mat43* pDst, const Vec4F32 VECREF src );

/// Set Row Z of a Mat43
NOALIAS __forceinline void SetRowZ( Mat43* pDst, const Vec4F32 VECREF src );

/// Set Row W of a Mat43
NOALIAS __forceinline void SetRowW( Mat43* pDst, const Vec4F32 VECREF src );


/// Set Row X of a Mat43
NOALIAS __forceinline void SetRowX( Mat43* pDst, const Vector3 PCREF src );

/// Set Row Y of a Mat43
NOALIAS __forceinline void SetRowY( Mat43* pDst, const Vector3 PCREF src );

/// Set Row Z of a Mat43
NOALIAS __forceinline void SetRowZ( Mat43* pDst, const Vector3 PCREF src );

/// Set Row W of a Mat43
NOALIAS __forceinline void SetRowW( Mat43* pDst, const Vector3 PCREF src );


/// Mat43 Get rotation Mat33
NOALIAS __forceinline const Mat33 Rotation( const Mat43 MATREF matrix );

/// Mat43 Get translation Vector3
NOALIAS __forceinline const Vector3 Translation( const Mat43 MATREF matrix );


/// Mat43 Addition operator
NOALIAS __forceinline const Mat43 operator + ( const Mat43 MATREF lhs, const Mat43 MATREF rhs );

/// Mat43 Subtraction operator
NOALIAS __forceinline const Mat43 operator - ( const Mat43 MATREF lhs, const Mat43 MATREF rhs );


/// Mat43 * VF32 Multiplication operator
NOALIAS __forceinline const Mat43 operator * ( const Mat43 MATREF matrix, const VF32 VECREF scalar );

/// VF32 * Mat43 Multiplication operator
NOALIAS __forceinline const Mat43 operator * ( const VF32 VECREF scalar, const Mat43 MATREF matrix );


/// Vec4F32 * Mat43 Multiplication operator : input = 4 element row vector, output = 4 element row vector
/// NOTE: Mat43 Column W is implicit (0,0,0,1)!
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF vector4, const Mat43 MATREF matrix );

/// Mat43 * Vec4F32 Multiplication operator : input = 4 element col vector, output = 4 element col vector
NOALIAS __forceinline const Vec4F32 operator * ( const Mat43 MATREF matrix, const Vec4F32 VECREF vector4 );


/// Vector3 Matrix Multiplication: Vector3 * Mat43
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const Mat43 MATREF m );

/// Vector3 Matrix Multiplication: Mat43 * Vector3             
NOALIAS __forceinline const Vector4 operator * ( const Mat43 MATREF m, const Vector3 PCREF v );
                                                               
/// Vector3 Matrix Multiplication Update: Vector3 *= Mat43
NOALIAS __forceinline void operator *= ( Vector3& v, const Mat43 MATREF m );


/// Point3 Matrix Multiplication: Point3 * Mat43
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const Mat43 MATREF m );

/// Point3 Matrix Multiplication Update: Point3 *= Mat43
NOALIAS __forceinline void operator *= ( Point3& p, const Mat43 MATREF m );


/// Mat43 Multiplication operator : NOTE Mat43 has implicit column W (0,0,0,1)
NOALIAS __forceinline const Mat43 operator * ( const Mat43 MATREF lhs, const Mat43 MATREF rhs );


/// Mat43 * Mat33 Multiplication operator
NOALIAS __forceinline const Mat43 operator * ( const Mat43 MATREF lhs, const Mat33 MATREF rhs );

/// Mat33 * Mat43 Multiplication operator
NOALIAS __forceinline const Mat43 operator * ( const Mat33 MATREF lhs, const Mat43 MATREF rhs );


/// Normalise the rotation axes of a Mat43 : returns normalised result (NOTE: Does NOT orthogonalise the axes!)
NOALIAS __forceinline const Mat43 Normalise( const Mat43 MATREF src );

/// Normalise the rotation axes of a Mat43 (NOTE: Does NOT orthogonalise the axes!)
NOALIAS __forceinline void Normalise( Mat43* pDst );

/// Ortho-Normalise the rotation axes of a Mat43 : returns normalised result (priority given to z axis then y axis with x axis fully recomputed) 
NOALIAS __forceinline const Mat43 OrthoNormalise( const Mat43 MATREF src );

/// Ortho-Normalise the rotation axes of a Mat43 (priority given to z axis then y axis with x axis fully recomputed) 
NOALIAS __forceinline void OrthoNormalise( Mat43* pDst );


/// Determinant of Mat43
NOALIAS __forceinline const VF32 Determinant( const Mat43 MATREF src );

/// Inverse of Mat43
NOALIAS const Mat43 Inverse( const Mat43 MATREF src );







/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


