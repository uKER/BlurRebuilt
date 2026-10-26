#ifndef FP_H
#define FP_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

//#if defined( _DEBUG ) && defined( USER_JDENTON )
//#define MATHS_UNMASK_FLOAT_EXCEPTIONS
//#endif

//
// Float & SIMD Initialisation & Exception Control
//

//void FP_Init( const bool floatPrecision24bit = false, const bool floatFlushDenormalsToZero = true );
//void FP_UnmaskExceptions();
//void FP_MaskExceptions();

inline void FP_Init( const bool floatPrecision24bit = false, const bool floatFlushDenormalsToZero = true )
{
    #if defined( COMPILER_VC )
    
    // Set float rounding mode to round-to-nearest
    //_controlfp( _RC_NEAR, _MCW_RC );
    unsigned int current = 0;
    _controlfp_s( &current, _RC_NEAR, _MCW_RC );
    
    #if !defined( _WIN64 ) && !defined( WIN64 )
    if ( floatPrecision24bit )
    {
        // Set float precision to 24 bits
        //_controlfp( _PC_24, _MCW_PC );
        _controlfp_s( &current, _PC_24, _MCW_PC );
    }
    #endif
    
    if ( floatFlushDenormalsToZero )
    {
        // Set float denormal flush to zero mode
        //_controlfp( _DN_FLUSH, _MCW_DN );
        _controlfp_s( &current, _DN_FLUSH, _MCW_DN );
    }
    
    // Clear float exceptions 
    _clearfp();
    
    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    //_controlfp( _EM_INVALID | _EM_ZERODIVIDE | _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    _controlfp_s( &current, _EM_INVALID | _EM_ZERODIVIDE | _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    
    #if defined( _SSE_ )
    
    // Clear SSE exception state
    _MM_SET_EXCEPTION_STATE( 0 );
    
    // Mask all SSE exceptions
    _MM_SET_EXCEPTION_MASK( _MM_MASK_INVALID | _MM_MASK_DIV_ZERO | _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
    
    // Set SSE rounding mode to round-to-nearest
    _MM_SET_ROUNDING_MODE( _MM_ROUND_NEAREST );
    
    // Set SSE denormal flush to zero mode
    _MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
    
    #endif
    
    #endif
}

inline void FP_UnmaskExceptions()
{
    #if defined( COMPILER_VC )
    
    // Clear float exceptions ( so we don't get exceptions here from previous float exceptions... ) 
    _clearfp();
    
    // UnMask float INVALID & ZERODIVIDE
    // Mask   float DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    //_controlfp( _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    unsigned int current = 0;
    _controlfp_s( &current, _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    
    #if defined( _SSE_ )
    
    // Clear SSE exception state ( so we don't get exceptions here from previous SSE exceptions... ) 
    _MM_SET_EXCEPTION_STATE( 0 );
    
    // UnMask SSE INVALID, DIV_ZERO
    // Mask   SSE DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _MM_SET_EXCEPTION_MASK( _MM_MASK_DENORM | /* _MM_MASK_OVERFLOW | */ _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
    
    #endif
    
    
    #endif
}

inline void FP_MaskExceptions()
{
    #if defined( COMPILER_VC )
    
    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    //_controlfp( _EM_INVALID | _EM_ZERODIVIDE | _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    unsigned int current = 0;
    _controlfp_s( &current, _EM_INVALID | _EM_ZERODIVIDE | _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    
    #if defined( _SSE_ )
    
    // Mask SSE INVALID, DIV_ZERO, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _MM_SET_EXCEPTION_MASK( _MM_MASK_INVALID | _MM_MASK_DIV_ZERO | _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
    
    #endif
    
    #endif
}

}; // namespace Maths

#endif
