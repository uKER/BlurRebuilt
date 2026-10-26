#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT66_H
#define SOFT_MATHS_HVEC_NEW_MAT66_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT66_H
#define MATHS_HVEC_NEW_MAT66_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat66
/// @{

/// Make Mat66 from 6 Vector6 rows
NOALIAS __forceinline const Mat66 MakeMat66( const Vector6& row0, const Vector6& row1, const Vector6& row2, const Vector6& row3,
                                             const Vector6& row4, const Vector6& row5 );

/// Mat66 constants
template <> struct Const< Mat66 > 
{
    NOALIAS __forceinline static const Mat66 Zero();     ///< Constant Mat66 with all elements 0
    NOALIAS __forceinline static const Mat66 Identity(); ///< Constant Mat66 with diagonal elements 1 & all other elements 0
};


/// Mat66 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat66& src );

/// Mat66 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat66* const __restrict pDst, const VF32 VECREF e );


/// Mat66 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat66& src );

/// Mat66 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat66* const __restrict pDst, const Vector6& row );


/// Mat66 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat66& src );

/// Mat66 Accessor: Set column
template < unsigned int C >
NOALIAS __forceinline void SetCol( Mat66* const __restrict pDst, const Vector6& col );


/// Mat66 Accessor: Assign column from another Mat66
template < unsigned int DstCol, unsigned int SrcCol >
#if defined( CFG_SPEED ) || defined( CFG_GOLD ) // JASOND: The 360 compiler takes forever if this is forceinlined in release builds...
NOALIAS __forceinline void ReplaceColumn( Mat66* const __restrict pDst, const Mat66& src );
#else
NOALIAS inline void ReplaceColumn( Mat66* const __restrict pDst, const Mat66& src );
#endif


/// Mat66 Transpose
NOALIAS __forceinline const Mat66 Transpose( const Mat66& src );


/// Mat66 Negation Operator
NOALIAS __forceinline const Mat66 operator - ( const Mat66& a );

/// Mat66 Addition Operator
NOALIAS __forceinline const Mat66 operator + ( const Mat66& a, const Mat66& b );

/// Mat66 Subtraction Operator
NOALIAS __forceinline const Mat66 operator - ( const Mat66& a, const Mat66& b );

/// Mat66 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat66& a, const Mat66& b );

/// Mat66 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat66& a, const Mat66& b );


/// Mat66 Scalar Multiplication Operator
NOALIAS __forceinline const Mat66 operator * ( const Mat66& m, const VF32 VECREF f );


/// Mat66 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat66& a, const Vector6& b );

/// Vector6 * Mat66 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector6& a, const Mat66& b );


/// Mat66 * Mat66 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat66& a, const Mat66& b );


/// Mat66 Matrix Multiply Operator
NOALIAS __forceinline const Mat66 operator * ( const Mat66& a, const Mat66& p );


/// Mat66 Split into four Mat33's
NOALIAS __forceinline void Split( const Mat66& m,
                                  Mat33* const __restrict pTL,
                                  Mat33* const __restrict pTR,
                                  Mat33* const __restrict pBL,
                                  Mat33* const __restrict pBR );

/// Make Mat66 from four Mat33's
NOALIAS __forceinline const Mat66 MakeMat66( const Mat33 MATREF tl, const Mat33 MATREF tr,
                                             const Mat33 MATREF bl, const Mat33 MATREF br );

/// Mat66 Split into top-left Mat55, top-right column Vector5, bottom-left row Vector5 & bottom-right VF32
NOALIAS __forceinline void Split( const Mat66& m,
                                  Mat55*   const __restrict pTL,
                                  Vector5* const __restrict pTR,
                                  Vector5* const __restrict pBL,
                                  VF32*    const __restrict pBR );

/// Make Mat66 from top-left Mat55, top-right column Vector5, bottom-left row Vector5 & bottom-right VF32
NOALIAS __forceinline const Mat55 MakeMat55( const Mat55&   tl, const Vector5&   tr,
                                             const Vector5& bl, const VF32 VECREF br );


/// Mat66 Split into top-left row Vector5, top-right VF32, bottom-left Mat55 & bottom-right column Vector5
NOALIAS __forceinline void Split( const Mat66& m55,
                                  Vector5* const __restrict pTL,
                                  VF32*    const __restrict pTR,
                                  Mat55*   const __restrict pBL,
                                  Vector5* const __restrict pBR );

/// Make Mat66 from top-left row Vector5, top-right VF32, bottom-left Mat55 & bottom-right column Vector5
NOALIAS __forceinline const Mat66 MakeMat66( const Vector5& tl, const VF32 VECREF tr,
                                             const Mat55&   bl, const Vector5&   br );


/// Make Mat66 from top-left column Vector5, top-right Mat55, bottom-left VF32 & bottom-right row Vector5
NOALIAS __forceinline const Mat66 MakeMat66( const Vector5&   tl, const Mat55&   tr,
                                             const VF32 VECREF bl, const Vector5& br );

/// Create a Mat55 sub-matrix from a Mat66 by deleting Row and Col
template < unsigned int Row, unsigned int Col >
NOALIAS __forceinline const Mat55 DeleteRowCol( const Mat66& src );
//{
//    #if !defined( PLATFORM_SPU )
//    BIZ_STATIC_ASSERT( false && "Invalid Row or Col!\n" );
//    #endif
//    Mat44 result;
//    return result;
//}

/// Determinant of Mat66
NOALIAS const VF32 Determinant( const Mat66& src );

/// Inverse of Mat66
NOALIAS const Mat66 Inverse( const Mat66& src );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


