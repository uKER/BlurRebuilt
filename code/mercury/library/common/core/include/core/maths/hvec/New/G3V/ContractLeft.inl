#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_LEFT_CONTRACT_INL
#define SOFT_MATHS_HVEC_NEW_G3V_LEFT_CONTRACT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_LEFT_CONTRACT_INL
#define MATHS_HVEC_NEW_G3V_LEFT_CONTRACT_INL
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

/// Left Contraction Operator: Scalar << Scalar
NOALIAS __forceinline const VF32 operator << ( const VF32 VECREF a, const VF32 VECREF b )
{
    /// @code
    ///                |  S    |
    ///   +------------+-------+
    ///   | LHS << RHS | 1     |
    /// --+------------+-------+
    /// S | 1          | 1     |
    /// --+------------+-------+
    /// @endcode
    
    return a * b; // scalar product!
}

/// Left Contraction Operator: Scalar << Vector
NOALIAS __forceinline const Vector operator << ( const VF32 VECREF a, const Vector PCREF b )
{
    /// @code
    ///                |           V           |
    ///   +------------+-------+-------+-------+
    ///   | LHS << RHS | e1    | e2    | e3    |
    /// --+------------+-------+-------+-------+
    /// S | 1          | e1    | e2    | e3    |
    /// --+------------+-------+-------+-------+
    /// @endcode
    
    //return MakeVector( a * b.m_V ); // same as the geometric product!
    return a * b;
}

/// Left Contraction Operator: Scalar << BiVector
NOALIAS __forceinline const BiVector operator << ( const VF32 VECREF a, const BiVector PCREF b )
{
    /// @code
    ///                |           B           |
    ///   +------------+-------+-------+-------+
    ///   | LHS << RHS | e12   | e23   | e31   |
    /// --+------------+-------+-------+-------+
    /// S | 1          | e12   | e23   | e31   |
    /// --+------------+-------+-------+-------+
    /// @endcode
    
    //return MakeBiVector( a * b.m_B ); // same as the geometric product!
    return a * b;
}

/// Left Contraction Operator: Scalar << TriVector
NOALIAS __forceinline const TriVector operator << ( const VF32 VECREF a, const TriVector PCREF b )
{
    /// @code
    ///                |   T   |
    ///   +------------+-------+
    ///   | LHS << RHS | e123  |
    /// --+------------+-------+
    /// S | 1          | e123  |
    /// --+------------+-------+
    /// @endcode
    
    //return MakeTriVector( a * b.m_T ); // same as the geometric product!
    return a * b;
}


/// Left Contraction Operator: Vector << Vector
NOALIAS __forceinline const VF32 operator << ( const Vector PCREF a, const Vector PCREF b )
{
    /// @code
    ///                |           V           |
    ///   +------------+-------+-------+-------+
    ///   | LHS << RHS | e1    | e2    | e3    |
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

/// Left Contraction Operator: Vector << BiVector
NOALIAS __forceinline const Vector operator << ( const Vector PCREF a, const BiVector PCREF b )
{
    /// @code
    ///                |           B           |
    ///   +------------+-------+-------+-------+
    ///   | LHS << RHS | e12   | e23   | e31   |
    /// --+------------+-------+-------+-------+
    ///   | e1         | +e2   |       | -e3   |
    ///   +------------+-------+-------+-------+
    /// V | e2         | -e1   | +e3   |       |
    ///   +------------+-------+-------+-------+
    ///   | e3         |       | -e2   | e1    |
    /// --+------------+-------+-------+-------+
    /// @endcode
    
    // result.e1 = (a.e3 * b.e31) - (a.e2 * b.e12)
    // result.e2 = (a.e1 * b.e12) - (a.e3 * b.e23)
    // result.e3 = (a.e2 * b.e23) - (a.e1 * b.e31)
    
    return MakeVector( ( VPermute<e3,e1,e2>( a ) * BPermute<e31,e12,e23>( b ) )
                      -( VPermute<e2,e3,e1>( a ) * BPermute<e12,e23,e31>( b ) ) ); // TODO: Use NegMulSub()!
}

/// Left Contraction Operator: Vector << TriVector
NOALIAS __forceinline const BiVector operator << ( const Vector PCREF a, const TriVector PCREF b )
{
    /// @code
    ///                |   T   |
    ///   +------------+-------+
    ///   | LHS << RHS | e123  |
    /// --+------------+-------+
    ///   | e1         | e23   |
    ///   +------------+-------+
    /// V | e2         | e31   |
    ///   +------------+-------+
    ///   | e3         | e12   |
    /// --+------------+-------+
    /// @endcode
    
    //return MakeBiVector( Permute<e3,e1,e2>( a ) * b.m_T ); // same as the geometric product!
    return a * b;
}


/// Left Contraction Operator: BiVector << BiVector
NOALIAS __forceinline const VF32 operator << ( const BiVector PCREF a, const BiVector PCREF b )
{
    /// @code
    ///                |           B           |
    ///   +------------+-------+-------+-------+
    ///   | LHS << RHS | e12   | e23   | e31   |
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

/// Left Contraction Operator: BiVector << TriVector
NOALIAS __forceinline const Vector operator << ( const BiVector PCREF a, const TriVector PCREF b )
{
    /// @code
    ///                |   T   |
    ///   +------------+-------+
    ///   | LHS << RHS | e123  |
    /// --+------------+-------+
    ///   | e12        | -e3   |
    ///   +------------+-------+
    /// B | e23        | -e1   |
    ///   +------------+-------+
    ///   | e31        | -e2   |
    /// --+------------+-------+
    /// @endcode
    
    //return MakeVector( -Permute<e23,e31,e12>( a ) * b.m_T ); // same as the geometric product!
    return a * b;
}


/// Left Contraction Operator: TriVector << TriVector
NOALIAS __forceinline const VF32 operator << ( const TriVector PCREF a, const TriVector PCREF b )
{
    /// @code
    ///                |   T   |
    ///   +------------+-------+
    ///   | LHS << RHS | e123  |
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


