#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_SIMD_INL
#define SOFT_MATHS_HVEC_SIMD_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_SIMD_INL
#define MATHS_HVEC_SIMD_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#if defined( PLATFORM_PS3 )
//#include <alloca.h>
#include <string.h>
#endif

#if !defined( PLATFORM_SPU )
#if defined(PLATFORM_PS3) && CELL_SDK_VERSION >= 0x240001
#include <sys/memory.h>
#else
#include <memory.h>
#endif
#endif

/*#if defined PLATFORM_PS3
#undef _alloca
#define _alloca(size) __builtin_alloca(size)
#endif*/

namespace Maths
{

//
// Temporary Stack Buffer Allocator: Data aligned and padded to cache line size ( 128 bytes ).
// Buffer is cache controlled on allocation & deallocation to prevent memory traffic pollution.
// Note: Memory is allocated using _alloca() and will therefore only be released on function exit!
//

NOALIAS __forceinline const U32 PadToCacheLineSize( const U32 size )
{
    //#if defined( PLATFORM_XENON ) || defined( PLATFORM_PS3 ) || defined( PLATFORM_SPU )
    // Pad to 128 byte cache line size
    return ( size + 0x7F ) & ~0x7F;
    //#else // XBox1, PS2, Windows
    // Pad to 64 byte cache line size
    //return ( size + 0x3F ) & ~0x3F;
    //#endif
}




#if defined( _DEBUG ) || defined( NO_INLINE_STACK_ALLOC_PS3 )

template < int BUFFERSIZE >
inline C_StackAllocTemplate<BUFFERSIZE>::C_StackAllocTemplate( const U32 size )
{
    // Pad m_Size to 128 bytes
    
    m_Size = ( ( size + 127 ) & ~127 );
	
	if ( ( m_Size + 128 ) <= BUFFERSIZE ) // NOTE: Extra 128 for alignment of m_pData!
		m_pMemory = m_buffer;
	else
	    m_pMemory = new char[ m_Size + 128 ] ;
    
    // Align m_pData to 128 bytes
    
    #if !defined( COMPILER_GNU )
    #pragma warning( push )
    #pragma warning( disable: 4311 )
    #pragma warning( disable: 4312 )
    #endif
	
    //#if defined( WORD_SIZE ) && ( WORD_SIZE == 64 )
    //m_pData = ( (char*)( ( (U64)m_pMemory + 127 ) & ~127 ) );
    //#else
    //m_pData = ( (char*)( ( (U32)m_pMemory + 127 ) & ~127 ) );
    //#endif
    m_pData = ( (char*)( ( PointerToInt( m_pMemory ) + 127 ) & ~127 ) );
    
    #if !defined( COMPILER_GNU )
    #pragma warning( pop )
    #endif
}

template < int BUFFERSIZE >
inline C_StackAllocTemplate<BUFFERSIZE>::~C_StackAllocTemplate()
{
	if ( ( m_Size + 128 ) <= BUFFERSIZE )
		return;
    
    delete [] m_pMemory;
}

template < int BUFFERSIZE >
inline char* C_StackAllocTemplate<BUFFERSIZE>::Data() const
{
    //#if defined( COMPILER_GNU )
    //return m_pMemory; // THIS IS NOT 128 BYTE ALIGNED AND WAS CAUSING CRASHES!
    //#else
	return m_pData;
    //#endif
}

#else

// NOTE: This relies on the function being inlined to work!
// The memory allocated with _alloca() will be freed on exit from 
// the enclosing function that this is inlined into!
#ifdef COMPILER_GNU
template <int BUFFERSIZE >
inline __attribute__ ((always_inline)) C_StackAllocTemplate<BUFFERSIZE>::C_StackAllocTemplate( const U32 size )
#else
template < int BUFFERSIZE >
__forceinline C_StackAllocTemplate<BUFFERSIZE>::C_StackAllocTemplate( const U32 size )
#endif
    //#if defined( PLATFORM_XENON )
    // Pad m_Size to 128 bytes
    : m_Size( ( size + 127 ) & ~127 )
    // Allocate m_Size + 128 bytes for alignment to 128 bytes
    // Note: This relies on Xenon stack allocations being already aligned to 16 bytes!
    //, m_pData( (char*)(U64)( ( (U32)(U64)_alloca( m_Size + 128 ) + 127 ) & ~127 ) )
    //#else
    //m_Size( size ),
    //m_pData( (char*)_alloca( size ) )
    //#endif
{
    //m_pData = (char*)(U64)( ( (U32)(U64)_alloca( m_Size + 128 ) + 127 ) & ~127 );
#ifdef COMPILER_GNU
	m_pData = (char*)( ( (U64)__builtin_alloca(m_Size + 128) + 127 ) & ~127 );
#else
	m_pData = (char*)( ( (U64)_alloca( m_Size + 128 ) + 127 ) & ~127 );
#endif
}

template < int BUFFERSIZE >
__forceinline C_StackAllocTemplate<BUFFERSIZE>::~C_StackAllocTemplate()
{
}

template < int BUFFERSIZE >
__forceinline char* C_StackAllocTemplate<BUFFERSIZE>::Data() const
{
    return m_pData;
}

#endif

// Allocates 128-byte padded size plus additional 128 bytes for 128-byte alignment
#if defined( PLATFORM_PPU_SPU )
#include <alloca.h>
#define _alloca128( size ) alloca( ( ( (size) + 0x7F ) & ~0x7F ) + 0x80 )
#else
#define _alloca128( size ) _alloca( ( ( (size) + 0x7F ) & ~0x7F ) + 0x80 )
#endif

NOALIAS __forceinline void* Align128( void* ptr ) __attribute__(( const, always_inline ));
NOALIAS __forceinline void* Align128( void* ptr )
{
    return (void*)( ( PointerToInt( ptr ) + 0x7F ) & ~0x7F);
}




//
// V128 Structure
//

//#if !defined( _SIMD_ )
//FORCE_INLINE
//V128::V128( const F32 x, const F32 y, const F32 z, const F32 w ) :
//    m_X( x ),
//    m_Y( y ),
//    m_Z( z ),
//    m_W( w )
//{
//}
//#endif




//
// Integer, Scalar & Vector Selector Classes
//

NOALIAS __forceinline C_SelectorBool::C_SelectorBool( const S32 initialState ) :
    //m_Mask( initialState ? 0xFFFFFFFF : 0x00000000 )
    m_Mask( ( initialState | -initialState ) >> 31 )
{
}

NOALIAS __forceinline C_SelectorBool::C_SelectorBool( const U32 initialState ) :
    //m_Mask( initialState ? 0xFFFFFFFF : 0x00000000 )
    m_Mask( ( (S32)initialState | -(S32)initialState ) >> 31 )
{
}

NOALIAS __forceinline C_SelectorBool::C_SelectorBool( const bool initialState ) :
    //m_Mask( initialState ? 0xFFFFFFFF : 0x00000000 )
    m_Mask( ( (S32)initialState << 31 ) >> 31 )
{
}

NOALIAS __forceinline const U32 Select( const C_SelectorBool& test, const U32 result_if_true, const U32 result_if_false )
{
    return ( result_if_true & test.m_Mask ) | ( result_if_false & ~test.m_Mask );
}

template < typename T >
NOALIAS __forceinline T* Select( const C_SelectorBool& test, T* result_if_true, T* result_if_false )
{
    // [Blur2PC][x64] m_Mask is 0 or 0xFFFFFFFF; sign-extend it so it covers the whole pointer.
    const INTPTR mask = (INTPTR)(S32)test.m_Mask;
    return (T*)( ( ((INTPTR)result_if_true) & mask ) | ( ((INTPTR)result_if_false) & ~mask ) );
}

template < typename T >
NOALIAS __forceinline T* Select( const C_SelectorBool& test, T const * result_if_true, T const * result_if_false )
{
    // [Blur2PC][x64] m_Mask is 0 or 0xFFFFFFFF; sign-extend it so it covers the whole pointer.
    const INTPTR mask = (INTPTR)(S32)test.m_Mask;
    return (T*)( ( ((INTPTR)result_if_true) & mask ) | ( ((INTPTR)result_if_false) & ~mask ) );
}


//template < typename T >
//NOALIAS __forceinline const Scalar_Bool SelectorEQ( T const * const & pA, T const * const & pB )
//{
//    #if defined( _VMX128_ )
//    return Scalar_Bool( __vcmpequw( __vspltw( __lvlx( &pA, 0x0 ), 0 ), __vspltw( __lvlx( &pB, 0x0 ), 0 ) ) );
//    #else
//    return Scalar_Bool( pA == pB );
//    #endif
//}

template < typename T >
NOALIAS __forceinline const Scalar_Bool SelectorEQ( const Vector_Ptr<T>& pA, const Vector_Ptr<T>& pB )
{
    #if defined( _VMX128_ )
    return Scalar_Bool( __vcmpequw( pA.m_Ptr, pB.m_Ptr ) );
    #else
    return Scalar_Bool( pA.m_Ptr == pB.m_Ptr );
    #endif
}







/// Undefined Constructor
NOALIAS __forceinline Scalar_Bool::Scalar_Bool( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline Scalar_Bool::Scalar_Bool( const Scalar_Bool& copy )
    #if defined( _SIMD_ )
    : m_V( copy.m_V )
    #else
    : m_F( copy.m_F )
    #endif
{
}

#if defined( _SPU_ )
/// Constructor: Initial state set from vector unsigned int
NOALIAS __forceinline Scalar_Bool::Scalar_Bool( const vector unsigned int src ) __restrict
    : m_V( src )
{
}
#elif defined( _PPU_ )
/// Constructor: Initial state set from vector bool int
NOALIAS __forceinline Scalar_Bool::Scalar_Bool( const vector bool int src ) __restrict
    : m_V( src )
{
}
#elif defined( _SIMD_ )
/// Constructor: Initial state set from V128
NOALIAS __forceinline Scalar_Bool::Scalar_Bool( const V128 PCREF src )
    : m_V( src )
{
}
#endif

/// Constructor: Initial state set from bool
NOALIAS __forceinline Scalar_Bool::Scalar_Bool( const bool initialState )
    #if defined( _SPU_ )
    : m_V( spu_splats( initialState ? (unsigned int)-1 : (unsigned int)0 ) )
    #elif defined( _PPU_ )
    //: m_V( __builtin_constant_p( initialState ) ? ( (vector bool int)( initialState ? vec_vspltisw( -1 ) : vec_vspltisw( 0 ) ) )
    : m_V( __builtin_constant_p( initialState ) ? ( (vector bool int)( initialState ? vec_splat_s32( -1 ) : vec_splat_s32( 0 ) ) )
                                              //: ( (vector bool int)vec_cmpgt( (vector unsigned int)vec_vspltb( vec_load_left_aligned( (unsigned char*)&initialState ), 0 ), (vector unsigned int)vec_vspltisw( 0 ) ) ) )
                                              : ( (vector bool int)vec_cmpgt( (vector unsigned int)vec_vspltb( vec_load_left_aligned( (unsigned char*)&initialState ), 0 ), vec_splat_u32( 0 ) ) ) )
    #elif defined( _VMX128_ )
    : m_V( __IsIntConst( initialState ) ? ( initialState ? __vspltisw( -1 ) : __vspltisw( 0 ) )
                                        : ( __vcmpgtub( __vspltb( __lvlx( &initialState, 0x0 ), 0 ), __vzero() ) ) )
    #elif defined( _SSE_ )
    : m_V( initialState ? SIMD_Mask<1,1,1,1>() : SIMD_Mask<0,0,0,0>() )
    #else
    : m_F( initialState ? 0xFFFFFFFF : 0x00000000 )
    #endif
{
}

/// Constructor: Initial state set from S32
NOALIAS __forceinline Scalar_Bool::Scalar_Bool( const S32 initialState )
    #if defined( _SPU_ )
    : m_V( spu_splats( initialState ? (unsigned int)-1 : (unsigned int)0 ) )
    #elif defined( _PPU_ )
    //: m_V( __builtin_constant_p( initialState ) ? ( (vector bool int)( initialState ? vec_vspltisw( -1 ) : vec_vspltisw( 0 ) ) )
    : m_V( __builtin_constant_p( initialState ) ? ( (vector bool int)( initialState ? vec_splat_s32( -1 ) : vec_splat_s32( 0 ) ) )
                                              //: ( (vector bool int)vec_cmpgt( vec_splat( (vector unsigned int)vec_load_left_aligned( &initialState ), VX ), (vector unsigned int)vec_vspltisw( 0 ) ) ) )
                                              : ( (vector bool int)vec_cmpgt( vec_splat( (vector unsigned int)vec_load_left_aligned( &initialState ), VX ), vec_splat_u32( 0 ) ) ) )
    #elif defined( _VMX128_ )
    : m_V( __IsIntConst( initialState ) ? ( initialState ? __vspltisw( -1 ) : __vspltisw( 0 ) )
                                        : ( __vcmpgtuw( __vspltw( __lvlx( &initialState, 0x0 ), VX ), __vzero() ) ) )
    #elif defined( _SSE_ )
    : m_V( initialState ? SIMD_Mask<1,1,1,1>() : SIMD_Mask<0,0,0,0>() )
    #else
    : m_F( initialState ? 0xFFFFFFFF : 0x00000000 )
    #endif
{
}

/// Constructor: Initial state set from U32
NOALIAS __forceinline Scalar_Bool::Scalar_Bool( const U32 initialState )
    #if defined( _SPU_ )
    : m_V( spu_splats( initialState ? (unsigned int)-1 : (unsigned int)0 ) )
    #elif defined( _PPU_ )
    //: m_V( __builtin_constant_p( initialState ) ? ( (vector bool int)( initialState ? vec_vspltisw( -1 ) : vec_vspltisw( 0 ) ) )
    : m_V( __builtin_constant_p( initialState ) ? ( (vector bool int)( initialState ? vec_splat_s32( -1 ) : vec_splat_s32( 0 ) ) )
                                                //: ( (vector bool int)vec_cmpgt( vec_splat( vec_load_left_aligned( &initialState ), 0 ), (vector unsigned int)vec_vspltisw( 0 ) ) ) )
                                                : ( (vector bool int)vec_cmpgt( vec_splat( vec_load_left_aligned( &initialState ), 0 ), vec_splat_u32( 0 ) ) ) )
    #elif defined( _VMX128_ )
    : m_V( __IsIntConst( initialState ) ? ( initialState ? __vspltisw( -1 ) : __vspltisw( 0 ) )
                                        : ( __vcmpgtuw( __vspltw( __lvlx( &initialState, 0x0 ), 0 ), __vzero() ) ) )
    #elif defined( _SSE_ )
    : m_V( initialState ? SIMD_Mask<1,1,1,1>() : SIMD_Mask<0,0,0,0>() )
    #else
    : m_F( initialState ? 0xFFFFFFFF : 0x00000000 )
    #endif
{
}

/// Copy Assignment
NOALIAS __forceinline const Scalar_Bool& Scalar_Bool::operator = ( const Scalar_Bool& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_F = copy.m_F;
    #endif
    return copy;
}

/// Scalar Selector Test : if ( selector == 0xFFFFFFFF ) returns 1 else returns 0
NOALIAS __forceinline Scalar_Bool::operator const bool () const __restrict
{
    #if defined( _SPU_ )
    const unsigned int result = spu_extract( m_V, 0 );
    return (bool)( result & 1 );
    #elif defined( _PPU_ )
    bool result;
    //vec_stvebx( vec_and( (vector char)m_V, vec_vspltisb( 1 ) ), 0x0, (signed char*)&result );
    vec_stvebx( vec_and( (vector char)m_V, vec_splat_s8( 1 ) ), 0x0, (signed char*)&result );
    return result;
    #elif defined( _VMX128_ )
    U32 CR;
    __vcmpequwR( m_V, __vzero(), &CR );
    const char result( (char)( ( CR & VMX_CR6_ALL_FALSE ) >> VMX_CR6_ALL_FALSE_BIT ) );
    return (bool&)result;
    //return IntToBool( CR & VMX_CR6_ALL_FALSE );
    #elif defined( _SSE_ )
    const char result( SIMD_TestScalarSelector( m_V ) );
    return (bool&)result;
    #else
    const char result( (char)( m_F & 1 ) );
    return (bool&)result;
    #endif
}

NOALIAS __forceinline const Scalar_Bool operator ~ ( const Scalar_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_nor( src.m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_vnor( src.m_V, src.m_V );
    return result;
    #elif defined( _VMX128_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vnor( src.m_V, src.m_V );
    return result;
    #elif defined( _SSE_ )
    return Scalar_Bool( SIMD_NOT( src.m_V ) );
    #else
    return Scalar_Bool( ~src.m_F );
    #endif
}

NOALIAS __forceinline const Scalar_Bool operator & ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_and( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    return Scalar_Bool( vec_and( src0.m_V, src1.m_V ) );
    #elif defined( _VMX128_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vand( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SSE_ )
    return Scalar_Bool( SIMD_AND( src0.m_V, src1.m_V ) );
    #else
    return Scalar_Bool( src0.m_F & src1.m_F );
    #endif
}

NOALIAS __forceinline const Scalar_Bool operator | ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_or( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    return Scalar_Bool( vec_vor( src0.m_V, src1.m_V ) );
    #elif defined( _VMX128_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SSE_ )
    return Scalar_Bool( SIMD_OR( src0.m_V, src1.m_V ) );
    #else
    return Scalar_Bool( src0.m_F | src1.m_F );
    #endif
}

NOALIAS __forceinline const Scalar_Bool operator ^ ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_xor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    return Scalar_Bool( vec_vxor( src0.m_V, src1.m_V ) );
    #elif defined( _VMX128_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vxor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SSE_ )
    return Scalar_Bool( SIMD_XOR( src0.m_V, src1.m_V ) );
    #else
    return Scalar_Bool( src0.m_F ^ src1.m_F );
    #endif
}

NOALIAS __forceinline const Scalar_Bool operator ! ( const Scalar_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_nor( src.m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    return Scalar_Bool( vec_vnor( src.m_V, src.m_V ) );
    #elif defined( _VMX128_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vnor( src.m_V, src.m_V );
    return result;
    #elif defined( _SSE_ )
    return Scalar_Bool( SIMD_NOT( src.m_V ) );
    #else
    return Scalar_Bool( ~src.m_F );
    #endif
}

//NOALIAS __forceinline const Scalar_Bool operator && ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 )
//{
//    #if defined( _SPU_ )
//    Scalar_Bool result( UNDEFINED );
//    result.m_V = spu_and( src0.m_V, src1.m_V );
//    return result;
//    #elif defined( _PPU_ )
//    return Scalar_Bool( vec_and( src0.m_V, src1.m_V ) );
//    #elif defined( _VMX128_ )
//    Scalar_Bool result( UNDEFINED );
//    result.m_V = __vand( src0.m_V, src1.m_V );
//    return result;
//    #elif defined( _SSE_ )
//    return Scalar_Bool( SIMD_AND( src0.m_V, src1.m_V ) );
//    #else
//    return Scalar_Bool( src0.m_F & src1.m_F );
//    #endif
//}
//
//NOALIAS __forceinline const Scalar_Bool operator || ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 )
//{
//    #if defined( _SPU_ )
//    Scalar_Bool result( UNDEFINED );
//    result.m_V = spu_or( src0.m_V, src1.m_V );
//    return result;
//    #elif defined( _PPU_ )
//    return Scalar_Bool( vec_vor( src0.m_V, src1.m_V ) );
//    #elif defined( _VMX128_ )
//    Scalar_Bool result( UNDEFINED );
//    result.m_V = __vor( src0.m_V, src1.m_V );
//    return result;
//    #elif defined( _SSE_ )
//    return Scalar_Bool( SIMD_OR( src0.m_V, src1.m_V ) );
//    #else
//    return Scalar_Bool( src0.m_F | src1.m_F );
//    #endif
//}

NOALIAS __forceinline void operator &= ( Scalar_Bool& src0, const Scalar_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    src0.m_V = spu_and( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    src0.m_V = vec_and( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    src0.m_V = __vand( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    src0.m_V = SIMD_AND( src0.m_V, src1.m_V );
    #else
    src0.m_F &= src1.m_F;
    #endif
}

NOALIAS __forceinline void operator |= ( Scalar_Bool& src0, const Scalar_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    src0.m_V = spu_or( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    src0.m_V = vec_vor( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    src0.m_V = __vor( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    src0.m_V = SIMD_OR( src0.m_V, src1.m_V );
    #else
    src0.m_F |= src1.m_F;
    #endif
}

//NOALIAS __forceinline const bool operator && ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 )
//{
//    return ((bool)src0) && ((bool)src1);
//}
//
//NOALIAS __forceinline const bool operator && ( const Scalar_Bool PCREF src0, const bool src1 )
//{
//    return ((bool)src0) && src1;
//}
//
//NOALIAS __forceinline const bool operator && ( const bool src0, const Scalar_Bool PCREF src1 )
//{
//    return src0 && ((bool)src1);
//}
//                                                                                        
//NOALIAS __forceinline const bool operator || ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 )
//{
//    return ((bool)src0) || ((bool)src1);
//}
//
//NOALIAS __forceinline const bool operator || ( const Scalar_Bool PCREF src0, const bool src1 )
//{
//    return ((bool)src0) || src1;
//}
//
//NOALIAS __forceinline const bool operator || ( const bool src0, const Scalar_Bool PCREF src1 )
//{
//    return src0 || ((bool)src1);
//}




/// Undefined Constructor
NOALIAS __forceinline Vector2_Bool::Vector2_Bool( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline Vector2_Bool::Vector2_Bool( const Vector2_Bool& src ) :
    #if defined( _SIMD_ )
    m_V( src.m_V )
    #else
    m_X( src.m_X ),
    m_Y( src.m_Y )
    #endif
{
}

/// Constructor from Scalar_Bool
NOALIAS __forceinline Vector2_Bool::Vector2_Bool( const Scalar_Bool PCREF src ) :
    #if defined( _SIMD_ )
    m_V( src.m_V )
    #else
    m_X( src.m_F ),
    m_Y( src.m_F )
    #endif
{
}

#if defined( _SPU_ )
NOALIAS __forceinline Vector2_Bool::Vector2_Bool( const vector unsigned int src ) __restrict :
    m_V( src )
{
}
#elif defined( _PPU_ )
NOALIAS __forceinline Vector2_Bool::Vector2_Bool( const vector bool int src ) __restrict :
    m_V( src )
{
}
#elif defined( _SIMD_ )
NOALIAS __forceinline Vector2_Bool::Vector2_Bool( const V128 PCREF src ) :
    m_V( src )
{
}
#else
NOALIAS __forceinline Vector2_Bool::Vector2_Bool( const U32 x, const U32 y ) __restrict :
    m_X( x ),
    m_Y( y )
{
}
#endif

NOALIAS __forceinline const Vector2_Bool operator ~ ( const Vector2_Bool PCREF src )
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_nor( src.m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = vec_vnor( src.m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = SIMD_NOT( src.m_V );
    return result;
    #else
    return Vector2_Bool( ~src.m_X, ~src.m_Y );
    #endif
}

NOALIAS __forceinline const Vector2_Bool operator & ( const Vector2_Bool PCREF src0, const Vector2_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_and( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = vec_and( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SIMD_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = SIMD_AND( src0.m_V, src1.m_V );
    return result;
    #else
    return Vector2_Bool( src0.m_X & src1.m_X,
                         src0.m_Y & src1.m_Y );
    #endif
}

NOALIAS __forceinline const Vector2_Bool operator | ( const Vector2_Bool PCREF src0, const Vector2_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_or( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = vec_vor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector2_Bool( SIMD_OR( src0.m_V, src1.m_V ) );
    #else
    return Vector2_Bool( src0.m_X | src1.m_X,
                         src0.m_Y | src1.m_Y );
    #endif
}

NOALIAS __forceinline const Vector2_Bool operator ^ ( const Vector2_Bool PCREF src0, const Vector2_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = spu_xor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector2_Bool result( UNDEFINED );
    result.m_V = vec_vxor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector2_Bool( SIMD_XOR( src0.m_V, src1.m_V ) );
    #else
    return Vector2_Bool( src0.m_X ^ src1.m_X,
                         src0.m_Y ^ src1.m_Y );
    #endif
}

NOALIAS __forceinline const Scalar_Bool X( const Vector2_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_splat<VX>( src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_splat( src.m_V, VX );
    return result;
    #elif defined( _SIMD_ )
    return Scalar_Bool( SIMD_Scalar_Vector2X( src.m_V ) );
    #else
    return Scalar_Bool( src.m_X );
    #endif
}

NOALIAS __forceinline const Scalar_Bool Y( const Vector2_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_splat<VY>( src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_splat( src.m_V, VY );
    return result;
    #elif defined( _SIMD_ )
    return Scalar_Bool( SIMD_Scalar_Vector2Y( src.m_V ) );
    #else
    return Scalar_Bool( src.m_Y );
    #endif
}








/// Undefined Constructor
NOALIAS __forceinline Vector3_Bool::Vector3_Bool( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline Vector3_Bool::Vector3_Bool( const Vector3_Bool& src ) :
    #if defined( _SIMD_ )
    m_V( src.m_V )
    #else
    m_X( src.m_X ),
    m_Y( src.m_Y ),
    m_Z( src.m_Z )
    #endif
{
}

/// Constructor from Scalar_Bool
NOALIAS __forceinline Vector3_Bool::Vector3_Bool( const Scalar_Bool PCREF src ) :
    #if defined( _SIMD_ )
    m_V( src.m_V )
    #else
    m_X( src.m_F ),
    m_Y( src.m_F ),
    m_Z( src.m_F )
    #endif
{
}


/// Constructor from Vec4Bool
NOALIAS __forceinline Vector3_Bool::Vector3_Bool( const Vec4Bool& src )
    #if defined( _SIMD_ )
    : m_V( src.m_V )
    #else
    : m_X( src.m_V[0] )
    , m_Y( src.m_V[1] )
    , m_Z( src.m_V[2] )
    #endif
{
}

/// Assignment operator from Vec4Bool
NOALIAS __forceinline const Vec4Bool& Vector3_Bool::operator = ( const Vec4Bool& src )
{
    #if defined( _SIMD_ )
    m_V = src.m_V;
    #else
    m_X = src.m_V[0];
    m_Y = src.m_V[1];
    m_Z = src.m_V[2];
    #endif
    return src;
}

/// Cast to Vec4Bool
NOALIAS __forceinline Vector3_Bool::operator const Vec4Bool () const
{
    Vec4Bool result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _VMX128_ ) || defined( _SSE_ )
    result.m_V = m_V;
    #else
    result.m_V[0] = IntToBool( m_X );
    result.m_V[1] = IntToBool( m_Y );
    result.m_V[2] = IntToBool( m_Z );
    result.m_V[3] = false;
    #endif
    return result;
}


#if defined( _SPU_ )
NOALIAS __forceinline Vector3_Bool::Vector3_Bool( const vector unsigned int src ) __restrict :
    m_V( src )
{
}
#elif defined( _PPU_ )
NOALIAS __forceinline Vector3_Bool::Vector3_Bool( const vector bool int src ) __restrict :
    m_V( src )
{
}
#elif defined( _SIMD_ )
NOALIAS __forceinline Vector3_Bool::Vector3_Bool( const V128 PCREF src ) :
    m_V( src )
{
}
#else
NOALIAS __forceinline Vector3_Bool::Vector3_Bool( const U32 x, const U32 y, const U32 z ) __restrict :
    m_X( x ),
    m_Y( y ),
    m_Z( z )
{
}
#endif

NOALIAS __forceinline const Vector3_Bool operator ~ ( const Vector3_Bool PCREF src )
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_nor( src.m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = vec_vnor( src.m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_NOT( src.m_V ) );
    #else
    return Vector3_Bool( ~src.m_X, ~src.m_Y, ~src.m_Z );
    #endif
}

NOALIAS __forceinline const Vector3_Bool operator & ( const Vector3_Bool PCREF src0, const Vector3_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_and( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = vec_and( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_AND( src0.m_V, src1.m_V ) );
    #else
    return Vector3_Bool( src0.m_X & src1.m_X,
                         src0.m_Y & src1.m_Y,
                         src0.m_Z & src1.m_Z );
    #endif
}

NOALIAS __forceinline const Vector3_Bool operator | ( const Vector3_Bool PCREF src0, const Vector3_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_or( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = vec_vor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_OR( src0.m_V, src1.m_V ) );
    #else
    return Vector3_Bool( src0.m_X | src1.m_X,
                         src0.m_Y | src1.m_Y,
                         src0.m_Z | src1.m_Z );
    #endif
}

NOALIAS __forceinline const Vector3_Bool operator ^ ( const Vector3_Bool PCREF src0, const Vector3_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = spu_xor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    result.m_V = vec_vxor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_XOR( src0.m_V, src1.m_V ) );
    #else
    return Vector3_Bool( src0.m_X ^ src1.m_X,
                         src0.m_Y ^ src1.m_Y,
                         src0.m_Z ^ src1.m_Z );
    #endif
}

NOALIAS __forceinline const Scalar_Bool X( const Vector3_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_splat<VX>( src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_splat( src.m_V, VX );
    return result;
    #elif defined( _SIMD_ )
    return Scalar_Bool( SIMD_Scalar_Vector3X( src.m_V ) );
    #else
    return Scalar_Bool( src.m_X );
    #endif
}

NOALIAS __forceinline const Scalar_Bool Y( const Vector3_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_splat<VY>( src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_splat( src.m_V, VY );
    return result;
    #elif defined( _SIMD_ )
    return Scalar_Bool( SIMD_Scalar_Vector3Y( src.m_V ) );
    #else
    return Scalar_Bool( src.m_Y );
    #endif
}

NOALIAS __forceinline const Scalar_Bool Z( const Vector3_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_splat<VZ>( src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_splat( src.m_V, VZ );
    return result;
    #elif defined( _SIMD_ )
    return Scalar_Bool( SIMD_Scalar_Vector3Z( src.m_V ) );
    #else
    return Scalar_Bool( src.m_Z );
    #endif
}

template < bool x, bool y, bool z >
NOALIAS __forceinline const Vector3_Bool Create_Vector3_Bool()
{
    #if defined( _SPU_ )
    Vector3_Bool result( UNDEFINED );
    //result.m_V = (vector unsigned int) { (unsigned int)x, (unsigned int)y, (unsigned int)z, (unsigned int)false };
	result.m_V = (vector unsigned int){ (unsigned int)x, (unsigned int)y, (unsigned int)z, (unsigned int)false };
    return result;
    #elif defined( _PPU_ )
    Vector3_Bool result( UNDEFINED );
    //result.m_V = (vector bool int) { (unsigned int)x, (unsigned int)y, (unsigned int)z, (unsigned int)false };
		//#if defined( vec_insert )
		//result.m_V = vec_insert( (int)x, vec_insert( (int)y, vec_insert( (int)z, (vector int)vec_promote( (int)false, 3 ), 2 ), 1 ), 0 );
		//#else
		//result.m_V = (vector bool int)( x, y, z, false );
		result.m_V = (vector bool int){ (int)x, (int)y, (int)z, (int)false };
		//#endif
    return result;
    #elif defined( _SIMD_ )
    return Vector3_Bool( SIMD_Mask<x,y,z,0>() );
    #else
    return Vector3_Bool( x ? 0xFFFFFFFF : 0x00000000,
                         y ? 0xFFFFFFFF : 0x00000000,
                         z ? 0xFFFFFFFF : 0x00000000 );
    #endif
}








/// Undefined Constructor
NOALIAS __forceinline Vector4_Bool::Vector4_Bool( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline Vector4_Bool::Vector4_Bool( const Vector4_Bool& src ) :
    #if defined( _SIMD_ )
    m_V( src.m_V )
    #else
    m_X( src.m_X ),
    m_Y( src.m_Y ),
    m_Z( src.m_Z ),
    m_W( src.m_W )
    #endif
{
}

/// Constructor from Scalar_Bool
NOALIAS __forceinline Vector4_Bool::Vector4_Bool( const Scalar_Bool PCREF src ) :
    #if defined( _SIMD_ )
    m_V( src.m_V )
    #else
    m_X( src.m_F ),
    m_Y( src.m_F ),
    m_Z( src.m_F ),
    m_W( src.m_F )
    #endif
{
}


/// Constructor from Vec4Bool
NOALIAS __forceinline Vector4_Bool::Vector4_Bool( const Vec4Bool& src )
    #if defined( _SIMD_ )
    : m_V( src.m_V )
    #else
    : m_X( src.m_V[0] )
    , m_Y( src.m_V[1] )
    , m_Z( src.m_V[2] )
    , m_W( src.m_V[3] )
    #endif
{
}

/// Assignment operator from Vec4Bool
NOALIAS __forceinline const Vec4Bool& Vector4_Bool::operator = ( const Vec4Bool& src )
{
    #if defined( _SIMD_ )
    m_V = src.m_V;
    #else
    m_X = src.m_V[0];
    m_Y = src.m_V[1];
    m_Z = src.m_V[2];
    m_W = src.m_V[3];
    #endif
    return src;
}

/// Cast to Vec4Bool
NOALIAS __forceinline Vector4_Bool::operator const Vec4Bool () const
{
    Vec4Bool result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _VMX128_ ) || defined( _SSE_ )
    result.m_V = m_V;
    #else
    result.m_V[0] = IntToBool( m_X );
    result.m_V[1] = IntToBool( m_Y );
    result.m_V[2] = IntToBool( m_Z );
    result.m_V[3] = IntToBool( m_W );
    #endif
    return result;
}


#if defined( _SPU_ )
NOALIAS __forceinline Vector4_Bool::Vector4_Bool( const vector unsigned int src ) __restrict :
    m_V( src )
{
}
#elif defined( _PPU_ )
NOALIAS __forceinline Vector4_Bool::Vector4_Bool( const vector bool int src ) __restrict :
    m_V( src )
{
}
#elif defined( _SIMD_ )
NOALIAS __forceinline Vector4_Bool::Vector4_Bool( const V128 PCREF src ) :
    m_V( src )
{
}
#else
NOALIAS __forceinline Vector4_Bool::Vector4_Bool( const U32 x, const U32 y, const U32 z, const U32 w ) __restrict :
    m_X( x ),
    m_Y( y ),
    m_Z( z ),
    m_W( w )
{
}
#endif

NOALIAS __forceinline const Vector4_Bool operator ~ ( const Vector4_Bool PCREF src )
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_nor( src.m_V, src.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_vnor( src.m_V, src.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_NOT( src.m_V ) );
    #else
    return Vector4_Bool( ~src.m_X, ~src.m_Y, ~src.m_Z, ~src.m_W );
    #endif
}

NOALIAS __forceinline const Vector4_Bool operator & ( const Vector4_Bool PCREF src0, const Vector4_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_and( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_and( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_AND( src0.m_V, src1.m_V ) );
    #else
    return Vector4_Bool( src0.m_X & src1.m_X,
                         src0.m_Y & src1.m_Y,
                         src0.m_Z & src1.m_Z,
                         src0.m_W & src1.m_W );
    #endif
}

NOALIAS __forceinline const Vector4_Bool operator | ( const Vector4_Bool PCREF src0, const Vector4_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_or( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_vor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_OR( src0.m_V, src1.m_V ) );
    #else
    return Vector4_Bool( src0.m_X | src1.m_X,
                         src0.m_Y | src1.m_Y,
                         src0.m_Z | src1.m_Z,
                         src0.m_W | src1.m_W );
    #endif
}

NOALIAS __forceinline const Vector4_Bool operator ^ ( const Vector4_Bool PCREF src0, const Vector4_Bool PCREF src1 )
{
    #if defined( _SPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = spu_xor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _PPU_ )
    Vector4_Bool result( UNDEFINED );
    result.m_V = vec_vxor( src0.m_V, src1.m_V );
    return result;
    #elif defined( _SIMD_ )
    return Vector4_Bool( SIMD_XOR( src0.m_V, src1.m_V ) );
    #else
    return Vector4_Bool( src0.m_X ^ src1.m_X,
                         src0.m_Y ^ src1.m_Y,
                         src0.m_Z ^ src1.m_Z,
                         src0.m_W ^ src1.m_W );
    #endif
}

NOALIAS __forceinline const Scalar_Bool X( const Vector4_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_splat<VX>( src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_splat( src.m_V, VX );
    return result;
    #elif defined( _SIMD_ )
    return Scalar_Bool( SIMD_Scalar_Vector4X( src.m_V ) );
    #else
    return Scalar_Bool( src.m_X );
    #endif
}

NOALIAS __forceinline const Scalar_Bool Y( const Vector4_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_splat<VY>( src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_splat( src.m_V, VY );
    return result;
    #elif defined( _SIMD_ )
    return Scalar_Bool( SIMD_Scalar_Vector4Y( src.m_V ) );
    #else
    return Scalar_Bool( src.m_Y );
    #endif
}

NOALIAS __forceinline const Scalar_Bool Z( const Vector4_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_splat<VZ>( src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_splat( src.m_V, VZ );
    return result;
    #elif defined( _SIMD_ )
    return Scalar_Bool( SIMD_Scalar_Vector4Z( src.m_V ) );
    #else
    return Scalar_Bool( src.m_Z );
    #endif
}

NOALIAS __forceinline const Scalar_Bool W( const Vector4_Bool PCREF src )
{
    #if defined( _SPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_splat<VW>( src.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_splat( src.m_V, VW );
    return result;
    #elif defined( _SIMD_ )
    return Scalar_Bool( SIMD_Scalar_Vector4W( src.m_V ) );
    #else
    return Scalar_Bool( src.m_W );
    #endif
}

NOALIAS __forceinline const S32 AllTrue( const Vector4_Bool PCREF src )
{
    #if defined( _PPU_ )
    //return vec_all_ne( src.m_V, (vector bool int)vec_vspltisw( 0 ) );
    return vec_all_ne( src.m_V, (vector bool int)vec_splat_s32( 0 ) );
    #elif defined( _VMX128_ )
    U32 CR;
    __vcmpequwR( src.m_V, __vzero(), &CR );
    return 1 & ( CR >> VMX_CR6_ALL_FALSE_BIT );
    #else
    return X( src ) & Y( src ) & Z( src ) & W( src );
    #endif
}

NOALIAS __forceinline const S32 AllFalse( const Vector4_Bool PCREF src )
{
    #if defined( _PPU_ )
    //return vec_all_eq( src.m_V, (vector bool int)vec_vspltisw( 0 ) );
    return vec_all_eq( src.m_V, (vector bool int)vec_splat_s32( 0 ) );
    #elif defined( _VMX128_ )
    U32 CR;
    __vcmpequwR( src.m_V, __vzero(), &CR );
    return 1 & ( CR >> VMX_CR6_ALL_TRUE_BIT );
    #else
    return !X( src ) & !Y( src ) & !Z( src ) & !W( src );
    #endif
}

NOALIAS __forceinline const S32 AnyTrue( const Vector4_Bool PCREF src )
{
    #if defined( _PPU_ )
    //return vec_any_ne( src.m_V, (vector bool int)vec_vspltisw( 0 ) );
    return vec_any_ne( src.m_V, (vector bool int)vec_splat_s32( 0 ) );
    #elif defined( _VMX128_ )
    U32 CR;
    __vcmpequwR( src.m_V, __vzero(), &CR );
    return 1 & ~( CR >> VMX_CR6_ALL_TRUE_BIT );
    #else
    return X( src ) | Y( src ) | Z( src ) | W( src );
    #endif
}

NOALIAS __forceinline const S32 AnyFalse( const Vector4_Bool PCREF src )
{
    #if defined( _PPU_ )
    //return vec_any_eq( src.m_V, (vector bool int)vec_vspltisw( 0 ) );
    return vec_any_eq( src.m_V, (vector bool int)vec_splat_s32( 0 ) );
    #elif defined( _VMX128_ )
    U32 CR;
    __vcmpequwR( src.m_V, __vzero(), &CR );
    return 1 & ~( CR >> VMX_CR6_ALL_FALSE_BIT );
    #else
    return !X( src ) | !Y( src ) | !Z( src ) | !W( src );
    #endif
}


///////////////////////////////////////////////////////////////////
//
// Compound helpers based on LCD functionality.
//

// transpose 4 axes - based on altivec 'mergehi/mergelo' / SSE 'unpacklo'/'unpackhi' / SPU shuffle..
#ifdef _SIMD_
NOALIAS __forceinline void
SIMD_Transpose(
	V128&	dstAX,	
	V128&	dstAY,
	V128&	dstAZ,
	V128&	dstAW, 
	const V128 PCREF srcAX,
	const V128 PCREF srcAY,
	const V128 PCREF srcAZ,
	const V128 PCREF srcAW
		) 
{   
	const V128 tmp0( SIMD_MergeHigh( srcAX,	srcAZ ) );
	const V128 tmp1( SIMD_MergeHigh( srcAY,	srcAW ) );
	const V128 tmp2( SIMD_MergeLow ( srcAX,	srcAZ ) );
	const V128 tmp3( SIMD_MergeLow ( srcAY,	srcAW ) );
	dstAX	= SIMD_MergeHigh( tmp0, tmp1 );
	dstAY	= SIMD_MergeLow ( tmp0, tmp1 );
	dstAZ	= SIMD_MergeHigh( tmp2, tmp3 );
	dstAW	= SIMD_MergeLow ( tmp2, tmp3 );
}
#endif

// TODO - others...


//
// --------------
// |-- V4_F32 --|
// --------------
//

/// Default Constructor
//FORCE_INLINE
//V4_F32::V4_F32() :
//    #if defined( _SIMD_ )
//    m_XYZW( SIMD_Zero() )
//    #else
//    m_X( 0.0f ),
//    m_Y( 0.0f ),
//    m_Z( 0.0f ),
//    m_W( 0.0f )
//    #endif
//{
//}

/// Copy Constructor
//FORCE_INLINE
//V4_F32::V4_F32( const V4_F32& copy ) :
//	#if defined( _SIMD_ )
//    m_XYZW( copy.m_XYZW )
//	#else
//	m_X( copy.m_X ),
//	m_Y( copy.m_Y ),
//	m_Z( copy.m_Z ),
//	m_W( copy.m_W )
//	#endif
//{
//}

/// F32 XYZW Constructor
//FORCE_INLINE
//V4_F32::V4_F32( const F32& x, const F32& y, const F32& z, const F32& w ) :
//    m_X( x ),
//    m_Y( y ),
//    m_Z( z ),
//    m_W( w )
//{
//}

#if defined( _SIMD_ )
/// V128 Constructor
//FORCE_INLINE
//V4_F32::V4_F32( const V128 PCREF src ) :
//    m_XYZW( src )
//{
//}
#endif


//
// --------------
// |-- V4_S32 --|
// --------------
//

/// Default Constructor
//FORCE_INLINE
//V4_S32::V4_S32() :
//    #if defined( _SIMD_ )
//    m_XYZW( SIMD_Zero() )
//    #else
//    m_X( 0 ),
//    m_Y( 0 ),
//    m_Z( 0 ),
//    m_W( 0 )
//    #endif
//{
//}

/// Copy Constructor
//FORCE_INLINE
//V4_S32::V4_S32( const V4_S32& copy ) :
//	#if defined( _SIMD_ )
//    m_XYZW( copy.m_XYZW )
//	#else
//	m_X( copy.m_X ),
//	m_Y( copy.m_Y ),
//	m_Z( copy.m_Z ),
//	m_W( copy.m_W )
//	#endif
//{
//}

/// S32 XYZW Constructor
//FORCE_INLINE
//V4_S32::V4_S32( const S32 x, const S32 y, const S32 z, const S32 w ) :
//	m_X( x ),
//	m_Y( y ),
//	m_Z( z ),
//	m_W( w )
//{
//}

#if defined( _SIMD_ )
/// V128 Constructor
//FORCE_INLINE
//V4_S32::V4_S32( const V128 PCREF src ) :
//    m_XYZW( src )
//{
//}
#endif











}; // namespace Maths


#endif // #if defined( INCLUDE_THIS )

