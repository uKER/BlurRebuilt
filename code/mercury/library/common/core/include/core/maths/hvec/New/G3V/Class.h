#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_CLASS_H
#define SOFT_MATHS_HVEC_NEW_G3V_CLASS_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_CLASS_H
#define MATHS_HVEC_NEW_G3V_CLASS_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V /// Vector Space 3d Geometric Algebra
{
/// @addtogroup G3V
/// @{
/// G3V Multiplication Table
/// @code
///               |  S    |           V           |           B           |   T   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | LHS * RHS | 1     | e1    | e2    | e3    | e12   | e23   | e31   | e123  |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// S | 1         | 1     | e1    | e2    | e3    | e12   | e23   | e31   | e123  |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | e1        | e1    | 1     | +e12  | -e31  | +e2   | e123  | -e3   | e23   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// V | e2        | e2    | -e12  | 1     | +e23  | -e1   | +e3   | e123  | e31   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | e3        | e3    | +e31  | -e23  | 1     | e123  | -e2   | e1    | e12   |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | e12       | e12   | -e2   | +e1   | e123  | -1    | -e31  | e23   | -e3   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// B | e23       | e23   | e123  | -e3   | +e2   | +e31  | -1    | -e12  | -e1   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | e31       | e31   | +e3   | e123  | -e1   | -e23  | +e12  | -1    | -e2   |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// T | e123      | e123  | e23   | e31   | e12   | -e3   | -e1   | -e2   | -1    |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// @endcode

/// Enum for Vector element indices
enum VectorElementIndex
{
    e1 = 0,
    e2 = 1,
    e3 = 2
};

/// Enum for BiVector element indices
enum BiVectorElementIndex
{
    e12 = 0,
    e23 = 1,
    e31 = 2
};

/// Enum for MultiVector element indices
enum MultiVectorElementIndex
{
    mv_e1   = 0,
    mv_e2   = 1,
    mv_e3   = 2,
    mv_1    = 3,
    mv_e12  = 4,
    mv_e23  = 5,
    mv_e31  = 6,
    mv_e123 = 7
};


/// Vector space 3d vector class ( grade 1 )
class ALIGN16 PASSINREG DARWIN64_ABI Vector
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vector() {}                                                                      ///< Vector Default Constructor: undefined
    NOALIAS __forceinline Vector( const Vector& copy ) : m_V( copy.m_V ) {}                                ///< Vector Copy Constructor
    NOALIAS __forceinline const Vector& operator = ( const Vector& copy ) { m_V = copy.m_V; return copy; } ///< Vector Copy Assignment
    #endif
    
    Vector3 m_V; ///< G3V Vector: X = vector.x, Y = vector.y, Z = vector.z, W = undefined
};


/// Vector space 3d bivector class ( grade 2 )
class ALIGN16 PASSINREG DARWIN64_ABI BiVector
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline BiVector() {}                                                                        ///< BiVector Default Constructor: undefined
    NOALIAS __forceinline BiVector( const BiVector& copy ) : m_B( copy.m_B ) {}                                ///< BiVector Copy Constructor
    NOALIAS __forceinline const BiVector& operator = ( const BiVector& copy ) { m_B = copy.m_B; return copy; } ///< BiVector Copy Assignment
    #endif
    
    Vector3 m_B; ///< G3V BiVector: X = bivector.x, Y = bivector.y, Z = bivector.z, W = undefined
};


/// Vector space 3d trivector class ( grade 3 )
class ALIGN16 PASSINREG DARWIN64_ABI TriVector
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline TriVector() {}                                                                         ///< TriVector Default Constructor: undefined
    NOALIAS __forceinline TriVector( const TriVector& copy ) : m_T( copy.m_T ) {}                                ///< TriVector Copy Constructor
    NOALIAS __forceinline const TriVector& operator = ( const TriVector& copy ) { m_T = copy.m_T; return copy; } ///< TriVector Copy Assignment
    #endif
    
    VF32 m_T; ///< G3V TriVector: X = Y = Z = W = trivector/pseudoscalar
};


/// Vector space 3d multivector class containing all grades
class ALIGN16 DARWIN64_ABI MultiVector
{
public:
    
    NOALIAS __forceinline MultiVector();                                             ///< MultiVector Default Constructor: undefined
    NOALIAS __forceinline MultiVector( const MultiVector& copy );                    ///< MultiVector Copy Constructor
    NOALIAS __forceinline const MultiVector& operator = ( const MultiVector& copy ); ///< MultiVector Copy Assignment
    
    Vector4 m_VS; ///< G3V MultiVector: vector & scalar      : X = vector.x,   Y = vector.y,   Z = vector.z,   W = scalar
    Vector4 m_BT; ///< G3V MultiVector: bivector & trivector : X = bivector.x, Y = bivector.y, Z = bivector.z, W = trivector/pseudoscalar
};


