
#pragma once

namespace Maths
{


/// Default Constructor
FORCE_INLINE
C_Complex::C_Complex()
{
}

/// Copy Constructor
FORCE_INLINE
C_Complex::C_Complex( const C_Complex& copy ) :
    m_RI( copy.m_RI )
{
}

/// Private Explicit HVec2 Constructor
FORCE_INLINE
C_Complex::C_Complex( const HVec2 PCREF ri ) :
    m_RI( ri )
{
}

/// HF32 Constructor
FORCE_INLINE
C_Complex::C_Complex( const HF32 PCREF real, const HF32 PCREF imag ) :
    m_RI( real, imag )
{
}

/// Accessor: Real Component 
FORCE_INLINE
const HF32 C_Complex::Real() const
{
    return m_RI.X();
}

/// Accessor: Imaginary Component 
FORCE_INLINE
const HF32 C_Complex::Imag() const
{
    return m_RI.Y();
}

/// Accessor: Modulus
FORCE_INLINE
const HF32 C_Complex::Modulus() const
{
    // http://mathworld.wolfram.com/ComplexModulus.html
    
    return m_RI.Length();
}

/// Accessor: Phase
FORCE_INLINE
const HF32 C_Complex::Phase() const
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    
    // TODO: Need SIMD version of atan2()
    
    const F32 x = m_RI.X();
    const F32 y = m_RI.Y();
    return HF32( atan2( y, x ) );
}

/// Copy Assignment Operator
FORCE_INLINE
void C_Complex::operator = ( const C_Complex& copy )
{
    m_RI = copy.m_RI;
}

/// Conjugate
FORCE_INLINE
const C_Complex C_Complex::operator ! () const
{
    return C_Complex( m_RI.NegatedY() );
}

/// Unary Negation
FORCE_INLINE
const C_Complex C_Complex::operator - () const
{
    return C_Complex( -m_RI );
}

/// Addition
FORCE_INLINE
const C_Complex C_Complex::operator + ( const C_Complex PCREF src ) const
{
    return C_Complex( m_RI + src.m_RI );
}

/// Subtraction
FORCE_INLINE
const C_Complex C_Complex::operator - ( const C_Complex PCREF src ) const
{
    return C_Complex( m_RI - src.m_RI );
}

/// Multiplication
FORCE_INLINE
const C_Complex C_Complex::operator * ( const C_Complex PCREF src ) const
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    
    const HVec2 ab( m_RI );
    const HVec2 cd( src.m_RI );
    
    const HF32 a( ab.X() );
    const HF32 b( ab.Y() );
    const HF32 c( cd.X() );
    const HF32 d( cd.Y() );
    
    return C_Complex( HVec2( a * c - b * d,
                             a * d + b * c ) );
}

/// Division
FORCE_INLINE
const C_Complex C_Complex::operator / ( const C_Complex PCREF src ) const
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    
    const HVec2 ab( m_RI );
    const HVec2 cd( src.m_RI );
    
    const HF32 one( c_ScalarOne );
    
    const HF32 a( ab.X() );
    const HF32 b( ab.Y() );
    const HF32 c( cd.X() );
    const HF32 d( cd.Y() );
    
    const HF32 denom( c * c + d * d );
    
    const HF32 rcpDenom( Rcp( denom ) );
    
    return C_Complex( HVec2( ( a * c + b * d ) * rcpDenom,
                             ( b * c - a * d ) * rcpDenom ) );
}

/// Sqrt
FORCE_INLINE
const C_Complex C_Complex::Sqrt() const
{
    #if 0
    
    // http://mathworld.wolfram.com/SquareRoot.html
    
    const HF32 zero;
    const HF32 sqrt2div2( Sqrt( 2.0f ) * 0.5f );
    
    const HF32 r( m_RI.Length() ); // == sqrt( x*x + y*y )
    const HF32 x( m_RI.X() );
    
	return C_Complex( HVec2( Maths::Sqrt( Max( r + x, zero ) ),
                             Maths::Sqrt( Max( r - x, zero ) ) /* * Sign( y ) */ ) * sqrt2div2 );
    
    #else
    
    // http://www.ping.be/~ping1339/complget.htm#Square-roots-of-a-co
    
    const HF32 zero(Constants<HF32>::Zero());
    const HF32 c_1_2( c_ScalarHalf );
    
    const HF32 r( Modulus() );
    const HF32 a( m_RI.X() );
    
    return C_Complex( Maths::Sqrt( Max( ( r + a ) * c_1_2, zero ) ),
                      Maths::Sqrt( Max( ( r - a ) * c_1_2, zero ) ) );
    
    #endif
}






/// Default Constructor
FORCE_INLINE
C_Complex64::C_Complex64()
{
}

/// Copy Constructor
FORCE_INLINE
C_Complex64::C_Complex64( const C_Complex64& copy ) :
    m_Real( copy.m_Real ),
    m_Imag( copy.m_Imag )
{
}

