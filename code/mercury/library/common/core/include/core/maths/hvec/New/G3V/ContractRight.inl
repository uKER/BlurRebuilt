#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_RIGHT_CONTRACT_INL
#define SOFT_MATHS_HVEC_NEW_G3V_RIGHT_CONTRACT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_RIGHT_CONTRACT_INL
#define MATHS_HVEC_NEW_G3V_RIGHT_CONTRACT_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_Contraction
/// @{

/// Right Contraction Operator: Scalar >> Scalar
NOALIAS __forceinline const VF32 operator >> ( const VF32 VECREF a, const VF32 VECREF b )
{
    /// @code
    ///                |  S    |
    ///   +------------+-------+
    ///   | LHS >> RHS | 1     |
    /// --+------------+-------+
    /// S | 1          | 1     |
    /// --+------------+-------+
    /// @endcode
    
    return a * b; // scalar product!
}

/// Right Contraction Operator: Vector >> Scalar
NOALIAS __forceinline const Vector operator >> ( const Vector PCREF a, const VF32 VECREF b )
{
    /// @code
    ///                |  S    |
    ///   +------------+-------+
    ///   | LHS >> RHS | 1     |
    /// --+------------+-------+
    ///   | e1         | e1    |
    ///   +------------+-------+
    /// V | e2         | e2    |
    ///   +------------+-------+
    ///   | e3         | e3    |
    /// --+------------+-------+
    /// @endcode
    
    //return MakeVector( a.m_V * b ); // same as the geometric product!
    return a * b;
}

/// Right Contraction Operator: BiVector >> Scalar
NOALIAS __forceinline const BiVector operator >> ( const BiVector PCREF a, const VF32 VECREF b )
{
    /// @code
    ///                |  S    |
    ///   +------------+-------+
    ///   | LHS >> RHS | 1     |
    /// --+------------+-------+
    ///   | e12        | e12   |
    ///   +------------+-------+
    /// B | e23        | e23   |
    ///   +------------+-------+
    ///   | e31        | e31   |
    /// --+------------+-------+
    /// @endcode
    
    //return MakeBiVector( a.m_B * b ); // same as the geometric product!
    return a * b;
}

/// Right Contraction Operator: TriVector >> Scalar
NOALIAS __forceinline const TriVector operator >> ( const TriVector PCREF a, const VF32 VECREF b )
{
    /// @code
    ///                |  S    |
    ///   +------------+-------+
    ///   | LHS >> RHS | 1     |
    /// --+------------+-------+
    /// T | e123       | e123  |
    /// --+------------+-------+
    /// @endcode
    
    //return MakeTriVector( a.m_T * b ); // same as the geometric product!
    return a * b;
}


/// Right Contraction Operator: Vector >> Vector
NOALIAS __forceinline const VF32 operator >> ( const Vector PCREF a, const Vector PCREF b )
{
    /// @code
    ///                |           V           |
    ///   +------------+-------+-------+-------+
    ///   | LHS >> RHS | e1    | e2    | e3    |
    /// --+------------+-------+-------+-------+
    ///   | e1         | 1     |       |       |
    ///   +------------+-------+-------+-------+
    /// V | e2         |       | 1     |       |
    ///   +------------+-------+-------+-------+
    ///   | e3         |       |       | 1     |
    /// --+------------+-------+-------+-------+
    /// @endcode
    
    //return Dot3( a.m_V, b.m_V ); // scalar product!
    return a % b;
}

/// Right Contraction Operator: BiVector >> Vector
NOALIAS __forceinline const Vector operator >> ( const BiVector PCREF a, const Vector PCREF b )
{
    /// @code
    ///                |           V           |
    ///   +------------+-------+-------+-------+
    ///   | LHS >> RHS | e1    | e2    | e3    |
    /// --+------------+-------+-------+-------+
    ///   | e12        | -e2   | +e1   |       |
    ///   +------------+-------+-------+-------+
    /// B | e23        |       | -e3   | +e2   |
    ///   +------------+-------+-------+-------+
    ///   | e31        | +e3   |       | -e1   |
    /// --+------------+-------+-------+-------+
    /// @endcode
    
    // result.e1 = (a.e12 * b.e2) - (a.e31 * b.e3)
    // result.e2 = (a.e23 * b.e3) - (a.e12 * b.e1)
    // result.e3 = (a.e31 * b.e1) - (a.e23 * b.e2)
    
    return MakeVector( ( BPermute<e12,e23,e31>( a ) * VPermute<e2,e3,e1>( b ) )
                      -( BPermute<e31,e12,e23>( a ) * VPermute<e3,e1,e2>( b ) ) );
}

/// Right Contraction Operator: TriVector >> Vector
NOALIAS __forceinline const BiVector operator >> ( const TriVector PCREF a, const Vector PCREF b )
{
    /// @code
    ///                |           V           |
    ///   +------------+-------+-------+-------+
    ///   | LHS >> RHS | e1    | e2    | e3    |
    /// --+------------+-------+-------+-------+
    /// T | e123       | e23   | e31   | e12   |
    /// --+------------+-------+-------+-------+
    /// @endcode
    
    //return MakeBiVector( a.m_T * Permute<e3,e1,e2>( b ) ); // same as the geometric product!
    return a * b;
}

/// Right Contraction Operator: BiVector >> BiVector
NOALIAS __forceinline const VF32 operator >> ( const BiVector PCREF a, const BiVector PCREF b )
{
    /// @code
    ///                |           B           |
    ///   +------------+-------+-------+-------+
    ///   | LHS >> RHS | e12   | e23   | e31   |
    /// --+------------+-------+-------+-------+
    ///   | e12        | -1    |       |       |
    ///   +------------+-------+-------+-------+
    /// B | e23        |       | -1    |       |
    ///   +------------+-------+-------+-------+
    ///   | e31        |       |       | -1    |
    /// --+------------+-------+-------+-------+
    /// @endcode
    
    //return -Dot3( a.m_B, b.m_B ); // scalar product!
    return a % b;
}

/// Right Contraction Operator: TriVector >> BiVector
NOALIAS __forceinline const Vector operator >> ( const TriVector PCREF a, const BiVector PCREF b )
{
    /// @code
    ///                |           B           |
    ///   +------------+-------+-------+-------+
    ///   | LHS >> RHS | e12   | e23   | e31   |
    /// --+------------+-------+-------+-------+
    /// T | e123       | -e3   | -e1   | -e2   |
    /// --+------------+-------+-------+-------+
    /// @endcode
    
    //return MakeVector( -a.m_T * Permute<e23,e31,e12>( b ) ); // same as the geometric product!
    return a * b;
}


/// Right Contraction Operator: TriVector >> TriVector
NOALIAS __forceinline const VF32 operator >> ( const TriVector PCREF a, const TriVector PCREF b )
{
    /// @code
    ///                |   T   |
    ///   +------------+-------+
    ///   | LHS >> RHS | e123  |
    /// --+------------+-------+
    /// T | e123       | -1    |
    /// --+------------+-------+
    /// @endcode
    
    //return -( a.m_T * b.m_T ); // scalar product!
    //return a % b; // same as the geometric product!
    return a * b;
}



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


