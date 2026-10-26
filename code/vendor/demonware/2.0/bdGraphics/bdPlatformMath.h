// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to store all the mathematical operations that bitdemon uses.

#ifndef BD_PLATFORM_MATH_H
#define BD_PLATFORM_MATH_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>


inline bdInt BD_CALL bdAbsi(const bdInt x);

inline bdFloat32 BD_CALL bdFabsf32(const bdFloat32 x);

inline bdFloat32 BD_CALL bdPowf32(const bdFloat32 x,
                                  const bdFloat32 y);

inline bdFloat32 BD_CALL bdSqrtf32(const bdFloat32 x);

inline bdFloat32 BD_CALL bdSinf32(const bdFloat32 x);

inline bdFloat32 BD_CALL bdCosf32(const bdFloat32 x);

inline bdFloat32 BD_CALL bdTanf32(const bdFloat32 x);

inline bdFloat32 BD_CALL bdAtanf32(const bdFloat32 x);


#include <bdGraphics/bdPlatformMath.inl>

#endif // BD_PLATFORM_MATH_H

