#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CUBIC_H
#define SOFT_MATHS_HVEC_NEW_CUBIC_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CUBIC_H
#define MATHS_HVEC_NEW_CUBIC_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Cubic
/// @{

/// VF32 Real Cube Root
NOALIAS __forceinline const VF32 CubeRoot( const VF32 VECREF a );

/// Cubic Root Solver: a3.z^3 + a2.z^2 + a1.z + a0 = 0
NOALIAS __forceinline void SolveCubic(
    const VF32 VECREF in_a3,
    const VF32 VECREF in_a2,
    const VF32 VECREF in_a1,
    const VF32 VECREF in_a0,
    Complex* const __restrict pRoot1,
    Complex* const __restrict pRoot2,
    Complex* const __restrict pRoot3 );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


