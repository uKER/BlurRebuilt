#include "CorePch.h"

namespace Maths
{

// Cos(z) == Sum[k=0,inf] { (-1)^k . z^(2k) / (2k)! }
ALIGN16 const F32 g_Const_CosSeries[16] =
{
#if PLATFORM_SPU
         1.0     ,
 -       0.5     ,
  4.16666679E-02 ,
 -1.38888892E-03 ,
  2.48015876E-05 ,
 -  2.755732E-07 ,
  2.08767559E-09 ,
 -1.14707454E-11 ,
  4.77947726E-14 ,
 -1.56192068E-16 ,
  4.11031759E-19 ,
 -8.89679096E-22 ,
  1.61173759E-24 ,
 -2.47959628E-27 ,
  3.27988916E-30 ,
 -3.76998781E-33 
#else
    /* k = 0  */ (F32)( PowerMinusOne<double>( 0  ) / Factorial<double>( 2 * 0  ) ),
    /* k = 1  */ (F32)( PowerMinusOne<double>( 1  ) / Factorial<double>( 2 * 1  ) ),
    /* k = 2  */ (F32)( PowerMinusOne<double>( 2  ) / Factorial<double>( 2 * 2  ) ),
    /* k = 3  */ (F32)( PowerMinusOne<double>( 3  ) / Factorial<double>( 2 * 3  ) ),
    /* k = 4  */ (F32)( PowerMinusOne<double>( 4  ) / Factorial<double>( 2 * 4  ) ),
    /* k = 5  */ (F32)( PowerMinusOne<double>( 5  ) / Factorial<double>( 2 * 5  ) ),
    /* k = 6  */ (F32)( PowerMinusOne<double>( 6  ) / Factorial<double>( 2 * 6  ) ),
    /* k = 7  */ (F32)( PowerMinusOne<double>( 7  ) / Factorial<double>( 2 * 7  ) ),
    /* k = 8  */ (F32)( PowerMinusOne<double>( 8  ) / Factorial<double>( 2 * 8  ) ),
    /* k = 9  */ (F32)( PowerMinusOne<double>( 9  ) / Factorial<double>( 2 * 9  ) ),
    /* k = 10 */ (F32)( PowerMinusOne<double>( 10 ) / Factorial<double>( 2 * 10 ) ),
    /* k = 11 */ (F32)( PowerMinusOne<double>( 11 ) / Factorial<double>( 2 * 11 ) ),
    /* k = 12 */ (F32)( PowerMinusOne<double>( 12 ) / Factorial<double>( 2 * 12 ) ),
    /* k = 13 */ (F32)( PowerMinusOne<double>( 13 ) / Factorial<double>( 2 * 13 ) ),
    /* k = 14 */ (F32)( PowerMinusOne<double>( 14 ) / Factorial<double>( 2 * 14 ) ),
    /* k = 15 */ (F32)( PowerMinusOne<double>( 15 ) / Factorial<double>( 2 * 15 ) )
#endif
};

}; // namespace Maths




