#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_TRIVECTOR_H
#define SOFT_MATHS_HVEC_NEW_G3V_TRIVECTOR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_TRIVECTOR_H
#define MATHS_HVEC_NEW_G3V_TRIVECTOR_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup G3V_TriVector
/// @{

/// TriVector constants
template <> struct Const< G3V::TriVector > 
{
    /// TriVector constant integer template
    template < int I > NOALIAS __forceinline static const G3V::TriVector Int();
    
    /// TriVector constant integer fraction template
    template < int Num, int Denom > NOALIAS __forceinline static const G3V::TriVector Frac();
    
	NOALIAS __forceinline static const G3V::TriVector Zero();               ///< TriVector constant 0.0
	NOALIAS __forceinline static const G3V::TriVector One();                ///< TriVector constant 1.0
	NOALIAS __forceinline static const G3V::TriVector MinusOne();           ///< TriVector constant -1.0
	
	NOALIAS __forceinline static const G3V::TriVector Half();               ///< TriVector constant 0.5
	
	NOALIAS __forceinline static const G3V::TriVector Min();                ///< TriVector constant FLT_MIN
	NOALIAS __forceinline static const G3V::TriVector Max();                ///< TriVector constant FLT_MAX
	NOALIAS __forceinline static const G3V::TriVector Epsilon();            ///< TriVector constant FLT_EPSILON
	NOALIAS __forceinline static const G3V::TriVector EpsilonSquared();     ///< TriVector constant FLT_EPSILON * FLT_EPSILON
	NOALIAS __forceinline static const G3V::TriVector Epsilon2();           ///< TriVector constant epsilon2 = 0.01
	NOALIAS __forceinline static const G3V::TriVector Epsilon3();           ///< TriVector constant epsilon3 = 1e-12
	NOALIAS __forceinline static const G3V::TriVector Pi();                 ///< TriVector constant PI
	NOALIAS __forceinline static const G3V::TriVector TwoPi();              ///< TriVector constant PI*2
	NOALIAS __forceinline static const G3V::TriVector RcpPi();              ///< TriVector constant 1/PI
	NOALIAS __forceinline static const G3V::TriVector Rcp2Pi();             ///< TriVector constant 1/(PI*2)
	NOALIAS __forceinline static const G3V::TriVector PiDiv2();             ///< TriVector constant PI/2
	NOALIAS __forceinline static const G3V::TriVector Sqrt2();              ///< TriVector constant sqrt(2)
	NOALIAS __forceinline static const G3V::TriVector Sqrt2div2();          ///< TriVector constant sqrt(2)/2
	NOALIAS __forceinline static const G3V::TriVector e();                  ///< TriVector constant natural logarithm base e ~= 2.718282
};

/// @}

