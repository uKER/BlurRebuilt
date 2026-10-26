#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECINT32_H
#define SOFT_MATHS_HVEC_NEW_VECINT32_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECINT32_H
#define MATHS_HVEC_NEW_VECINT32_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup IntegerVector
/// @{

/// Make Vec4S32 from Vec4F32 with Signed Saturation to [ -2^31-1, +2^31-1 ]
/// WARNING: results are undefined for input F32's outside the range of an S32!
NOALIAS __forceinline const Vec4S32 MakeVec4S32( const Vec4F32 VECREF src );


/// Pack two Vec4S32's into one Vec8S16 with Signed Saturation to [ -2^15, +2^15-1 ]
NOALIAS __forceinline const Vec8S16 PackSS( const Vec4S32 PCREF a, const Vec4S32 PCREF b );

/// Pack two Vec4U32's into one Vec8U16 with Unsigned Saturation to [ 0, +2^16-1 ]
//NOALIAS __forceinline const Vec8U16 PackUS( const Vec4U32 PCREF a, const Vec4U32 PCREF b );

/// Unpack Vec8S16 into two Vec4S32's
NOALIAS __forceinline void Unpack( const Vec8S16 PCREF a, Vec4S32* const __restrict pB, Vec4S32* const __restrict pC );

/// Unpack Vec8U16 into two Vec4S32's
//NOALIAS __forceinline void Unpack( const Vec8U16 PCREF a, Vec4S32* const __restrict pB, Vec4S32* const __restrict pC );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


