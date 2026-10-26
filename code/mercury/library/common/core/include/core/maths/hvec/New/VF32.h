#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VF32_H
#define SOFT_MATHS_HVEC_NEW_VF32_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VF32_H
#define MATHS_HVEC_NEW_VF32_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup VF32
/// @{

/// Load VF32 from an F32
template <> NOALIAS __forceinline const VF32 Load( F32 const * const __restrict pSrc );

/// Store VF32 into an F32
NOALIAS __forceinline void Store( const VF32 VECREF src, F32* const __restrict pDst );

/// StorageVF32: Load VF32
NOALIAS __forceinline const VF32 Load( StorageVF32 const * const __restrict pSrc );

/// StorageVF32: Store VF32
NOALIAS __forceinline void Store( const VF32 VECREF src, StorageVF32 * const __restrict pDst );


/// Make VF32 from F32
NOALIAS __forceinline const VF32 MakeVF32( const F32& x );

/// Make VF32 from F64
NOALIAS __forceinline const VF32 MakeVF32( const F64& x );

/// Make VF32 from S32
NOALIAS __forceinline const VF32 MakeVF32( const S32& x );

/// Make VF32 from U32
NOALIAS __forceinline const VF32 MakeVF32( const U32& x );

/// Make VF32 from F32 pointer
NOALIAS __forceinline const VF32 MakeVF32( F32 const * const __restrict ptr );


/// Convert VF32 to F32
NOALIAS __forceinline const F32 GetF32( const VF32 VECREF x );

/// Convert VF32 to int S32
NOALIAS __forceinline const S32 GetS32( const VF32 VECREF x );


/// VF32 Write to Stream
template < typename T_Stream > void Serialise( const VF32 VECREF src, T_Stream& stream );

/// VF32 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, VF32* const __restrict pDst );


/// VF32 Abs
NOALIAS __forceinline const VF32 Abs( const VF32 VECREF x );

/// VF32 Sign : Returns -1.0f if scalar < 0.0f, +1.0f if scalar > 0.0f, or 0.0f if scalar == 0.0f 
NOALIAS __forceinline const VF32 Sign( const VF32 VECREF x );

/// VF32 CopySign : Returns a VF32 with it's absolute value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const VF32 CopySign( const VF32 VECREF magnitude, const VF32 VECREF sign );

/// VF32 XorSign : Returns a VF32 with it's absolute value taken from 'magnitude' and it's sign bit from the XOR of the sign bits of 'sign' and 'magnitude'
NOALIAS __forceinline const VF32 XorSign( const VF32 VECREF magnitude, const VF32 VECREF sign );


/// VF32 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Rcp( const VF32 VECREF x );

/// VF32 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Rcp24( const VF32 VECREF x );

/// VF32 Inverse (Reciprocal) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Inverse( const VF32 VECREF x );

/// VF32 Inverse (Reciprocal) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Inverse24( const VF32 VECREF x );

/// VF32 Division (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Div( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Division (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Div24( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 RcpSqrt( const VF32 VECREF x );

/// VF32 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 RcpSqrt24( const VF32 VECREF x );

/// VF32 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Sqrt( const VF32 VECREF x );

/// VF32 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Sqrt24( const VF32 VECREF x );

/// VF32 Square Root Estimate (very fast but only 11 bit precision)
NOALIAS __forceinline const VF32 SqrtEst( const VF32 VECREF x );

/// VF32 Reciprocals And Square Root : Returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const VF32 VECREF x,
                                       VF32* const __restrict pRcp,
                                       VF32* const __restrict pSqrt,
                                       VF32* const __restrict pRcpSqrt );

/// VF32 Reciprocals And Square Root : Returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const VF32 VECREF x,
                                         VF32* const __restrict pRcp,
                                         VF32* const __restrict pSqrt,
                                         VF32* const __restrict pRcpSqrt );

/// VF32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const VF32 VECREF x,
                                           VF32* const __restrict pSqrt,
                                           VF32* const __restrict pRcpSqrt );

/// VF32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const VF32 VECREF x,
                                             VF32* const __restrict pSqrt,
                                             VF32* const __restrict pRcpSqrt );


/// VF32 Quadratic Equation Solver
NOALIAS __forceinline const VBool SolveQuadratic( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, VF32* const __restrict pRoot1, VF32* const __restrict pRoot2 );

/// VF32 Quadratic Equation Solver
NOALIAS __forceinline const QuadraticResult SolveQuadratic( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c );