namespace G3V
{
/// @addtogroup G3V_TriVector
/// @{

/// Make TriVector from VF32
NOALIAS __forceinline const TriVector MakeTriVector( const VF32 VECREF a );

/// Get VF32 from TriVector
NOALIAS __forceinline const VF32 GetVF32( const TriVector PCREF a );

/// Make TriVector from F32
NOALIAS __forceinline const TriVector MakeTriVector( const F32& a );

/// Get F32 from TriVector
NOALIAS __forceinline const F32 GetF32( const TriVector PCREF a );



/// TriVector Abs
NOALIAS __forceinline const TriVector Abs( const TriVector PCREF a );

/// TriVector Sign : Returns -1.0f if scalar < 0.0f, +1.0f if scalar > 0.0f, or 0.0f if scalar == 0.0f 
NOALIAS __forceinline const TriVector Sign( const TriVector PCREF a );

/// TriVector CopySign : Returns a TriVector with it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const TriVector CopySign( const TriVector PCREF magnitude, const TriVector PCREF sign );


/// TriVector Reciprocal
NOALIAS __forceinline const TriVector Rcp( const TriVector PCREF a );

/// TriVector Division
NOALIAS __forceinline const TriVector Div( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Reciprocal Square Root
NOALIAS __forceinline const TriVector RcpSqrt( const TriVector PCREF a );

/// TriVector Square Root
NOALIAS __forceinline const TriVector Sqrt( const TriVector PCREF a );


/// TriVector Negation Operator
NOALIAS __forceinline const TriVector operator - ( const TriVector PCREF a );

/// TriVector Sum Operator
NOALIAS __forceinline const TriVector operator + ( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Difference Operator
NOALIAS __forceinline const TriVector operator - ( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Multiplication Operator
//NOALIAS __forceinline const TriVector operator * ( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Division Operator
//NOALIAS __forceinline const TriVector operator / ( const TriVector PCREF a, const TriVector PCREF b );


/// TriVector Add Assign Operator
NOALIAS __forceinline const TriVector& operator += ( TriVector& a, const TriVector PCREF b );

/// TriVector Subtract Assign Operator
NOALIAS __forceinline const TriVector& operator -= ( TriVector& a, const TriVector PCREF b );

/// TriVector Multiply Assign Operator
//NOALIAS __forceinline const TriVector& operator *= ( TriVector& a, const TriVector PCREF b );

/// TriVector Divide Assign Operator
//NOALIAS __forceinline const TriVector& operator /= ( TriVector& a, const TriVector PCREF b );


/// TriVector MulAdd : a * b + c
//NOALIAS __forceinline const TriVector MulAdd( const TriVector PCREF a, const TriVector PCREF b, const TriVector PCREF c );

/// TriVector MulSub : a * b - c
//NOALIAS __forceinline const TriVector MulSub( const TriVector PCREF a, const TriVector PCREF b, const TriVector PCREF c );

/// TriVector NegMulAdd : -( a * b + c )
//NOALIAS __forceinline const TriVector NegMulAdd( const TriVector PCREF a, const TriVector PCREF b, const TriVector PCREF c );

/// TriVector NegMulSub : -( a * b - c ) == c - a * b
//NOALIAS __forceinline const TriVector NegMulSub( const TriVector PCREF a, const TriVector PCREF b, const TriVector PCREF c );


/// TriVector Min
NOALIAS __forceinline const TriVector Min( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Max
NOALIAS __forceinline const TriVector Max( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Min Assign
NOALIAS __forceinline const TriVector& MinEq( TriVector* const __restrict pA, const TriVector PCREF b );

/// TriVector Max Assign
NOALIAS __forceinline const TriVector& MaxEq( TriVector* const __restrict pA, const TriVector PCREF b );

/// TriVector Clamp
NOALIAS __forceinline const TriVector Clamp( const TriVector PCREF a, const TriVector PCREF lo, const TriVector PCREF hi );

/// TriVector Clamp Assign
NOALIAS __forceinline const TriVector& ClampEq( TriVector* const __restrict pA, const TriVector PCREF lo, const TriVector PCREF hi );


/// TriVector Round to integer towards minus infinity
NOALIAS __forceinline const TriVector Floor( const TriVector PCREF a ) __attribute__(( /* pure, ? */ always_inline ));

/// TriVector Round to integer towards plus infinity
NOALIAS __forceinline const TriVector Ceil ( const TriVector PCREF a ) __attribute__(( /* pure, ? */ always_inline ));

/// TriVector Round to integer towards nearest
NOALIAS __forceinline const TriVector Round( const TriVector PCREF a ) __attribute__(( /* pure, ? */ always_inline ));

/// TriVector Round to integer towards zero
NOALIAS __forceinline const TriVector Trunc( const TriVector PCREF a ) __attribute__(( /* pure, ? */ always_inline ));


/// TriVector Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const TriVector Mod( const TriVector PCREF x, const TriVector PCREF y );

/// TriVector Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const TriVector Mod( const TriVector PCREF x, const TriVector PCREF y, const TriVector PCREF rcp_y );

/// TriVector Fractional Part : Returns fractional part of x, f = x - i [ x = i + f ], where i is an integer, i & f have the same sign as x
NOALIAS __forceinline const TriVector Frac( const TriVector PCREF x );

/// TriVector Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const TriVector WrapToInterval( const TriVector PCREF x, const TriVector PCREF p );

/// TriVector Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const TriVector WrapToInterval( const TriVector PCREF x, const TriVector PCREF p, const TriVector PCREF rcp_p );

/// TriVector Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const TriVector WrapToIntervalOffset( const TriVector PCREF x, const TriVector PCREF p, const TriVector PCREF o );

/// TriVector Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const TriVector WrapToIntervalOffset( const TriVector PCREF x, const TriVector PCREF p, const TriVector PCREF rcp_p, const TriVector PCREF o );


/// TriVector Comparison a == b : Returns VBool representing results of a == b
NOALIAS __forceinline const VBool operator == ( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Comparison a != b : Returns VBool representing results of a != b
NOALIAS __forceinline const VBool operator != ( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Comparison a <= b : Returns VBool representing results of a <= b
NOALIAS __forceinline const VBool operator <= ( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Comparison a >= b : Returns VBool representing results of a >= b
NOALIAS __forceinline const VBool operator >= ( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Comparison a < b : Returns VBool representing results of a < b
NOALIAS __forceinline const VBool operator < ( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Comparison a > b : Returns VBool representing results of a > b
NOALIAS __forceinline const VBool operator > ( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Ordered Test ( true if neither a nor b is a NaN )
NOALIAS __forceinline const VBool Ordered( const TriVector PCREF a, const TriVector PCREF b );

/// TriVector Unordered Test ( true if a or b is a NaN )
NOALIAS __forceinline const VBool Unordered( const TriVector PCREF a, const TriVector PCREF b );


/// TriVector NaN Test
NOALIAS __forceinline const VBool IsNan( const TriVector PCREF a );

/// TriVector Finite Test ( true if a is not a NaN and abs(a) <= FLT_MAX )
NOALIAS __forceinline const VBool Finite( const TriVector PCREF a );


/// TriVector Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const TriVector Select( const VBool PCREF condition, const TriVector PCREF result_if_true, const TriVector PCREF result_if_false );



//
// Promote ( TriVector, VF32 ) binary ops to ( TriVector, TriVector )
//

//NOALIAS __forceinline const TriVector operator + ( const TriVector PCREF a, const VF32 VECREF b );
//NOALIAS __forceinline const TriVector operator - ( const TriVector PCREF a, const VF32 VECREF b );
//NOALIAS __forceinline const TriVector operator * ( const TriVector PCREF a, const VF32 VECREF b );
//NOALIAS __forceinline const TriVector operator / ( const TriVector PCREF a, const VF32 VECREF b );

//
// Promote ( VF32, TriVector ) binary ops to ( TriVector, TriVector )
//

//NOALIAS __forceinline const TriVector operator + ( const VF32 VECREF a, const TriVector PCREF b );
//NOALIAS __forceinline const TriVector operator - ( const VF32 VECREF a, const TriVector PCREF b );
//NOALIAS __forceinline const TriVector operator * ( const VF32 VECREF a, const TriVector PCREF b );
//NOALIAS __forceinline const TriVector operator / ( const VF32 VECREF a, const TriVector PCREF b );

//
// Promote ( TriVector, VF32 ) binary update ops to ( TriVector, TriVector )
//

//NOALIAS __forceinline const TriVector& operator += ( TriVector& a, const VF32 VECREF b );
//NOALIAS __forceinline const TriVector& operator -= ( TriVector& a, const VF32 VECREF b );
//NOALIAS __forceinline const TriVector& operator *= ( TriVector& a, const VF32 VECREF b );
//NOALIAS __forceinline const TriVector& operator /= ( TriVector& a, const VF32 VECREF b );


//
// Promote ( TriVector, F32 ) binary ops to ( TriVector, TriVector )
//

//NOALIAS __forceinline const TriVector operator + ( const TriVector PCREF a, const F32& b );
//NOALIAS __forceinline const TriVector operator - ( const TriVector PCREF a, const F32& b );
//NOALIAS __forceinline const TriVector operator * ( const TriVector PCREF a, const F32& b );
//NOALIAS __forceinline const TriVector operator / ( const TriVector PCREF a, const F32& b );

//
// Promote ( F32, TriVector ) binary ops to ( TriVector, TriVector )
//

//NOALIAS __forceinline const TriVector operator + ( const F32& a, const TriVector PCREF b );
//NOALIAS __forceinline const TriVector operator - ( const F32& a, const TriVector PCREF b );
//NOALIAS __forceinline const TriVector operator * ( const F32& a, const TriVector PCREF b );
//NOALIAS __forceinline const TriVector operator / ( const F32& a, const TriVector PCREF b );

//
// Promote ( TriVector, F32 ) binary update ops to ( TriVector, TriVector )
//

//NOALIAS __forceinline const TriVector& operator += ( TriVector& a, const F32& b );
//NOALIAS __forceinline const TriVector& operator -= ( TriVector& a, const F32& b );
//NOALIAS __forceinline const TriVector& operator *= ( TriVector& a, const F32& b );
//NOALIAS __forceinline const TriVector& operator /= ( TriVector& a, const F32& b );




/// TriVector Reversion
NOALIAS __forceinline const TriVector Reverse( const TriVector PCREF tv );

/// TriVector Grade-Involution
NOALIAS __forceinline const TriVector Involution( const TriVector PCREF tv );














/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


