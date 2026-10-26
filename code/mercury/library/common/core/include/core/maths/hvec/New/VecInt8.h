#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECINT8_H
#define SOFT_MATHS_HVEC_NEW_VECINT8_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECINT8_H
#define MATHS_HVEC_NEW_VECINT8_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup IntegerVector
/// @{

/// Vec16U8: Load first four elements from _UByte4, last 12 elements are undefined!
NOALIAS __forceinline const Vec16U8 Load( MathsStorage::_UByte4 const * const __restrict pSrc );

/// Vec16U8: Store first four elements into _UByte4
NOALIAS __forceinline void Store( const Vec16U8 PCREF data, MathsStorage::_UByte4 * const __restrict pDst );


/// Vec16U8: Load first four elements from _UByteN4, last 12 elements are undefined!
NOALIAS __forceinline const Vec16U8 Load( MathsStorage::_UByteN4 const * const __restrict pSrc );

/// Vec16U8: Store first four elements into _UByteN4
NOALIAS __forceinline void Store( const Vec16U8 PCREF data, MathsStorage::_UByteN4 * const __restrict pDst );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


