#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_SCALAR_PRODUCT_INL
#define SOFT_MATHS_HVEC_NEW_G3V_SCALAR_PRODUCT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_SCALAR_PRODUCT_INL
#define MATHS_HVEC_NEW_G3V_SCALAR_PRODUCT_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_ScalarProduct
/// @{

/// Scalar Product Operator: Scalar % Scalar
NOALIAS __forceinline const VF32 operator % ( const VF32 VECREF a, const VF32 VECREF b )
{
    /// @code
    ///               |  S    |
    ///   +-----------+-------+
    ///   | LHS % RHS | 1     |
    /// --+-----------+-------+
    /// S | 1         | 1     |
    /// --+-----------+-------+
    /// @endcode
    
    return a * b;
}

/// Scalar Product Operator: Vector % Vector
NOALIAS __forceinline const VF32 operator % ( const Vector PCREF a, const Vector PCREF b )
{
    /// @code
    ///               |           V           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS % RHS | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    ///   | e1        | 1     |       |       |
    ///   +-----------+-------+-------+-------+
    /// V | e2        |       | 1     |       |
    ///   +-----------+-------+-------+-------+
    ///   | e3        |       |       | 1     |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    return Dot( a.m_V, b.m_V );
}

/// Scalar Product Operator: BiVector % BiVector
NOALIAS __forceinline const VF32 operator % ( const BiVector PCREF a, const BiVector PCREF b )
{
    /// @code
    ///               |           B           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS % RHS | e12   | e23   | e31   |
    /// --+-----------+-------+-------+-------+
    ///   | e12       | -1    |       |       |
    ///   +-----------+-------+-------+-------+
    /// B | e23       |       | -1    |       |
    ///   +-----------+-------+-------+-------+
    ///   | e31       |       |       | -1    |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    return -Dot( a.m_B, b.m_B );
}

/// Scalar Product Operator: TriVector % TriVector
NOALIAS __forceinline const VF32 operator % ( const TriVector PCREF a, const TriVector PCREF b )
{
    /// @code
    ///               |   T   |
    ///   +-----------+-------+
    ///   | LHS % RHS | e123  |
    /// --+-----------+-------+
    /// T | e123      | -1    |
    /// --+-----------+-------+
    /// @endcode
    
    //return -( a.m_T * b.m_T ); // same as the geometric product!
    return a * b;
}





/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


