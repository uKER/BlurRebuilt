#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_COMPLEX_INL
#define SOFT_MATHS_HVEC_NEW_COMPLEX_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_COMPLEX_INL
#define MATHS_HVEC_NEW_COMPLEX_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Complex
/// @{

/// Make Complex from Real F32, imaginary part is zeroed
NOALIAS __forceinline const Complex MakeComplex( const F32& real )
{
    Complex result;
    result.m_V = MakeVec4F32( real, 0.0f );
    return result;
}

/// Make Complex from Real & Imaginary pair of F32's
NOALIAS __forceinline const Complex MakeComplex( const F32& real, const F32& imag )
{
    Complex result;
    result.m_V = MakeVec4F32( real, imag );
    return result;
}


/// Make Complex from Real VF32, imaginary part is zeroed
NOALIAS __forceinline const Complex MakeComplex( const VF32 VECREF real )
{
    Complex result;
    const VF32 zero( Const<VF32>::Zero() );
    result.m_V = MakeVec4F32( real, zero );
    return result;
}

/// Make Complex from Real & Imaginary pair of VF32's
NOALIAS __forceinline const Complex MakeComplex( const VF32 VECREF real, const VF32 VECREF imag )
{
    Complex result;
    result.m_V = MakeVec4F32( real, imag );
    return result;
}

/// Make Complex from Modulus & Argument pair of VF32's
NOALIAS __forceinline const Complex MakeComplex_ModArg( const VF32 VECREF mod, const VF32 VECREF arg )
{
    VF32 real, imag;
    SinCos( arg, &imag, &real );
    Complex result;
    result.m_V = mod * MakeVec4F32( real, imag );
    return result;
}


/// Complex get Real part
NOALIAS __forceinline const VF32 Real( const Complex PCREF z )
{
    return X( z.m_V );
}

/// Complex get Imaginary part
NOALIAS __forceinline const VF32 Imag( const Complex PCREF z )
{
    return Y( z.m_V );
}

/// Complex get Mod (aka norm or absolute value)
NOALIAS __forceinline const VF32 Mod( const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexModulus.html
    return Length2_24( z.m_V );
}

/// Complex get Abs (aka norm or modulus)
NOALIAS __forceinline const VF32 Abs( const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexModulus.html
    return Length2_24( z.m_V );
}

/// Complex get Absolute Squared (aka mod squared)
NOALIAS __forceinline const VF32 AbsSquared( const Complex PCREF z )
{
    // http://mathworld.wolfram.com/AbsoluteSquare.html
    return LengthSquared2( z.m_V );
}

/// Complex get Phase (aka argument)
NOALIAS __forceinline const VF32 Phase( const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    return ArcTan( Y( z.m_V ), X( z.m_V ) );
}

/// Complex get Argument (aka phase)
NOALIAS __forceinline const VF32 Arg( const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    return ArcTan( Y( z.m_V ), X( z.m_V ) );
}

/// Complex Select
NOALIAS __forceinline const Complex Select( const VBool PCREF condition, const Complex PCREF result_if_true, const Complex PCREF result_if_false )
{
    Complex result;
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    return result;
}

/// Complex Conjugate
NOALIAS __forceinline const Complex operator ! ( const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexConjugate.html
    Complex result;
    result.m_V = NegateY( z.m_V );
    return result;
}

/// Complex Negation
NOALIAS __forceinline const Complex operator - ( const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = -z.m_V;
    return result;
}

/// Complex Addition
NOALIAS __forceinline const Complex operator + ( const Complex PCREF z1, const Complex PCREF z2 )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = z1.m_V + z2.m_V;
    return result;
}

/// Complex Subtraction
NOALIAS __forceinline const Complex operator - ( const Complex PCREF z1, const Complex PCREF z2 )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = z1.m_V - z2.m_V;
    return result;
}

/// Complex Multiplication
NOALIAS __forceinline const Complex operator * ( const Complex PCREF z1, const Complex PCREF z2 )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    const VF32 a( Real( z1 ) );
    const VF32 b( Imag( z1 ) );
    const VF32 c( Real( z2 ) );
    const VF32 d( Imag( z2 ) );
    Complex result;
    //result.m_V = MakeVec4F32( a * c - b * d,
    //                          a * d + b * c );
    //result.m_V = a * MakeVec4F32( c, d ) + b * MakeVec4F32( -d, c );
    result.m_V = MulAdd( a, MakeVec4F32( c, d ), b * MakeVec4F32( -d, c ) );
    return result;
}

/// Complex Division
NOALIAS __forceinline const Complex operator / ( const Complex PCREF z1, const Complex PCREF z2 )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    const VF32 a( Real( z1 ) );
    const VF32 b( Imag( z1 ) );
    const VF32 c( Real( z2 ) );
    const VF32 d( Imag( z2 ) );
    const VF32 rcpDenom( Rcp24( AbsSquared( z2 ) ) );
    Complex result;
    //result.m_V = MakeVec4F32( a * c + b * d,
    //                          b * c - a * d ) * rcpDenom;
    //result.m_V = ( MakeVec4F32( a, b ) * c + MakeVec4F32( b, -a ) * d ) * rcpDenom;
    result.m_V = ( MulAdd( MakeVec4F32( a, b ), c, MakeVec4F32( b, -a ) * d ) ) * rcpDenom;
    return result;
}


/// Complex-Scalar Addition
NOALIAS __forceinline const Complex operator + ( const Complex PCREF z, const VF32 VECREF real )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = z.m_V + MakeVec4F32( real, Const<VF32>::Zero() );
    return result;
}