/// F64 Constructor
FORCE_INLINE
C_Complex64::C_Complex64( const F64 real, const F64 imag ) :
    m_Real( real ),
    m_Imag( imag )
{
}

/// Accessor: Real Component 
FORCE_INLINE
const F64 C_Complex64::Real() const
{
    return m_Real;
}

/// Accessor: Imaginary Component 
FORCE_INLINE
const F64 C_Complex64::Imag() const
{
    return m_Imag;
}

/// Accessor: Modulus
FORCE_INLINE
const F64 C_Complex64::Modulus() const
{
    // http://mathworld.wolfram.com/ComplexModulus.html
    
    return Maths::Sqrt( m_Real * m_Real + m_Imag * m_Imag );
}

/// Accessor: Phase
FORCE_INLINE
const F64 C_Complex64::Phase() const
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    
    return atan2( m_Imag, m_Real );
}

/// Copy Assignment Operator
FORCE_INLINE
void C_Complex64::operator = ( const C_Complex64& copy )
{
    m_Real = copy.m_Real;
    m_Imag = copy.m_Imag;
}

/// Conjugate
FORCE_INLINE
const C_Complex64 C_Complex64::operator ! () const
{
    return C_Complex64( m_Real, -m_Imag );
}

/// Unary Negation
FORCE_INLINE
const C_Complex64 C_Complex64::operator - () const
{
    return C_Complex64( -m_Real, -m_Imag );
}

/// Addition
FORCE_INLINE
const C_Complex64 C_Complex64::operator + ( const C_Complex64 src ) const
{
    return C_Complex64( m_Real + src.m_Real,
                        m_Imag + src.m_Imag );
}

/// Subtraction
FORCE_INLINE
const C_Complex64 C_Complex64::operator - ( const C_Complex64 src ) const
{
    return C_Complex64( m_Real - src.m_Real,
                        m_Imag - src.m_Imag );
}

/// Multiplication
FORCE_INLINE
const C_Complex64 C_Complex64::operator * ( const C_Complex64 src ) const
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    
    const F64 a( m_Real );
    const F64 b( m_Imag );
    const F64 c( src.m_Real );
    const F64 d( src.m_Imag );
    
    return C_Complex64( a * c - b * d,
                        a * d + b * c );
}

/// Division
FORCE_INLINE
const C_Complex64 C_Complex64::operator / ( const C_Complex64 src ) const
{
    // http://mathworld.wolfram.com/ComplexNumber.html
    
    const F64 a( m_Real );
    const F64 b( m_Imag );
    const F64 c( src.m_Real );
    const F64 d( src.m_Imag );
    
    const F64 rcpDenom( 1.0 / ( c * c + d * d ) );
    
    return C_Complex64( ( a * c + b * d ) * rcpDenom,
                        ( b * c - a * d ) * rcpDenom );
}


/// Sqrt
FORCE_INLINE
const C_Complex64 C_Complex64::Sqrt() const
{
    #if 0
    
    /// NOTE: THIS DOESN'T WORK FOR UNKNOWN REASON!!!
    
    // http://mathworld.wolfram.com/SquareRoot.html
    
    const F64 sqrt2div2( Sqrt( 2.0 ) * 0.5 );
    
    const F64 r( Modulus() );
    const F64 x( m_Real );
    
    return C_Complex64( Maths::Sqrt( Max( r + x, 0.0 ) ) * sqrt2div2,
                        Maths::Sqrt( Max( r - x, 0.0 ) ) * sqrt2div2 /* * Sign( y ) */ );
    
    #else
    
    // http://www.ping.be/~ping1339/complget.htm#Square-roots-of-a-co
    
    const F64 zero = 0.0;
    const F64 half = 0.5;
    const F64 r( Modulus() );
    const F64 x( m_Real );
    
    return C_Complex64( Maths::Sqrt( Max( ( r + x ) * half, zero ) ),
                        Maths::Sqrt( Max( ( r - x ) * half, zero ) ) );
    
    #endif
}





/// C_Complex Constructor
FORCE_INLINE
C_CubicResult::C_CubicResult( const C_Complex& root0, const C_Complex& root1, const C_Complex& root2 ) :
    m_Root0( root0 ),
    m_Root1( root1 ),
    m_Root2( root2 )
{
}



/// C_Complex64 Constructor
FORCE_INLINE
C_CubicResult64::C_CubicResult64( const C_Complex64& root0, const C_Complex64& root1, const C_Complex64& root2 ) :
    m_Root0( root0 ),
    m_Root1( root1 ),
    m_Root2( root2 )
{
}




FORCE_INLINE
const C_Complex operator / ( const HF32 PCREF numerator, const C_Complex& divisor )
{
    return C_Complex( numerator ) / divisor;
}


FORCE_INLINE
const C_Complex64 operator / ( const F64 numerator, const C_Complex64& divisor )
{
    return C_Complex64( numerator ) / divisor;
}





} // namespace Maths




