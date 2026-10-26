#ifndef MATHS_H
#define MATHS_H

#ifdef _MSC_VER
#pragma once
#endif

#if !defined( PLATFORM_PS3 ) && !defined( PLATFORM_SPU )
#include <malloc.h>
#endif

#include <core/Core.h>
#include "utility/utility.h"

//#if defined( USER_JDENTON ) && defined( PLATFORM_PC ) // Needed for DebugBreak() declaration
//#include <windows.h>
//#include <winbase.h>
////__forceinline void DebugStop() { __asm { int 3 }; }
//#endif

#if defined( COMPILER_VC )
#pragma warning( push )
#pragma warning( once : 4714 )   // Enable warning once only: __forceinline failure
#pragma warning( disable: 4201 ) // warning C4201: nonstandard extension used : nameless struct/union
#pragma warning( disable: 4324 ) // warning C4324: structure was padded due to __declspec(align())
#pragma warning( disable: 4127 ) // warning C4127: conditional expression is constant
#endif




/*****************************************************************************/
// Software version of maths library for all implementations
/*****************************************************************************/

//#if 0 && defined( USER_JDENTON )
//
//#pragma message( "*** SOFT MATHS START ***" )
//
//#define SOFT_MATHS
//
//#define PCREF &
//#define ALIGN16
//#define PASSINREG
//
//namespace Soft
//{
//#include "maths/constants.h"
//#include "maths/storage.h"
//#include "maths/MathsUtil.h"
//#include "maths/hvec/SIMD.h"
//#include "maths/hvec/VectorConst.h"
//#include "maths/hvec/Storage.h"
////#include "maths/hvec/New/IncludeComplete.h"
//#include "maths/hvec/New/IncludeClasses.h"
//
//// JASOND: FIX THIS UP LATER, JUST GET THE HEADERS COMPILING FOR NOW...
////#include "maths/hvec/SIMD.inl"
////#include "maths/hvec/VectorConst.inl"
////#include "maths/hvec/Storage.inl"
//#include "maths/hvec/New/IncludeComplete.inl"
//}
//
//#undef PCREF
//#undef ALIGN16
//#undef PASSINREG
//
//#undef SOFT_MATHS
//
//#pragma message( "*** SOFT MATHS END ***" )
//
//#endif // #if defined( USER_JDENTON )


/*****************************************************************************/
// PC
/*****************************************************************************/

// Vector Maths Compiler Options
// _MMX_ (?)
// _SSE_
// _SSE2_
// _SSE3_
// _SSSE3_
// _SSE4_
// _SSE4_1_
// _SSE4_2_

#if defined( PLATFORM_PC )

#if !defined( _DEBUG ) // 16 byte stack alignment gets screwed up by the compiler in DEBUG builds... There's nothing else we can do except use standard FP!

// [LloydW] Disable SSE by default for PC.
//#if 0 // !defined (USER_CLUBBUILD01U) && !defined (USER_IANW) && !defined( USER_STEPHENP ) && !defined( USER_LLOYDW ) && !defined( USER_PAULM ) && !defined( USER_WILLB ) && !defined( USER_PEREID ) && !defined( USER_MARTINB ) && !defined( USER_DAVIDW )  && !defined( USER_DAVIDG ) && !defined( USER_STEPHENM ) && !defined( USER_CHARLIEB ) && !defined( USER_LEEW ) && !defined( USER_DAVIDH ) && !defined(USER_KIERS)
#if 0 // defined( USER_JDENTON )

// NOTE: If any (S)SSE{2,3,4} option is defined, all earlier SSE versions will also be defined!
//       MMX is not supported for Win64 builds and may NOT be defined with later SSE versions!
#define _SIMD_
#define _MMX_
#define _SSE_
#define _SSE2_

#if ( _MSC_VER >= 1500 ) // VS 2008 is required for the following SSE versions!
#define _SSE3_
#define _SSSE3_
//#define _SSE4_      // IMPLIES _SSE4_1_ & _SSE4_2_ DEFINED!
//#define _SSE4_1_
//#define _SSE4_2_
//#define _SSE4a      // AMD SSE4a (not supported by Intel...)
//#define _SSE5_      // AMD SSE5 (not supported by Intel...)
//#define _AVX_       // Intel Advanced Vector Extensions (not supported by AMD...)
#endif

