#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_COMPLEX_H
#define SOFT_MATHS_HVEC_NEW_COMPLEX_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_COMPLEX_H
#define MATHS_HVEC_NEW_COMPLEX_H
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
NOALIAS __forceinline const Complex MakeComplex( const F32& real );

/// Make Complex from Real & Imaginary pair of F32's
NOALIAS __forceinline const Complex MakeComplex( const F32& real, const F32& imag );


/// Make Complex from Real VF32, imaginary part is zeroed
NOALIAS __forceinline const Complex MakeComplex( const VF32 VECREF real );

/// Make Complex from Real & Imaginary pair of VF32's
NOALIAS __forceinline const Complex MakeComplex( const VF32 VECREF real, const VF32 VECREF imag );

/// Make Complex from Modulus & Argument pair of VF32's
NOALIAS __forceinline const Complex MakeComplex_ModArg( const VF32 VECREF mod, const VF32 VECREF arg );


/// Complex get Real part
NOALIAS __forceinline const VF32 Real( const Complex PCREF z );

/// Complex get Imaginary part
NOALIAS __forceinline const VF32 Imag( const Complex PCREF z );

/// Complex get Modulus (aka abs)
NOALIAS __forceinline const VF32 Mod( const Complex PCREF z );

/// Complex get Abs (aka modulus)
NOALIAS __forceinline const VF32 Abs( const Complex PCREF z );

/// Complex get Absolute Squared (aka mod squared)
NOALIAS __forceinline const VF32 AbsSquared( const Complex PCREF z );

/// Complex get Phase (aka argument)
NOALIAS __forceinline const VF32 Phase( const Complex PCREF z );

/// Complex get Argument (aka phase)
NOALIAS __forceinline const VF32 Arg( const Complex PCREF z );

/// Complex Select
NOALIAS __forceinline const Complex Select( const VBool PCREF condition, const Complex PCREF result_if_true, const Complex PCREF result_if_false );


/// Complex Conjugate
NOALIAS __forceinline const Complex operator ! ( const Complex PCREF z );

/// Complex Negation
NOALIAS __forceinline const Complex operator - ( const Complex PCREF z );


/// Complex Addition
NOALIAS __forceinline const Complex operator + ( const Complex PCREF z1, const Complex PCREF z2 );

/// Complex Subtraction
NOALIAS __forceinline const Complex operator - ( const Complex PCREF z1, const Complex PCREF z2 );

/// Complex Multiplication
NOALIAS __forceinline const Complex operator * ( const Complex PCREF z1, const Complex PCREF z2 );

/// Complex Division
NOALIAS __forceinline const Complex operator / ( const Complex PCREF z1, const Complex PCREF z2 );


/// Complex-Scalar Addition
NOALIAS __forceinline const Complex operator + ( const Complex PCREF z, const VF32 VECREF real );

/// Scalar-Complex Addition
NOALIAS __forceinline const Complex operator + ( const VF32 VECREF real, const Complex PCREF z );


/// Complex-Scalar Subtraction
NOALIAS __forceinline const Complex operator - ( const Complex PCREF z, const VF32 VECREF real );

/// Scalar-Complex Subtraction
NOALIAS __forceinline const Complex operator - ( const VF32 VECREF real, const Complex PCREF z );


/// Complex-Scalar Multiplication
NOALIAS __forceinline const Complex operator * ( const Complex PCREF z, const VF32 VECREF real );

/// Scalar-Complex Multiplication
NOALIAS __forceinline const Complex operator * ( const VF32 VECREF real, const Complex PCREF z );


/// Scalar-Complex Division
NOALIAS __forceinline const Complex operator / ( const VF32 VECREF real, const Complex PCREF z );

/// Complex-Scalar Division
NOALIAS __forceinline const Complex operator / ( const Complex PCREF z, const VF32 VECREF real );


/// Complex Square Root: Note: returns one of the 2 complex roots, the second is the negation of the first.
NOALIAS __forceinline const Complex Sqrt( const Complex PCREF z );

/// Complex Cube Root: returns the three cube roots of z
NOALIAS __forceinline void CubeRoot(
    const Complex PCREF z,
    Complex* const __restrict pRoot1,
    Complex* const __restrict pRoot2,
    Complex* const __restrict pRoot3 );

/// Complex Inverse: z * inverse(z) = 1
NOALIAS __forceinline const Complex Inverse( const Complex PCREF z );

/// Complex Power: z^n
NOALIAS __forceinline const Complex Pow( const Complex PCREF z, const VF32 VECREF n );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

