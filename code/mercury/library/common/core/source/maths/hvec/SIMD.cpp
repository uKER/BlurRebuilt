#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

#if defined( _SPU_ )
#include "SIMD.cpp.inl"
#endif

//static const U32 c_iSet      = 0xFFFFFFFF;
//static const U32 c_iClear    = 0x00000000;
//static const U32 c_iAbsMask  = 0x7FFFFFFF;
//static const U32 c_iSignMask = 0x80000000;
//#if defined( _SSE_ )

//namespace Maths
//{

const F32 c_fSet      = TypePun_IntToFloat( 0xFFFFFFFF );
const F32 c_fClear    = TypePun_IntToFloat( 0x00000000 );
const F32 c_fAbsMask  = TypePun_IntToFloat( 0x7FFFFFFF );
const F32 c_fSignMask = TypePun_IntToFloat( 0x80000000 );

#if !(defined( _PPU_ ) || defined( _VMX128_ ) || defined( _SSE_ ))

const F32 c_ScalarQuarter = 0.25f;
const F32 c_ScalarHalf    = 0.5f;
const F32 c_ScalarOne     = 1.0f;
const F32 c_ScalarTwo     = 2.0f;
const F32 c_ScalarThree   = 3.0f;
const F32 c_ScalarFour    = 4.0f;
const F32 c_ScalarEpsilon = FLT_EPSILON;

#endif


//const F32 c_fSet      = TypePun_IntToFloat( 0xFFFFFFFF );
//const F32 c_fClear    = TypePun_IntToFloat( 0x00000000 );
//const F32 c_fAbsMask  = TypePun_IntToFloat( 0x7FFFFFFF );
//const F32 c_fSignMask = TypePun_IntToFloat( 0x80000000 );

//}; // namespace Maths

//#endif // #if defined( _SSE_ )



