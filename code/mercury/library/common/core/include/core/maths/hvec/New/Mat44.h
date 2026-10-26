#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT44_H
#define SOFT_MATHS_HVEC_NEW_MAT44_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT44_H
#define MATHS_HVEC_NEW_MAT44_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat44
/// @{

/// Load Mat44 from an F32 array. NOTE: Matrix transposition is platform dependent!
template <>
NOALIAS __forceinline const Mat44 Load( F32 const * const __restrict pSrc );

/// Store Mat44 into an F32 array. NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat44 MATREF src, F32* const __restrict pDst );


/// Load Mat44 from a Float44. NOTE: Matrix transposition is platform dependent!
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::Float44 const * const __restrict pSrc );

/// Store Mat44 into a Float44. NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::Float44* const __restrict pDst );


/// Load Mat44 from a MatrixRow44
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::MatrixRow44 const * const __restrict pSrc );

/// Store Mat44 into a MatrixRow44
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::MatrixRow44* const __restrict pDst );


/// Load Mat44 from a MatrixCol44
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::MatrixCol44 const * const __restrict pSrc );

/// Store Mat44 into a MatrixCol44
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::MatrixCol44* const __restrict pDst );


/// Load Mat44 from a MatrixRow44A16
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::MatrixRow44A16 const * const __restrict pSrc );

/// Store Mat44 into a MatrixRow44A16
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::MatrixRow44A16* const __restrict pDst );


/// Load Mat44 from a MatrixCol44A16
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::MatrixCol44A16 const * const __restrict pSrc );

/// Store Mat44 into a MatrixCol44A16
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::MatrixCol44A16* const __restrict pDst );


/// Mat44 constants
template <> struct Const< Mat44 > 
{
    NOALIAS __forceinline static const Mat44& Zero();     ///< Mat44 constant: Zero
    NOALIAS __forceinline static const Mat44& Identity(); ///< Mat44 constant: Identity
    
private:
    
    static const Mat44 m_Zero;     ///< Mat44 constant: (private)
    static const Mat44 m_Identity; ///< Mat44 constant: (private)
};


/// Make Mat44 from 16 float elements (row major)
NOALIAS __forceinline const Mat44 MakeMat44( const F32& m00, const F32& m01, const F32& m02, const F32& m03,
                                             const F32& m10, const F32& m11, const F32& m12, const F32& m13,
                                             const F32& m20, const F32& m21, const F32& m22, const F32& m23,
                                             const F32& m30, const F32& m31, const F32& m32, const F32& m33 );


/// Make Mat44 from 4 row vectors (4 element)
NOALIAS __forceinline const Mat44 MakeMat44( const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ, const Vec4F32 VECREF rowW );

/// Make Mat44 from 4 row vectors (4 element)
NOALIAS __forceinline const Mat44 MakeMat44( const Vector4 PCREF rowX, const Vector4 PCREF rowY, const Vector4 PCREF rowZ, const Vector4 PCREF rowW );


/// Make Mat44 from a Mat33 - missing elements are zeroed
NOALIAS __forceinline const Mat44 MakeMat44( const Mat33 MATREF src );

/// Make Mat44 from a Mat43 - missing elements are zeroed
NOALIAS __forceinline const Mat44 MakeMat44( const Mat43 MATREF src );


/// Make Mat44 from a Reflection Plane
NOALIAS __forceinline const Mat44 MakeMat44( const Plane PCREF plane );


/// Mat44 Write to Stream
template < typename T_Stream > void Serialise( const Mat44 MATREF src, T_Stream& stream );

/// Mat44 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Mat44* const __restrict pDst );


/// Get rows from a Mat44
NOALIAS __forceinline void GetRows( const Mat44 MATREF src, Vec4F32* __restrict pRowX, Vec4F32* __restrict pRowY, Vec4F32* __restrict pRowZ, Vec4F32* __restrict pRowW );

/// Set rows of a Mat44
NOALIAS __forceinline void SetRows( Mat44* pDst, const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ, const Vec4F32 VECREF rowW );


/// Get Row X from a Mat44
NOALIAS __forceinline const Vector4 RowX( const Mat44 MATREF src );

/// Get Row Y from a Mat44
NOALIAS __forceinline const Vector4 RowY( const Mat44 MATREF src );

/// Get Row Z from a Mat44
NOALIAS __forceinline const Vector4 RowZ( const Mat44 MATREF src );

/// Get Row W from a Mat44
NOALIAS __forceinline const Vector4 RowW( const Mat44 MATREF src );


