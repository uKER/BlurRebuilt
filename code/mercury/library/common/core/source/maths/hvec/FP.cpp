#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

namespace Maths
{ 

//
// These functions have been made inline and moved into FP.h!
//

//void FP_Init( const bool floatPrecision24bit, const bool floatFlushDenormalsToZero )
//{
//    #if defined( COMPILER_VC )
//    
//    // Set float rounding mode to round-to-nearest
//    _controlfp( _RC_NEAR, _MCW_RC );
//    
//    #if !defined( _WIN64 ) && !defined( WIN64 )
//    if ( floatPrecision24bit )
//    {
//        // Set float precision to 24 bits
//        _controlfp( _PC_24, _MCW_PC );
//    }
//    #endif
//    
//    if ( floatFlushDenormalsToZero )
//    {
//        // Set float denormal flush to zero mode
//        _controlfp( _DN_FLUSH, _MCW_DN );
//    }
//    
//    // Clear float exceptions 
//    _clearfp();
//    
//    #if defined( PLATFORM_XENON )
//    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _controlfp( _EM_INVALID | _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
//    #else
//    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _control87( _EM_INVALID | _EM_ZERODIVIDE | _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
//    #endif
//    
//    #if defined( _SSE_ )
//    
//    // Clear SSE exception state
//    _MM_SET_EXCEPTION_STATE( 0 );
//    
//    // Mask all SSE exceptions
//    _MM_SET_EXCEPTION_MASK( _MM_MASK_INVALID | _MM_MASK_DIV_ZERO | _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
//    
//    // Set SSE rounding mode to round-to-nearest
//    _MM_SET_ROUNDING_MODE( _MM_ROUND_NEAREST );
//    
//    // Set SSE denormal flush to zero mode
//    _MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
//    
//    #endif
//    
//    #endif
//}
//
//void FP_UnmaskExceptions()
//{
//    #if defined( COMPILER_VC )
//    
//    // Clear float exceptions 
//    _clearfp();
//    
//    #if defined( PLATFORM_XENON )
//    // UnMask float INVALID & ZERODIVIDE
//    // Mask   float DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _controlfp( _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
//    #else
//    // UnMask float INVALID & ZERODIVIDE
//    // Mask   float DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _control87( _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
//    #endif
//    
//    #if defined( _SSE_ )
//    
//    // Clear SSE exception state
//    _MM_SET_EXCEPTION_STATE( 0 );
//    
//    // UnMask SSE INVALID, DIV_ZERO
//    // Mask   SSE DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _MM_SET_EXCEPTION_MASK( _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
//    
//    #endif
//    
//    #if defined( PLATFORM_XENON )
//    __asm
//    {
//        mfmsr r11;
//        ori r11, r11, 0x900;
//        mtmsr r11;
//    }
//    #endif
//    
//    #endif
//}
//
//void FP_MaskExceptions()
//{
//    #if defined( COMPILER_VC )
//    
//    // Clear float exceptions 
//    _clearfp();
//    
//    #if defined( PLATFORM_XENON )
//    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _controlfp( _EM_INVALID | _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
//    #else
//    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _control87( _EM_INVALID | _EM_ZERODIVIDE | _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
//    #endif
//    
//    #if defined( _SSE_ )
//    
//    // Clear SSE exception state
//    _MM_SET_EXCEPTION_STATE( 0 );
//    
//    // Mask SSE INVALID, DIV_ZERO, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _MM_SET_EXCEPTION_MASK( _MM_MASK_INVALID | _MM_MASK_DIV_ZERO | _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
//    
//    #endif
//    
//    #endif
//}

}; // namespace Maths