/// VF32 Unary Positive Operator
NOALIAS __forceinline const VF32 operator + ( const VF32 VECREF x );

/// VF32 Unary Negation Operator
NOALIAS __forceinline const VF32 operator - ( const VF32 VECREF x );

/// VF32 Binary Addition Operator
NOALIAS __forceinline const VF32 operator + ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Binary Subtraction Operator
NOALIAS __forceinline const VF32 operator - ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Binary Multiplication Operator
NOALIAS __forceinline const VF32 operator * ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Binary Division Operator
NOALIAS __forceinline const VF32 operator / ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Add Assign Operator
NOALIAS __forceinline const VF32& operator += ( VF32& x, const VF32 VECREF y );

/// VF32 Subtract Assign Operator
NOALIAS __forceinline const VF32& operator -= ( VF32& x, const VF32 VECREF y );

/// VF32 Multiply Assign Operator
NOALIAS __forceinline const VF32& operator *= ( VF32& x, const VF32 VECREF y );

/// VF32 Divide Assign Operator
NOALIAS __forceinline const VF32& operator /= ( VF32& x, const VF32 VECREF y );


/// VF32 Square
NOALIAS __forceinline const VF32 Square( const VF32 VECREF a );


/// VF32 MulAdd : a * b + c
NOALIAS __forceinline const VF32 MulAdd( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c );

/// VF32 MulSub : a * b - c
NOALIAS __forceinline const VF32 MulSub( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c );

/// VF32 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const VF32 NegMulAdd( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c );

/// VF32 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const VF32 NegMulSub( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c );



