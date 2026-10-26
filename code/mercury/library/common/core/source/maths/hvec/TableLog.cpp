#include "CorePch.h"

namespace Maths
{

// Log(1+z) == Sum[k=1,+inf] { (-1)^(k-1) . z^k) / k } /; |z| < 1
ALIGN16 const F32 g_Const_LogSeries[32] =
{
#if PLATFORM_SPU
         1.0    ,
 -       0.5    ,
  3.33333343E-01,
 -      0.25    ,
  2.00000003E-01,
 -1.66666672E-01,
  1.42857149E-01,
 -     0.125    ,
  1.11111112E-01,
 -1.00000001E-01,
  9.09090936E-02,
 -8.33333358E-02,
  7.69230798E-02,
 -7.14285746E-02,
  6.66666701E-02,
 -    0.0625    ,
  5.88235296E-02,
 - 5.5555556E-02,
  5.26315793E-02,
 -5.00000007E-02,
  4.76190485E-02,
 -4.54545468E-02,
  4.34782617E-02,
 -4.16666679E-02,
  3.99999991E-02,
 -3.84615399E-02,
  3.70370373E-02,
 -3.57142873E-02,
  3.44827585E-02,
 -3.33333351E-02,
  3.22580636E-02,
 -   0.03125    
#else
    /* k = 1  */ (F32)( PowerMinusOne<double>( 0  ) / 1.0  ),
    /* k = 2  */ (F32)( PowerMinusOne<double>( 1  ) / 2.0  ),
    /* k = 3  */ (F32)( PowerMinusOne<double>( 2  ) / 3.0  ),
    /* k = 4  */ (F32)( PowerMinusOne<double>( 3  ) / 4.0  ),
    /* k = 5  */ (F32)( PowerMinusOne<double>( 4  ) / 5.0  ),
    /* k = 6  */ (F32)( PowerMinusOne<double>( 5  ) / 6.0  ),
    /* k = 7  */ (F32)( PowerMinusOne<double>( 6  ) / 7.0  ),
    /* k = 8  */ (F32)( PowerMinusOne<double>( 7  ) / 8.0  ),
    /* k = 9  */ (F32)( PowerMinusOne<double>( 8  ) / 9.0  ),
    /* k = 10 */ (F32)( PowerMinusOne<double>( 9  ) / 10.0 ),
    /* k = 11 */ (F32)( PowerMinusOne<double>( 10 ) / 11.0 ),
    /* k = 12 */ (F32)( PowerMinusOne<double>( 11 ) / 12.0 ),
    /* k = 13 */ (F32)( PowerMinusOne<double>( 12 ) / 13.0 ),
    /* k = 14 */ (F32)( PowerMinusOne<double>( 13 ) / 14.0 ),
    /* k = 15 */ (F32)( PowerMinusOne<double>( 14 ) / 15.0 ),
    /* k = 16 */ (F32)( PowerMinusOne<double>( 15 ) / 16.0 ),
    /* k = 17 */ (F32)( PowerMinusOne<double>( 16 ) / 17.0 ),
    /* k = 18 */ (F32)( PowerMinusOne<double>( 17 ) / 18.0 ),
    /* k = 19 */ (F32)( PowerMinusOne<double>( 18 ) / 19.0 ),
    /* k = 20 */ (F32)( PowerMinusOne<double>( 19 ) / 20.0 ),
    /* k = 21 */ (F32)( PowerMinusOne<double>( 20 ) / 21.0 ),
    /* k = 22 */ (F32)( PowerMinusOne<double>( 21 ) / 22.0 ),
    /* k = 23 */ (F32)( PowerMinusOne<double>( 22 ) / 23.0 ),
    /* k = 24 */ (F32)( PowerMinusOne<double>( 23 ) / 24.0 ),
    /* k = 25 */ (F32)( PowerMinusOne<double>( 24 ) / 25.0 ),
    /* k = 26 */ (F32)( PowerMinusOne<double>( 25 ) / 26.0 ),
    /* k = 27 */ (F32)( PowerMinusOne<double>( 26 ) / 27.0 ),
    /* k = 28 */ (F32)( PowerMinusOne<double>( 27 ) / 28.0 ),
    /* k = 29 */ (F32)( PowerMinusOne<double>( 28 ) / 29.0 ),
    /* k = 30 */ (F32)( PowerMinusOne<double>( 29 ) / 30.0 ),
    /* k = 31 */ (F32)( PowerMinusOne<double>( 30 ) / 31.0 ),
    /* k = 32 */ (F32)( PowerMinusOne<double>( 31 ) / 32.0 )
#endif
};

}; // namespace Maths




