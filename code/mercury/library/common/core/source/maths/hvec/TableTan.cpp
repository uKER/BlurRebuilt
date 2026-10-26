#include "CorePch.h"

namespace Maths
{

// Tan(z) == Sum[k=1,inf] { z^(2k-1) . -1^(k-1) . 2^(2k) . (2^(2k))-1 . B(2k) / (2k)! } : |z| < PI/2
ALIGN16 const F32 g_Const_TanSeries[16] =
{
#if PLATFORM_SPU
        1.0     ,
 3.33333343E-01 ,
 0.13333334     ,
 5.39682582E-02 ,
 2.18694899E-02 ,
 8.86323582E-03 ,
 3.59212793E-03 ,
 1.45583437E-03 ,
 5.90027426E-04 ,
  2.3912912E-04 ,
 9.69153771E-05 ,
 3.92783222E-05 ,
  1.5918904E-05 ,
 6.45168939E-06 ,
 2.61477112E-06 ,
-       0.0     
#else
    /* k = 1  */ 1.0f * (F32)( PowerMinusOne<double>( 1  - 1 ) * Power<double>( 2, 2 * 1  ) * ( Power<double>( 2, 2 * 1  ) - 1 ) * Bernoulli<double>( 2 * 1  ) / Factorial<double>( 2 * 1  ) ),
    /* k = 2  */ 1.0f * (F32)( PowerMinusOne<double>( 2  - 1 ) * Power<double>( 2, 2 * 2  ) * ( Power<double>( 2, 2 * 2  ) - 1 ) * Bernoulli<double>( 2 * 2  ) / Factorial<double>( 2 * 2  ) ),
    /* k = 3  */ 1.0f * (F32)( PowerMinusOne<double>( 3  - 1 ) * Power<double>( 2, 2 * 3  ) * ( Power<double>( 2, 2 * 3  ) - 1 ) * Bernoulli<double>( 2 * 3  ) / Factorial<double>( 2 * 3  ) ),
    /* k = 4  */ 1.0f * (F32)( PowerMinusOne<double>( 4  - 1 ) * Power<double>( 2, 2 * 4  ) * ( Power<double>( 2, 2 * 4  ) - 1 ) * Bernoulli<double>( 2 * 4  ) / Factorial<double>( 2 * 4  ) ),
    /* k = 5  */ 1.0f * (F32)( PowerMinusOne<double>( 5  - 1 ) * Power<double>( 2, 2 * 5  ) * ( Power<double>( 2, 2 * 5  ) - 1 ) * Bernoulli<double>( 2 * 5  ) / Factorial<double>( 2 * 5  ) ),
    /* k = 6  */ 1.0f * (F32)( PowerMinusOne<double>( 6  - 1 ) * Power<double>( 2, 2 * 6  ) * ( Power<double>( 2, 2 * 6  ) - 1 ) * Bernoulli<double>( 2 * 6  ) / Factorial<double>( 2 * 6  ) ),
    /* k = 7  */ 1.0f * (F32)( PowerMinusOne<double>( 7  - 1 ) * Power<double>( 2, 2 * 7  ) * ( Power<double>( 2, 2 * 7  ) - 1 ) * Bernoulli<double>( 2 * 7  ) / Factorial<double>( 2 * 7  ) ),
    /* k = 8  */ 1.0f * (F32)( PowerMinusOne<double>( 8  - 1 ) * Power<double>( 2, 2 * 8  ) * ( Power<double>( 2, 2 * 8  ) - 1 ) * Bernoulli<double>( 2 * 8  ) / Factorial<double>( 2 * 8  ) ),
    /* k = 9  */ 1.0f * (F32)( PowerMinusOne<double>( 9  - 1 ) * Power<double>( 2, 2 * 9  ) * ( Power<double>( 2, 2 * 9  ) - 1 ) * Bernoulli<double>( 2 * 9  ) / Factorial<double>( 2 * 9  ) ),
    /* k = 10 */ 1.0f * (F32)( PowerMinusOne<double>( 10 - 1 ) * Power<double>( 2, 2 * 10 ) * ( Power<double>( 2, 2 * 10 ) - 1 ) * Bernoulli<double>( 2 * 10 ) / Factorial<double>( 2 * 10 ) ),
    /* k = 11 */ 1.0f * (F32)( PowerMinusOne<double>( 11 - 1 ) * Power<double>( 2, 2 * 11 ) * ( Power<double>( 2, 2 * 11 ) - 1 ) * Bernoulli<double>( 2 * 11 ) / Factorial<double>( 2 * 11 ) ),
    /* k = 12 */ 1.0f * (F32)( PowerMinusOne<double>( 12 - 1 ) * Power<double>( 2, 2 * 12 ) * ( Power<double>( 2, 2 * 12 ) - 1 ) * Bernoulli<double>( 2 * 12 ) / Factorial<double>( 2 * 12 ) ),
    /* k = 13 */ 1.0f * (F32)( PowerMinusOne<double>( 13 - 1 ) * Power<double>( 2, 2 * 13 ) * ( Power<double>( 2, 2 * 13 ) - 1 ) * Bernoulli<double>( 2 * 13 ) / Factorial<double>( 2 * 13 ) ),
    /* k = 14 */ 1.0f * (F32)( PowerMinusOne<double>( 14 - 1 ) * Power<double>( 2, 2 * 14 ) * ( Power<double>( 2, 2 * 14 ) - 1 ) * Bernoulli<double>( 2 * 14 ) / Factorial<double>( 2 * 14 ) ),
    /* k = 15 */ 1.0f * (F32)( PowerMinusOne<double>( 15 - 1 ) * Power<double>( 2, 2 * 15 ) * ( Power<double>( 2, 2 * 15 ) - 1 ) * Bernoulli<double>( 2 * 15 ) / Factorial<double>( 2 * 15 ) ),
    /* k = 16 */ 1.0f * (F32)( PowerMinusOne<double>( 16 - 1 ) * Power<double>( 2, 2 * 16 ) * ( Power<double>( 2, 2 * 16 ) - 1 ) * Bernoulli<double>( 2 * 16 ) / Factorial<double>( 2 * 16 ) )
#endif
};

}; // namespace Maths




