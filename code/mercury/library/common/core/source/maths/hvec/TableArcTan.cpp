#include "CorePch.h"

namespace Maths
{

// ArcTan(z) == Sum[k=0,inf] { (-1)^k . z^(2k+1) / (2k+1) }
ALIGN16 const F32 g_Const_ArcTanSeries[32] =                                        
{
#if PLATFORM_SPU
         1.0     ,
 -3.33333343E-01 ,
  2.00000003E-01 ,
 -1.42857149E-01 ,
  1.11111112E-01 ,
 -9.09090936E-02 ,
  7.69230798E-02 ,
 -6.66666701E-02 ,
  5.88235296E-02 ,
 -5.26315793E-02 ,
  4.76190485E-02 ,
 -4.34782617E-02 ,
  3.99999991E-02 ,
 -3.70370373E-02 ,
  3.44827585E-02 ,
 -3.22580636E-02 ,
  3.03030312E-02 ,
 -2.85714287E-02 ,
  2.70270277E-02 ,
 - 2.5641026E-02 ,
   2.4390243E-02 ,
 -2.32558139E-02 ,
  2.22222228E-02 ,
 -2.12765951E-02 ,
  2.04081628E-02 ,
 -1.96078438E-02 ,
  1.88679248E-02 ,
 -1.81818176E-02 ,
  1.75438598E-02 ,
 -1.69491526E-02 ,
  1.63934417E-02 ,
 -8.06240551E-03 
#else
    /* k = 0  */ (F32)( PowerMinusOne<double>( 0  ) / ( 2.0 * 0.0  + 1.0 ) ),
    /* k = 1  */ (F32)( PowerMinusOne<double>( 1  ) / ( 2.0 * 1.0  + 1.0 ) ),
    /* k = 2  */ (F32)( PowerMinusOne<double>( 2  ) / ( 2.0 * 2.0  + 1.0 ) ),
    /* k = 3  */ (F32)( PowerMinusOne<double>( 3  ) / ( 2.0 * 3.0  + 1.0 ) ),
    /* k = 4  */ (F32)( PowerMinusOne<double>( 4  ) / ( 2.0 * 4.0  + 1.0 ) ),
    /* k = 5  */ (F32)( PowerMinusOne<double>( 5  ) / ( 2.0 * 5.0  + 1.0 ) ),
    /* k = 6  */ (F32)( PowerMinusOne<double>( 6  ) / ( 2.0 * 6.0  + 1.0 ) ),
    /* k = 7  */ (F32)( PowerMinusOne<double>( 7  ) / ( 2.0 * 7.0  + 1.0 ) ),
    /* k = 8  */ (F32)( PowerMinusOne<double>( 8  ) / ( 2.0 * 8.0  + 1.0 ) ),
    /* k = 9  */ (F32)( PowerMinusOne<double>( 9  ) / ( 2.0 * 9.0  + 1.0 ) ),
    /* k = 10 */ (F32)( PowerMinusOne<double>( 10 ) / ( 2.0 * 10.0 + 1.0 ) ),
    /* k = 11 */ (F32)( PowerMinusOne<double>( 11 ) / ( 2.0 * 11.0 + 1.0 ) ),
    /* k = 12 */ (F32)( PowerMinusOne<double>( 12 ) / ( 2.0 * 12.0 + 1.0 ) ),
    /* k = 13 */ (F32)( PowerMinusOne<double>( 13 ) / ( 2.0 * 13.0 + 1.0 ) ),
    /* k = 14 */ (F32)( PowerMinusOne<double>( 14 ) / ( 2.0 * 14.0 + 1.0 ) ),
    /* k = 15 */ (F32)( PowerMinusOne<double>( 15 ) / ( 2.0 * 15.0 + 1.0 ) ),
    /* k = 16 */ (F32)( PowerMinusOne<double>( 16 ) / ( 2.0 * 16.0 + 1.0 ) ),
    /* k = 17 */ (F32)( PowerMinusOne<double>( 17 ) / ( 2.0 * 17.0 + 1.0 ) ),
    /* k = 18 */ (F32)( PowerMinusOne<double>( 18 ) / ( 2.0 * 18.0 + 1.0 ) ),
    /* k = 19 */ (F32)( PowerMinusOne<double>( 19 ) / ( 2.0 * 19.0 + 1.0 ) ),
    /* k = 20 */ (F32)( PowerMinusOne<double>( 20 ) / ( 2.0 * 20.0 + 1.0 ) ),
    /* k = 21 */ (F32)( PowerMinusOne<double>( 21 ) / ( 2.0 * 21.0 + 1.0 ) ),
    /* k = 22 */ (F32)( PowerMinusOne<double>( 22 ) / ( 2.0 * 22.0 + 1.0 ) ),
    /* k = 23 */ (F32)( PowerMinusOne<double>( 23 ) / ( 2.0 * 23.0 + 1.0 ) ),
    /* k = 24 */ (F32)( PowerMinusOne<double>( 24 ) / ( 2.0 * 24.0 + 1.0 ) ),
    /* k = 25 */ (F32)( PowerMinusOne<double>( 25 ) / ( 2.0 * 25.0 + 1.0 ) ),
    /* k = 26 */ (F32)( PowerMinusOne<double>( 26 ) / ( 2.0 * 26.0 + 1.0 ) ),
    /* k = 27 */ (F32)( PowerMinusOne<double>( 27 ) / ( 2.0 * 27.0 + 1.0 ) ),
    /* k = 28 */ (F32)( PowerMinusOne<double>( 28 ) / ( 2.0 * 28.0 + 1.0 ) ),
    /* k = 29 */ (F32)( PowerMinusOne<double>( 29 ) / ( 2.0 * 29.0 + 1.0 ) ),
    /* k = 30 */ (F32)( PowerMinusOne<double>( 30 ) / ( 2.0 * 30.0 + 1.0 ) ),
    #if 0
    /* k = 31 */ (F32)( PowerMinusOne<double>( 31 ) / ( 2.0 * 31.0 + 1.0 ) ) // This series should sum to PI/4!
    #else
    /* k = 31 */ (F32)( PowerMinusOne<double>( 31 ) / ( 2.0 * 31.0 + 1.0 ) ) + 0.0078106117734483096156608458198757f
    #endif
#endif
};

}; // namespace Maths