#if defined( _MMX_ )
#include <mmintrin.h>
#endif

#if defined( _SSE_ )
#include <xmmintrin.h>
#endif

#if defined( _SSE2_ )
#include <emmintrin.h>
#endif

#if defined( _SSE3_ )
#include <pmmintrin.h>
#endif

#if defined( _SSSE3_ )
#include <tmmintrin.h>
#endif

#if defined( _SSE4_ ) || defined( _SSE4_1_ )
#include <smmintrin.h>
#endif

#if defined( _SSE4_ ) || defined( _SSE4_2_ )
#include <nmmintrin.h>
#endif

//// intrin.h includes assert.h, we include CoreDebug.h to define assert again
#undef assert
#include <core\utility\CoreDebug.h>

#endif // #if !defined( USER_IANW ) ...

#if defined( _WIN64 ) // MMX is not supported on 64 bit windows platforms, SSE2 & SSE3 instructions must be used instead!
#undef  _MMX_
#define _SSE2_
#define _SSE3_
#endif // #if defined( _WIN64 )

#endif // #if !defined( _DEBUG )

#endif // #if defined( PLATFORM_PC )


/*****************************************************************************/
// Xenon
/*****************************************************************************/


#if defined( COMPILER_VC ) && !defined( __INTEL_COMPILER )

extern "C" DEPRECATED void _ReadWriteBarrier();
extern "C" DEPRECATED void _ReadBarrier();
extern "C" DEPRECATED void _WriteBarrier();

#pragma intrinsic( _ReadWriteBarrier )
#pragma intrinsic( _ReadBarrier )
#pragma intrinsic( _WriteBarrier )

/// ReadWriteBarrier: Force compiler to prevent reads or writes from crossing this barrier
/// NOTE: This only applies to read & write instructions, NOT the underlying memory system
/// which may still reorder reads & writes. (Use a sync instruction to achieve this, e.g. __lwsync() ).
#pragma warning( push )
#pragma warning( disable: 4996 )
NOALIAS __forceinline void ReadWriteBarrier()
{
    // NOTE: For some unknown reason, all 3 are required by the 360 compiler to
    // properly achieve this... no other combination seems to work correctly.
    _ReadBarrier();
    _WriteBarrier();
    _ReadWriteBarrier();
}
#pragma warning( pop )

#elif defined( PLATFORM_PS3 )

#define ReadWriteBarrier()

#elif defined( PLATFORM_SPU )

#define ReadWriteBarrier()

#else

#define ReadWriteBarrier()

#endif

#if defined( _VMX128_ )
#include <vectorintrinsics.h>
#include <ppcintrinsics.h>
#include <xboxmath.h>
#endif


/*****************************************************************************/
// PS3
/*****************************************************************************/

#if defined( PLATFORM_PS3 )
#ifndef CFG_SPEED
//#define MATHS_CHECK_VECTOR_ALIGNMENT
#endif
#define _SIMD_
#define _PPU_
#include <altivec.h>
#endif

#if ( defined( __SPU__ ) || defined( PLATFORM_SPU ) )
#ifndef CFG_SPEED
//#define MATHS_CHECK_VECTOR_ALIGNMENT
#endif
#define _SIMD_
#define _SPU_
#include <spu_internals.h>
#include <spu_intrinsics.h>
#include <vmx2spu.h>
#endif

#if defined( _SIMD_ ) && defined( COMPILER_GNU )
#define ALIGN16 __attribute__(( aligned( 16 ) ))
#elif defined( _SIMD_ ) && !defined( _NOALIGNMENT_ )
#define ALIGN16 __declspec( align( 16 ) )
#else
#define ALIGN16
#endif

#if defined( _SIMD_ ) && defined( COMPILER_GNU )
#define ALIGN128 __attribute__(( aligned( 128 ) ))
#elif defined( _SIMD_ ) && !defined( _NOALIGNMENT_ )
#define ALIGN128 __declspec( align( 128 ) )
#else
#define ALIGN128
#endif

#ifndef DEPRECATED
#define DEPRECATED __declspec(deprecated)
#endif 