/// VF32 Min
NOALIAS __forceinline const VF32 Min( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Max
NOALIAS __forceinline const VF32 Max( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Min Assign
DEPRECATED NOALIAS __forceinline const VF32& MinEq( VF32* const __restrict pX, const VF32 VECREF y );

/// VF32 Max Assign
DEPRECATED NOALIAS __forceinline const VF32& MaxEq( VF32* const __restrict pX, const VF32 VECREF y );

/// VF32 Min
NOALIAS __forceinline void Min( VF32* const __restrict pX, const VF32 VECREF y );

/// VF32 Max
NOALIAS __forceinline void Max( VF32* const __restrict pX, const VF32 VECREF y );


/// VF32 Clamp
NOALIAS __forceinline const VF32 Clamp( const VF32 VECREF x, const VF32 VECREF lo, const VF32 VECREF hi );

/// VF32 Clamp Assign
DEPRECATED NOALIAS __forceinline const VF32& ClampEq( VF32* const __restrict pX, const VF32 VECREF lo, const VF32 VECREF hi );

/// VF32 Clamp
NOALIAS __forceinline void Clamp( VF32* const __restrict pX, const VF32 VECREF lo, const VF32 VECREF hi );


/// VF32 Round to integer towards minus infinity
NOALIAS __forceinline const VF32 Floor( const VF32 VECREF x ) __attribute__(( /* pure, ? */ always_inline ));

/// VF32 Round to integer towards plus infinity
NOALIAS __forceinline const VF32 Ceil ( const VF32 VECREF x ) __attribute__(( /* pure, ? */ always_inline ));

/// VF32 Round to integer towards nearest
NOALIAS __forceinline const VF32 Round( const VF32 VECREF x ) __attribute__(( /* pure, ? */ always_inline ));

/// VF32 Round to integer towards zero
NOALIAS __forceinline const VF32 Trunc( const VF32 VECREF x ) __attribute__(( /* pure, ? */ always_inline ));


/// VF32 Flush to Zero: flushes value to zero if less than or equal to tolerance
NOALIAS __forceinline const VF32 FlushToZero( const VF32 VECREF value, const VF32 VECREF tolerance );


/// VF32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const VF32 Mod( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const VF32 Mod( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF rcp_y );

/// VF32 Fractional Part : Returns fractional part of x, f = x - i [ x = i + f ], where i is an integer, i & f have the same sign as x
NOALIAS __forceinline const VF32 Frac( const VF32 VECREF x );

/// VF32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const VF32 WrapToInterval( const VF32 VECREF x, const VF32 VECREF p );

/// VF32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const VF32 WrapToInterval( const VF32 VECREF x, const VF32 VECREF p, const VF32 VECREF rcp_p );

/// VF32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const VF32 WrapToIntervalOffset( const VF32 VECREF x, const VF32 VECREF p, const VF32 VECREF o );

/// VF32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const VF32 WrapToIntervalOffset( const VF32 VECREF x, const VF32 VECREF p, const VF32 VECREF rcp_p, const VF32 VECREF o );


/// VF32 Mean
NOALIAS __forceinline const VF32 Mean( const VF32 VECREF a, const VF32 VECREF b );

/// VF32 Mean
NOALIAS __forceinline const VF32 Mean( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c );

/// VF32 Mean
NOALIAS __forceinline const VF32 Mean( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, const VF32 VECREF d );

/// VF32 Mean
NOALIAS __forceinline const VF32 Mean( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, const VF32 VECREF d,
                                       const VF32 VECREF e, const VF32 VECREF f, const VF32 VECREF g, const VF32 VECREF h );


/// VF32 RMS: Root Mean Square
NOALIAS __forceinline const VF32 RMS( const VF32 VECREF a, const VF32 VECREF b );

/// VF32 RMS: Root Mean Square
NOALIAS __forceinline const VF32 RMS( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c );

/// VF32 RMS: Root Mean Square
NOALIAS __forceinline const VF32 RMS( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, const VF32 VECREF d );

/// VF32 RMS: Root Mean Square
NOALIAS __forceinline const VF32 RMS( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, const VF32 VECREF d,
                                      const VF32 VECREF e, const VF32 VECREF f, const VF32 VECREF g, const VF32 VECREF h );


/// VF32 Comparison x == y : Returns VBool representing results of x == y
NOALIAS __forceinline const VBool operator == ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Comparison x != y : Returns VBool representing results of x != y
NOALIAS __forceinline const VBool operator != ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Comparison x <= y : Returns VBool representing results of x <= y
NOALIAS __forceinline const VBool operator <= ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Comparison x >= y : Returns VBool representing results of x >= y
NOALIAS __forceinline const VBool operator >= ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Comparison x < y : Returns VBool representing results of x < y
NOALIAS __forceinline const VBool operator < ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Comparison x > y : Returns VBool representing results of x > y
NOALIAS __forceinline const VBool operator > ( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Ordered Test ( true if neither x nor y is a NaN )
NOALIAS __forceinline const VBool Ordered( const VF32 VECREF x, const VF32 VECREF y );

/// VF32 Unordered Test ( true if x or y is a NaN )
NOALIAS __forceinline const VBool Unordered( const VF32 VECREF x, const VF32 VECREF y );


/// VF32 NaN Test
NOALIAS __forceinline const VBool IsNan( const VF32 VECREF x );

/// VF32 Finite Test ( true if x is not a NaN and abs(x) <= FLT_MAX )
NOALIAS __forceinline const VBool Finite( const VF32 VECREF x );


/// VF32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const VF32 Select( const VBool PCREF condition, const VF32 VECREF result_if_true, const VF32 VECREF result_if_false );


/// VF32 Branchless Conditional Store: stores src into pDst if condition is true
/// NOTE: This performs a read from pDst first which will load the data into the L1 cache rather than just writing through to L2 (if not already in L1)
NOALIAS __forceinline void StoreConditional( const VBool PCREF condition, const VF32 VECREF src, F32* const __restrict pDst );


/// VF32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const VF32 AND( const VF32 VECREF a, const VBool PCREF b );

/// VF32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const VF32 ANDC( const VF32 VECREF a, const VBool PCREF b );







//
// Promote ( VF32, F32 ) binary ops to ( VF32, VF32 )
//

NOALIAS __forceinline const VF32 operator + ( const VF32 VECREF x, const F32& y );
NOALIAS __forceinline const VF32 operator - ( const VF32 VECREF x, const F32& y );
NOALIAS __forceinline const VF32 operator * ( const VF32 VECREF x, const F32& y );
NOALIAS __forceinline const VF32 operator / ( const VF32 VECREF x, const F32& y );

NOALIAS __forceinline const VF32& operator += ( VF32& x, const F32& y );
NOALIAS __forceinline const VF32& operator -= ( VF32& x, const F32& y );
NOALIAS __forceinline const VF32& operator *= ( VF32& x, const F32& y );
NOALIAS __forceinline const VF32& operator /= ( VF32& x, const F32& y );

//
// Promote ( F32, VF32 ) binary ops to ( VF32, VF32 )
//

NOALIAS __forceinline const VF32 operator + ( const F32& x, const VF32 VECREF y );
NOALIAS __forceinline const VF32 operator - ( const F32& x, const VF32 VECREF y );
NOALIAS __forceinline const VF32 operator * ( const F32& x, const VF32 VECREF y );
NOALIAS __forceinline const VF32 operator / ( const F32& x, const VF32 VECREF y );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


