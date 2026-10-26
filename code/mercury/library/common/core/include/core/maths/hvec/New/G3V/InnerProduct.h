#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_INNER_PRODUCT_H
#define SOFT_MATHS_HVEC_NEW_G3V_INNER_PRODUCT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_INNER_PRODUCT_H
#define MATHS_HVEC_NEW_G3V_INNER_PRODUCT_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_InnerProduct
/// @{

/// Hestenes Inner Product Multiplication Table
/// Result is a blade of grade: abs( grade(LHS) - grade(RHS) )
/// NOTE: Any product involving a scalar is zero!
/// @code
///               |  S    |           V           |           B           |   T   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | LHS | RHS | 1     | e1    | e2    | e3    | e12   | e23   | e31   | e123  |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// S | 1         | 0     | 0     | 0     | 0     | 0     | 0     | 0     | 0     |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | e1        | 0     | 1     | +e12  | -e31  | +e2   | e123  | -e3   | e23   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// V | e2        | 0     | -e12  | 1     | +e23  | -e1   | +e3   | e123  | e31   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | e3        | 0     | +e31  | -e23  | 1     | e123  | -e2   | e1    | e12   |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | e12       | 0     | -e2   | +e1   | e123  | -1    | -e31  | e23   | -e3   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// B | e23       | 0     | e123  | -e3   | +e2   | +e31  | -1    | -e12  | -e1   |
///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
///   | e31       | 0     | +e3   | e123  | -e1   | -e23  | +e12  | -1    | -e2   |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// T | e123      | 0     | e23   | e31   | e12   | -e3   | -e1   | -e2   | -1    |
/// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
/// @endcode


/// Hestenes Inner Product Operator: Scalar | Scalar
//NOALIAS __forceinline const VF32 operator | ( const VF32 VECREF a, const VF32 VECREF b ); // Zero!

/// Hestenes Inner Product Operator: Scalar | Vector
//NOALIAS __forceinline const Vector operator | ( const VF32 VECREF a, const Vector PCREF b ); // Zero!

/// Hestenes Inner Product Operator: Scalar | BiVector
//NOALIAS __forceinline const BiVector operator | ( const VF32 VECREF a, const BiVector PCREF b ); // Zero!

/// Hestenes Inner Product Operator: Scalar | TriVector
//NOALIAS __forceinline const TriVector operator | ( const VF32 VECREF a, const TriVector PCREF b ); // Zero!

/// Hestenes Inner Product Operator: Vector | Scalar
//NOALIAS __forceinline const Vector operator | ( const Vector PCREF a, const VF32 VECREF b ); // Zero!

/// Hestenes Inner Product Operator: BiVector | Scalar
//NOALIAS __forceinline const BiVector operator | ( const BiVector PCREF a, const VF32 VECREF b ); // Zero!

/// Hestenes Inner Product Operator: TriVector | Scalar
//NOALIAS __forceinline const TriVector operator | ( const TriVector PCREF a, const VF32 VECREF b ); // Zero!


/// Hestenes Inner Product Operator: Vector | Vector
NOALIAS __forceinline const VF32 operator | ( const Vector PCREF a, const Vector PCREF b );

/// Hestenes Inner Product Operator: Vector | BiVector
NOALIAS __forceinline const Vector operator | ( const Vector PCREF a, const BiVector PCREF b );

/// Hestenes Inner Product Operator: Vector | TriVector
NOALIAS __forceinline const BiVector operator | ( const Vector PCREF a, const TriVector PCREF b );

/// Hestenes Inner Product Operator: BiVector | Vector
NOALIAS __forceinline const Vector operator | ( const BiVector PCREF a, const Vector PCREF b );

/// Hestenes Inner Product Operator: TriVector | Vector
NOALIAS __forceinline const BiVector operator | ( const TriVector PCREF a, const Vector PCREF b );


/// Hestenes Inner Product Operator: BiVector | BiVector
NOALIAS __forceinline const VF32 operator | ( const BiVector PCREF a, const BiVector PCREF b );

/// Hestenes Inner Product Operator: BiVector | TriVector
NOALIAS __forceinline const Vector operator | ( const BiVector PCREF a, const TriVector PCREF b );

/// Hestenes Inner Product Operator: TriVector | BiVector
NOALIAS __forceinline const Vector operator | ( const TriVector PCREF a, const BiVector PCREF b );


/// Hestenes Inner Product Operator: TriVector | TriVector
NOALIAS __forceinline const VF32 operator | ( const TriVector PCREF a, const TriVector PCREF b );





/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


