#include "CorePch.h"

namespace Maths
{

// ArcSin(z) == Sum[k=0,inf] { z^(2k+1) . Pochhammer( 0.5, k ) / (2k+1)k! }
ALIGN16 const F32 g_Const_ArcSinSeries[16] =
{
#if PLATFORM_SPU
        1.0     ,
 1.66666672E-01 ,
  7.5000003E-02 ,
 4.46428582E-02 ,
  3.0381944E-02 ,
 2.23721582E-02 ,
 1.73527636E-02 ,
 1.39648439E-02 ,
 1.15518011E-02 ,
 9.76160914E-03 ,
 8.39033537E-03 ,
   7.312526E-03 ,
 6.44721044E-03 ,
 5.74003765E-03 ,
 5.15330955E-03 ,
 4.66014352E-03 
#else
    /* k = 0  */ (F32)( Pochhammer( 0.5, 0  ) / ( ( 2.0 * 0.0  + 1.0 ) * Factorial<double>( 0  ) ) ),
    /* k = 1  */ (F32)( Pochhammer( 0.5, 1  ) / ( ( 2.0 * 1.0  + 1.0 ) * Factorial<double>( 1  ) ) ),
    /* k = 2  */ (F32)( Pochhammer( 0.5, 2  ) / ( ( 2.0 * 2.0  + 1.0 ) * Factorial<double>( 2  ) ) ),
    /* k = 3  */ (F32)( Pochhammer( 0.5, 3  ) / ( ( 2.0 * 3.0  + 1.0 ) * Factorial<double>( 3  ) ) ),
    /* k = 4  */ (F32)( Pochhammer( 0.5, 4  ) / ( ( 2.0 * 4.0  + 1.0 ) * Factorial<double>( 4  ) ) ),
    /* k = 5  */ (F32)( Pochhammer( 0.5, 5  ) / ( ( 2.0 * 5.0  + 1.0 ) * Factorial<double>( 5  ) ) ),
    /* k = 6  */ (F32)( Pochhammer( 0.5, 6  ) / ( ( 2.0 * 6.0  + 1.0 ) * Factorial<double>( 6  ) ) ),
    /* k = 7  */ (F32)( Pochhammer( 0.5, 7  ) / ( ( 2.0 * 7.0  + 1.0 ) * Factorial<double>( 7  ) ) ),
    /* k = 8  */ (F32)( Pochhammer( 0.5, 8  ) / ( ( 2.0 * 8.0  + 1.0 ) * Factorial<double>( 8  ) ) ),
    /* k = 9  */ (F32)( Pochhammer( 0.5, 9  ) / ( ( 2.0 * 9.0  + 1.0 ) * Factorial<double>( 9  ) ) ),
    /* k = 10 */ (F32)( Pochhammer( 0.5, 10 ) / ( ( 2.0 * 10.0 + 1.0 ) * Factorial<double>( 10 ) ) ),
    /* k = 11 */ (F32)( Pochhammer( 0.5, 11 ) / ( ( 2.0 * 11.0 + 1.0 ) * Factorial<double>( 11 ) ) ),
    /* k = 12 */ (F32)( Pochhammer( 0.5, 12 ) / ( ( 2.0 * 12.0 + 1.0 ) * Factorial<double>( 12 ) ) ),
    /* k = 13 */ (F32)( Pochhammer( 0.5, 13 ) / ( ( 2.0 * 13.0 + 1.0 ) * Factorial<double>( 13 ) ) ),
    /* k = 14 */ (F32)( Pochhammer( 0.5, 14 ) / ( ( 2.0 * 14.0 + 1.0 ) * Factorial<double>( 14 ) ) ),
    /* k = 15 */ (F32)( Pochhammer( 0.5, 15 ) / ( ( 2.0 * 15.0 + 1.0 ) * Factorial<double>( 15 ) ) )
#endif
};

}; // namespace Maths