/// Vector space 3d multivector class containing only grades 0 & 1 ( scalar & vector )
class ALIGN16 PASSINREG DARWIN64_ABI MultiVecSV
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline MultiVecSV() {}                                                                            ///< MultiVecSV Default Constructor: undefined
    NOALIAS __forceinline MultiVecSV( const MultiVecSV& copy ) : m_SV( copy.m_SV ) {}                                ///< MultiVecSV Copy Constructor
    NOALIAS __forceinline const MultiVecSV& operator = ( const MultiVecSV& copy ) { m_SV = copy.m_SV; return copy; } ///< MultiVecSV Copy Assignment
    #endif
    
    Vector4 m_SV; ///< G3V MultiVecSV: vector & scalar : X = vector.x, Y = vector.y, Z = vector.z, W = scalar
};


/// Vector space 3d multivector class containing only grades 0 & 2 ( scalar & bivector )
class ALIGN16 PASSINREG DARWIN64_ABI MultiVecSB
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline MultiVecSB() {}                                                                            ///< MultiVecSB Default Constructor: undefined
    NOALIAS __forceinline MultiVecSB( const MultiVecSB& copy ) : m_SB( copy.m_SB ) {}                                ///< MultiVecSB Copy Constructor
    NOALIAS __forceinline const MultiVecSB& operator = ( const MultiVecSB& copy ) { m_SB = copy.m_SB; return copy; } ///< MultiVecSB Copy Assignment
    #endif
    
    Vector4 m_SB; ///< G3V MultiVecSB: bivector & scalar : X = bivector.x, Y = bivector.y, Z = bivector.z, W = scalar
};


/// Vector space 3d multivector class containing only grades 0 & 3 ( scalar & trivector )
class ALIGN16 PASSINREG DARWIN64_ABI MultiVecST
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline MultiVecST() {}                                                                            ///< MultiVecST Default Constructor: undefined
    NOALIAS __forceinline MultiVecST( const MultiVecST& copy ) : m_ST( copy.m_ST ) {}                                ///< MultiVecST Copy Constructor
    NOALIAS __forceinline const MultiVecST& operator = ( const MultiVecST& copy ) { m_ST = copy.m_ST; return copy; } ///< MultiVecST Copy Assignment
    #endif
    
    Vec4F32 m_ST; ///< G3V MultiVecST: scalar & trivector : X = scalar, Y = trivector, Z = W = undefined
};


/// Vector space 3d multivector class containing only grades 1 & 2 ( vector & bivector )
class ALIGN16 DARWIN64_ABI MultiVecVB
{
public:
    
    NOALIAS __forceinline MultiVecVB();                                            ///< MultiVecVB Default Constructor: undefined
    NOALIAS __forceinline MultiVecVB( const MultiVecVB& copy );                    ///< MultiVecVB Copy Constructor
    NOALIAS __forceinline const MultiVecVB& operator = ( const MultiVecVB& copy ); ///< MultiVecVB Copy Assignment
    
    Vector3 m_V; ///< G3V MultiVecVB: vector   : X = vector.x,   Y = vector.y,   Z = vector.z,   W = undefined
    Vector3 m_B; ///< G3V MultiVecVB: bivector : X = bivector.x, Y = bivector.y, Z = bivector.z, W = undefined
};


/// Vector space 3d multivector class containing only grades 1 & 3 ( vector & trivector )
class ALIGN16 PASSINREG DARWIN64_ABI MultiVecVT
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline MultiVecVT() {}                                                                            ///< MultiVecVT Default Constructor: undefined
    NOALIAS __forceinline MultiVecVT( const MultiVecVT& copy ) : m_VT( copy.m_VT ) {}                                ///< MultiVecVT Copy Constructor
    NOALIAS __forceinline const MultiVecVT& operator = ( const MultiVecVT& copy ) { m_VT = copy.m_VT; return copy; } ///< MultiVecVT Copy Assignment
    #endif
    
    Vector4 m_VT; ///< G3V MultiVecVT: vector & trivector : X = vector.x, Y = vector.y, Z = vector.z, W = trivector/pseudoscalar
};


/// Vector space 3d multivector class containing only grades 2 & 3 ( bivector & trivector )
class ALIGN16 PASSINREG DARWIN64_ABI MultiVecBT
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline MultiVecBT() {}                                                                            ///< MultiVecBT Default Constructor: undefined
    NOALIAS __forceinline MultiVecBT( const MultiVecBT& copy ) : m_BT( copy.m_BT ) {}                                ///< MultiVecBT Copy Constructor
    NOALIAS __forceinline const MultiVecBT& operator = ( const MultiVecBT& copy ) { m_BT = copy.m_BT; return copy; } ///< MultiVecBT Copy Assignment
    #endif
    
    Vector4 m_BT; ///< G3V MultiVecBT: bivector & trivector : X = bivector.x, Y = bivector.y, Z = bivector.z, W = trivector/pseudoscalar
};


/// @}
}; // namespace G3V


#if 0 // TODO: MOVE THESE INTO SEPARATE HEADER!

namespace H3GA /// Homogeneous 3d Geometric Algebra
{
/// @addtogroup G3H
/// @{

/// @}
}; // namespace G3H

namespace C3GA /// Conformal 3d Geometric Algebra
{
/// @addtogroup G3C
/// @{

/// @}
}; // namespace G3C

#endif

}; // namespace Maths


#endif // #if defined( INCLUDE_THIS )


