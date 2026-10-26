#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECINT16_H
#define SOFT_MATHS_HVEC_NEW_VECINT16_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECINT16_H
#define MATHS_HVEC_NEW_VECINT16_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS


namespace Maths
{
/// @addtogroup IntegerVector
/// @{

/// Vec8S16: Load first four elements from Short4, last 4 elements are undefined!
NOALIAS __forceinline const Vec8S16 Load( MathsStorage::Short4 const * const __restrict pSrc );

/// Vec8S16: Store first four elements into Short4
NOALIAS __forceinline void Store( const Vec8S16 PCREF data, MathsStorage::Short4 * const __restrict pDst );

/// Vec8S16: Load first four elements from ShortN4, last 4 elements are undefined!
NOALIAS __forceinline const Vec8S16 Load( MathsStorage::ShortN4 const * const __restrict pSrc );

/// Vec8S16: Store first four elements into ShortN4
NOALIAS __forceinline void Store( const Vec8S16 PCREF data, MathsStorage::ShortN4 * const __restrict pDst );


/// Pack two Vec8S16's into one Vec16S8 with Signed Saturation to [ -2^7, +2^7-1 ]
NOALIAS __forceinline const Vec16S8 PackSS( const Vec8S16 PCREF a, const Vec8S16 PCREF b );

/// Pack two Vec8S16's into one Vec16U8 with Unsigned Saturation to [ 0, +2^8-1 ]
NOALIAS __forceinline const Vec16U8 PackUS( const Vec8S16 PCREF a, const Vec8S16 PCREF b );

/// Unpack Vec16U8 into two Vec8S16's
NOALIAS __forceinline void Unpack( const Vec16U8 PCREF a, Vec8S16* const __restrict pB, Vec8S16* const __restrict pC );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