/// Scalar-Complex Addition
NOALIAS __forceinline const Complex operator + ( const VF32 VECREF real, const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = z.m_V + MakeVec4F32( real, Const<VF32>::Zero() );
    return result;
}


/// Complex-Scalar Subtraction
NOALIAS __forceinline const Complex operator - ( const Complex PCREF z, const VF32 VECREF real )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = z.m_V - MakeVec4F32( real, Const<VF32>::Zero() );
    return result;
}

/// Scalar-Complex Subtraction
NOALIAS __forceinline const Complex operator - ( const VF32 VECREF real, const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = MakeVec4F32( real, Const<VF32>::Zero() ) - z.m_V;
    return result;
}


/// Complex-Scalar Multiplication
NOALIAS __forceinline const Complex operator * ( const Complex PCREF z, const VF32 VECREF real )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = z.m_V * real;
    return result;
}

/// Scalar-Complex Multiplication
NOALIAS __forceinline const Complex operator * ( const VF32 VECREF real, const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = z.m_V * real;
    return result;
}


/// Scalar-Complex Division
NOALIAS __forceinline const Complex operator / ( const VF32 VECREF real, const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    const VF32 rcpDenom( Rcp24( AbsSquared( z ) ) );
    Complex result;
    //result.m_V = MakeVec4F32( a * c + b * d,
    //                          b * c - a * d ) * rcpDenom;
    //result.m_V = MakeVec4F32(  a * c,
    //                          -a * d ) * rcpDenom;
    //result.m_V = MakeVec4F32( a, -a ) * z.m_V * rcpDenom;
    result.m_V = real * NegateY( z.m_V ) * rcpDenom;
    return result;
}

/// Complex-Scalar Division
NOALIAS __forceinline const Complex operator / ( const Complex PCREF z, const VF32 VECREF real )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    Complex result;
    result.m_V = Div24( z.m_V, real );
    return result;
}


/// Complex Square Root: Note: returns one of the 2 complex roots, the second is the negation of the first.
NOALIAS __forceinline const Complex Sqrt( const Complex PCREF z )
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    // http://www.ping.be/~ping1339/complget.htm#Square-roots-of-a-co
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 half( Const<VF32>::Half() );
    const VF32 epsilon( Const<VF32>::Epsilon() );
    const VF32 r( Mod( z ) );
    const VF32 a( Real( z ) );
    const VF32 b( Imag( z ) );
    const VF32 y1_squared( Max( ( r - a ) * half, zero ) );
    Vec4F32 sqrt, rcp_sqrt;
    SqrtAndRcpSqrt24( MakeVec4F32( a, b, y1_squared ), &sqrt, &rcp_sqrt );
    const VF32 sqrt_a( X( sqrt ) );
    const VF32 y1( Z( sqrt ) );
    const VF32 rcp_y1( Z( rcp_sqrt ) );
    const VF32 x0( b * half * rcp_y1 );
    const VF32 x1( Select( ( a >= zero ) & ( Abs(b) <= epsilon ),
                           sqrt_a, // Note: if z is a strict positive real number, then
                           x0 ) ); // b=0, y1=0 and rcp_y1=infinity! In this case we use x1=sqrt(a)
    return MakeComplex( x1, y1 ); // Note: x2 = -x1, y2 = -y1
}

/// Complex Cube Root: returns the three cube roots of z
NOALIAS __forceinline void CubeRoot(
    const Complex PCREF z,
    Complex* const __restrict pRoot1,
    Complex* const __restrict pRoot2,
    Complex* const __restrict pRoot3 )
{
    // http://scholar.hw.ac.uk/site/maths/topic19.asp?outline=
    // http://en.wikipedia.org/wiki/Cube_root
    
    const VF32 half( Const<VF32>::Half() );
    const VF32 third( Const<VF32>::Third() );
    const VF32 sqrt_3_div_2( MakeVF32( 0.86602540378443864676372317075294f ) );
    const VF32 theta( Phase( z ) * third );
    const VF32 realCubeRoot_z( CubeRoot( Mod( z ) ) );
    VF32 sinTheta, cosTheta;
    SinCos( theta, &sinTheta, &cosTheta );
    const Complex root1( MakeComplex( cosTheta, sinTheta ) * realCubeRoot_z );
    *pRoot1 = root1;
    *pRoot2 = root1 * MakeComplex( -half,  sqrt_3_div_2 );
    *pRoot3 = root1 * MakeComplex( -half, -sqrt_3_div_2 );
}

/// Complex Inverse: z * inverse(z) = 1
NOALIAS __forceinline const Complex Inverse( const Complex PCREF z )
{
    // http://www.ping.be/~ping1339/complget.htm#Square-roots-of-a-co
    
    const VF32 a( Real( z ) );
    const VF32 b( Imag( z ) );
    const VF32 rcpAbsSquared( Rcp24( AbsSquared( z ) ) );
    return MakeComplex( a * rcpAbsSquared, -b * rcpAbsSquared );
}

/// Complex Power: z^n
NOALIAS __forceinline const Complex Pow( const Complex PCREF z, const VF32 VECREF n )
{
    const VF32  zero( Const<VF32>::Zero() );
    const VF32  arg( Arg( z ) );
    const VF32  mod( Mod( z ) );
    const VF32  rcp_mod( Rcp24( mod ) );
    const VF32  abs_n( Abs( n ) );
    const VBool positive_n( n >= zero );
    return MakeComplex_ModArg( Pow( Select( positive_n, mod, rcp_mod ), abs_n ),
                               arg * Select( positive_n, n, -n ) );
}





/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

