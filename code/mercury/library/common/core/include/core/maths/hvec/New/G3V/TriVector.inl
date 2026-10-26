#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_TRIVECTOR_INL
#define SOFT_MATHS_HVEC_NEW_G3V_TRIVECTOR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_TRIVECTOR_INL
#define MATHS_HVEC_NEW_G3V_TRIVECTOR_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup G3V_TriVector
/// @{

/// TriVector constant integer template
template < int I > NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Int()
{
    return G3V::MakeTriVector( Const<VF32>::Int<I>() );
}

/// TriVector constant integer fraction template
template < int Num, int Denom > NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Frac()
{
    return G3V::MakeTriVector( Const<VF32>::Frac<Num,Denom>() );
}

NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Zero()              { return G3V::MakeTriVector( Maths::Const<VF32>::Zero() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::One()               { return G3V::MakeTriVector( Maths::Const<VF32>::One() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::MinusOne()          { return G3V::MakeTriVector( Maths::Const<VF32>::MinusOne() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Half()              { return G3V::MakeTriVector( Maths::Const<VF32>::Half() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Min()               { return G3V::MakeTriVector( Maths::Const<VF32>::Min() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Max()               { return G3V::MakeTriVector( Maths::Const<VF32>::Max() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Epsilon()           { return G3V::MakeTriVector( Maths::Const<VF32>::Epsilon() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::EpsilonSquared()    { return G3V::MakeTriVector( Maths::Const<VF32>::EpsilonSquared() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Epsilon2()          { return G3V::MakeTriVector( Maths::Const<VF32>::Epsilon2() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Epsilon3()          { return G3V::MakeTriVector( Maths::Const<VF32>::Epsilon3() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Pi()                { return G3V::MakeTriVector( Maths::Const<VF32>::Pi() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::TwoPi()             { return G3V::MakeTriVector( Maths::Const<VF32>::TwoPi() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::RcpPi()             { return G3V::MakeTriVector( Maths::Const<VF32>::RcpPi() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Rcp2Pi()            { return G3V::MakeTriVector( Maths::Const<VF32>::Rcp2Pi() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::PiDiv2()            { return G3V::MakeTriVector( Maths::Const<VF32>::PiDiv2() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Sqrt2()             { return G3V::MakeTriVector( Maths::Const<VF32>::Sqrt2() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::Sqrt2div2()         { return G3V::MakeTriVector( Maths::Const<VF32>::Sqrt2div2() ); }
NOALIAS __forceinline const G3V::TriVector Const< G3V::TriVector >::e()                 { return G3V::MakeTriVector( Maths::Const<VF32>::e() ); }

/// @}


namespace G3V
{
/// @addtogroup G3V_TriVector
/// @{

/// Make TriVector from VF32
NOALIAS __forceinline const TriVector MakeTriVector( const VF32 VECREF a )
{
    TriVector result;
    result.m_T = a;
    return result;
}

/// Get VF32 from TriVector
NOALIAS __forceinline const VF32 GetVF32( const TriVector PCREF a )
{
    return a.m_T;
}

/// Make TriVector from F32
NOALIAS __forceinline const TriVector MakeTriVector( const F32& a )
{
    return MakeTriVector( MakeVF32( a ) );
}

/// Get F32 from TriVector
NOALIAS __forceinline const F32 GetF32( const TriVector PCREF a )
{
    return GetF32( a.m_T );
}




/// TriVector Abs
NOALIAS __forceinline const TriVector Abs( const TriVector PCREF a )
{
    return MakeTriVector( Abs( a.m_T ) );
}

/// TriVector Sign : Returns -1.0f if scalar < 0.0f, +1.0f if scalar > 0.0f, or 0.0f if scalar == 0.0f 
NOALIAS __forceinline const TriVector Sign( const TriVector PCREF a )
{
    return MakeTriVector( Sign( a.m_T ) );
}

/// TriVector CopySign : Returns a TriVector with it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const TriVector CopySign( const TriVector PCREF magnitude, const TriVector PCREF sign )
{
    return MakeTriVector( CopySign( magnitude.m_T, sign.m_T ) );
}


/// TriVector Reciprocal
NOALIAS __forceinline const TriVector Rcp( const TriVector PCREF a )
{
    return MakeTriVector( Rcp( a.m_T ) );
}

/// TriVector Division
NOALIAS __forceinline const TriVector Div( const TriVector PCREF a, const TriVector PCREF b )
{
    return MakeTriVector( Div( a.m_T, b.m_T ) );
}

/// TriVector Reciprocal Square Root
NOALIAS __forceinline const TriVector RcpSqrt( const TriVector PCREF a )
{
    return MakeTriVector( RcpSqrt( a.m_T ) );
}

/// TriVector Square Root
NOALIAS __forceinline const TriVector Sqrt( const TriVector PCREF a )
{
    return MakeTriVector( Sqrt( a.m_T ) );
}


/// TriVector Negation Operator
NOALIAS __forceinline const TriVector operator - ( const TriVector PCREF a )
{
    return MakeTriVector( -a.m_T );
}

/// TriVector Sum Operator
NOALIAS __forceinline const TriVector operator + ( const TriVector PCREF a, const TriVector PCREF b )
{
    return MakeTriVector( a.m_T + b.m_T );
}

/// TriVector Difference Operator
NOALIAS __forceinline const TriVector operator - ( const TriVector PCREF a, const TriVector PCREF b )
{
    return MakeTriVector( a.m_T - b.m_T );
}

/// TriVector Multiplication Operator
//NOALIAS __forceinline const TriVector operator * ( const TriVector PCREF a, const TriVector PCREF b )
//{
//    return MakeTriVector( a.m_T * b.m_T );
//}

/// TriVector Division Operator
//NOALIAS __forceinline const TriVector operator / ( const TriVector PCREF a, const TriVector PCREF b )
//{
//    return MakeTriVector( a.m_T / b.m_T );
//}


/// TriVector Add Assign Operator
NOALIAS __forceinline const TriVector& operator += ( TriVector& a, const TriVector PCREF b )
{
    a.m_T += b.m_T;
    return a;
}

/// TriVector Subtract Assign Operator
NOALIAS __forceinline const TriVector& operator -= ( TriVector& a, const TriVector PCREF b )
{
    a.m_T -= b.m_T;
    return a;
}

/// TriVector Multiply Assign Operator
//NOALIAS __forceinline const TriVector& operator *= ( TriVector& a, const TriVector PCREF b )
//{
//    a.m_T *= b.m_T;
//    return a;
//}

/// TriVector Divide Assign Operator
//NOALIAS __forceinline const TriVector& operator /= ( TriVector& a, const TriVector PCREF b )
//{
//    a.m_T /= b.m_T;
//    return a;
//}


/// TriVector MulAdd : a * b + c
//NOALIAS __forceinline const TriVector MulAdd( const TriVector PCREF a, const TriVector PCREF b, const TriVector PCREF c )
//{
//    return MakeTriVector( MulAdd( a.m_T, b.m_T, c.m_T ) );
//}

/// TriVector MulSub : a * b - c
//NOALIAS __forceinline const TriVector MulSub( const TriVector PCREF a, const TriVector PCREF b, const TriVector PCREF c )
//{
//    return MakeTriVector( MulSub( a.m_T, b.m_T, c.m_T ) );
//}

/// TriVector NegMulAdd : -( a * b + c )
//NOALIAS __forceinline const TriVector NegMulAdd( const TriVector PCREF a, const TriVector PCREF b, const TriVector PCREF c )
//{
//    return MakeTriVector( NegMulAdd( a.m_T, b.m_T, c.m_T ) );
//}

/// TriVector NegMulSub : -( a * b - c ) == c - a * b
//NOALIAS __forceinline const TriVector NegMulSub( const TriVector PCREF a, const TriVector PCREF b, const TriVector PCREF c )
//{
//    return MakeTriVector( NegMulSub( a.m_T, b.m_T, c.m_T ) );
//}


/// TriVector Min
NOALIAS __forceinline const TriVector Min( const TriVector PCREF a, const TriVector PCREF b )
{
    return MakeTriVector( Min( a.m_T, b.m_T ) );
}

/// TriVector Max
NOALIAS __forceinline const TriVector Max( const TriVector PCREF a, const TriVector PCREF b )
{
    return MakeTriVector( Max( a.m_T, b.m_T ) );
}

/// TriVector Min Assign
NOALIAS __forceinline const TriVector& MinEq( TriVector* const __restrict pA, const TriVector PCREF b )
{
    MinEq( &pA->m_T, b.m_T );
    return *pA;
}

/// TriVector Max Assign
NOALIAS __forceinline const TriVector& MaxEq( TriVector* const __restrict pA, const TriVector PCREF b )
{
    MaxEq( &pA->m_T, b.m_T );
    return *pA;
}

/// TriVector Clamp
NOALIAS __forceinline const TriVector Clamp( const TriVector PCREF a, const TriVector PCREF lo, const TriVector PCREF hi )
{
    return MakeTriVector( Clamp( a.m_T, lo.m_T, hi.m_T ) );
}

/// TriVector Clamp Assign
NOALIAS __forceinline const TriVector& ClampEq( TriVector* const __restrict pA, const TriVector PCREF lo, const TriVector PCREF hi )
{
    ClampEq( &pA->m_T, lo.m_T, hi.m_T );
    return *pA;
}


/// TriVector Round to integer towards minus infinity
NOALIAS __forceinline const TriVector Floor( const TriVector PCREF a )
{
    return MakeTriVector( Floor( a.m_T ) );
}

/// TriVector Round to integer towards plus infinity
NOALIAS __forceinline const TriVector Ceil( const TriVector PCREF a )
{
    return MakeTriVector( Ceil( a.m_T ) );
}

/// TriVector Round to integer towards nearest
NOALIAS __forceinline const TriVector Round( const TriVector PCREF a )
{
    return MakeTriVector( Round( a.m_T ) );
}

/// TriVector Round to integer towards zero
NOALIAS __forceinline const TriVector Trunc( const TriVector PCREF a )
{
    return MakeTriVector( Trunc( a.m_T ) );
}


/// TriVector Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const TriVector Mod( const TriVector PCREF x, const TriVector PCREF y )
{
    return MakeTriVector( Mod( x.m_T, y.m_T ) );
}

/// TriVector Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const TriVector Mod( const TriVector PCREF x, const TriVector PCREF y, const TriVector PCREF rcp_y )
{
    return MakeTriVector( Mod( x.m_T, y.m_T, rcp_y.m_T ) );
}

/// TriVector Fractional Part : Returns fractional part of x, f = x - i [ x = i + f ], where i is an integer, i & f have the same sign as x
NOALIAS __forceinline const TriVector Frac( const TriVector PCREF x )
{
    return MakeTriVector( Frac( x.m_T ) );
}

/// TriVector Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const TriVector WrapToInterval( const TriVector PCREF x, const TriVector PCREF p )
{
    return MakeTriVector( WrapToInterval( x.m_T, p.m_T ) );
}

/// TriVector Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const TriVector WrapToInterval( const TriVector PCREF x, const TriVector PCREF p, const TriVector PCREF rcp_p )
{
    return MakeTriVector( WrapToInterval( x.m_T, p.m_T, rcp_p.m_T ) );
}

/// TriVector Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const TriVector WrapToIntervalOffset( const TriVector PCREF x, const TriVector PCREF p, const TriVector PCREF o )
{
    return MakeTriVector( WrapToIntervalOffset( x.m_T, p.m_T, o.m_T ) );
}

/// TriVector Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const TriVector WrapToIntervalOffset( const TriVector PCREF x, const TriVector PCREF p, const TriVector PCREF rcp_p, const TriVector PCREF o )
{
    return MakeTriVector( WrapToIntervalOffset( x.m_T, p.m_T, rcp_p.m_T, o.m_T ) );
}


/// TriVector Comparison a == b : Returns VBool representing results of a == b
NOALIAS __forceinline const VBool operator == ( const TriVector PCREF a, const TriVector PCREF b )
{
    return a.m_T == b.m_T;
}

/// TriVector Comparison a != b : Returns VBool representing results of a != b
NOALIAS __forceinline const VBool operator != ( const TriVector PCREF a, const TriVector PCREF b )
{
    return a.m_T != b.m_T;
}

/// TriVector Comparison a <= b : Returns VBool representing results of a <= b
NOALIAS __forceinline const VBool operator <= ( const TriVector PCREF a, const TriVector PCREF b )
{
    return a.m_T <= b.m_T;
}

/// TriVector Comparison a >= b : Returns VBool representing results of a >= b
NOALIAS __forceinline const VBool operator >= ( const TriVector PCREF a, const TriVector PCREF b )
{
    return a.m_T >= b.m_T;
}

/// TriVector Comparison a < b : Returns VBool representing results of a < b
NOALIAS __forceinline const VBool operator < ( const TriVector PCREF a, const TriVector PCREF b )
{
    return a.m_T < b.m_T;
}

/// TriVector Comparison a > b : Returns VBool representing results of a > b
NOALIAS __forceinline const VBool operator > ( const TriVector PCREF a, const TriVector PCREF b )
{
    return a.m_T > b.m_T;
}

/// TriVector Ordered Test ( true if neither a nor b is a NaN )
NOALIAS __forceinline const VBool Ordered( const TriVector PCREF a, const TriVector PCREF b )
{
    return Ordered( a.m_T, b.m_T );
}

/// TriVector Unordered Test ( true if a or b is a NaN )
NOALIAS __forceinline const VBool Unordered( const TriVector PCREF a, const TriVector PCREF b )
{
    return Unordered( a.m_T, b.m_T );
}


/// TriVector NaN Test
NOALIAS __forceinline const VBool IsNan( const TriVector PCREF a )
{
    return IsNan( a.m_T );
}

/// TriVector Finite Test ( true if a is not a NaN and abs(a) <= FLT_MAX )
NOALIAS __forceinline const VBool Finite( const TriVector PCREF a )
{
    return Finite( a.m_T );
}


/// TriVector Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const TriVector Select( const VBool PCREF condition, const TriVector PCREF result_if_true, const TriVector PCREF result_if_false )
{
    return MakeTriVector( Select( condition, result_if_true.m_T, result_if_false.m_T ) );
}


/// Promote ( TriVector + VF32 ) to ( TriVector + TriVector )
//NOALIAS __forceinline const TriVector operator + ( const TriVector PCREF a, const VF32 VECREF b ) { return a + MakeTriVector( b ); }

/// Promote ( TriVector - VF32 ) to ( TriVector - TriVector )
//NOALIAS __forceinline const TriVector operator - ( const TriVector PCREF a, const VF32 VECREF b ) { return a - MakeTriVector( b ); }

/// Promote ( TriVector * VF32 ) to ( TriVector * TriVector )
//NOALIAS __forceinline const TriVector operator * ( const TriVector PCREF a, const VF32 VECREF b ) { return a * MakeTriVector( b ); }

/// Promote ( TriVector / VF32 ) to ( TriVector / TriVector )
//NOALIAS __forceinline const TriVector operator / ( const TriVector PCREF a, const VF32 VECREF b ) { return a / MakeTriVector( b ); }


/// Promote ( VF32 + TriVector ) to ( TriVector + TriVector )
//NOALIAS __forceinline const TriVector operator + ( const VF32 VECREF a, const TriVector PCREF b ) { return MakeTriVector( a ) + b; }

/// Promote ( VF32 - TriVector ) to ( TriVector - TriVector )
//NOALIAS __forceinline const TriVector operator - ( const VF32 VECREF a, const TriVector PCREF b ) { return MakeTriVector( a ) - b; }

/// Promote ( VF32 * TriVector ) to ( TriVector * TriVector )
//NOALIAS __forceinline const TriVector operator * ( const VF32 VECREF a, const TriVector PCREF b ) { return MakeTriVector( a ) * b; }

/// Promote ( VF32 / TriVector ) to ( TriVector / TriVector )
//NOALIAS __forceinline const TriVector operator / ( const VF32 VECREF a, const TriVector PCREF b ) { return MakeTriVector( a ) / b; }


/// Promote ( TriVector += VF32 ) to ( TriVector += TriVector )
//NOALIAS __forceinline const TriVector& operator += ( TriVector& a, const VF32 VECREF b ) { a += MakeTriVector( b ); return a; }

/// Promote ( TriVector -= VF32 ) to ( TriVector -= TriVector )
//NOALIAS __forceinline const TriVector& operator -= ( TriVector& a, const VF32 VECREF b ) { a -= MakeTriVector( b ); return a; }

/// Promote ( TriVector *= VF32 ) to ( TriVector *= TriVector )
//NOALIAS __forceinline const TriVector& operator *= ( TriVector& a, const VF32 VECREF b ) { a *= MakeTriVector( b ); return a; }

/// Promote ( TriVector /= VF32 ) to ( TriVector /= TriVector )
//NOALIAS __forceinline const TriVector& operator /= ( TriVector& a, const VF32 VECREF b ) { a /= MakeTriVector( b ); return a; }


/// Promote ( TriVector + F32 ) to ( TriVector + TriVector )
//NOALIAS __forceinline const TriVector operator + ( const TriVector PCREF a, const F32& b ) { return a + MakeTriVector( MakeVF32( b ) ); }

/// Promote ( TriVector - F32 ) to ( TriVector - TriVector )
//NOALIAS __forceinline const TriVector operator - ( const TriVector PCREF a, const F32& b ) { return a - MakeTriVector( MakeVF32( b ) ); }

/// Promote ( TriVector * F32 ) to ( TriVector * TriVector )
//NOALIAS __forceinline const TriVector operator * ( const TriVector PCREF a, const F32& b ) { return a * MakeTriVector( MakeVF32( b ) ); }

/// Promote ( TriVector / F32 ) to ( TriVector / TriVector )
//NOALIAS __forceinline const TriVector operator / ( const TriVector PCREF a, const F32& b ) { return a / MakeTriVector( MakeVF32( b ) ); }


/// Promote ( F32 + TriVector ) to ( TriVector + TriVector )
//NOALIAS __forceinline const TriVector operator + ( const F32& a, const TriVector PCREF b ) { return MakeTriVector( MakeVF32( a ) ) + b; }

/// Promote ( F32 - TriVector ) to ( TriVector - TriVector )
//NOALIAS __forceinline const TriVector operator - ( const F32& a, const TriVector PCREF b ) { return MakeTriVector( MakeVF32( a ) ) - b; }

/// Promote ( F32 * TriVector ) to ( TriVector * TriVector )
//NOALIAS __forceinline const TriVector operator * ( const F32& a, const TriVector PCREF b ) { return MakeTriVector( MakeVF32( a ) ) * b; }

/// Promote ( F32 / TriVector ) to ( TriVector / TriVector )
//NOALIAS __forceinline const TriVector operator / ( const F32& a, const TriVector PCREF b ) { return MakeTriVector( MakeVF32( a ) ) / b; }


/// Promote ( TriVector += F32 ) to ( TriVector += TriVector )
//NOALIAS __forceinline const TriVector& operator += ( TriVector& a, const F32& b ) { a += MakeTriVector( MakeVF32( b ) ); return a; }

/// Promote ( TriVector -= F32 ) to ( TriVector -= TriVector )
//NOALIAS __forceinline const TriVector& operator -= ( TriVector& a, const F32& b ) { a -= MakeTriVector( MakeVF32( b ) ); return a; }

/// Promote ( TriVector *= F32 ) to ( TriVector *= TriVector )
//NOALIAS __forceinline const TriVector& operator *= ( TriVector& a, const F32& b ) { a *= MakeTriVector( MakeVF32( b ) ); return a; }

/// Promote ( TriVector /= F32 ) to ( TriVector /= TriVector )
//NOALIAS __forceinline const TriVector& operator /= ( TriVector& a, const F32& b ) { a /= MakeTriVector( MakeVF32( b ) ); return a; }





/// TriVector Reversion
NOALIAS __forceinline const TriVector Reverse( const TriVector PCREF tv )
{
    return -tv;
}

/// TriVector Grade-Involution
NOALIAS __forceinline const TriVector Involution( const TriVector PCREF tv )
{
    return -tv;
}



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