/// Get Col X from a Mat44
NOALIAS __forceinline const Vector4 ColX( const Mat44 MATREF src );

/// Get Col Y from a Mat44
NOALIAS __forceinline const Vector4 ColY( const Mat44 MATREF src );

/// Get Col Z from a Mat44
NOALIAS __forceinline const Vector4 ColZ( const Mat44 MATREF src );

/// Get Col W from a Mat44
NOALIAS __forceinline const Vector4 ColW( const Mat44 MATREF src );


/// Set Row X of a Mat44
NOALIAS __forceinline void SetRowX( Mat44* pDst, const Vec4F32 VECREF src );

/// Set Row Y of a Mat44
NOALIAS __forceinline void SetRowY( Mat44* pDst, const Vec4F32 VECREF src );

/// Set Row Z of a Mat44
NOALIAS __forceinline void SetRowZ( Mat44* pDst, const Vec4F32 VECREF src );

/// Set Row W of a Mat44
NOALIAS __forceinline void SetRowW( Mat44* pDst, const Vec4F32 VECREF src );


/// Set Row X of a Mat44
NOALIAS __forceinline void SetRowX( Mat44* pDst, const Vector4 PCREF src );

/// Set Row Y of a Mat44
NOALIAS __forceinline void SetRowY( Mat44* pDst, const Vector4 PCREF src );

/// Set Row Z of a Mat44
NOALIAS __forceinline void SetRowZ( Mat44* pDst, const Vector4 PCREF src );

/// Set Row W of a Mat44
NOALIAS __forceinline void SetRowW( Mat44* pDst, const Vector4 PCREF src );


/// Mat44 Get element
template < unsigned int Row, unsigned int Col >
NOALIAS __forceinline const VF32 Element( const Mat44 MATREF src );

/// Mat44 Set element
template < unsigned int Row, unsigned int Col >
NOALIAS __forceinline void SetElement( Mat44* const __restrict pDst, const VF32 VECREF e );


/// Mat44 Assign column from another Mat44
template < unsigned int DstCol, unsigned int SrcCol >
NOALIAS __forceinline void ReplaceColumn( Mat44* const __restrict pDst, const Mat44 MATREF src );


/// Mat44 Negation operator
NOALIAS __forceinline const Mat44 operator - ( const Mat44 MATREF src );

/// Mat44 Addition operator
NOALIAS __forceinline const Mat44 operator + ( const Mat44 MATREF lhs, const Mat44 MATREF rhs );

/// Mat44 Subtraction operator
NOALIAS __forceinline const Mat44 operator - ( const Mat44 MATREF lhs, const Mat44 MATREF rhs );


/// Mat44 * VF32 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat44 MATREF matrix, const VF32 VECREF scalar );

/// VF32 * Mat44 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const VF32 VECREF scalar, const Mat44 MATREF matrix );


/// Vec4F32 * Mat44 Multiplication operator : input = 4 element row vector, output = 4 element row vector
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF vector4, const Mat44 MATREF matrix );

/// Mat44 * Vec4F32 Multiplication operator : input = 4 element col vector, output = 4 element col vector
NOALIAS __forceinline const Vec4F32 operator * ( const Mat44 MATREF matrix, const Vec4F32 VECREF vector4 );


/// Vector3 Matrix Multiplication: Vector3 * Mat44
NOALIAS __forceinline const Vector4 operator * ( const Vector3 PCREF v, const Mat44 MATREF m );

/// Vector3 Matrix Multiplication: Mat44 * Vector3             
NOALIAS __forceinline const Vector4 operator * ( const Mat44 MATREF m, const Vector3 PCREF v );


/// Vector4 * Mat44 Multiplication operator : input = 4 element row vector, output = 4 element row vector
//NOALIAS __forceinline const Vec4F32 operator * ( const Vector4 PCREF vector4, const Mat44 MATREF matrix );

/// Mat44 * Vector4 Multiplication operator : input = 4 element col vector, output = 4 element col vector
//NOALIAS __forceinline const Vec4F32 operator * ( const Mat44 MATREF matrix, const Vector4 PCREF vector4 );


/// Point3 Matrix Multiplication: Point3 * Mat44
//NOALIAS __forceinline const Vector4 operator * ( const Point3 PCREF p, const Mat44 MATREF m );
NOALIAS __forceinline const HPoint3 operator * ( const Point3 PCREF p, const Mat44 MATREF m );


/// Mat44 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat44 MATREF lhs, const Mat44 MATREF rhs );


