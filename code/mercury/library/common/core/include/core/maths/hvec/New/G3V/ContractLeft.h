#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_LEFT_CONTRACT_H
#define SOFT_MATHS_HVEC_NEW_G3V_LEFT_CONTRACT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_LEFT_CONTRACT_H
#define MATHS_HVEC_NEW_G3V_LEFT_CONTRACT_H
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
NOALIAS __forceinline const VF32 operator << ( const VF32 VECREF a, const VF32 VECREF b );

/// Left Contraction Operator: Scalar << Vector
NOALIAS __forceinline const Vector operator << ( const VF32 VECREF a, const Vector PCREF b );

/// Left Contraction Operator: Scalar << BiVector
NOALIAS __forceinline const BiVector operator << ( const VF32 VECREF a, const BiVector PCREF b );

/// Left Contraction Operator: Scalar << TriVector
NOALIAS __forceinline const TriVector operator << ( const VF32 VECREF a, const TriVector PCREF b );


/// Left Contraction Operator: Vector << Vector
NOALIAS __forceinline const VF32 operator << ( const Vector PCREF a, const Vector PCREF b );

/// Left Contraction Operator: Vector << BiVector
NOALIAS __forceinline const Vector operator << ( const Vector PCREF a, const BiVector PCREF b );

/// Left Contraction Operator: Vector << TriVector
NOALIAS __forceinline const BiVector operator << ( const Vector PCREF a, const TriVector PCREF b );


/// Left Contraction Operator: BiVector << BiVector
NOALIAS __forceinline const VF32 operator << ( const BiVector PCREF a, const BiVector PCREF b );

/// Left Contraction Operator: BiVector << TriVector
NOALIAS __forceinline const Vector operator << ( const BiVector PCREF a, const TriVector PCREF b );


/// Left Contraction Operator: TriVector << TriVector
NOALIAS __forceinline const VF32 operator << ( const TriVector PCREF a, const TriVector PCREF b );




/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