/*****************************************************************************/
// Instruction set debug
/*****************************************************************************/

//#if defined( USER_JDENTON )
//#define SHOW_MATHS_INSTRUCTION_SET
//#endif

#if defined( SHOW_MATHS_INSTRUCTION_SET )
    
	#if defined( _SIMD_)
	#pragma message( "Maths: _SIMD_" )
	#endif
	
	#if defined( _VMX_ ) || defined( _VMX128_ )
	    
	    #if defined ( _VMX_ )
		#pragma message( "Maths: _VMX_" )
	    #endif
	    
	    #if defined ( _VMX128_ )
		#pragma message( "Maths: _VMX128_" )
	    #endif
	    
	#elif defined( _MMX_ ) || defined( _SSE_)
	    
	    #if defined( _MMX_ )
		#pragma message( "Maths: _MMX_" )
	    #endif
	    
	    #if defined( _SSE_ )
		#pragma message( "Maths: _SSE_" )
	    #endif
	    
	    #if defined( _SSE2_ )
		#pragma message( "Maths: _SSE2_" )
	    #endif
	    
	    #if defined( _SSE3_ )
		#pragma message( "Maths: _SSE3_" )
	    #endif
	    
	    #if defined( _S_SSE3_ )
		#pragma message( "Maths: _S_SSE3_" )
	    #endif
	    
	    #if defined( _SSE4_ )
		#pragma message( "Maths: _SSE4_" )
	    #endif
	    
	    #if defined( _SSE4_1_ )
		#pragma message( "Maths: _SSE4_1_" )
	    #endif
	    
	    #if defined( _SSE4_2_ )
		#pragma message( "Maths: _SSE4_2_" )
	    #endif
	    
    #elif defined( _PPU_ )
        
		#pragma message( "Maths: _PPU_" )
		
    #elif defined( _SPU_ )
		
		#pragma message( "Maths: _SPU_" )
		
	#else
	#pragma message( "Maths: _SIMD_ NOT defined : using standard float maths" )
	#endif
	
#endif

#if !defined( PCREF ) || !defined( VECREF ) || !defined( MATREF )

/// PCREF, VECREF & MATREF NOTES:
/// These are modifiers for maths function parameters. Usage examples:
/// void Function1( const VF32 VECREF a );
/// void Function2( const Vector3 VECREF b );
/// void Function3( const Mat43 MATREF c );
/// The parameters that these modifiers are used with should *always* be const.
/// They should also be presumed to be const references. They *may* be optimised to be const values.
/// PCREF is the original modifier, designed to pass vectors by reference on PC & by value on other platforms.
/// Vectors & Matrices cannot be passed by value on the PC due to a compiler issue.
/// VECREF & MATREF are an extension to PCREF to enable you to differentiate between vectors & matrices (or similar size types).
/// Vectors (or similar size types, <= 4 floats) can be passed by value efficiently on the PS3 and on the 360.
/// Matrices (or similar size types, > 4 floats) can be passed by value efficiently on the PS3 but NOT on the 360.
/// WARNING: You should be aware of potential aliasing bugs when passing parameters by const reference!
/// This can occur with member functions that pass a non-const member variable by const reference to another member function.
/// The callee function will then have 2 aliases for the same member variable, one const and the other non-const!

#if defined( PLATFORM_PC ) && !defined( __INTEL_COMPILER )

/// PCREF - modifier for single vector register maths function parameters: reference on PC
#define PCREF &

/// VECREF - modifier for vector (single vector register, <= 4 floats) maths function parameters: reference on PC
#define VECREF &

/// MATREF - modifier for matrix (multi vector register, > 4 floats) maths function parameters: reference on PC
#define MATREF &

#else

/// PCREF - modifier for single vector register maths function parameters: value on PS3
#define PCREF

/// VECREF - modifier for vector (single vector register, <= 4 floats) maths function parameters: value on PS3
#define VECREF

/// MATREF - modifier for matrix (multi vector register, > 4 floats) maths function parameters: value on PS3
#define MATREF

#endif

#endif // #if !defined( PCREF ) || !defined( VECREF ) || !defined( MATREF )

#if !defined( PASSINREG )
#define PASSINREG
#endif // #if !defined( PASSINREG )

