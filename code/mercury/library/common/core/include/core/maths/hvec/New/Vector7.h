#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR7_H
#define SOFT_MATHS_HVEC_NEW_VECTOR7_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR7_H
#define MATHS_HVEC_NEW_VECTOR7_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector7
/// @{

/// Load Vector7 from F32 ptr
template <> NOALIAS __forceinline const Vector7 Load( F32 const * const __restrict pSrc );

/// Store Vector7 to F32 ptr
NOALIAS __forceinline void Store( const Vector7& data, F32 * const __restrict pDst );


/// Load Vector7 from StorageVector7
NOALIAS __forceinline const Vector7 Load( StorageVector7 const * const __restrict pSrc );

/// Store Vector7 in StorageVector7
NOALIAS __forceinline void Store( const Vector7& data, StorageVector7 * const __restrict pDst );


/// Make Vector7 from broadcast VF32
NOALIAS __forceinline const Vector7 MakeVector7( const VF32 VECREF src );

/// Make Vector7 from 7 VF32's
NOALIAS __forceinline const Vector7 MakeVector7( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6 );

/// Vector7 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector7& v );

/// Vector7 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector7* const __restrict pDst, const VF32 VECREF ei );


/// Vector7 Abs of each element
NOALIAS __forceinline const Vector7 Abs( const Vector7& v );

/// Vector7 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector7 Sign( const Vector7& v );

/// Vector7 CopySign of each element : Returns a Vector7 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector7 CopySign( const Vector7& magnitude, const Vector7& sign );


/// Vector7 Negation
NOALIAS __forceinline const Vector7 operator - ( const Vector7& a );

/// Vector7 Sum
NOALIAS __forceinline const Vector7 operator + ( const Vector7& a, const Vector7& b );

/// Vector7 Difference
NOALIAS __forceinline const Vector7 operator - ( const Vector7& a, const Vector7& b );

/// Vector7 Addition Update
NOALIAS __forceinline void operator += ( Vector7& a, const Vector7& b );

/// Vector7 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector7& a, const Vector7& b );


/// Vector7 Scalar Right Multiplication
NOALIAS __forceinline const Vector7 operator * ( const Vector7& a, const VF32 VECREF b );

/// Vector7 Scalar Left Multiplication
NOALIAS __forceinline const Vector7 operator * ( const VF32 VECREF a, const Vector7& b );

/// Vector7 Scalar Right Division
NOALIAS __forceinline const Vector7 operator / ( const Vector7& a, const VF32 VECREF b );

/// Vector7 Scalar Left Division
NOALIAS __forceinline const Vector7 operator / ( const VF32 VECREF a, const Vector7& b );

/// Vector7 Scalar Right Multiplication Update
NOALIAS __forceinline void operator *= ( Vector7& a, const VF32 VECREF b );

/// Vector7 Scalar Right Division Update
NOALIAS __forceinline void operator /= ( Vector7& a, const VF32 VECREF b );


/// Vector7 Multiplication Operator
NOALIAS __forceinline const Vector7 operator * ( const Vector7& a, const Vector7& b );

/// Vector7 Division Operator
NOALIAS __forceinline const Vector7 operator / ( const Vector7& a, const Vector7& b );

/// Vector7 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector7& a, const Vector7& b );

/// Vector7 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector7& a, const Vector7& b );


/// Vector7 Comparison a == b : Returns VBool representing results of a[0] == b[0] && ... && a[6] == b[6]
NOALIAS __forceinline const VBool operator == ( const Vector7& a, const Vector7& b );

/// Vector7 Comparison a != b : Returns VBool representing results of a[0] != b[0] || ... || a[6] != b[6]
NOALIAS __forceinline const VBool operator != ( const Vector7& a, const Vector7& b );

/// Vector7 Comparison a <= b : Returns Vec8Bool representing results of a[i] <= b[i], for each element i [0,6]
NOALIAS __forceinline const Vec8Bool operator <= ( const Vector7& a, const Vector7& b );

/// Vector7 Comparison a >= b : Returns Vec8Bool representing results of a[i] >= b[i], for each element i [0,6]
NOALIAS __forceinline const Vec8Bool operator >= ( const Vector7& a, const Vector7& b );

/// Vector7 Comparison a < b : Returns Vec8Bool representing results of a[i] < b[i], for each element i [0,6]
NOALIAS __forceinline const Vec8Bool operator < ( const Vector7& a, const Vector7& b );

/// Vector7 Comparison a > b : Returns Vec8Bool representing results of a[i] > b[i], for each element i [0,6]
NOALIAS __forceinline const Vec8Bool operator > ( const Vector7& a, const Vector7& b );

/// Vector7 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector7& a, const Vector7& b );

/// Vector7 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector7& a, const Vector7& b );


/// Vector7 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vector7& a );

/// Vector7 Finite Test : true if a contains no elements that are NaN's and for each element i [0,6], abs(x[i]) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vector7& a );


/// Vector7 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector7 Select( const VBool PCREF condition, const Vector7& result_if_true, const Vector7& result_if_false );

/// Vector7 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector7 Select( const Vec8Bool PCREF condition, const Vector7& result_if_true, const Vector7& result_if_false );


/// Vector7 Bitwise AND: a & b : for each element i, returns ( b ? a[i] : zero )
NOALIAS __forceinline const Vector7 AND( const Vector7& a, const VBool PCREF b );

/// Vector7 Bitwise ANDC: a & ~b : for each element i, returns ( b ? zero : a[i] )
NOALIAS __forceinline const Vector7 ANDC( const Vector7& a, const VBool PCREF b );


/// Vector7 Bitwise AND: a & b : for each element i, returns ( b[i] ? a[i] : zero )
NOALIAS __forceinline const Vector7 AND( const Vector7& a, const Vec8Bool PCREF b );

/// Vector7 Bitwise ANDC: a & ~b : for each element i, returns ( b[i] ? zero : a[i] )
NOALIAS __forceinline const Vector7 ANDC( const Vector7& a, const Vec8Bool PCREF b );


/// Vector7 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector7 Rcp( const Vector7& v );

/// Vector7 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector7 Rcp24( const Vector7& v );

/// Vector7 Dot Product
NOALIAS __forceinline const VF32 Dot( const Vector7& a, const Vector7& b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