/// Mat44 * Mat33 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat44 MATREF lhs, const Mat33 MATREF rhs );

/// Mat33 * Mat44 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat33 MATREF lhs, const Mat44 MATREF rhs );


/// Mat44 * Mat43 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat44 MATREF lhs, const Mat43 MATREF rhs );

/// Mat43 * Mat44 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat43 MATREF lhs, const Mat44 MATREF rhs );



/// Transpose Mat44 : returns transposed result
NOALIAS __forceinline const Mat44 Transpose( const Mat44 MATREF src );

/// Transpose Mat44 : transposes pDst
NOALIAS __forceinline void Transpose( Mat44 * const __restrict pDst );


/// Trace of Mat44 : sum of diagonal elements
NOALIAS __forceinline const VF32 Trace( const Mat44 MATREF src );


/// Create a Mat33 sub-matrix from a Mat44 by deleting Row and Col
template < unsigned int Row, unsigned int Col >
NOALIAS __forceinline const Mat33 DeleteRowCol( const Mat44 MATREF src )
{
    #if !defined( PLATFORM_SPU )
    BIZ_STATIC_ASSERT( false && "Invalid Row or Col!\n" );
    #endif
    Mat33 result;
    return result;
}

/// Determinant of Mat44
NOALIAS const VF32 Determinant( const Mat44 MATREF src );

/// Inverse of Mat44 (faster but only 22 bit precision)
NOALIAS const Mat44 Inverse( const Mat44 MATREF src );

/// Inverse of Mat44 (slower but full 24 bit precision)
NOALIAS const Mat44 Inverse24( const Mat44 MATREF src );


/// Mat44 split into top-left Mat33, top-right column Vector3, bottom-left row Vector3 & bottom-right VF32
NOALIAS __forceinline void Split( const Mat44 MATREF src,
                                  Mat33   * const __restrict pTL,
                                  Vector3 * const __restrict pTR,
                                  Vector3 * const __restrict pBL,
                                  VF32    * const __restrict pBR );

/// Make Mat44 from top-left Mat33, top-right column Vector3, bottom-left row Vector3 & bottom-right VF32
NOALIAS __forceinline const Mat44 MakeMat44(
    const Mat33 MATREF        tl,
    const Vector3 PCREF tr,
    const Vector3 PCREF bl,
    const VF32    PCREF br );


/// Mat44 split into top-left VF32, top-right row Vector3, bottom-left column Vector3 & bottom-right Mat33
NOALIAS __forceinline void Split( const Mat44 MATREF src,
                                  VF32    * const __restrict pTL,
                                  Vector3 * const __restrict pTR,
                                  Vector3 * const __restrict pBL,
                                  Mat33   * const __restrict pBR );

/// Make Mat44 from top-left VF32, top-right row Vector3, bottom-left column Vector3 & bottom-right Mat33
NOALIAS __forceinline const Mat44 MakeMat44(
    const VF32    PCREF tl,
    const Vector3 PCREF tr,
    const Vector3 PCREF bl,
    const Mat33 MATREF        br );


/// Mat44 split into top-left row Vector3, top-right VF32, bottom-left Mat33 & bottom-right column Vector3
NOALIAS __forceinline void Split( const Mat44 MATREF src,
                                  Vector3 * const __restrict pTL,
                                  VF32    * const __restrict pTR,
                                  Mat33   * const __restrict pBL,
                                  Vector3 * const __restrict pBR );

/// Make Mat44 from top-left row Vector3, top-right VF32, bottom-left Mat33 & bottom-right column Vector3
NOALIAS __forceinline const Mat44 MakeMat44(
    const Vector3 PCREF tl,
    const VF32    PCREF tr,
    const Mat33 MATREF        bl,
    const Vector3 PCREF br );


/// Mat44 split into top-left column Vector3, top-right Mat33, bottom-left VF32 & bottom-right row Vector3
NOALIAS __forceinline void Split( const Mat44 MATREF src,
                                  Vector3 * const __restrict pTL,
                                  Mat33   * const __restrict pTR,
                                  VF32    * const __restrict pBL,
                                  Vector3 * const __restrict pBR );

/// Make Mat44 from top-left column Vector3, top-right Mat33, bottom-left VF32 & bottom-right row Vector3
NOALIAS __forceinline const Mat44 MakeMat44(
    const Vector3 PCREF tl,
    const Mat33 MATREF        tr,
    const VF32    PCREF bl,
    const Vector3 PCREF br );




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


