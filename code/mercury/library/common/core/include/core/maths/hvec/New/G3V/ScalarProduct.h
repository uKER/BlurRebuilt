#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_SCALAR_PRODUCT_H
#define SOFT_MATHS_HVEC_NEW_G3V_SCALAR_PRODUCT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_SCALAR_PRODUCT_H
#define MATHS_HVEC_NEW_G3V_SCALAR_PRODUCT_H
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
NOALIAS __forceinline const VF32 operator % ( const VF32 VECREF a, const VF32 VECREF b );

/// Scalar Product Operator: Vector % Vector
NOALIAS __forceinline const VF32 operator % ( const Vector PCREF a, const Vector PCREF b );

/// Scalar Product Operator: BiVector % BiVector
NOALIAS __forceinline const VF32 operator % ( const BiVector PCREF a, const BiVector PCREF b );

/// Scalar Product Operator: TriVector % TriVector
NOALIAS __forceinline const VF32 operator % ( const TriVector PCREF a, const TriVector PCREF b );





/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


