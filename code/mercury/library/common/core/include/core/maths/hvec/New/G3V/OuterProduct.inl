#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_OUTER_PRODUCT_INL
#define SOFT_MATHS_HVEC_NEW_G3V_OUTER_PRODUCT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_OUTER_PRODUCT_INL
#define MATHS_HVEC_NEW_G3V_OUTER_PRODUCT_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_OuterProduct
/// @{

//
// Symmetry/Anti-symmetry property of the outer product:
//  Ak ^ Bl = -1^(k*l)(Bl ^ Ak), k=grade(A), l=grade(B)
// 

/// Outer Product Operator: VF32 ^ VF32: commutes -> a^b = b^a
NOALIAS __forceinline const VF32 operator ^ ( const VF32 VECREF a, const VF32 VECREF b )
{
    /// @code
    ///               |  S    |
    ///   +-----------+-------+
    ///   | LHS ^ RHS | 1     |
    /// --+-----------+-------+
    /// S | 1         | 1     |
    /// --+-----------+-------+
    /// @endcode
    
    return a * b;
}

/// Outer Product Operator: VF32 ^ Vector: commutes -> a^b = b^a
NOALIAS __forceinline const Vector operator ^ ( const VF32 VECREF a, const Vector PCREF b )
{
    /// @code
    ///               |           V           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS ^ RHS | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    /// S | 1         | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    //return MakeVector( a * b.m_V ); // same as the geometric product!
    return a * b;
}

/// Outer Product Operator: Vector ^ VF32: commutes -> a^b = b^a
NOALIAS __forceinline const Vector operator ^ ( const Vector PCREF a, const VF32 VECREF b )
{
    /// @code
    ///               |  S    |
    ///   +-----------+-------+
    ///   | LHS ^ RHS | 1     |
    /// --+-----------+-------+
    ///   | e1        | e1    |
    ///   +-----------+-------+
    /// V | e2        | e2    |
    ///   +-----------+-------+
    ///   | e3        | e3    |
    /// --+-----------+-------+
    /// @endcode
    
    //return MakeVector( a.m_V * b ); // same as the geometric product!
    return a * b;
}

/// Outer Product Operator: VF32 ^ BiVector: commutes -> a^b = b^a
NOALIAS __forceinline const BiVector operator ^ ( const VF32 VECREF a, const BiVector PCREF b )
{
    /// @code
    ///               |           B           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS ^ RHS | e12   | e23   | e31   |
    /// --+-----------+-------+-------+-------+
    /// S | 1         | e12   | e23   | e31   |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    //return MakeBiVector( a * b.m_B ); // same as the geometric product!
    return a * b;
}

/// Outer Product Operator: BiVector ^ VF32: commutes -> a^b = b^a
NOALIAS __forceinline const BiVector operator ^ ( const BiVector PCREF a, const VF32 VECREF b )
{
    /// @code
    ///               |  S    |
    ///   +-----------+-------+
    ///   | LHS ^ RHS | 1     |
    /// --+-----------+-------+
    ///   | e12       | e12   |
    ///   +-----------+-------+
    /// B | e23       | e23   |
    ///   +-----------+-------+
    ///   | e31       | e31   |
    /// --+-----------+-------+
    /// @endcode
    
    //return MakeBiVector( a.m_B * b ); // same as the geometric product!
    return a * b;
}

/// Outer Product Operator: VF32 ^ TriVector: commutes -> a^b = b^a
NOALIAS __forceinline const TriVector operator ^ ( const VF32 VECREF a, const TriVector PCREF b )
{
    /// @code
    ///               |   T   |
    ///   +-----------+-------+
    ///   | LHS ^ RHS | e123  |
    /// --+-----------+-------+
    /// S | 1         | e123  |
    /// --+-----------+-------+
    /// @endcode
    
    //return MakeTriVector( a * b.m_T ); // same as the geometric product!
    return a * b;
}

/// Outer Product Operator: TriVector ^ VF32: commutes -> a^b = b^a
NOALIAS __forceinline const TriVector operator ^ ( const TriVector PCREF a, const VF32 VECREF b )
{
    /// @code
    ///               |  S    |
    ///   +-----------+-------+
    ///   | LHS ^ RHS | 1     |
    /// --+-----------+-------+
    /// T | e123      | e123  |
    /// --+-----------+-------+
    /// @endcode
    
    //return MakeTriVector( a.m_T * b ); // same as the geometric product!
    return a * b;
}

/// Outer Product Operator: Vector ^ Vector: anticommutes -> a^b = -b^a
NOALIAS __forceinline const BiVector operator ^ ( const Vector PCREF a, const Vector PCREF b )
{
    /// @code
    ///               |           V           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS ^ RHS | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    ///   | e1        |       | +e12  | -e31  |
    ///   +-----------+-------+-------+-------+
    /// V | e2        | -e12  |       | +e23  |
    ///   +-----------+-------+-------+-------+
    ///   | e3        | +e31  | -e23  |       |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    // result.e12 = a.e1 * b.e2 - a.e2 * b.e1;
    // result.e23 = a.e2 * b.e3 - a.e3 * b.e2;
    // result.e31 = a.e3 * b.e1 - a.e1 * b.e3;
    
    BiVector result;
    //result.m_B = ( a.m_V * Permute<e2,e3,e1>( b ) )
    //           - ( b.m_V * Permute<e2,e3,e1>( a ) );
    result.m_B = NegMulSub( b.m_V,  VPermute<e2,e3,e1>( a ),
                            a.m_V * VPermute<e2,e3,e1>( b ) );
    return result;
}

/// Outer Product Operator: Vector ^ BiVector: commutes -> a^b = b^a
NOALIAS __forceinline const TriVector operator ^ ( const Vector PCREF a, const BiVector PCREF b )
{
    /// @code
    ///               |           B           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS ^ RHS | e12   | e23   | e31   |
    /// --+-----------+-------+-------+-------+
    ///   | e1        |       | e123  |       |
    ///   +-----------+-------+-------+-------+
    /// V | e2        |       |       | e123  |
    ///   +-----------+-------+-------+-------+
    ///   | e3        | e123  |       |       |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    // result.e123 = a.e1 * b.e23
    //             + a.e2 * b.e31
    //             + a.e3 * b.e12
    
    TriVector result;
    //result.m_T = Dot3( Permute<e3,e1,e2>( a ), b.m_B );
    result.m_T = Dot( a.m_V, BPermute<e23,e31,e12>( b ) );
    return result;
}

/// Outer Product Operator: BiVector ^ Vector: commutes -> a^b = b^a
NOALIAS __forceinline const TriVector operator ^ ( const BiVector PCREF a, const Vector PCREF b )
{
    /// @code
    ///               |           V           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS ^ RHS | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    ///   | e12       |       |       | e123  |
    ///   +-----------+-------+-------+-------+
    /// B | e23       | e123  |       |       |
    ///   +-----------+-------+-------+-------+
    ///   | e31       |       | e123  |       |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    TriVector result;
    result.m_T = Dot( a.m_B, VPermute<e3,e1,e2>( b ) );
    return result;
}



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


