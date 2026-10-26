#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_COMPLEX_H
#define SOFT_MATHS_HVEC_NEW_CLASS_COMPLEX_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_COMPLEX_H
#define MATHS_HVEC_NEW_CLASS_COMPLEX_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Complex
/// @{

/// Complex: Vector register complex number class.
class ALIGN16 PASSINREG DARWIN64_ABI Complex
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    
    /// Complex Default Constructor: Undefined
    NOALIAS __forceinline Complex() {}
    
    /// Complex Copy Constructor
    NOALIAS __forceinline Complex( const Complex& copy ) : m_V( copy.m_V ) {}
    
    /// Complex Copy Assignment Operator
    NOALIAS __forceinline const Complex& operator = ( const Complex& copy ) { m_V = copy.m_V; return copy; }
    
    #endif
    
    Vec4F32 m_V; ///< m_V.x = real part, m_V.y = imaginary part
};





/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


