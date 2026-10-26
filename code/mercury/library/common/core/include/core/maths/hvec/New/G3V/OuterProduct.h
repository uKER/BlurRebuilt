#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_OUTER_PRODUCT_H
#define SOFT_MATHS_HVEC_NEW_G3V_OUTER_PRODUCT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_OUTER_PRODUCT_H
#define MATHS_HVEC_NEW_G3V_OUTER_PRODUCT_H
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
NOALIAS __forceinline const VF32 operator ^ ( const VF32 VECREF a, const VF32 VECREF b );

/// Outer Product Operator: VF32 ^ Vector: commutes -> a^b = b^a
NOALIAS __forceinline const Vector operator ^ ( const VF32 VECREF a, const Vector PCREF b );

/// Outer Product Operator: Vector ^ VF32: commutes -> a^b = b^a
NOALIAS __forceinline const Vector operator ^ ( const Vector PCREF a, const VF32 VECREF b );

/// Outer Product Operator: VF32 ^ BiVector: commutes -> a^b = b^a
NOALIAS __forceinline const BiVector operator ^ ( const VF32 VECREF a, const BiVector PCREF b );

/// Outer Product Operator: BiVector ^ VF32: commutes -> a^b = b^a
NOALIAS __forceinline const BiVector operator ^ ( const BiVector PCREF a, const VF32 VECREF b );

/// Outer Product Operator: VF32 ^ TriVector: commutes -> a^b = b^a
NOALIAS __forceinline const TriVector operator ^ ( const VF32 VECREF a, const TriVector PCREF b );

/// Outer Product Operator: TriVector ^ VF32: commutes -> a^b = b^a
NOALIAS __forceinline const TriVector operator ^ ( const TriVector PCREF a, const VF32 VECREF b );

/// Outer Product Operator: Vector ^ Vector: anticommutes -> a^b = -b^a
NOALIAS __forceinline const BiVector operator ^ ( const Vector PCREF a, const Vector PCREF b );

/// Outer Product Operator: Vector ^ BiVector: commutes -> a^b = b^a
NOALIAS __forceinline const TriVector operator ^ ( const Vector PCREF a, const BiVector PCREF b );

/// Outer Product Operator: BiVector ^ Vector: commutes -> a^b = b^a
NOALIAS __forceinline const TriVector operator ^ ( const BiVector PCREF a, const Vector PCREF b );


/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


