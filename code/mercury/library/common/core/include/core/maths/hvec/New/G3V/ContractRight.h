#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_RIGHT_CONTRACT_H
#define SOFT_MATHS_HVEC_NEW_G3V_RIGHT_CONTRACT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_RIGHT_CONTRACT_H
#define MATHS_HVEC_NEW_G3V_RIGHT_CONTRACT_H
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
NOALIAS __forceinline const VF32 operator >> ( const VF32 VECREF a, const VF32 VECREF b );

/// Right Contraction Operator: Vector >> Scalar
NOALIAS __forceinline const Vector operator >> ( const Vector PCREF a, const VF32 VECREF b );

/// Right Contraction Operator: BiVector >> Scalar
NOALIAS __forceinline const BiVector operator >> ( const BiVector PCREF a, const VF32 VECREF b );

/// Right Contraction Operator: TriVector >> Scalar
NOALIAS __forceinline const TriVector operator >> ( const TriVector PCREF a, const VF32 VECREF b );


/// Right Contraction Operator: Vector >> Vector
NOALIAS __forceinline const VF32 operator >> ( const Vector PCREF a, const Vector PCREF b );

/// Right Contraction Operator: BiVector >> Vector
NOALIAS __forceinline const Vector operator >> ( const BiVector PCREF a, const Vector PCREF b );

/// Right Contraction Operator: TriVector >> Vector
NOALIAS __forceinline const BiVector operator >> ( const TriVector PCREF a, const Vector PCREF b );


/// Right Contraction Operator: BiVector >> BiVector
NOALIAS __forceinline const VF32 operator >> ( const BiVector PCREF a, const BiVector PCREF b );

/// Right Contraction Operator: TriVector >> BiVector
NOALIAS __forceinline const Vector operator >> ( const TriVector PCREF a, const BiVector PCREF b );


/// Right Contraction Operator: TriVector >> TriVector
NOALIAS __forceinline const VF32 operator >> ( const TriVector PCREF a, const TriVector PCREF b );





/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


