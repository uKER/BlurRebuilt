#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT55_H
#define SOFT_MATHS_HVEC_NEW_MAT55_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT55_H
#define MATHS_HVEC_NEW_MAT55_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat55
/// @{

/// Make Mat55 from 5 Vector5 rows
NOALIAS __forceinline const Mat55 MakeMat55( const Vector5& row0, const Vector5& row1, const Vector5& row2, const Vector5& row3,
                                             const Vector5& row4 );

/// Mat55 constants
template <> struct Const< Mat55 > 
{
    NOALIAS __forceinline static const Mat55 Zero(); ///< Constant Mat55 with all elements zeroed
};


/// Mat55 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat55& src );

/// Mat55 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat55* const __restrict pDst, const VF32 VECREF e );


/// Mat55 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector5 Row( const Mat55& src );

/// Mat55 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat55* const __restrict pDst, const Vector5& row );


/// Mat55 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector5 Col( const Mat55& src );


/// Mat55 Negation Operator
NOALIAS __forceinline const Mat55 operator - ( const Mat55& a );

/// Mat55 Addition Operator
NOALIAS __forceinline const Mat55 operator + ( const Mat55& a, const Mat55& b );

/// Mat55 Subtraction Operator
NOALIAS __forceinline const Mat55 operator - ( const Mat55& a, const Mat55& b );

/// Mat55 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat55& a, const Mat55& b );

/// Mat55 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat55& a, const Mat55& b );


/// Mat55 * Vector5 Multiplication Operator
NOALIAS __forceinline const Vector5 operator * ( const Mat55& m, const Vector5& v );

/// Vector5 * Mat55 Multiplication Operator
NOALIAS __forceinline const Vector5 operator * ( const Vector5& v, const Mat55& m );


/// Mat55 Scalar Multiplication
NOALIAS __forceinline const Mat55 operator * ( const Mat55& m, const VF32 VECREF f );


/// Mat55 * Mat55 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector5 DiagonalMultiply( const Mat55& a, const Mat55& b );


/// Mat55 Matrix Multiply
NOALIAS __forceinline const Mat55 operator * ( const Mat55& a, const Mat55& p );


/// Create a Mat44 sub-matrix from a Mat55 by deleting Row and Col
template < unsigned int Row, unsigned int Col >
NOALIAS __forceinline const Mat44 DeleteRowCol( const Mat55& src );
//{
//    #if !defined( PLATFORM_SPU )
//    BIZ_STATIC_ASSERT( false && "Invalid Row or Col!\n" );
//    #endif
//    Mat44 result;
//    return result;
//}


/// Make Mat55 from top-left Mat44, top-right column Vector4, bottom-left row Vector4 & bottom-right scalar
NOALIAS __forceinline const Mat55 MakeMat55( const Mat44 MATREF        tl, const Vector4 PCREF tr,
                                             const Vector4 PCREF bl, const VF32 VECREF    br );

/// Make Mat55 from top-left column Vector4, top-right Mat44, bottom-left scalar & bottom-right row Vector4
NOALIAS __forceinline const Mat55 MakeMat55( const Vector4 PCREF tl, const Mat44 MATREF        tr,
                                             const VF32 VECREF    bl, const Vector4 PCREF br );


/// Mat55 Split into top-left Mat44, top-right column Vector4, bottom-left row Vector4 & bottom-right scalar
NOALIAS __forceinline void Split( const Mat55& m,
                                  Mat44*   const __restrict pTL,
                                  Vector4* const __restrict pTR,
                                  Vector4* const __restrict pBL,
                                  VF32*    const __restrict pBR );

/// Mat55 Split into top-left row Vector4, top-right scalar, bottom-left Mat44 & bottom-right column Vector4
NOALIAS __forceinline void Split( const Mat55& m,
                                  Vector4* const __restrict pTL,
                                  VF32*    const __restrict pTR,
                                  Mat44*   const __restrict pBL,
                                  Vector4* const __restrict pBR );





/// Determinant of Mat55
NOALIAS const VF32 Determinant( const Mat55& src );

/// Inverse of Mat55
NOALIAS const Mat55 Inverse( const Mat55& src );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


