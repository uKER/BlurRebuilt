
#pragma once

namespace Maths
{

class C_CubicResult;
class C_CubicResult64;


class C_Complex
{
public:
    
    /// Default Constructor
    C_Complex();
    
    /// Copy Constructor
    C_Complex( const C_Complex& copy );
    
    /// HF32 Constructor
    C_Complex( const HF32 PCREF real, const HF32 PCREF imag = HF32(ZERO_INIT) );
    
    /// Accessor: Real Component 
    const HF32              Real() const;
    
    /// Accessor: Imaginary Component 
    const HF32              Imag() const;
    
    /// Accessor: Modulus
    const HF32              Modulus() const;
    
    /// Accessor: Phase
    const HF32              Phase() const;
    
    /// Copy Assignment Operator
    void                    operator = ( const C_Complex& copy );
    
    /// Conjugate
    const C_Complex         operator ! () const;
    
    /// Unary Negation
    const C_Complex         operator - () const;
    
    /// Addition
    const C_Complex         operator + ( const C_Complex PCREF src ) const;
    
    /// Subtraction
    const C_Complex         operator - ( const C_Complex PCREF src ) const;
    
    /// Multiplication
    const C_Complex         operator * ( const C_Complex PCREF src ) const;
    
    /// Division
    const C_Complex         operator / ( const C_Complex PCREF src ) const;
    
    /// Sqrt
    const C_Complex         Sqrt() const;
    
    /// Cubic Root
    const C_CubicResult     CubeRoot() const;
    
private:
    
    /// Private Explicit HVec2 Constructor
    explicit C_Complex( const HVec2 PCREF ri );
    
    HVec2                   m_RI;
};



class C_Complex64
{
public:
    
    /// Default Constructor
    C_Complex64();
    
    /// Copy Constructor
    C_Complex64( const C_Complex64& copy );
    
    /// F64 Constructor
    C_Complex64( const F64 real, const F64 imag = 0.0 );
    
    /// Accessor: Real Component 
    const F64               Real() const;
    
    /// Accessor: Imaginary Component 
    const F64               Imag() const;
    
    /// Accessor: Modulus
    const F64               Modulus() const;
    
    /// Accessor: Phase
    const F64               Phase() const;
    
    /// Copy Assignment Operator
    void                    operator = ( const C_Complex64& copy );
    
    /// Conjugate
    const C_Complex64       operator ! () const;
    
    /// Unary Negation
    const C_Complex64       operator - () const;
    
    /// Addition
    const C_Complex64       operator + ( const C_Complex64 src ) const;
    
    /// Subtraction
    const C_Complex64       operator - ( const C_Complex64 src ) const;
    
    /// Multiplication
    const C_Complex64       operator * ( const C_Complex64 src ) const;
    
    /// Division
    const C_Complex64       operator / ( const C_Complex64 src ) const;
    
    /// Sqrt
    const C_Complex64       Sqrt() const;
    
    /// Cubic Root
    const C_CubicResult64   CubeRoot() const;
    
private:
    
    F64                     m_Real;
    F64                     m_Imag;
};



class C_CubicResult
{
public:
    
    C_CubicResult( const C_Complex& root0, const C_Complex& root1, const C_Complex& root2 );
    
public:
    
    C_Complex m_Root0;
    C_Complex m_Root1;
    C_Complex m_Root2;
};


class C_CubicResult64
{
public:
    
    C_CubicResult64( const C_Complex64& root0, const C_Complex64& root1, const C_Complex64& root2 );
    
public:
    
    C_Complex64 m_Root0;
    C_Complex64 m_Root1;
    C_Complex64 m_Root2;
};


/// Cubic Equation Solver
const C_CubicResult SolveCubic( const HF32 PCREF a0, const HF32 PCREF a1, const HF32 PCREF a2, const HF32 PCREF a3 );








} // namespace Maths




