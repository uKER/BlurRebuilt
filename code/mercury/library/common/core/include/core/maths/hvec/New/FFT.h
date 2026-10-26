#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_FFT_H
#define SOFT_MATHS_HVEC_NEW_FFT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_FFT_H
#define MATHS_HVEC_NEW_FFT_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#define USING_CLIB_SINCOS
// required size for SpectralReverb for now
#define USING_1024_FFT

namespace Maths
{
/// @addtogroup FFT
/// @{

DEPRECATED void RealFFT_Split0( F32* const __restrict pData, const S32 n );
DEPRECATED void IRealFFT_Split0( F32* const __restrict pData, const S32 n );

void RealFFT_Split( F32* const __restrict pData, const S32 n );
void IRealFFT_Split( F32* const __restrict pData, const S32 n );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