#if !defined( DARWIN64_ABI )
#if defined( PLATFORM_PPU )
#define DARWIN64_ABI __attribute__((d64_abi))
#define DARWIN64_ABI_ENABLED
#else
#define DARWIN64_ABI
#endif
#endif // #if !defined( PPU_VECRETURN )

#if !defined( VECRETURN )
#if defined( PLATFORM_PPU ) && !defined( DARWIN64_ABI_ENABLED )
#define VECRETURN __attribute__((vecreturn))
#else
#define VECRETURN
#endif
#endif // #if !defined( PPU_VECRETURN )

#if !defined( PLATFORM_PPU )
#define __vecreg
#endif

/// @defgroup Maths
/// Main classes are:
/// @li @ref Maths::HF32
/// @li @ref Maths::HVec2
/// @li @ref Maths::HVec3
/// @li @ref Maths::HVec4
/// @li @ref Maths::HMat33
/// @li @ref Maths::HMat34
/// @li @ref Maths::HMat44
/// @li @ref Maths::C_Quaternion

#define MATHS_VMX_ZERO_PAD_ELEMENTS

/// Header files

#include "maths/hvec/New/IncludeClasses.h"

#include "maths/Constants.h"
//#include "maths/storage.h"
#include "maths/MathsUtil.h"

/// Include Class Headers first
/// ( Xenon vector passinreg optimisation: Operators/Functions require complete class definition to work.
///   Partial defs (i.e. op/func in class def) and forward declarations will break the passinreg optimisations! )
#include "maths/hvec/Param.h"
#include "maths/hvec/FP.h"
//#include "maths/hvec/SIMD.h"
//#include "maths/hvec/VectorConst.h"
//#include "maths/hvec/Storage.h"

//#if defined( _PPU_ )
//#include "maths/hvec/SIMD_PPU.inl"
//#elif defined( _SPU_ )
//#include "maths/hvec/SIMD_SPU.inl"
//#elif defined( _VMX128_ )
//#include "maths/hvec/SIMD_VMX.inl"
//#elif defined( _SSE_ )
//#include "maths/hvec/SIMD_SSE.inl"
//#endif

//#include "maths/hvec/New/IncludeClasses.h"

#include "maths/hvec/SinCos.h"
#include "maths/hvec/Accessor.h"
#include "maths/hvec/Scalar.h"
#include "maths/hvec/Vector.h"
#include "maths/hvec/Matrix.h"
#include "maths/hvec/RGBA.h"
#include "maths/hvec/Complex.h"
#include "maths/hvec/Quaternion.h"
#include "maths/hvec/Geometry.h"
#include "maths/hvec/Geometry2d.h"
#include "maths/hvec/SI.h"
#include "maths/hvec/Trigonometry.h"
#include "maths/hvec/BigMatrix.h"
#include "maths/hvec/MathsTest.h"

/// Include Operator & Function Headers last
/// ( Xenon vector passinreg optimisation: Operators/Functions require complete class definition to work.
///   Partial defs (i.e. op/func in class def) and forward declarations will break the passinreg optimisations! )
#include "maths/hvec/VectorOp.h"
#include "maths/hvec/VecMatOp.h"

#include "maths/hvec/Constants.h"
#include "maths/hvec/Constants.inl"

/// Inline implementation files

#include "maths/hvec/SIMD.inl"
#include "maths/hvec/VectorConst.inl"
#include "maths/hvec/Accessor.inl"
#include "maths/hvec/Storage.inl"
#include "maths/hvec/Scalar.inl"
#include "maths/hvec/Vector.inl"
#include "maths/hvec/Matrix.inl"
#include "maths/hvec/RGBA.inl"
#include "maths/hvec/Complex.inl"
#include "maths/hvec/Quaternion.inl"
#include "maths/hvec/Geometry.inl"
#include "maths/hvec/Geometry2d.inl"
#include "maths/hvec/SI.inl"
#include "maths/hvec/Trigonometry.inl"
#include "maths/hvec/BigMatrix.inl"

#include "maths/hvec/VectorOp.inl"
#include "maths/hvec/VecMatOp.inl"

#if defined( COMPILER_VC )
#pragma warning( pop )
#endif

#endif // #ifndef MATHS_H
